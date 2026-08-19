/*
 * Copyright (C) 2011-2018 smc-psip (http://www.seami.cloud)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

 // smc-psip.cpp : Defines the class behaviors for the application.
 //
#include "stdafx.h"
#include "smc-psip.h"
#include "mainDlg.h"
#include "const.h"
#include "settings.h"

#include "Strsafe.h"

#include <afxinet.h>
#include <Psapi.h>
#include <Dbghelp.h>
//#include "SDL.h"

#pragma comment(lib, "Psapi")
#pragma comment(lib, "Dbghelp")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static void onLogFileRolled(SMC_PSIP::CLog* pLogger, const char* pNewLogFile)
{
	char szBrief[1024] = { 0 };

	snprintf(szBrief, sizeof(szBrief) - 1,
		" ACD    [PID:%lu]============ [_WINDOWS_] %s ============\n",
		(unsigned long)GetCurrentProcessId(), "1.0.3.0");

	pLogger->Write(szBrief, strlen(szBrief));
}
// CSmcPSipApp

BEGIN_MESSAGE_MAP(CSmcPSipApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSmcPSipApp construction

CSmcPSipApp::CSmcPSipApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CSmcPSipApp object

CSmcPSipApp theApp;

// CSmcPSipApp initialization

CStringA wineVersion() {
	static const char* (CDECL * pwine_get_version)(void);
	HMODULE hntdll = GetModuleHandle(_T("ntdll.dll"));
	if (hntdll) {
		pwine_get_version = (const char* (*)())(void*)GetProcAddress(hntdll, "wine_get_version");
		if (pwine_get_version) {
			return pwine_get_version();
		}
	}
	return "n/a";
}

LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo)
{
	CTime tm = CTime::GetCurrentTime();
	CString filename;
	CFile file;
	// mini dump
	filename.Format(_T("%sdump%s.dmp"), accountSettings.pathLocal, tm.Format(_T("%Y%m%d%H%M%S")));
	if (file.Open(filename, CFile::modeCreate | CFile::modeReadWrite)) {
		MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInfo;
		MinidumpExceptionInfo.ThreadId = GetCurrentThreadId();
		MinidumpExceptionInfo.ExceptionPointers = ExceptionInfo;
		MinidumpExceptionInfo.ClientPointers = FALSE;
		if (MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			file.m_hFile,
			MiniDumpNormal,
			//MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory | MiniDumpWithFullMemory),
			&MinidumpExceptionInfo,
			NULL,
			NULL
		)) {
		}
		file.Close();
	}
	if (pj_ready && pjsua_var.state == PJSUA_STATE_RUNNING && tm.GetTime() - startTime.GetTime() > 10) {
		// automatic restart after sip crash
		ShellExecute(NULL, NULL, accountSettings.exeFile, NULL, NULL, SW_SHOWDEFAULT);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	else {
		AfxMessageBox(_T("We are sorry but SmcPSip crash happened.\r\nIf this problem is permanent and you know how to reproduce it, please contact us with details for fixing this error. Email: info@seami.cloud"), MB_ICONERROR);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

struct MsipEnumWindowsProcData {
	HINSTANCE hInst;
	HWND hWnd;
};

BOOL CALLBACK MsipEnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	MsipEnumWindowsProcData* data = (MsipEnumWindowsProcData*)lParam;
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	if (hInstance && hInstance == data->hInst && GetWindow(hWnd, GW_OWNER) == (HWND)0) {
		TCHAR className[256];
		if (GetClassName(hWnd, className, 256)) {
			if (StrCmp(className, _T(_GLOBAL_NAME)) == 0) {
				//--
				DWORD dwProcessID;
				GetWindowThreadProcessId(hWnd, &dwProcessID);
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
					PROCESS_VM_READ, FALSE, dwProcessID);
				if (hProcess) {
					TCHAR exeFilePath[MAX_PATH];
					if (GetModuleFileNameEx(hProcess, NULL, exeFilePath, MAX_PATH)) {
						if (StrCmpI(exeFilePath, accountSettings.exeFile) == 0) {
							data->hWnd = hWnd;
							return FALSE;
						}
					}
					CloseHandle(hProcess);
				}
				//--
			}
		}
	}
	return TRUE;
}

BOOL CSmcPSipApp::InitInstance()
{
	SetUnhandledExceptionFilter(ExceptionFilter);
	MsipEnumWindowsProcData data;
	data.hInst = AfxGetInstanceHandle();
	HWND hWndRunning = NULL;
	if (!EnumWindows(MsipEnumWindowsProc, (LPARAM)&data)) {
		hWndRunning = data.hWnd;
	}
	if (hWndRunning) {
		if (lstrcmp(theApp.m_lpCmdLine, _T("/exit")) == 0) {
			::SendMessage(hWndRunning, WM_CLOSE, NULL, NULL);
		}
		else if (lstrcmp(theApp.m_lpCmdLine, _T("/minimized")) == 0) {
		}
		else if (lstrcmp(theApp.m_lpCmdLine, _T("/hidden")) == 0) {
		}
		else {
			if (!accountSettings.silent) {
				::ShowWindow(hWndRunning, SW_SHOW);
				::SetForegroundWindow(hWndRunning);
			}
			if (lstrlen(theApp.m_lpCmdLine)) {
				COPYDATASTRUCT cd;
				cd.dwData = 1;
				cd.lpData = theApp.m_lpCmdLine;
				cd.cbData = sizeof(TCHAR) * (lstrlen(theApp.m_lpCmdLine) + 1);
				::SendMessage(hWndRunning, WM_COPYDATA, NULL, (LPARAM)&cd);
			}
		}
		return FALSE;
	}
	else {
		if (lstrcmp(theApp.m_lpCmdLine, _T("/exit")) == 0
			|| lstrcmp(theApp.m_lpCmdLine, _T("/answer")) == 0
			|| lstrcmp(theApp.m_lpCmdLine, _T("/hangupall")) == 0
			) {
			return FALSE;
		}
	}

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	// Set this to include all the common control classes you want to use
	// in your application.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_LISTVIEW_CLASSES |
		ICC_LINK_CLASS |
		ICC_BAR_CLASSES |
		ICC_LINK_CLASS |
		ICC_STANDARD_CLASSES |
		ICC_TAB_CLASSES |
		ICC_UPDOWN_CLASS;

	InitCommonControlsEx(&InitCtrls);

	//AfxEnableControlContainer();

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	AfxInitRichEdit2();

	WNDCLASS wc;
	// Get the info for this class.
	// #32770 is the default class name for dialogs boxes.
	::GetClassInfo(AfxGetInstanceHandle(), L"#32770", &wc);
	wc.lpszClassName = _T(_GLOBAL_NAME);
	// Register this class so that MFC can use it.
	if (!::AfxRegisterClass(&wc)) return FALSE;

	g_log_ptr = new SMC_PSIP::CLog(NULL, _GLOBAL_NAME, SMC_PSIP::lvDEBUG);
	if (g_log_ptr)
	{
		g_log_ptr->SetBackupCount(100);
		g_log_ptr->Set_onLogFileRolled(onLogFileRolled);
		g_log_ptr->StartLogFileManagerThread();
	}
	CMainDlg* mainDlg = new CMainDlg;
	m_pMainWnd = mainDlg;

	if (!m_pMainWnd) {
		// Since the dialog has been closed, return FALSE so that we exit the
		//  application, rather than start the application's message pump.
		return FALSE;
	}

	//--
	LRESULT pResult;
	mainDlg->OnTcnSelchangeTab(NULL, &pResult);

	mainDlg->OnCreated();

	//--

	return TRUE;

}
