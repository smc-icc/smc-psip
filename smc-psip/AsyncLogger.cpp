#include "stdafx.h"
#include "AsyncLogger.h"
#include "zip.h"

#include <process.h>
#include <deque>
#include <vector>
#include <string>
#include <algorithm>

namespace SMC_PSIP
{
	const char g_LOGLEVELStr[][10] =
	{
		"DEBUG",
		"INFO",
		"ALARM!!",
		"ERROR**",
		"FATAL##"
	};

	namespace
	{
		const unsigned kBlockCap = 64 * 1024;
		const unsigned kMaxQueuedBlocks = 1024; // ~64MB backlog cap; drop rather than stall SIP threads
		const unsigned kFmtBufSize = 4096;

		struct LogBlock
		{
			char data[kBlockCap];
			unsigned used;
			LogBlock() : used(0) {}
		};

		void MakeDirRecursiveA(const char* dir)
		{
			if (!dir || !dir[0])
				return;

			char tmp[MAX_PATH];
			strncpy_s(tmp, dir, _TRUNCATE);
			size_t n = strlen(tmp);
			if (n > 0 && (tmp[n - 1] == '\\' || tmp[n - 1] == '/'))
				tmp[n - 1] = 0;

			for (char* p = tmp + 1; *p; ++p)
			{
				if (*p == '\\' || *p == '/')
				{
					char c = *p;
					*p = 0;
					CreateDirectoryA(tmp, NULL);
					*p = c;
				}
			}
			CreateDirectoryA(tmp, NULL);
		}

		void Utf8OrAnsiToWide(const char* src, wchar_t* dst, size_t dstcch)
		{
			if (!src || !dst || dstcch == 0)
				return;
			int n = MultiByteToWideChar(CP_ACP, 0, src, -1, dst, (int)dstcch);
			if (n <= 0)
				dst[0] = 0;
		}

		struct FileItem
		{
			std::wstring path;
			FILETIME mtime;
		};

		bool NewerFirst(const FileItem& a, const FileItem& b)
		{
			return CompareFileTime(&a.mtime, &b.mtime) > 0;
		}
	}

	struct CLog::Impl
	{
		LOGLEVEL level;
		unsigned maxSize;
		unsigned backupCount;
		char dir[MAX_PATH];
		char prefix[64];
		char currentFileA[MAX_PATH];
		wchar_t currentFileW[MAX_PATH];
		HANDLE file;
		unsigned currentSize;
		CLog* owner;
		LogFunc_onLogFileRolled onRolled;

		CRITICAL_SECTION cs;
		CRITICAL_SECTION csCompress;
		HANDLE evWrite;
		HANDLE evCompress;
		HANDLE evFlushDone;
		HANDLE thWrite;
		HANDLE thCompress;
		volatile bool running;
		volatile bool flushReq;
		volatile unsigned dropCount;
		volatile unsigned dropPending;

		LogBlock* active;
		std::deque<LogBlock*> ready;
		std::deque<LogBlock*> idle;
		std::deque<std::wstring> compressQ;

		Impl()
			: level(lvDEBUG)
			, maxSize(50 * 1024 * 1024)
			, backupCount(100)
			, file(INVALID_HANDLE_VALUE)
			, currentSize(0)
			, owner(NULL)
			, onRolled(NULL)
			, evWrite(NULL)
			, evCompress(NULL)
			, evFlushDone(NULL)
			, thWrite(NULL)
			, thCompress(NULL)
			, running(false)
			, flushReq(false)
			, dropCount(0)
			, dropPending(0)
			, active(NULL)
		{
			dir[0] = 0;
			prefix[0] = 0;
			currentFileA[0] = 0;
			currentFileW[0] = 0;
			InitializeCriticalSection(&cs);
			InitializeCriticalSection(&csCompress);
		}

		~Impl()
		{
			Stop();
			CloseFile();
			DeleteAllBlocks();
			DeleteCriticalSection(&cs);
			DeleteCriticalSection(&csCompress);
		}

		LogBlock* AllocBlock()
		{
			if (!idle.empty())
			{
				LogBlock* b = idle.front();
				idle.pop_front();
				b->used = 0;
				return b;
			}
			return new LogBlock();
		}

		void RecycleBlock(LogBlock* b)
		{
			if (!b)
				return;
			if (idle.size() < 64)
			{
				b->used = 0;
				idle.push_back(b);
			}
			else
			{
				delete b;
			}
		}

