#ifndef THREADCTRL_HPP_INCLUDED
#define THREADCTRL_HPP_INCLUDED

#include <iostream>
#include <thread>
#include <signal.h>

class ThreadCtrl
{
public:
    ThreadCtrl()
    {
        m_bQuitThread = true;
        m_bThreadRunning = false;
    }

    virtual ~ThreadCtrl()
    {
        Stop(true);
    }

	/**
	@brief Starts the thread
	**/
	short Start(bool bKill=false)
	{
		if(m_bQuitThread)
		{
			m_bQuitThread=false;
			//m_thread = new std::thread(ThreadWrapper, this);//TODO uncomment
			m_thread = new std::thread(ThreadCtrl::ThreadWrapper, this);//TODO uncomment
		}
		else
		{
			std::cerr<<"Cannot start thread : The thread is already running"<<std::endl;
			return -1;
		}
		return 1;
	}

	/**
	@brief Stops the thread
	**/
	short Stop(bool bKill=false)
	{
	    if(bKill)
	    {
            m_thread->detach();
            delete m_thread;
	    }
	    else
	    {
            if(!m_bQuitThread)
            {
                m_bQuitThread=true;
            }
            else
            {
                std::cerr<<"Cannot stop thread : The thread is already stopping/stopped"<<std::endl;
                return -1;
            }
	    }
		return 1;
	}

	bool GetIsRunning(){return m_bThreadRunning;}

protected:
	virtual void OnThreadStart(){};
	virtual void ThreadProcess()=0;
	virtual void PostThreadProcess(){}
	virtual void OnThreadEnd(){};

	bool GetIsThreadQuitting(){return m_bQuitThread;}

private:
	static void* ThreadWrapper(void* obj)
	{
		ThreadCtrl* ctrl = reinterpret_cast<ThreadCtrl*>(obj);
		ctrl->ThreadFunction();
		return 0;
	}

	void ThreadFunction()
	{
	    m_bThreadRunning = true;
		OnThreadStart();
		while(!m_bQuitThread)
		{
			ThreadProcess();
			PostThreadProcess();
		}
		OnThreadEnd();
		m_bThreadRunning = false;
	}

    bool m_bThreadRunning;
	bool m_bQuitThread;
	std::thread* m_thread;


};

#endif // THREADCTRL_HPP_INCLUDED
