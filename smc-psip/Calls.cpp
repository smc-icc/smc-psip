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

#include "StdAfx.h"
#include "Calls.h"
#include "smc-psip.h"
#include "global.h"
#include "settings.h"
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>
#include "mainDlg.h"
#include "langpack.h"
#include "CSVFile.h"


enum
{
	EU_CALLS_LIST_ICON_MISS,
	EU_CALLS_LIST_ICON_ACITVE,
	EU_CALLS_LIST_ICON_CALLOUT,
	EU_CALLS_LIST_ICON_CALLIN
};

Calls::Calls(CWnd* pParent /*=NULL*/)
: CBaseDialog(Calls::IDD, pParent)
{
	Create (IDD, pParent);
}

Calls::~Calls(void)
{
	
}

BOOL Calls::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	AutoMove(IDC_CALLS,0,0,100,100);
	AutoMove(IDC_SEARCH_PICTURE,0,100,0,0);
	AutoMove(IDC_FILER_VALUE,0,100,100,0);

	TranslateDialog(this->m_hWnd);

	addCallCallDlg = new AddCallOutDlg(this);
	addSipAgentDlg = new AddSipAgentDlg(this);
	nextKey = 0;
	lastDay = 0;
	
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);

	list->SetExtendedStyle( list->GetExtendedStyle() |  LVS_EX_FULLROWSELECT );

	imageList = new CImageList();
	imageList->Create(16,16,ILC_COLOR32,3,3);
	imageList->SetBkColor(RGB(255, 255, 255));
	imageList->Add(theApp.LoadIcon(IDI_CALL_MISS));
	imageList->Add(theApp.LoadIcon(IDI_ACTIVE));
	imageList->Add(theApp.LoadIcon(IDI_CALL_OUT));
	imageList->Add(theApp.LoadIcon(IDI_CALL_IN));
	list->SetImageList(imageList,LVSIL_SMALL);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_NUMBER,Translate(_T("Number")),LVCFMT_LEFT, accountSettings.callsColumnNumberWidth>0?accountSettings.callsColumnNumberWidth :80);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_NAME, Translate(_T("Name")), LVCFMT_LEFT, accountSettings.callsColumnNameWidth > 0 ? accountSettings.callsColumnNameWidth : 80);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_PASSWD, Translate(_T("Passwd")), LVCFMT_LEFT, accountSettings.callsColumnPasswdWidth > 0 ? accountSettings.callsColumnPasswdWidth : 80);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_DOMAIN, Translate(_T("Domain")), LVCFMT_LEFT, accountSettings.callsColumnUrlWidth > 0 ? accountSettings.callsColumnUrlWidth : 150);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_PORT, Translate(_T("Port")), LVCFMT_LEFT, accountSettings.callsColumnPortWidth > 0 ? accountSettings.callsColumnPortWidth : 80);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_CALLED, Translate(_T("Called")), LVCFMT_LEFT, accountSettings.callsColumnCalledWidth > 0 ? accountSettings.callsColumnCalledWidth : 80);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_DTMF, Translate(_T("DTMF")), LVCFMT_LEFT, 50);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_DTMF_DURATION, Translate(_T("DTMFTimer")), LVCFMT_LEFT,70);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_DURATION, Translate(_T("Duration")), LVCFMT_LEFT, accountSettings.callsColumnDurationWidth > 0 ? accountSettings.callsColumnDurationWidth : 80);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_REGTIMEOUT, Translate(_T("RegTimeout")), LVCFMT_LEFT, accountSettings.callsColumnRegTimeoutWidth > 0 ? accountSettings.callsColumnRegTimeoutWidth : 80);
	list->InsertColumn(EU_CALLS_LIST_COLUMN_STATUS, Translate(_T("Status")), LVCFMT_LEFT, accountSettings.callsColumnStatusWidth > 0 ? accountSettings.callsColumnStatusWidth : 200);	
	list->InsertColumn(EU_CALLS_LIST_COLUMN_RTCP, Translate(_T("RTCP")), LVCFMT_LEFT, accountSettings.callsColumnRTCPWidth > 0 ? accountSettings.callsColumnRTCPWidth : 400);

	CallsLoad();
	SetTimer(EU_CALL_TIMER_RTCP, 1000, NULL);
	return TRUE;
}

void Calls::OnCreated()
{
	//m_SortItemsExListCtrl.SetSortColumn(1, false);
	//Init();
}

void Calls::PostNcDestroy()
{
	CBaseDialog::PostNcDestroy();
	mainDlg->SetCalls(NULL);
	//KillTimer(EU_CALL_TIMER_RTCP);
	delete imageList;
	delete this;
	//UnInit();
}

void Calls::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_CALLS, m_SortItemsExListCtrl);
}


BEGIN_MESSAGE_MAP(Calls, CBaseDialog)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_NOTIFY(HDN_ENDTRACK, 0, OnEndtrack)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_EN_CHANGE(IDC_FILER_VALUE, OnFilterValueChange)
	ON_COMMAND(ID_ADD_CALLOUT_TASK, OnMenuAddCallOutTask)
	ON_COMMAND(ID_ADD_SIP_AGENT, OnMenuAddSipAgent)
	ON_COMMAND(ID_CALL_PICKUP, OnMenuCallPickup)
	ON_COMMAND(ID_CALL,OnMenuCall)
	ON_COMMAND(ID_CHAT,OnMenuChat)
	ON_COMMAND(ID_COPY,OnMenuCopy)
	ON_COMMAND(ID_DELETE,OnMenuDelete)

	ON_COMMAND(ID_IMPORT_GOOGLE, OnMenuImportGoogle)
	ON_NOTIFY(NM_DBLCLK, IDC_CALLS, &Calls::OnNMDblclkCalls)
	ON_MESSAGE(WM_CONTEXTMENU,OnContextMenu)
#ifdef _GLOBAL_VIDEO
	ON_COMMAND(ID_VIDEOCALL,OnMenuCallVideo)
#endif
END_MESSAGE_MAP()

BOOL Calls::PreTranslateMessage(MSG* pMsg)
{
	BOOL catched = FALSE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
		CEdit* edit = (CEdit*)GetDlgItem(IDC_FILER_VALUE);
		if (edit == GetFocus()) {
			catched = TRUE;
			if (isFiltered()) {
				filterReset();
			}
		}
	}
	if (!catched) {
		return CBaseDialog::PreTranslateMessage(pMsg);
	} else {
		return TRUE;
	}
}

