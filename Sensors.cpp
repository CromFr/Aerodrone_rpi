#include "Sensors.hpp"

#include <WiringPi.h>
#include "ConfigFile.hpp"


Sensors::Sensors(const ConfigFile* cfg)
{
    //ADC Pins
    for(int i=0 ; i<8 ; i++)
    {
        m_nDataPins[i] = cfg->GetValue<int>("PIN_ADC", i);
        pinMode(m_nSelectPins[i], INPUT);
    }

    //Selector pins
    m_nSelectPins[0] = cfg->GetValue<int>("PIN_Selector", 0);
    m_nSelectPins[1] = cfg->GetValue<int>("PIN_Selector", 1);
    pinMode(m_nSelectPins[0], OUTPUT);
    pinMode(m_nSelectPins[0], OUTPUT);
    digitalWrite(m_nSelectPins[0], false);
    digitalWrite(m_nSelectPins[1], false);
    m_nCurrentSelectPinsState[0] = false;
    m_nCurrentSelectPinsState[1] = false;

    selectionDelay.tv_sec=0; selectionDelay.tv_nsec=cfg->GetValue<float>("SEN_SelectionDelay")*1000.0;
}

float Sensors::GetAcceleroX()
{
    ChangeSelection(0,0);
    m_fAccelX = GetPinsValue()/100.0;//TODO Convert byte to acceleration in m.s-2
    return m_fAccelX;
}


float Sensors::GetAcceleroY()
{
    ChangeSelection(0,1);
    m_fAccelY = GetPinsValue()/100.0;//TODO Convert byte to acceleration in m.s-2
    return m_fAccelY;
}


float Sensors::GetAcceleroZ()
{
    ChangeSelection(1,0);
    m_fAccelZ = GetPinsValue()/100.0;//TODO Convert byte to acceleration in m.s-2
    return m_fAccelZ;
}


float Sensors::GetGyro()
{
    ChangeSelection(1,1);
    m_fRotSpeed = GetPinsValue()/100.0;//TODO Convert byte to rot speed in m.s-1
    return m_fRotSpeed;
}





void Sensors::ChangeSelection(bool b1, bool b0)
{
    if(!(m_nCurrentSelectPinsState[0] == 0 && m_nCurrentSelectPinsState[1] == 0))
    {
        digitalWrite(m_nSelectPins[0], b0);
        digitalWrite(m_nSelectPins[1], b1);
        nanosleep(&selectionDelay, NULL);
    }
}

short Sensors::GetPinsValue()
{
    short ret=0;
    for(short i=0, coeff=1 ; i<8 ; i++, coeff*=2)
        ret += coeff*digitalRead(m_nSelectPins[i]);
    return ret;
}