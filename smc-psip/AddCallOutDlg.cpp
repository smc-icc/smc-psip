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

#include "stdafx.h"
#include "smc-psip.h"
#include "AddCallOutDlg.h"
#include "mainDlg.h"
#include "langpack.h"

AddCallOutDlg::AddCallOutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AddCallOutDlg::IDD, pParent)
{
	Create (IDD, pParent);
}

AddCallOutDlg::~AddCallOutDlg()
{
}

int AddCallOutDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl) {
		ModifyStyleEx(0,WS_EX_LAYOUTRTL);
	}
	return 0;
}

BOOL AddCallOutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	listIndex = -1;
	GetDlgItem(IDC_EDIT_CALLOUT_NUMBER)->SetWindowText(_T("990001"));
	GetDlgItem(IDC_EDIT_CALLOUT_NAME)->SetWindowText(_T("990001"));
	GetDlgItem(IDC_EDIT_CALLOUT_PASSWD)->SetWindowText(_T("123456"));
	GetDlgItem(IDC_EDIT_CALLOUT_DOMAIN)->SetWindowText(_T("192.168.2.100"));
	GetDlgItem(IDC_EDIT_CALLOUT_PORT)->SetWindowText(_T("5060"));
	GetDlgItem(IDC_EDIT_CALLOUT_CALLED)->SetWindowText(_T("990000"));
	GetDlgItem(IDC_EDIT_CALLOUT_COUNT)->SetWindowText(_T("10"));
	GetDlgItem(IDC_EDIT_CALLOUT_DURATION)->SetWindowText(_T("300"));
	GetDlgItem(IDC_EDIT_CALLOUT_REGTIMEOUT)->SetWindowText(_T("3600"));
	((CButton *)GetDlgItem(IDC_CALLOUT_NUMBER_INCREASE))->SetCheck(1);
	GetDlgItem(IDC_EDIT_CALLOUT_NUMBER)->SetFocus();

	TranslateDialog(this->m_hWnd);
	return TRUE;
}

void AddCallOutDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}


BEGIN_MESSAGE_MAP(AddCallOutDlg, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &AddCallOutDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &AddCallOutDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


void AddCallOutDlg::OnClose()
{
	this->ShowWindow(SW_HIDE);
}

void AddCallOutDlg::OnBnClickedOk()
{
	CString number;
	CString name;
	CString passwd;
	CString domain;
	CString port;
	CString called;
	CString count;
	CString duration;
	CString regtimeout;
	CString dtmf;
	CString dtmfduration;
	BOOL numberIncrease;
	BOOL calledIncrease;

			
	GetDlgItem(IDC_EDIT_CALLOUT_NUMBER)->GetWindowText(number);
	GetDlgItem(IDC_EDIT_CALLOUT_NAME)->GetWindowText(name);
	GetDlgItem(IDC_EDIT_CALLOUT_PASSWD)->GetWindowText(passwd);
	GetDlgItem(IDC_EDIT_CALLOUT_DOMAIN)->GetWindowText(domain);
	GetDlgItem(IDC_EDIT_CALLOUT_PORT)->GetWindowText(port);
	GetDlgItem(IDC_EDIT_CALLOUT_CALLED)->GetWindowText(called);
	GetDlgItem(IDC_EDIT_CALLOUT_COUNT)->GetWindowText(count);
	GetDlgItem(IDC_EDIT_CALLOUT_DURATION)->GetWindowText(duration);
	GetDlgItem(IDC_EDIT_CALLOUT_REGTIMEOUT)->GetWindowText(regtimeout);
	GetDlgItem(IDC_EDIT_CALLOUT_DTMF)->GetWindowText(dtmf);
	GetDlgItem(IDC_EDIT_CALLOUT_DTMF_DURATION)->GetWindowText(dtmfduration);
	numberIncrease = ((CButton*)GetDlgItem(IDC_CALLOUT_NUMBER_INCREASE))->GetCheck();
	calledIncrease = ((CButton*)GetDlgItem(IDC_CALLOUT_CALLED_INCREASE))->GetCheck();


	number.Trim();
	if (number.IsEmpty() || !IsNumber((const char*)number.GetString()))
	{
		AfxMessageBox(_T("number is empty or number is not number!"));
		return;
	}

	name.Trim();
	if (name.IsEmpty() || !IsNumber((const char*)name.GetString()))
	{
		AfxMessageBox(_T("name is empty or name is not number!"));
		return;
	}

	passwd.Trim();
	if (passwd.IsEmpty())
	{
		AfxMessageBox(_T("passwd is empty!"));
		return;
	}

	domain.Trim();
	if (domain.IsEmpty())
	{
		AfxMessageBox(_T("domain is empty!"));
		return;
	}

	port.Trim();
	if (port.IsEmpty() || !IsNumber((const char*)port.GetString()))
	{
		AfxMessageBox(_T("port is empty or port is not number!"));
		return;
	}

	called.Trim();
	if (called.IsEmpty())
	{
		AfxMessageBox(_T("called is empty!"));
		return;
	}

	dtmf.Trim();

	count.Trim();
	if (count.IsEmpty() || !IsNumber((const char*)count.GetString()))
	{
		AfxMessageBox(_T("count is empty or count is not number!"));
		return;
	}

	duration.Trim();
	if (duration.IsEmpty() || !IsNumber((const char*)duration.GetString()))
	{
		AfxMessageBox(_T("period is empty or period is not number!"));
		return;
	}  

	regtimeout.Trim();
	if (regtimeout.IsEmpty() || !IsNumber((const char*)regtimeout.GetString()))
	{
		AfxMessageBox(_T("period is empty or period is not number!"));
		return;
	}

	dtmfduration.Trim();
	if (!dtmfduration.IsEmpty() && !IsNumber((const char*)dtmfduration.GetString()))
	{
		AfxMessageBox(_T("dtmfduration is not number!"));
		return;
	}

	for (int i = 0; i < _ttoi(count);i++)
	{
		mainDlg->GetCalls() ? mainDlg->GetCalls()->Add(number,
			name,
			passwd, 
			domain, 
			called,
			dtmf,
			EU_CALL_CALLOUT,
			_ttoi(port),
			_ttoi(duration),
			_ttoi(regtimeout),
			_ttoi(dtmfduration),
			0) : NULL;
		if (numberIncrease)
		{
			number.Format(_T("%d"), _ttoi(number) + 1);
			name.Format(_T("%d"), _ttoi(name) + 1);
		}

		if (calledIncrease)
		{
			called.Format(_T("%d"), _ttoi(called) + 1);
		}
	}		

	mainDlg->PJAccountAdd();

	OnClose();

}

void AddCallOutDlg::OnBnClickedCancel()
{
	OnClose();
}
