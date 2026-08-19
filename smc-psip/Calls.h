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

#pragma once

#include "resource.h"
#include "global.h"
#include "BaseDialog.h"
#include "CListCtrl_SortItemsEx.h"
#include "AddSipAgentDlg.h"
#include "AddCallOutDlg.h"
#include "CallTaskThread.h"

enum 
{
	EU_CALL_TIMER_NORMAL,
	EU_CALL_TIMER_RTCP,
	EU_CALL_TIMER_CHECK_CALL_TASK
};

class Calls :
	public CBaseDialog,public CCallTaskThread
{
public:
	Calls(CWnd* pParent = NULL);	// standard constructor
	~Calls();
	enum { IDD = IDD_CALLS };

	//CListCtrl_SortItemsEx m_SortItemsExListCtrl;
	AddCallOutDlg* addCallCallDlg;
	AddSipAgentDlg* addSipAgentDlg;

	virtual bool Start();
	virtual	bool Stop();

	void Add(CString number,
		CString name,
		CString passwd,
		CString domain,
		CString called,
		CString dtmf,
		int type,
		int port,
		int duration,
		int regtimeout,
		int dtmfduration,
		int autoanswer);

	void SetCallRegistered(int id, int code,CString info);
	void SetCallCallStatus(int id, CallStatus*);
	Call* SetCallCallStatus(int id,int operate, CString caller, CString called, CString info,int code);

	void Delete(int i);

	void CallsLoad();
	void CallsClear();
	bool isFiltered(Call *pCall = NULL);
	void filterReset();

	void OnCreated();

	CallsMap& GetCallMap() { return m_Calls; }
	void OnCallMediaChanged(pjsua_call_id callid, bool mediaActive);
	void SetListenCall(Call *pCall);
	void SetMicBroadcast(bool enable);
	void StartCallRecord(Call *pCall, int mode);
	void StopCallRecord(Call *pCall);
protected:
	virtual void OnMakeCallSuccess(pjsua_acc_id accid, pjsua_call_id callid);
	virtual void OnMakeCallFailed(pjsua_acc_id accid);
private:
	CImageList* imageList;
	int lastDay;
	int nextKey;

	CallsMap	m_Calls;
	CString m_listenNumber;
	int m_recordingCount;
	CButton m_chkRecord;
	CComboBox m_cmbRecordMode;
	CComboBox m_cmbRecordScope;
	CEdit m_editRecordMax;
	CEdit m_editRecordDir;
	CButton m_chkMicBroadcast;
	CStatic m_lblListen;
	void CreateMediaBar();
	void LoadMediaBarFromSettings();
	void SaveMediaBarToSettings();
	void AttachPlayback(Call *pCall, bool connect);
	void ApplyMicToCall(Call *pCall, bool connect);
	void EnsureSoundDevice(bool need);
	pjsua_conf_port_id GetLocalPlayPort();
	pjsua_conf_port_id GetCallConfSlot(Call *pCall);
	int ResolveRecordMode(Call *pCall);
	bool ShouldStartRecord(Call *pCall);
	void ApplyRecordConnect(Call *pCall);
	Call* FindNextListenCall(Call *exclude);
	void UpdateCallRowVisual(Call *pCall, int defaultImage = -1);
	void UpdateListenHint();
	CString RecordModeText(int mode);
	void CallSave(Call *pCall);
	void CallDecode(CString str, Call *pCall);
	CString CallEncode(Call *pCall);
	void Insert(Call *pCall,int pos = 0);
	Call* GetByAccID(int accid);
	Call* GetByCallID(int callid);
	Call* GetByCaller(CString caller);
	void MessageDlgOpen(BOOL isCall = FALSE, BOOL hasVideo = FALSE);
	void OnDumpCallState();
	void OnCheckCallTask();
	void SendPendingDtmf(Call *pCall, double elapsed);
	void ReleasePjsipAccount(Call *pCall);
	CString DumpMediaStat(const char *indent,const pjmedia_rtcp_stat *stat);
	bool CallDialDTMF(pjsua_call_id callid, CString strDTMF);
protected:
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnFilterValueChange();
	afx_msg void OnMenuAddCallOutTask();
	afx_msg void OnMenuAddSipAgent();
	afx_msg void OnMenuCall(); 
	afx_msg void OnMenuChat();
	afx_msg void OnMenuCopy();
	afx_msg void OnMenuDelete();  
	afx_msg void OnMenuImportGoogle();
	afx_msg void OnMenuCallPickup();
	afx_msg LRESULT OnContextMenu(WPARAM wParam,LPARAM lParam);
	afx_msg void OnNMDblclkCalls(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEndtrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT_PTR TimerVal);
	afx_msg void OnRecordSettingsChanged();
	afx_msg void OnMicBroadcastClicked();
	afx_msg void OnMenuListenCall();
	afx_msg void OnMenuRecordStart();
	afx_msg void OnMenuRecordStop();
	afx_msg void OnMenuRecordModeBoth();
	afx_msg void OnMenuRecordModeLocal();
	afx_msg void OnMenuRecordModeRemote();
	afx_msg void OnCustomDrawCalls(NMHDR *pNMHDR, LRESULT *pResult);
#ifdef _GLOBAL_VIDEO
	afx_msg void OnMenuCallVideo(); 
#endif
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

