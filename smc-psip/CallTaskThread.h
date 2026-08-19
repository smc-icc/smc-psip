#pragma once

#include <vector>
#include "global.h"


typedef std::vector<Call>					CallStack;
typedef CallStack::const_iterator			CallStackIt;

class CCallTaskThread 
{
public:
	CCallTaskThread();					
	virtual ~CCallTaskThread();
public:
    bool    Init();
	bool	UnInit();

	bool	AddCall(Call* pCall);

	virtual bool	SetEvent();
	virtual bool	PulseEvent();
protected:
	void	CheckDoCall();	   
	void	DoWorkThread();
	virtual void OnMakeCallSuccess(pjsua_acc_id accid, pjsua_call_id callid) {};
	virtual void OnMakeCallFailed(pjsua_acc_id accid) {};
private:
	bool	MakeOneCall(const Call& pCall);
	void	DestroyThread();

	static int CallTaskWorkThread(void* arg);
	
private:
	CallStack			m_CallStack;
	pj_mutex_t*			m_Mutex;
	pj_event_t*			m_DoSomeEvent;
	pj_thread_t*		m_pThread;
	bool				m_bInit;
	volatile bool		m_bQuit;
	//pj_event_t*			m_QuitEvent;
};