void Calls::OnEndtrack(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *)pNMHDR;
	int width = phdn->pitem->cxy;
	switch (phdn->iItem) {
	case EU_CALLS_LIST_COLUMN_NUMBER:
		accountSettings.callsColumnNumberWidth = width;
		break;
	case EU_CALLS_LIST_COLUMN_NAME:
		accountSettings.callsColumnNameWidth = width;
		break;
	case EU_CALLS_LIST_COLUMN_PASSWD:
		accountSettings.callsColumnPasswdWidth = width;
		break;
	case EU_CALLS_LIST_COLUMN_DOMAIN:
		accountSettings.callsColumnUrlWidth = width;
		break;
	case EU_CALLS_LIST_COLUMN_PORT:
		accountSettings.callsColumnPortWidth = width;
		break;
	case EU_CALLS_LIST_COLUMN_CALLED:
		accountSettings.callsColumnCalledWidth = width;	
		break; 
	case EU_CALLS_LIST_COLUMN_DURATION:
		accountSettings.callsColumnDurationWidth = width;
		break;
	case EU_CALLS_LIST_COLUMN_REGTIMEOUT:
		accountSettings.callsColumnRegTimeoutWidth = width;
		break;
	case EU_CALLS_LIST_COLUMN_STATUS:
		accountSettings.callsColumnStatusWidth = width;
		break;	
	case EU_CALLS_LIST_COLUMN_RTCP:
		accountSettings.callsColumnRTCPWidth = width;
		break;
	}
	mainDlg->AccountSettingsPendingSave();
	*pResult = 0;
}


void Calls::OnTimer(UINT_PTR TimerVal)
{
	if (TimerVal == EU_CALL_TIMER_RTCP)
	{
		OnDumpCallState();
	}
	else if (TimerVal == EU_CALL_TIMER_CHECK_CALL_TASK)
	{
		OnCheckCallTask();
	}
}

void Calls::OnBnClickedOk()
{
	MessageDlgOpen(accountSettings.singleMode);
}

void Calls::OnBnClickedCancel()
{
	mainDlg->ShowWindow(SW_HIDE);
}

void Calls::OnFilterValueChange()
{
	CallsClear();
	CallsLoad();
}

bool Calls::isFiltered(Call *pCall) {
	CEdit* edit = (CEdit*)GetDlgItem(IDC_FILER_VALUE);
	CString str;
	edit->GetWindowText(str);
	if (!str.IsEmpty()) {
		if (!pCall ) {
			return true;
		}
		str.MakeLower();
		CString name = pCall->name;
		CString number = pCall->number;
		name.MakeLower();
		number.MakeLower();
		if (name.Find(str) ==-1 && number.Find(str) ==-1) {
			return true;
		}
	}
	return false;
}

void Calls::filterReset()
{
	CEdit* edit = (CEdit*)GetDlgItem(IDC_FILER_VALUE);
	edit->SetWindowText(_T(""));
}

LRESULT Calls::OnContextMenu(WPARAM wParam,LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);
	POINT pt = { x, y };
	RECT rc;
	CListCtrl *list = (CListCtrl*)GetDlgItem(IDC_CALLS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	int selectedItem = -1;
	if (pos) {
		selectedItem = list->GetNextSelectedItem(pos);
	}
	if (x != -1 || y != -1) {
		ScreenToClient(&pt);
		GetClientRect(&rc);
		if (!PtInRect(&rc, pt)) {
			x = y = -1;
		}
	}
	else {
		if (selectedItem != -1) {
			list->GetItemPosition(selectedItem, &pt);
			list->ClientToScreen(&pt);
			x = 40 + pt.x;
			y = 8 + pt.y;
		}
		else {
			::ClientToScreen((HWND)wParam, &pt);
			x = 10 + pt.x;
			y = 10 + pt.y;
		}
	}
	if (x != -1 || y != -1) {
		CMenu menu;
		menu.LoadMenu(IDR_MENU_CONTACT);
		CMenu* tracker = menu.GetSubMenu(0);
		TranslateMenu(tracker->m_hMenu);
		if (selectedItem != -1) {
			Contact *pContact = (Contact *)list->GetItemData(selectedItem);
			if (pContact->ringing) {
				tracker->InsertMenu(ID_CALL, 0, ID_CALL_PICKUP, Translate(_T("Call Pickup")));
			}
			tracker->EnableMenuItem(ID_CALL, FALSE);
#ifdef _GLOBAL_VIDEO
			tracker->EnableMenuItem(ID_VIDEOCALL, FALSE);
#endif
			tracker->EnableMenuItem(ID_CHAT, FALSE);
			tracker->EnableMenuItem(ID_EDIT, FALSE);
			tracker->EnableMenuItem(ID_COPY, FALSE);
			tracker->EnableMenuItem(ID_DELETE, FALSE);
		}
		else {
			tracker->EnableMenuItem(ID_CALL, TRUE);
#ifdef _GLOBAL_VIDEO
			tracker->EnableMenuItem(ID_VIDEOCALL, TRUE);
#endif
			tracker->EnableMenuItem(ID_CHAT, TRUE);
			tracker->EnableMenuItem(ID_EDIT, TRUE);
			tracker->EnableMenuItem(ID_COPY, TRUE);
			tracker->EnableMenuItem(ID_DELETE, TRUE);
		}
		tracker->AppendMenu(0, MF_SEPARATOR);
		CMenu importMenu;
		importMenu.CreatePopupMenu();
		importMenu.AppendMenu(MF_STRING, ID_IMPORT_GOOGLE, Translate(_T("Google CSV")));
		tracker->AppendMenu(MF_POPUP, (UINT_PTR)importMenu.m_hMenu, Translate(_T("Import")));

		tracker->TrackPopupMenu(0, x, y, this);
		return TRUE;
	}
	return DefWindowProc(WM_CONTEXTMENU,wParam,lParam);
}

