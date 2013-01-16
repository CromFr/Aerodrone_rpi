#include "MotorHdl.hpp"

#include "Motor.hpp"
#include "ConfigFile.hpp"


//==========================================================================
MotorHdl::MotorHdl(const ConfigFile* cfg)
{
    for(int i=0 ; i<4 ; i++)
        m_mot[i] = new Motor(cfg->GetValue<int>("PIN_Motors", i));


    //The PWM freq and the time between two times the pin is set to 0
    m_fPWMFreq = cfg->GetValue<float>("MOT_PwmFreq");
    m_nDelayPwmUS = (1.f/m_fPWMFreq)*1000000;

    //Inits the time between two checks to change the pin value to 1
    m_PWMCheckSleep.tv_sec=0;
    m_PWMCheckSleep.tv_nsec=cfg->GetValue<float>("MOT_PwmPrecision") * 100000;
}
MotorHdl::~MotorHdl()
{
    for(int i=0 ; i<4 ; i++)
        delete m_mot[i];
}



void MotorHdl::ThreadProcess()
{

    struct timeval begin, current;

    //Init pins to LO
    for(short i=0 ; i<4 ; i++)
        m_mot[i]->SetPin(false);

    //Start timer
    gettimeofday(&begin, NULL);


    //Calculating delays for the motors
    int fDelayMotUS[4];
    for(short i=0 ; i<4 ; i++)
    {
		fDelayMotUS[i] = m_nDelayPwmUS*( (50+(m_mot[i]->GetSpeed()/2.0))/100.0  ); //NOTE m_fMotorMinSpeed not used
    }


    //Processing pwm
    bool bMot[4] = {true};
    int nMot = 0;
    int nElapsedTimeUS;
    do
    {
        gettimeofday(&current, NULL);

        //Calculate the elapsed time since the pins were set to 0
        if(current.tv_usec < begin.tv_usec)current.tv_usec+=1000000;
        nElapsedTimeUS = current.tv_usec - begin.tv_usec;

        //Change the pin value to 1 when its time has come
        for(short i=0 ; i<4 && nMot<4 ; i++)
        {
            if(bMot[i] && nElapsedTimeUS >= fDelayMotUS[i])
            {
                m_mot[i]->SetPin(true);
            	nMot++;
                bMot[i] = false;
            }
        }
        nanosleep(&m_PWMCheckSleep, NULL);
    }while(nElapsedTimeUS <= m_nDelayPwmUS);



}


void MotorHdl::OnThreadEnd()
{
    for(short i=0 ; i<4 ; i++)
        m_mot[i]->SetPin(false);
}


void MotorHdl::SetSpeed(int motID, float fPercent)
{
    m_mot[motID-1]->SetSpeed(fPercent);
}

float MotorHdl::GetSpeed(int motID)
{
    return m_mot[motID-1]->GetSpeed();
}