		void DeleteAllBlocks()
		{
			delete active;
			active = NULL;
			while (!ready.empty())
			{
				delete ready.front();
				ready.pop_front();
			}
			while (!idle.empty())
			{
				delete idle.front();
				idle.pop_front();
			}
		}

		void BuildPath(const char* path, const char* pref)
		{
			if (pref && pref[0])
				strncpy_s(prefix, pref, _TRUNCATE);
			else
				strncpy_s(prefix, "psip", _TRUNCATE);

			if (!path || !path[0])
				_snprintf_s(dir, _TRUNCATE, "log\\%s", prefix);
			else
				strncpy_s(dir, path, _TRUNCATE);

			size_t n = strlen(dir);
			if (n > 0 && dir[n - 1] != '\\' && dir[n - 1] != '/')
				strncat_s(dir, "\\", _TRUNCATE);

			MakeDirRecursiveA(dir);
		}

		void MakeNewFileName()
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			_snprintf_s(currentFileA, _TRUNCATE,
				"%s%s_%04u%02u%02u_%02u%02u%02u_%03u.log",
				dir, prefix,
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			Utf8OrAnsiToWide(currentFileA, currentFileW, MAX_PATH);
		}

		bool OpenNewFile()
		{
			CloseFile();
			MakeNewFileName();
			file = CreateFileW(currentFileW, GENERIC_WRITE, FILE_SHARE_READ, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			currentSize = 0;
			if (file == INVALID_HANDLE_VALUE)
				return false;
			if (onRolled && owner)
				onRolled(owner, currentFileA);
			return true;
		}

		void CloseFile()
		{
			if (file != INVALID_HANDLE_VALUE)
			{
				FlushFileBuffers(file);
				CloseHandle(file);
				file = INVALID_HANDLE_VALUE;
			}
		}

		void EnqueueRaw(const char* data, int len)
		{
			if (!data || len <= 0)
				return;

			EnterCriticalSection(&cs);
			while (len > 0)
			{
				if (ready.size() >= kMaxQueuedBlocks)
				{
					dropCount++;
					dropPending++;
					break;
				}
				if (!active)
					active = AllocBlock();

				unsigned room = kBlockCap - active->used;
				unsigned n = (unsigned)len < room ? (unsigned)len : room;
				memcpy(active->data + active->used, data, n);
				active->used += n;
				data += n;
				len -= (int)n;

				if (active->used >= kBlockCap)
				{
					ready.push_back(active);
					active = NULL;
					SetEvent(evWrite);
				}
			}
			LeaveCriticalSection(&cs);
		}

		void FlushActiveLocked()
		{
			if (active && active->used > 0)
			{
				ready.push_back(active);
				active = NULL;
			}
		}

		void WriteBlocks(std::deque<LogBlock*>& blocks)
		{
			if (file == INVALID_HANDLE_VALUE && !OpenNewFile())
			{
				EnterCriticalSection(&cs);
				for (size_t i = 0; i < blocks.size(); ++i)
					RecycleBlock(blocks[i]);
				LeaveCriticalSection(&cs);
				blocks.clear();
				return;
			}

			if (dropPending)
			{
				char warn[128];
				int n = _snprintf_s(warn, _TRUNCATE, "[logger] dropped %u messages due to backlog\n", dropPending);
				DWORD wr = 0;
				WriteFile(file, warn, (DWORD)n, &wr, NULL);
				currentSize += wr;
				dropPending = 0;
			}

			for (size_t i = 0; i < blocks.size(); ++i)
			{
				LogBlock* b = blocks[i];
				if (b->used > 0)
				{
					DWORD wr = 0;
					WriteFile(file, b->data, b->used, &wr, NULL);
					currentSize += wr;
				}

				EnterCriticalSection(&cs);
				RecycleBlock(b);
				LeaveCriticalSection(&cs);

				if (currentSize >= maxSize)
					Rotate();
			}
			blocks.clear();
		}

		void Rotate()
		{
			if (file == INVALID_HANDLE_VALUE)
				return;

			wchar_t oldFile[MAX_PATH];
			wcsncpy_s(oldFile, currentFileW, _TRUNCATE);
			CloseFile();

			EnterCriticalSection(&csCompress);
			compressQ.push_back(oldFile);
			LeaveCriticalSection(&csCompress);
			SetEvent(evCompress);

			OpenNewFile();
			PurgeOldFiles();
		}

		void CompressFile(const std::wstring& logPath)
		{
			if (logPath.empty() || GetFileAttributesW(logPath.c_str()) == INVALID_FILE_ATTRIBUTES)
				return;

			std::wstring zipPath = logPath;
			size_t pos = zipPath.rfind(L".log");
			if (pos == std::wstring::npos)
				zipPath += L".zip";
			else
				zipPath.replace(pos, 4, L".zip");

			std::wstring nameInZip = logPath;
			size_t slash = nameInZip.find_last_of(L"\\/");
			if (slash != std::wstring::npos)
				nameInZip = nameInZip.substr(slash + 1);

			HZIP hz = CreateZip(zipPath.c_str(), 0);
			if (!hz)
				return;

			ZRESULT zr = ZipAdd(hz, nameInZip.c_str(), logPath.c_str());
			CloseZip(hz);
			if (zr == ZR_OK)
				DeleteFileW(logPath.c_str());
			else
				DeleteFileW(zipPath.c_str());
		}

		void CollectArchiveFiles(std::vector<FileItem>& items)
		{
			char patternA[MAX_PATH];
			_snprintf_s(patternA, _TRUNCATE, "%s%s_*.*", dir, prefix);
			wchar_t patternW[MAX_PATH];
			Utf8OrAnsiToWide(patternA, patternW, MAX_PATH);

			WIN32_FIND_DATAW fd;
			HANDLE h = FindFirstFileW(patternW, &fd);
			if (h == INVALID_HANDLE_VALUE)
				return;

			do
			{
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;

				wchar_t full[MAX_PATH];
				wchar_t dirW[MAX_PATH];
				Utf8OrAnsiToWide(dir, dirW, MAX_PATH);
				_snwprintf_s(full, _TRUNCATE, L"%s%s", dirW, fd.cFileName);

				if (_wcsicmp(full, currentFileW) == 0)
					continue;

				FileItem it;
				it.path = full;
				it.mtime = fd.ftLastWriteTime;
				items.push_back(it);
			} while (FindNextFileW(h, &fd));
			FindClose(h);
		}

		void PurgeOldFiles()
		{
			if (backupCount == 0)
				return;

			std::vector<FileItem> items;
			CollectArchiveFiles(items);
			if (items.size() <= backupCount)
				return;

			std::sort(items.begin(), items.end(), NewerFirst);
			for (size_t i = backupCount; i < items.size(); ++i)
				DeleteFileW(items[i].path.c_str());
		}

		static unsigned __stdcall WriteThread(void* p)
		{
			((Impl*)p)->WriteLoop();
			return 0;
		}

		static unsigned __stdcall CompressThread(void* p)
		{
			((Impl*)p)->CompressLoop();
			return 0;
		}

		void WriteLoop()
		{
			std::deque<LogBlock*> local;
			while (running)
			{
				WaitForSingleObject(evWrite, 200);
				EnterCriticalSection(&cs);
				FlushActiveLocked();
				local.swap(ready);
				bool doFlush = flushReq;
				LeaveCriticalSection(&cs);

				WriteBlocks(local);

				if (doFlush)
				{
					if (file != INVALID_HANDLE_VALUE)
						FlushFileBuffers(file);
					flushReq = false;
					SetEvent(evFlushDone);
				}
			}

			EnterCriticalSection(&cs);
			FlushActiveLocked();
			local.swap(ready);
			LeaveCriticalSection(&cs);
			WriteBlocks(local);
			if (file != INVALID_HANDLE_VALUE)
				FlushFileBuffers(file);
		}

		void CompressLoop()
		{
			while (running)
			{
				WaitForSingleObject(evCompress, 500);
				std::wstring path;
				EnterCriticalSection(&csCompress);
				if (!compressQ.empty())
				{
					path = compressQ.front();
					compressQ.pop_front();
				}
				LeaveCriticalSection(&csCompress);

				if (!path.empty())
				{
					CompressFile(path);
					PurgeOldFiles();
				}
			}

			for (;;)
			{
				std::wstring path;
				EnterCriticalSection(&csCompress);
				if (!compressQ.empty())
				{
					path = compressQ.front();
					compressQ.pop_front();
				}
				LeaveCriticalSection(&csCompress);
				if (path.empty())
					break;
				CompressFile(path);
			}
			PurgeOldFiles();
		}

		bool Start()
		{
			if (running)
				return true;

			if (!OpenNewFile())
				return false;

			evWrite = CreateEvent(NULL, FALSE, FALSE, NULL);
			evCompress = CreateEvent(NULL, FALSE, FALSE, NULL);
			evFlushDone = CreateEvent(NULL, FALSE, FALSE, NULL);
			running = true;
			thWrite = (HANDLE)_beginthreadex(NULL, 0, WriteThread, this, 0, NULL);
			thCompress = (HANDLE)_beginthreadex(NULL, 0, CompressThread, this, 0, NULL);
			return thWrite != NULL && thCompress != NULL;
		}

		void Stop()
		{
			if (!running)
				return;
			running = false;
			if (evWrite)
				SetEvent(evWrite);
			if (evCompress)
				SetEvent(evCompress);
			if (thWrite)
			{
				WaitForSingleObject(thWrite, 8000);
				CloseHandle(thWrite);
				thWrite = NULL;
			}
			if (thCompress)
			{
				WaitForSingleObject(thCompress, 30000);
				CloseHandle(thCompress);
				thCompress = NULL;
			}
			if (evWrite) { CloseHandle(evWrite); evWrite = NULL; }
			if (evCompress) { CloseHandle(evCompress); evCompress = NULL; }
			if (evFlushDone) { CloseHandle(evFlushDone); evFlushDone = NULL; }
		}

		void RequestFlush()
		{
			if (!running)
				return;
			flushReq = true;
			SetEvent(evWrite);
			WaitForSingleObject(evFlushDone, 2000);
		}

		void WriteFmt(LOGLEVEL lv, const char* fmt, va_list args)
		{
			if (lv < level || !fmt)
				return;

			char buf[kFmtBufSize];
			SYSTEMTIME st;
			GetLocalTime(&st);
			int head = _snprintf_s(buf, _TRUNCATE, "%02u:%02u:%02u.%03u [%s] ",
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, g_LOGLEVELStr[lv]);
			if (head < 0)
				head = 0;

			int body = _vsnprintf_s(buf + head, kFmtBufSize - head, _TRUNCATE, fmt, args);
			if (body < 0)
				body = (int)strlen(buf + head);

			int total = head + body;
			if (total < (int)kFmtBufSize - 2)
			{
				if (total == 0 || buf[total - 1] != '\n')
				{
					buf[total++] = '\n';
					buf[total] = 0;
				}
			}
			EnqueueRaw(buf, total);
			if (evWrite && lv >= lvALARM)
				SetEvent(evWrite);
		}
	};