void Calls::MessageDlgOpen(BOOL isCall, BOOL hasVideo)
{
	if (accountSettings.singleMode && call_get_count_noincoming() && isCall) {
		mainDlg->GotoTab(0);
		return;
	}
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		int i = list->GetNextSelectedItem(pos);
		Call *pCall = (Call *) list->GetItemData(i);
		if (isCall) {
			mainDlg->MakeCall(pCall->number, hasVideo);
		} else {
			mainDlg->MessagesOpen(pCall->number);
		}
	}
}

CString Calls::DumpMediaStat(const char *indent,const pjmedia_rtcp_stat *stat)
{
	char buff[1024];
	char last_update[64];
	char packets[32], bytes[32], ipbytes[32], avg_bps[32], avg_ipbps[32];
	pj_time_val media_duration, now;
	int len = 0;

	if (stat->rx.update_cnt == 0)
		strcpy(last_update, "never");
	else {
		pj_gettimeofday(&now);
		PJ_TIME_VAL_SUB(now, stat->rx.update);
		sprintf(last_update, "%02ldh:%02ldm:%02ld.%03lds ago",
			now.sec / 3600,
			(now.sec % 3600) / 60,
			now.sec % 60,
			now.msec);
	}

	pj_gettimeofday(&media_duration);
	PJ_TIME_VAL_SUB(media_duration, stat->start);
	if (PJ_TIME_VAL_MSEC(media_duration) == 0)
		media_duration.msec = 1;

	len += pj_ansi_snprintf(buff, 1024 - len,
		"%s		   last update:%s\n"
		"%s        total %spkt %sB (%sB +IP hdr) @avg=%sbps/%sbps\n"
		"%s        pkt loss=%d (%3.1f%%), discrd=%d (%3.1f%%), dup=%d (%2.1f%%), reord=%d (%3.1f%%)\n"
		"%s              (msec)    min     avg     max     last    dev\n"
		"%s        loss period: %7.3f %7.3f %7.3f %7.3f %7.3f\n"
		"%s        jitter     : %7.3f %7.3f %7.3f %7.3f %7.3f\n"
#if defined(PJMEDIA_RTCP_STAT_HAS_RAW_JITTER) && PJMEDIA_RTCP_STAT_HAS_RAW_JITTER!=0
		"%s        raw jitter : %7.3f %7.3f %7.3f %7.3f %7.3f\n"
#endif
#if defined(PJMEDIA_RTCP_STAT_HAS_IPDV) && PJMEDIA_RTCP_STAT_HAS_IPDV!=0
		"%s        IPDV       : %7.3f %7.3f %7.3f %7.3f %7.3f\n"
#endif
		"%s",
		indent,
		last_update,
		indent,
		good_number(packets, stat->rx.pkt),
		good_number(bytes, stat->rx.bytes),
		good_number(ipbytes, stat->rx.bytes + stat->rx.pkt * 40),
		good_number(avg_bps, (pj_int32_t)((pj_int64_t)stat->rx.bytes * 8 * 1000 / PJ_TIME_VAL_MSEC(media_duration))),
		good_number(avg_ipbps, (pj_int32_t)(((pj_int64_t)stat->rx.bytes + stat->rx.pkt * 40) * 8 * 1000 / PJ_TIME_VAL_MSEC(media_duration))),
		indent,
		stat->rx.loss,
		(stat->rx.loss ? stat->rx.loss * 100.0 / (stat->rx.pkt + stat->rx.loss) : 0),
		stat->rx.discard,
		(stat->rx.discard ? stat->rx.discard * 100.0 / (stat->rx.pkt + stat->rx.loss) : 0),
		stat->rx.dup,
		(stat->rx.dup ? stat->rx.dup * 100.0 / (stat->rx.pkt + stat->rx.loss) : 0),
		stat->rx.reorder,
		(stat->rx.reorder ? stat->rx.reorder * 100.0 / (stat->rx.pkt + stat->rx.loss) : 0),
		indent, indent,
		stat->rx.loss_period.min / 1000.0,
		stat->rx.loss_period.mean / 1000.0,
		stat->rx.loss_period.max / 1000.0,
		stat->rx.loss_period.last / 1000.0,
		pj_math_stat_get_stddev(&stat->rx.loss_period) / 1000.0,
		indent,
		stat->rx.jitter.min / 1000.0,
		stat->rx.jitter.mean / 1000.0,
		stat->rx.jitter.max / 1000.0,
		stat->rx.jitter.last / 1000.0,
		pj_math_stat_get_stddev(&stat->rx.jitter) / 1000.0,
#if defined(PJMEDIA_RTCP_STAT_HAS_RAW_JITTER) && PJMEDIA_RTCP_STAT_HAS_RAW_JITTER!=0
		indent,
		stat->rx_raw_jitter.min / 1000.0,
		stat->rx_raw_jitter.mean / 1000.0,
		stat->rx_raw_jitter.max / 1000.0,
		stat->rx_raw_jitter.last / 1000.0,
		pj_math_stat_get_stddev(&stat->rx_raw_jitter) / 1000.0,
#endif
#if defined(PJMEDIA_RTCP_STAT_HAS_IPDV) && PJMEDIA_RTCP_STAT_HAS_IPDV!=0
		indent,
		stat->rx_ipdv.min / 1000.0,
		stat->rx_ipdv.mean / 1000.0,
		stat->rx_ipdv.max / 1000.0,
		stat->rx_ipdv.last / 1000.0,
		pj_math_stat_get_stddev(&stat->rx_ipdv) / 1000.0,
#endif
		""
	);


	if (stat->tx.update_cnt == 0)
		strcpy(last_update, "never");
	else {
		pj_gettimeofday(&now);
		PJ_TIME_VAL_SUB(now, stat->tx.update);
		sprintf(last_update, "%02ldh:%02ldm:%02ld.%03lds ago",
			now.sec / 3600,
			(now.sec % 3600) / 60,
			now.sec % 60,
			now.msec);
	}

	len += pj_ansi_snprintf(buff, 1024 - len,
		"%s		   last update:%s\n"
		"%s        total %spkt %sB (%sB +IP hdr) @avg=%sbps/%sbps\n"
		"%s        pkt loss=%d (%3.1f%%), dup=%d (%3.1f%%), reorder=%d (%3.1f%%)\n"
		"%s              (msec)    min     avg     max     last    dev \n"
		"%s        loss period: %7.3f %7.3f %7.3f %7.3f %7.3f\n"
		"%s        jitter     : %7.3f %7.3f %7.3f %7.3f %7.3f\n",
		indent,
		last_update,

		indent,
		good_number(packets, stat->tx.pkt),
		good_number(bytes, stat->tx.bytes),
		good_number(ipbytes, stat->tx.bytes + stat->tx.pkt * 40),
		good_number(avg_bps, (pj_int32_t)((pj_int64_t)stat->tx.bytes * 8 * 1000 / PJ_TIME_VAL_MSEC(media_duration))),
		good_number(avg_ipbps, (pj_int32_t)(((pj_int64_t)stat->tx.bytes + stat->tx.pkt * 40) * 8 * 1000 / PJ_TIME_VAL_MSEC(media_duration))),

		indent,
		stat->tx.loss,
		(stat->tx.loss ? stat->tx.loss * 100.0 / stat->tx.pkt : 0),
		stat->tx.dup,
		(stat->tx.dup ? stat->tx.dup * 100.0 / stat->tx.pkt : 0),
		stat->tx.reorder,
		(stat->tx.reorder ? stat->tx.reorder * 100.0 / stat->tx.pkt : 0),

		indent, indent,
		stat->tx.loss_period.min / 1000.0,
		stat->tx.loss_period.mean / 1000.0,
		stat->tx.loss_period.max / 1000.0,
		stat->tx.loss_period.last / 1000.0,
		pj_math_stat_get_stddev(&stat->tx.loss_period) / 1000.0,
		indent,
		stat->tx.jitter.min / 1000.0,
		stat->tx.jitter.mean / 1000.0,
		stat->tx.jitter.max / 1000.0,
		stat->tx.jitter.last / 1000.0,
		pj_math_stat_get_stddev(&stat->tx.jitter) / 1000.0
	);

	len += pj_ansi_snprintf(buff, 1024 - len,
		"%s     RTT msec      : %7.3f %7.3f %7.3f %7.3f %7.3f\n",
		indent,
		stat->rtt.min / 1000.0,
		stat->rtt.mean / 1000.0,
		stat->rtt.max / 1000.0,
		stat->rtt.last / 1000.0,
		pj_math_stat_get_stddev(&stat->rtt) / 1000.0
	);

	return CString(buff);
}


