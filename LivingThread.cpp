#include "LivingThread.hpp"

#include <iostream>
#include <thread>

LivingThread::LivingThread(const char* sName)
{
	m_bQuitThread = true;
	m_bThreadRunning = false;
	m_sName = sName;
}
LivingThread::~LivingThread()
{
	Stop(true);
}


bool LivingThread::Start()
{
	if(m_bQuitThread)
	{
		m_bQuitThread=false;
		m_thread = new std::thread(&ThreadWrapper, this);
	}
	else
	{
		std::cerr<<"Cannot start thread : The thread is already running"<<std::endl;
		return false;
	}
	return true;
}

bool LivingThread::WaitUntilStarted(float fTimeoutS)
{
	struct timespec delay;
	delay.tv_sec = 0;
	delay.tv_nsec = 1000000;//1ms

	int nTimes = fTimeoutS*1000.0;
	for(int i=0 ; i<nTimes ; i++)
	{
		if(m_bThreadRunning == true)
			return true;
		nanosleep(&delay, NULL);
	}
	return false;
}


bool LivingThread::Stop(bool bKill)
{
	if(bKill)
	{
		m_thread->detach();
		delete m_thread;
		OnThreadEnd();
		m_bThreadRunning = false;
		std::clog<<"-> "<<m_sName<<" \e[31m[Killed]\e[m"<<std::endl;
	}
	else
	{
		if(!m_bQuitThread)
		{
			m_bQuitThread=true;
		}
		else
		{
			std::cerr<<"Cannot stop thread "<<m_sName<<" : The thread is already stopping/stopped"<<std::endl;
			return false;
		}
	}
	return true;
}

void LivingThread::ThreadWrapper(void* obj)
{
	LivingThread* ctrl = reinterpret_cast<LivingThread*>(obj);
	ctrl->ThreadFunction();
}

void LivingThread::ThreadFunction()
{
	std::clog<<"-> "<<m_sName<<" \e[32m[Started]\e[m"<<std::endl;
	OnThreadStart();
	m_bThreadRunning = true;
	while(!m_bQuitThread)
	{
		ThreadProcess();
	}
	OnThreadEnd();
	m_bThreadRunning = false;
	std::clog<<"-> "<<m_sName<<" \e[31m[Stopped]\e[m"<<std::endl;
}