	CLog::CLog(const char* path, const char* prefix, LOGLEVEL level,
		LOGSTYLE /*style*/, unsigned int maxsize, unsigned short /*manageInterval*/)
	{
		m_impl = new Impl();
		m_impl->owner = this;
		m_impl->level = level;
		m_impl->maxSize = maxsize < 1024 * 1024 ? 1024 * 1024 : maxsize;
		m_impl->BuildPath(path, prefix);
	}

	CLog::~CLog()
	{
		delete m_impl;
		m_impl = NULL;
	}

	void CLog::SetBackupCount(unsigned long nBackupCount)
	{
		m_impl->backupCount = nBackupCount;
	}

	bool CLog::StartLogFileManagerThread()
	{
		return m_impl->Start();
	}

	void CLog::Set_onLogFileRolled(LogFunc_onLogFileRolled pFunc)
	{
		m_impl->onRolled = pFunc;
	}

	void CLog::Debug(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		m_impl->WriteFmt(lvDEBUG, fmt, args);
		va_end(args);
	}

	void CLog::Info(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		m_impl->WriteFmt(lvINFO, fmt, args);
		va_end(args);
	}

	void CLog::Alarm(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		m_impl->WriteFmt(lvALARM, fmt, args);
		va_end(args);
	}

	void CLog::Error(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		m_impl->WriteFmt(lvERROR, fmt, args);
		va_end(args);
	}

	void CLog::Fatal(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		m_impl->WriteFmt(lvFATAL, fmt, args);
		va_end(args);
	}

	void CLog::WriteLogEx(LOGLEVEL /*level*/, bool /*bCache*/, const char* data, int nlen)
	{
		m_impl->EnqueueRaw(data, nlen);
	}

	bool CLog::Write(const char* pStr, int len, bool bEndLine)
	{
		if (!pStr || len <= 0)
			return false;
		m_impl->EnqueueRaw(pStr, len);
		if (bEndLine)
			m_impl->EnqueueRaw("\n", 1);
		if (m_impl->evWrite)
			SetEvent(m_impl->evWrite);
		return true;
	}

	void CLog::Flush()
	{
		m_impl->RequestFlush();
	}
}