bool Calls::CallDialDTMF(pjsua_call_id callid, CString strDTMF)
{
	int ret = PJ_SUCCESS;
	if (callid != PJSUA_INVALID_ID) {
		pjsua_call_info call_info;
		pjsua_call_get_info(callid, &call_info);
		if (call_info.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
			pj_str_t pj_digits = StrToPjStr(strDTMF);
			if (accountSettings.DTMFMethod == 1) {
				// in-band
				return call_play_digit(callid, StrToPj(strDTMF));
			}
			else if (accountSettings.DTMFMethod == 2) {
				// RFC2833
				return pjsua_call_dial_dtmf(callid, &pj_digits) == PJ_SUCCESS;
			}
			else if (accountSettings.DTMFMethod == 3) {
				// sip-info
				return msip_call_send_dtmf_info(callid, pj_digits);
			}
			else {
				// auto
				if (pjsua_call_dial_dtmf(callid, &pj_digits) != PJ_SUCCESS) {
					return false;
				}
			}
		}
	}
	return true;
}

void Calls::OnDumpCallState()
{
#if defined(PJMEDIA_HAS_RTCP_XR) && (PJMEDIA_HAS_RTCP_XR != 0)
	#define  MAX_BUFF_SIZE	 1024 * 10
#else
	#define  MAX_BUFF_SIZE   1024 * 3
#endif

	CString strInfo = _T("");
	CString strPanelInfo = _T("");
	int nRegistered = 0;
	int nUnRegistered = 0;
	int nCalling = 0;
	int nIncoming = 0;
	int nEarly = 0;
	int nConnecting = 0;
	int nConfirmed = 0;

	CListCtrl *pList = (CListCtrl*)GetDlgItem(IDC_CALLS);
	for (CallsMapIt it = m_Calls.begin(); it != m_Calls.end(); ++it)
	{
		if (it->second.callid != PJSUA_INVALID_ID)
		{
			pjsua_call_info callinfo;
			if (pjsua_call_get_info(it->second.callid,&callinfo) == PJ_SUCCESS)
			{
				if (callinfo.state == PJSIP_INV_STATE_CONFIRMED)
				{
					pjsua_stream_stat	streamstat;
					if (pjsua_call_get_stream_stat(it->second.callid,0, &streamstat) == PJ_SUCCESS)
					{
						if (streamstat.rtcp.rx.pkt && streamstat.rtcp.rx.pkt)
						{
							strInfo.Format(_T("%s%7.3f %s%7.3f %s%d"),
								Translate(_T("RX Lost:")), (double)(streamstat.rtcp.rx.loss / streamstat.rtcp.rx.pkt) * 100.0,
								Translate(_T("TX Lost:")), (double)(streamstat.rtcp.tx.loss / streamstat.rtcp.rx.pkt) * 100.0,
								Translate(_T("Delay:")), streamstat.jbuf.avg_delay);

							pList->SetItemText(it->second.listIndex, EU_CALLS_LIST_COLUMN_RTCP, strInfo);
						}
					}
					nConfirmed++;
					/*
					char buffer[MAX_BUFF_SIZE];
					if (pjsua_call_dump(it->second.callid,PJ_TRUE,buffer, MAX_BUFF_SIZE,"") == PJ_SUCCESS)
					{

					}*/
				}
				else if(callinfo.state == PJSIP_INV_STATE_CALLING)
				{
					nCalling++;
				}
				else if (callinfo.state == PJSIP_INV_STATE_INCOMING)
				{
					nIncoming++;
				}
				else if (callinfo.state == PJSIP_INV_STATE_EARLY)
				{
					nEarly++;
				}
				else if (callinfo.state == PJSIP_INV_STATE_CONNECTING)
				{
					nConnecting++;
				}
				else if (callinfo.state == PJSIP_INV_STATE_DISCONNECTED)
				{
					nConnecting++;
				}
			}
		}
		else
		{
			if (!pList->GetItemText(it->second.listIndex, EU_CALLS_LIST_COLUMN_RTCP).IsEmpty())
			{
				pList->SetItemText(it->second.listIndex, EU_CALLS_LIST_COLUMN_RTCP, _T(""));
			}
		}

		if (it->second.accid != PJSUA_INVALID_ID && it->second.regCode == 200)
		{
			nRegistered++;
		}
		else
		{
			nUnRegistered++;
		}
	}

	strPanelInfo.Format(_T("%s:%d %s:%d %s:%d %s:%d %s:%d %s:%d %s:%d"),Translate(_T("Registered")),nRegistered
									,Translate(_T("UnRegistered")), nUnRegistered 
									,Translate(_T("Calling")), nCalling 
									,Translate(_T("Incoming Call")), nIncoming
									,Translate(_T("Early")), nEarly
									,Translate(_T("Connecting")), nConnecting
									,Translate(_T("Connected")), nConfirmed);
	mainDlg->UpdateWindowText(strPanelInfo, IDI_ONLINE);
}


