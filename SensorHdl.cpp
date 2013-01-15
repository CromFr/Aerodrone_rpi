#include "SensorHdl.hpp"

#include "ConfigFile.hpp"
#include "Sensor.hpp"
#include "timetools.h"


SensorHdl::SensorHdl(const ConfigFile* cfg) : m_fPos(0,0,0), m_fSpeed(0,0,0), m_fAccel(0,0,0)
{
    m_sen = new Sensor(cfg);
    ResetPosition();
    ResetSpeed();

    m_IntegSleepTime.tv_sec=0;
    m_IntegSleepTime.tv_nsec = cfg->GetValue<float>("SEN_IntegrationDelay")*1000000.0;
}

void SensorHdl::ResetPosition()
{
    m_fPos.Set(0,0,0);
    m_fPosRot=0;
}

void SensorHdl::ResetSpeed()
{
    m_fSpeed.Set(0,0,0);
    m_fSpeedRot=0;

    m_vGravityAccel = Vector3D<double>(m_sen->GetAcceleroX(), m_sen->GetAcceleroY(), m_sen->GetAcceleroZ());
}

Vector3D<double> SensorHdl::GetPosition()
{
    return m_fPos;
}

double SensorHdl::GetAngularPosition()
{
    return m_fPosRot;
}

Vector3D<double> SensorHdl::GetSpeed()
{
    return m_fSpeed;
}

double SensorHdl::GetAngularSpeed()
{
    return m_fPosRot;
}

Vector3D<double> SensorHdl::GetAcceleration()
{
    return m_fAccel;
}

Vector3D<double> SensorHdl::GetGravityAcceleration()
{
    return m_vGravityAccel;
}




int SensorHdl::GetElapsedTimeUSSince(struct timeval* begin)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    now = (now - *begin);
    return now.tv_sec*1000000 + now.tv_usec;
}


void SensorHdl::OnThreadStart()
{
    gettimeofday(&m_dateAccelX, NULL);
    m_dateAccelY = m_dateAccelX; m_dateAccelZ = m_dateAccelX; m_dateGyro = m_dateAccelX;
}

void SensorHdl::ThreadProcess()
{
    std::cout<<"I";
    m_fAccel.x = m_sen->GetAcceleroX();
    int nElapsedAX = GetElapsedTimeUSSince(&m_dateAccelX);
    gettimeofday(&m_dateAccelX, NULL);

    m_fAccel.y = m_sen->GetAcceleroY();
    int nElapsedAY = GetElapsedTimeUSSince(&m_dateAccelY);
    gettimeofday(&m_dateAccelY, NULL);

    m_fAccel.z = m_sen->GetAcceleroZ();
    int nElapsedAZ = GetElapsedTimeUSSince(&m_dateAccelZ);
    gettimeofday(&m_dateAccelZ, NULL);

    m_fSpeedRot = m_sen->GetGyro();
    int nElapsedSR = GetElapsedTimeUSSince(&m_dateGyro);
    gettimeofday(&m_dateGyro, NULL);


    m_fSpeed.x += (m_fAccel.x-m_vGravityAccel.x)*nElapsedAX/1000.f;
    m_fSpeed.y += (m_fAccel.y-m_vGravityAccel.y)*nElapsedAY/1000.f;
    m_fSpeed.z += (m_fAccel.z-m_vGravityAccel.z)*nElapsedAZ/1000.f;

    m_fPos.x += m_fSpeed.x*nElapsedAX/1000.f;
    m_fPos.y += m_fSpeed.y*nElapsedAY/1000.f;
    m_fPos.z += m_fSpeed.z*nElapsedAZ/1000.f;
    m_fPosRot += m_fSpeedRot*nElapsedSR/1000.f;

    nanosleep(&m_IntegSleepTime, NULL);
}
