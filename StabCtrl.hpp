#ifndef STABCTRL_INCLUDED
#define STABCTRL_INCLUDED

#include "ThreadCtrl.hpp"

class Device;
class ConfigFile;

class StabCtrl : public ThreadCtrl
{
public:
    StabCtrl(ConfigFile* cfg);

	/**
	@brief Land the engine within max fMaxSec seconds and stops the motors
	**/
	void LandRoutine(float fMaxSec);

    /**
    @brief Change the motors speed by adding fAdd to the current speed
    **/
	void ChangeMotorSpeed(float fAdd);


private:

	void ThreadProcess();


	float m_fGlobalMotorSpeed;
	float m_fZRotCompensation;

	float m_fSensibility;
	float m_fRotSensibility;

	struct timespec m_sleepTime;
};

#endif // STABCTRL_INCLUDED