void Calls::OnCheckCallTask()
{
	time_t curtime;
	time(&curtime);

	for (CallsMapIt it = m_Calls.begin(); it != m_Calls.end(); ++it)
	{
		if (it->second.callid != PJSUA_INVALID_ID)
		{
			pjsua_call_info callinfo;
			if (pjsua_call_get_info(it->second.callid, &callinfo) == PJ_SUCCESS)
			{
				if (callinfo.state == PJSIP_INV_STATE_CONFIRMED)
				{
					if (it->second.callduration
						&& (it->second.lastcalltime != (time_t)-1)
						&& difftime(curtime,it->second.lastcalltime) > it->second.callduration)
					{
						msip_call_hangup_fast(it->second.callid);
						it->second.recall = true;
					}

					if (it->second.dtmfduration > 0 
						&& (!it->second.dtmf.IsEmpty()) 
						&& (it->second.lastcalltime != (time_t)-1 )
						&& difftime(curtime, it->second.lastcalltime) > it->second.dtmfduration)
					{
						CallDialDTMF(it->second.callid, it->second.dtmf);
					}
				}
			}
		}
		else
		{
			if (it->second.callduration
				&& (it->second.lastcalltime != (time_t)-1)
				&& difftime(curtime, it->second.lastcalltime) > it->second.callduration)
			{
				AddCall(&(it->second));
				SetEvent();
			}
		}
	}
}

void Calls::OnNMDblclkCalls(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem!=-1) {
		MessageDlgOpen(accountSettings.singleMode);
	}
	*pResult = 0;
}

void Calls::OnMenuAddCallOutTask()
{
	if (!addCallCallDlg->IsWindowVisible()) {
		addCallCallDlg->ShowWindow(SW_SHOW);
	}
	else {
		addCallCallDlg->SetForegroundWindow();
	}
}


void Calls::OnMenuAddSipAgent()
{
	if (!addSipAgentDlg->IsWindowVisible()) {
		addSipAgentDlg->ShowWindow(SW_SHOW);
	}
	else {
		addSipAgentDlg->SetForegroundWindow();
	}
}

void Calls::OnMenuCallPickup()
{
	CListCtrl *list = (CListCtrl*)GetDlgItem(IDC_CALLS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		int i = list->GetNextSelectedItem(pos);
		Call *pListCall = (Call *)list->GetItemData(i);
		if (pListCall)
		{
			Call* pCall = GetByCaller(pListCall->name);
			if (pCall && pCall->callid != PJSUA_INVALID_ID)
			{
				pjsua_call_setting call_setting;
				pjsua_call_setting_default(&call_setting);
				if (pjsua_call_answer2(pCall->callid, &call_setting, 200, NULL, NULL) == PJ_SUCCESS) {

				}
			}
		}
	}
}

void Calls::OnMenuCall()
{
	MessageDlgOpen(TRUE);
}

#ifdef _GLOBAL_VIDEO
void Calls::OnMenuCallVideo()
{
	MessageDlgOpen(TRUE, TRUE);
}
#endif

void Calls::OnMenuChat()
{
	MessageDlgOpen();
}

void Calls::OnMenuCopy()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos)
	{
		int i = list->GetNextSelectedItem(pos);
		Call *pCall = (Call *) list->GetItemData(i);
		mainDlg->CopyStringToClipboard(pCall->number);
	}
}

void Calls::OnMenuDelete()
{
	CListCtrl *pList= (CListCtrl*)GetDlgItem(IDC_CALLS);
	POSITION pos = pList->GetFirstSelectedItemPosition();
	while (pos)	{
		Delete(pList->GetNextSelectedItem(pos));
		pos = pList->GetFirstSelectedItemPosition();
	}
}

bool Calls::Start()
{ 
	for (CallsMapIt it = m_Calls.begin();it != m_Calls.end();++it)
	{
		if (it->second.accid != PJSUA_INVALID_ID && it->second.regCode == 200)
		{
			AddCall(&(it->second));
		}
	}
	SetTimer(EU_CALL_TIMER_CHECK_CALL_TASK, 1, NULL);
	return SetEvent();
}

bool Calls::Stop()
{
	bool ret = CCallTaskThread::PulseEvent();
	KillTimer(EU_CALL_TIMER_CHECK_CALL_TASK);
	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned count = PJSUA_MAX_CALLS;
	if (pjsua_var.state == PJSUA_STATE_RUNNING && pjsua_enum_calls(call_ids, &count) == PJ_SUCCESS) {
		for (unsigned i = 0; i < count; ++i) {
			pjsua_call_info call_info;
			pjsua_call_get_info(call_ids[i], &call_info);
			if (call_info.role != PJSIP_ROLE_UAS || (call_info.state != PJSIP_INV_STATE_INCOMING && call_info.state != PJSIP_INV_STATE_EARLY)) {
				msip_call_hangup_fast(call_ids[i]);
			}
		}
	}

	return ret;
}

