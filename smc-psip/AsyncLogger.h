#pragma once

#include <stdarg.h>

namespace SMC_PSIP
{
	enum LOGLEVEL
	{
		lvDEBUG = 0,
		lvINFO,
		lvALARM,
		lvERROR,
		lvFATAL
	};

	enum LOGSTYLE
	{
		lsFILE = 0x01,
		lsCONSOLE = 0x02,
		lsBOTH = 0x03
	};

	extern const char g_LOGLEVELStr[][10];

	class CLog;
	typedef void (*LogFunc_onLogFileRolled)(CLog* pLogger, const char* pNewLogFile);

	// Async logger: enqueue on caller thread, write/rotate/compress/purge on worker threads.
	class CLog
	{
	public:
		CLog(const char* path, const char* prefix, LOGLEVEL level = lvDEBUG,
			LOGSTYLE style = lsFILE, unsigned int maxsize = 50 * 1024 * 1024,
			unsigned short manageInterval = 300);
		~CLog();

		void SetBackupCount(unsigned long nBackupCount);
		bool StartLogFileManagerThread();
		void Set_onLogFileRolled(LogFunc_onLogFileRolled pFunc);

		void Debug(const char* fmt, ...);
		void Info(const char* fmt, ...);
		void Alarm(const char* fmt, ...);
		void Error(const char* fmt, ...);
		void Fatal(const char* fmt, ...);
		void WriteLogEx(LOGLEVEL level, bool bCache, const char* data, int nlen);
		bool Write(const char* pStr, int len, bool bEndLine = false);
		void Flush();

	private:
		CLog(const CLog&);
		CLog& operator=(const CLog&);

		struct Impl;
		Impl* m_impl;
	};
}
