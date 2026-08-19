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
#include "AddSipAgentDlg.h"
#include "mainDlg.h"
#include "langpack.h"

AddSipAgentDlg::AddSipAgentDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AddSipAgentDlg::IDD, pParent)
{
	Create (IDD, pParent);
}

AddSipAgentDlg::~AddSipAgentDlg()
{
}

int AddSipAgentDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl) {
		ModifyStyleEx(0,WS_EX_LAYOUTRTL);
	}
	return 0;
}

BOOL AddSipAgentDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	listIndex = -1;
	GetDlgItem(IDC_EDIT_SIPAGENT_NUMBER)->SetWindowText(_T("990001"));
	GetDlgItem(IDC_EDIT_SIPAGENT_NAME)->SetWindowText(_T("990001"));
	GetDlgItem(IDC_EDIT_SIPAGENT_PASSWD)->SetWindowText(_T("123456"));
	GetDlgItem(IDC_EDIT_SIPAGENT_DOMAIN)->SetWindowText(_T("192.168.2.100"));
	GetDlgItem(IDC_EDIT_SIPAGENT_PORT)->SetWindowText(_T("5060"));
	GetDlgItem(IDC_EDIT_SIPAGENT_COUNT)->SetWindowText(_T("10"));
	GetDlgItem(IDC_EDIT_SIPAGENT_TIMEOUT)->SetWindowText(_T("3600"));
	((CButton *)GetDlgItem(IDC_SIPAGENT_AUTOANSWER))->SetCheck(1);
													
	GetDlgItem(IDC_EDIT_SIPAGENT_NUMBER)->SetFocus();
	TranslateDialog(this->m_hWnd);					  
	return TRUE;
}

void AddSipAgentDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}


BEGIN_MESSAGE_MAP(AddSipAgentDlg, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &AddSipAgentDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &AddSipAgentDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


void AddSipAgentDlg::OnClose()
{
	this->ShowWindow(SW_HIDE);
}

void AddSipAgentDlg::OnBnClickedOk()
{

	BOOL autoAnswer;
	CString number;
	CString name;
	CString passwd;
	CString domain;
	CString port;
	CString count;
	CString regtimeout;

	GetDlgItem(IDC_EDIT_SIPAGENT_NUMBER)->GetWindowText(number);
	GetDlgItem(IDC_EDIT_SIPAGENT_NAME)->GetWindowText(name);
	GetDlgItem(IDC_EDIT_SIPAGENT_PASSWD)->GetWindowText(passwd);
	GetDlgItem(IDC_EDIT_SIPAGENT_DOMAIN)->GetWindowText(domain);
	GetDlgItem(IDC_EDIT_SIPAGENT_PORT)->GetWindowText(port);
	GetDlgItem(IDC_EDIT_SIPAGENT_TIMEOUT)->GetWindowText(regtimeout);
	GetDlgItem(IDC_EDIT_SIPAGENT_COUNT)->GetWindowText(count);
	autoAnswer = ((CButton*)GetDlgItem(IDC_SIPAGENT_AUTOANSWER))->GetCheck();

	number.Trim();
	if (number.IsEmpty() && !IsNumber((const char*)number.GetString()))
	{
		AfxMessageBox(_T("number is empty or number is not number!"));
		return;
	}

	name.Trim();
	if (name.IsEmpty() && !IsNumber((const char*)name.GetString()))
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
	if (port.IsEmpty() && !IsNumber((const char*)port.GetString()))
	{
		AfxMessageBox(_T("port is empty or port is not number!"));
		return;
	}

	count.Trim();
	if (count.IsEmpty() && !IsNumber((const char*)count.GetString()))
	{
		AfxMessageBox(_T("count is empty or count is not number!"));
		return;
	}

	regtimeout.Trim();
	if (regtimeout.IsEmpty() && !IsNumber((const char*)regtimeout.GetString()))
	{
		AfxMessageBox(_T("regtimeout is empty or period is not number!"));
		return;
	}

	regtimeout.Trim();
	if (regtimeout.IsEmpty() && !IsNumber((const char*)regtimeout.GetString()))
	{
		AfxMessageBox(_T("period is empty or period is not number!"));
		return;
	}

	for (int i = 0; i < _ttoi(count); i++)
	{
		mainDlg->GetCalls() ? mainDlg->GetCalls()->Add(number, 
			name, 
			passwd, 
			domain,
			_T(""), 
			_T(""),
			EU_CALL_SIPAGENT, 
			_ttoi(port),
			0, 
			_ttoi(regtimeout), 
			0,
			autoAnswer) : NULL;
		number.Format(_T("%d"), _ttoi(number) + 1);
		name.Format(_T("%d"), _ttoi(name) + 1);
	}

	mainDlg->PJAccountAdd();

	OnClose();
}

void AddSipAgentDlg::OnBnClickedCancel()
{
	OnClose();
}