void Calls::OnMenuImportGoogle()
{
	CFileDialog dlgFile(TRUE, _T("cvs"), 0, OFN_HIDEREADONLY, _T("CSV Files (*.csv)|*.csv|"), this);
	if (dlgFile.DoModal() == IDOK) {
		bool changed = false;
		if (isFiltered()) {
			filterReset();
		}
		CCSVFile CSVFile;
		CSVFile.SetCodePage(CP_UTF8);
		if (CSVFile.Open(dlgFile.GetPathName(), CCSVFile::modeRead | CFile::typeText | CFile::shareDenyWrite)) {
			CStringArray arr;
			int numberIndex = -1,
				nameIndex = -1,
				passwordIndex = -1,
				domainIndex = -1,
				calledIndex = -1,
				dtmfIndex = -1,
				typeIndex = -1, 
				portIndex = -1, 
				durationIndex = -1, 
				regtimeoutIndex = -1,
				dtmfdurationInfex = -1,
				autoanswerIndex = -1;
			while (CSVFile.ReadData(arr)) {
				if (numberIndex == -1) {
					for (int i = 0; i < arr.GetCount(); i++) {
						CString s = arr.GetAt(i);
						if (numberIndex == -1 && arr.GetAt(i).CompareNoCase(_T("Number")) == 0) {
							numberIndex = i;
						}
						if (nameIndex == -1 && arr.GetAt(i).CompareNoCase(_T("Name")) == 0) {
							nameIndex = i;
						}
						if (nameIndex == -1 && arr.GetAt(i).CompareNoCase(_T("Password")) == 0) {
							nameIndex = i;
						}
						if (passwordIndex == -1 && arr.GetAt(i).CompareNoCase(_T("Domain")) == 0) {
							passwordIndex = i;
						}
						if (calledIndex == -1 && arr.GetAt(i).CompareNoCase(_T("Called")) == 0) {
							calledIndex = i;
						}
						if (dtmfIndex == -1 && arr.GetAt(i).CompareNoCase(_T("DTMF")) == 0) {
							dtmfIndex = i;
						}
						if (typeIndex == -1 && arr.GetAt(i).CompareNoCase(_T("Type")) == 0) {
							typeIndex = i;
						}
						if (portIndex == -1 && arr.GetAt(i).CompareNoCase(_T("Port")) == 0) {
							portIndex = i;
						}
						if (durationIndex == -1 && arr.GetAt(i).CompareNoCase(_T("Duration")) == 0) {
							durationIndex = i;
						}
						if (regtimeoutIndex == -1 && arr.GetAt(i).CompareNoCase(_T("RegTimeout")) == 0) {
							regtimeoutIndex = i;
						}
						if (dtmfdurationInfex == -1 && arr.GetAt(i).CompareNoCase(_T("DTMFDuration")) == 0) {
							dtmfdurationInfex = i;
						}

						if (autoanswerIndex == -1 && arr.GetAt(i).CompareNoCase(_T("AutoAnswer")) == 0) {
							autoanswerIndex = i;
						}
					}
					if (numberIndex == -1
						|| nameIndex == -1 
						|| passwordIndex == -1 
						|| domainIndex == -1 
						|| calledIndex == -1 
						|| typeIndex == -1 
						|| portIndex == -1 
						|| durationIndex == -1 
						|| regtimeoutIndex == -1 
						|| autoanswerIndex == -1
						|| dtmfIndex == -1
						|| dtmfdurationInfex == -1) {
						AfxMessageBox(_T("Unknown format"));
						break;
					}
				}
				else if (arr.GetCount() > numberIndex
					&& arr.GetCount() > nameIndex
					&& arr.GetCount() > passwordIndex
					&& arr.GetCount() > domainIndex
					&& arr.GetCount() > calledIndex
					&& arr.GetCount() > typeIndex
					&& arr.GetCount() > portIndex
					&& arr.GetCount() > durationIndex
					&& arr.GetCount() > regtimeoutIndex
					&& arr.GetCount() > autoanswerIndex
					&& arr.GetCount() > dtmfIndex
					&& arr.GetCount() > dtmfdurationInfex) {
					CString number = arr.GetAt(numberIndex);
					CString name = arr.GetAt(nameIndex);
					CString passwd = arr.GetAt(passwordIndex);
					CString domain = arr.GetAt(domainIndex);
					CString called = arr.GetAt(calledIndex);
					CString type = arr.GetAt(typeIndex);
					CString port = arr.GetAt(portIndex);
					CString duration = arr.GetAt(durationIndex);
					CString regtimeout = arr.GetAt(regtimeoutIndex);
					CString autoanswer = arr.GetAt(autoanswerIndex); 
					CString dtmf = arr.GetAt(dtmfIndex);
					CString dtmfduration = arr.GetAt(dtmfdurationInfex);

					Add(number,name,passwd,domain,called, dtmf,_ttoi(type),_ttoi(port),_ttoi(duration),_ttoi(regtimeout),_ttoi(dtmfduration),_ttoi(autoanswer));
				}
			}
			CSVFile.Close();
		}
	}
}

void Calls::Add(CString number, 
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
	int autoanswer)
{
	if (m_Calls.find(number) != m_Calls.end())
		return;

	//CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	
	Call *pCall = new Call();

	pCall->number = number;
	pCall->name = name;
	pCall->passwd = passwd;
	pCall->domain = domain;
	pCall->called = called;
	pCall->type = type;
	pCall->port = port;
	pCall->callduration = duration;
	pCall->regtimeout = regtimeout;
	pCall->regCode = 0;
	pCall->callStatus = 0;
	pCall->accid = -1;
	pCall->callid = -1;
	pCall->autoanswer = autoanswer;
	pCall->dtmf = dtmf;
	pCall->dtmfduration = dtmfduration;

	if (nextKey >= 10000) {
		nextKey = 0;
	}
	pCall->key = nextKey;
	Insert(pCall,m_Calls.size());
	CallSave(pCall);
	
}

void Calls::Delete(int i)
{
	CListCtrl *list = (CListCtrl*)GetDlgItem(IDC_CALLS);
	Call *pCall = (Call *)list->GetItemData(i);
	if (pCall->callid != PJSUA_INVALID_ID)
	{
		pjsua_call_hangup(pCall->callid, 0, NULL, NULL);
	}

	if (pCall->accid != PJSUA_INVALID_ID)
	{
		pjsua_acc_del(pCall->accid);
	}
	if (m_Calls.find(pCall->number) != m_Calls.end())
	{
		m_Calls.erase(pCall->number);
	}

	for (CallsMapIt it = m_Calls.begin();it !=  m_Calls.end();++it)
	{
		if (it->second.listIndex > i)
		{
			it->second.listIndex--;
		}
	}
	
	pCall->number = _T("");
	CallSave(pCall);
	delete pCall;
	list->DeleteItem(i);
}
//void Calls::SetDuration(pjsua_acc_id id, int sec) {
//	Call* pCall = Get(id);
//	if (pCall) {
//		CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
//		pCall->duration = sec;
//		list->SetItemText(pCall->key,2,GetDuration(pCall->duration));
//		CallSave(pCall);
//	}
//}

