#include "CallTaskThread.h"
#include "settings.h"

int CCallTaskThread::CallTaskWorkThread(void* arg)
{
	CCallTaskThread* pThis = (CCallTaskThread*)arg;
	if (pThis)
	{
		pThis->DoWorkThread();
		pThis->DestroyThread();
	}
   
	return 0;
}

CCallTaskThread::CCallTaskThread()
{
	m_bQuit = FALSE;
	m_CallStack.clear();
}

CCallTaskThread::~CCallTaskThread()
{
	UnInit();
}

bool	CCallTaskThread::Init()
{
	pj_status_t status;
	/*status = pj_thread_init();

	if (status != PJ_SUCCESS)
		return false;*/

	status = pj_mutex_create_simple(pjsua_get_var()->pool, "call task thread", &m_Mutex);
    
	if (status != PJ_SUCCESS)
		return false;

	/*pj_event_create(pjsua_get_var()->pool, "quit thread", PJ_TRUE, PJ_FALSE, &m_QuitEvent);
	if (status != PJ_SUCCESS)
		return false;*/

	pj_event_create(pjsua_get_var()->pool, "call task thread", PJ_TRUE, PJ_FALSE, &m_DoSomeEvent);
	if (status != PJ_SUCCESS)
		return false;
	status = pj_thread_create(pjsua_get_var()->pool, "call task thread", &CallTaskWorkThread, this, 0, 0, &m_pThread);

	m_bInit = (status == PJ_SUCCESS);
	return m_bInit;
}

bool	CCallTaskThread::UnInit()
{
	if (m_bInit)
	{
		m_bQuit = true;
		pj_event_set(m_DoSomeEvent);
		//pj_event_set(m_QuitEvent);
		pj_thread_join(m_pThread);
		pj_thread_destroy(m_pThread);

		m_bInit = false;

	}
	return m_bInit;
}

void	CCallTaskThread::DoWorkThread()
{
	while(!m_bQuit)
	{
		pj_event_wait(m_DoSomeEvent);
		CheckDoCall();
	}

}


void	CCallTaskThread::CheckDoCall()
{
	pj_mutex_lock(m_Mutex);
	if (!m_CallStack.empty())
	{
		CallStackIt  CurCall = m_CallStack.begin();
		Call realCall = *CurCall;
		m_CallStack.erase(CurCall);
		MakeOneCall(realCall);
	}
	else
	{
		pj_event_pulse(m_DoSomeEvent);
	}

	pj_mutex_unlock(m_Mutex);
}

bool	CCallTaskThread::AddCall(Call* pCall)
{
	pj_mutex_lock(m_Mutex);
	m_CallStack.push_back(*pCall);
	pj_mutex_unlock(m_Mutex);
	return true;
}

bool	CCallTaskThread::SetEvent()
{
	return pj_event_set(m_DoSomeEvent) == PJ_SUCCESS;
}

bool	CCallTaskThread::PulseEvent()
{
	return pj_event_pulse(m_DoSomeEvent) == PJ_SUCCESS;
}

bool CCallTaskThread::MakeOneCall(const Call& Call)
{
	pjsua_call_id call_id;

	pjsua_call_setting call_setting;
	pjsua_call_setting_default(&call_setting);
	call_setting.flag = 0;
	call_setting.vid_cnt = 0;

	pjsua_msg_data msg_data;
	pjsua_msg_data_init(&msg_data);
	CString    strURL;
	strURL.Format(_T("sip:%s@%s:%d"), Call.called, Call.domain, Call.port);
	AddTransportSuffix(strURL);
	strURL = _T("<") + strURL + _T(">");

	pj_str_t pj_uri = StrToPjStr(strURL);
	pj_status_t status = pjsua_call_make_call(
		Call.accid,
		&pj_uri,
		&call_setting,
		NULL,
		&msg_data,
		&call_id);
	if (status == PJ_SUCCESS) {
		OnMakeCallSuccess(Call.accid, call_id);
		return true;
	}
	else
	{
		PJ_LOG(1, (THIS_FILE, "account:%s call out to:%s errorcode:%d", Call.number,Call.called,status));
	}

	return false;
}


void CCallTaskThread::DestroyThread()
{
	//pj_event_wait(m_QuitEvent);
	pj_mutex_destroy(m_Mutex);
	pj_event_destroy(m_DoSomeEvent);
	//pj_event_destroy(m_QuitEvent);
}