void Calls::SetCallRegistered(int id, int code, CString info)
{
	int	iImage = -1;
	CString strStatus = _T("");
	Call* pCall = GetByAccID(id);
	if (pCall) {
		CListCtrl *list = (CListCtrl*)GetDlgItem(IDC_CALLS);
		if (code == 200)
		{
			if (pCall->regCode != code)
			{
				iImage = EU_CALLS_LIST_ICON_ACITVE;
				strStatus = Translate(_T("Registered"));
			}
			else
			{
				return;
			}
		}
		else
		{
			iImage = EU_CALLS_LIST_ICON_MISS;

			strStatus = info;
		}
		pCall->regCode = code;
		list->SetItemText(pCall->listIndex, EU_CALLS_LIST_COLUMN_STATUS, strStatus);
		list->SetItem(pCall->listIndex, 0, LVIF_IMAGE, NULL, iImage, 0, 0, 0);
	}
}

void Calls::SetCallCallStatus(int id, CallStatus* callStatue)
{
	CString strStatus;
	int	iImage = -1;
	Call* pCall = GetByCallID(id);
	if (pCall) {
		CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);

		if (callStatue->operate == EU_CALL_OPR_CALLOUT)
			iImage = EU_CALLS_LIST_ICON_CALLOUT;
		else
			iImage = EU_CALLS_LIST_ICON_CALLIN;

		pCall->callStatus = callStatue->code;
		strStatus = callStatue->info;
		if (pCall->callStatus == PJSIP_INV_STATE_DISCONNECTED)
		{
			pCall->callid = PJSUA_INVALID_ID;
			iImage = EU_CALLS_LIST_ICON_ACITVE;
			if (pCall->recall)
			{
				AddCall(pCall);
				SetEvent();
				pCall->recall = false;
			}
		}
																			
		list->SetItemText(pCall->listIndex, EU_CALLS_LIST_COLUMN_STATUS, strStatus);
		list->SetItem(pCall->listIndex,0, LVIF_IMAGE,NULL, iImage,0,0,0);
	}									 
}

Call* Calls::SetCallCallStatus(int id, int operate, CString caller, CString called, CString info,int code)
{
	CString strStatus;
	int iImage = -1;
	CListCtrl *list = (CListCtrl*)GetDlgItem(IDC_CALLS);
	Call* pCall = GetByCaller(caller);
	if (pCall) {
		pCall->callid = id;
		pCall->called = called;

		if (operate == EU_CALL_OPR_CALLOUT)
			iImage = EU_CALLS_LIST_ICON_CALLOUT;
		else
			iImage = EU_CALLS_LIST_ICON_CALLIN;

		pCall->callStatus = code;
		strStatus = info;
		if (pCall->callStatus == PJSIP_INV_STATE_DISCONNECTED)
		{
			pCall->callid = PJSUA_INVALID_ID;
			iImage = EU_CALLS_LIST_ICON_ACITVE;
			if (pCall->recall)
			{
				AddCall(pCall);
				SetEvent();
				pCall->recall = false;
			}
		}

		list->SetItemText(pCall->listIndex, EU_CALLS_LIST_COLUMN_CALLED, called);
		list->SetItemText(pCall->listIndex, EU_CALLS_LIST_COLUMN_STATUS, strStatus);
		list->SetItem(pCall->listIndex, 0, LVIF_IMAGE, NULL, iImage, 0, 0, 0);
	}

	return pCall;
}

Call* Calls::GetByAccID(int id)
{
	for (CallsMapIt it = m_Calls.begin();it != m_Calls.end();++it)
	{
		if (it->second.accid == id)
		{
			return &(it->second);
		}
	}

	return NULL;
}

Call* Calls::GetByCallID(int id)
{
	for (CallsMapIt it = m_Calls.begin();it != m_Calls.end();++it)
	{
		if (it->second.callid == id)
		{
			return &(it->second);
		}
	}

	return NULL;
}

Call* Calls::GetByCaller(CString caller)
{
	for (CallsMapIt it = m_Calls.begin(); it != m_Calls.end(); ++it)
	{
		if (it->second.name == caller)
		{
			return &(it->second);
		}
	}

	return NULL;
}


void Calls::Insert(Call *pCall, int pos)
{
	if (isFiltered(pCall)) {
		return;
	}
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	CString number;
	CString tmp;
	if (pCall->name != pCall->number) {
		number.AppendFormat(_T("%s (%s)"),pCall->name,pCall->number);
	} else {
		number = pCall->name;
	}
	if (list->InsertItem(LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, pos, number, 0, 0, EU_CALLS_LIST_ICON_MISS, (LPARAM)pCall) != -1)
	{
		list->SetItemText(pos, EU_CALLS_LIST_COLUMN_NAME, pCall->name);
		list->SetItemText(pos, EU_CALLS_LIST_COLUMN_PASSWD, pCall->passwd);
		list->SetItemText(pos, EU_CALLS_LIST_COLUMN_DOMAIN, pCall->domain);
		tmp.Format(_T("%d"), pCall->port);
		list->SetItemText(pos, EU_CALLS_LIST_COLUMN_PORT, tmp);
		list->SetItemText(pos, EU_CALLS_LIST_COLUMN_CALLED, pCall->called);
		list->SetItemText(pos, EU_CALLS_LIST_COLUMN_DTMF, pCall->dtmf);
		tmp.Format(_T("%d"), pCall->dtmfduration);
		list->SetItemText(pos, EU_CALLS_LIST_COLUMN_DTMF_DURATION, tmp);
		tmp.Format(_T("%d"), pCall->callduration);
		list->SetItemText(pos, EU_CALLS_LIST_COLUMN_DURATION, tmp);
		tmp.Format(_T("%d"), pCall->regtimeout);
		list->SetItemText(pos, EU_CALLS_LIST_COLUMN_REGTIMEOUT, tmp);
		//Status.Format(_T("%d"), pCall->Status);
		list->SetItemText(pos, EU_CALLS_LIST_COLUMN_STATUS, Translate(_T("UnRegistered")));
		pCall->listIndex = pos;
		m_Calls.insert(std::make_pair( pCall->number,*pCall));
	}

}

void Calls::CallsClear()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	int count = list->GetItemCount();
	for (int i=0;i<count;i++) {
		Call *pCall = (Call *) list->GetItemData(i);
		delete pCall;
	}
	list->DeleteAllItems();
	m_Calls.clear();
}

void Calls::CallSave(Call *pCall)
{
	CString key;
	// pCall->number == "" means delete
	CString data = !pCall->number.IsEmpty() ? CallEncode(pCall) : _T("null");
	key.Format(_T("%d"), pCall->key);
	WritePrivateProfileString(_T("Calls"), key, data, accountSettings.iniFile);
	if (pCall->key == nextKey) {
		nextKey++;
	}
}

void Calls::CallsLoad()
{
	CString key;
	CString val;
	LPTSTR ptr = val.GetBuffer(255);
	int prevTime = 0;
	int inserted = 0;
	nextKey=0;
	int i=0;
	while (true) {
		key.Format(_T("%d"),i);
		if (GetPrivateProfileString(_T("Calls"), key, NULL, ptr, 256, accountSettings.iniFile)) {
			if (val != _T("null")) {
				Call *pCall =  new Call();
				CallDecode(ptr, pCall);
				bool skip = false;
				if (isFiltered(pCall)) {
					skip = true;
					delete pCall;
				}
				if (!skip) {
					pCall->key = i;
					Insert(pCall, inserted);
					inserted++;
				}
			}
		} else {
			i--;
			break;
		}
		i++;
	}
	
	//m_SortItemsExListCtrl.SortColumn(m_SortItemsExListCtrl.GetSortColumn(),m_SortItemsExListCtrl.IsAscending());
}

CString Calls::CallEncode(Call *pCall)
{
	CString data;
	data.Format(_T("%s;%s;%s;%s;%d;%d;%d;%d;%d;%s;%d;%s"),
		pCall->number, 
		pCall->name, 
		pCall->passwd,
		pCall->domain,
		pCall->port, 
		pCall->type,
		pCall->callduration,
		pCall->regtimeout,
		pCall->autoanswer, 
		pCall->dtmf,
		pCall->dtmfduration,
		pCall->called);
	return data;
}

void Calls::CallDecode(CString str, Call *pCall)
{
	pCall->number=str;

	CString rab;
	int begin;
	int end;
	begin = 0;
	end = str.Find(';', begin);

	if (end != -1)
	{
		pCall->number=str.Mid(begin, end-begin);
		begin = end + 1;
		end = str.Find(';', begin);
		if (end != -1)
		{
			pCall->name=str.Mid(begin, end-begin);
			begin = end + 1;
			end = str.Find(';', begin);
			if (end != -1)
			{
				pCall->passwd= str.Mid(begin, end - begin);
				begin = end + 1;
				end = str.Find(';', begin);
				if (end != -1)
				{
					pCall->domain = str.Mid(begin, end - begin);
					begin = end + 1;
					end = str.Find(';', begin);
					if (end != -1)
					{
						pCall->port = atoi(CStringA(str.Mid(begin, end - begin)));
						begin = end + 1;
						end = str.Find(';', begin);
						if (end != -1)
						{
							pCall->type = atoi(CStringA(str.Mid(begin, end - begin)));
							begin = end + 1;
							end = str.Find(';', begin);
							if (end != -1)							  
							{
								pCall->callduration = atoi(CStringA(str.Mid(begin, end - begin)));
								begin = end + 1;
								end = str.Find(';', begin);
								if (end != -1)
								{
									pCall->regtimeout = atoi(CStringA(str.Mid(begin, end - begin)));
									begin = end + 1;
									end = str.Find(';', begin);
									if (end != -1)
									{
										pCall->autoanswer = atoi(CStringA(str.Mid(begin, end - begin)));
										begin = end + 1;
										end = str.Find(';', begin);
										if (end != -1)
										{
											pCall->dtmf = str.Mid(begin, end - begin);
											begin = end + 1;
											end = str.Find(';', begin);
											if (end != -1)
											{
												pCall->dtmfduration = atoi(CStringA(str.Mid(begin, end - begin)));
												begin = end + 1;
												end = str.Find(';', begin);
												if (end != -1)
												{
													pCall->called = str.Mid(begin, end - begin);
													begin = end + 1;
													end = str.Find(';', begin);
												}
												else
												{

													pCall->called = str.Mid(begin);
												}
											}
											else
											{

												pCall->dtmfduration = atoi(CStringA(str.Mid(begin)));
											}
										}
										else
										{
											pCall->dtmf = str.Mid(begin);
										}
									}
									else
									{
										pCall->autoanswer = atoi(CStringA(str.Mid(begin, end - begin)));
									}
								}
								else
								{
									pCall->regtimeout = atoi(CStringA(str.Mid(begin)));
								}
							}
							else
							{ 
								pCall->callduration = atoi(CStringA(str.Mid(begin)));
							}
						}
						else
						{	 
							pCall->type = atoi(CStringA(str.Mid(begin, end - begin)));
						}
					}
					else
					{			
						pCall->port = atoi(CStringA(str.Mid(begin, end - begin)));
					}
				}
			}
		}
	}
}

void Calls::OnMakeCallSuccess(pjsua_acc_id accid, pjsua_call_id callid)
{
	Call* pCall = GetByAccID(accid);
	pCall ? pCall->callid = callid : NULL;
	time(&(pCall->lastcalltime));
}

/*
CString Calls::GetNameByNumber(CString number)
{
	CString name;
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);

	CString sipURI = GetSIPURI(number);
	int n = list->GetItemCount();
	for (int i=0; i<n; i++) {
		Call* pCall = (Call *) list->GetItemData(i);
		if (GetSIPURI(pCall->number) == sipURI)
		{
			name = pCall->name;
			break;
		}
	}
	return name;
}
*/
