#ifndef _CORE_TIMER_H_
#define _CORE_TIMER_H_
#include "CoreUtil.h"

class CoreTimer {
private:
	double mSecondPerCount;
	double mDeltaTime;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;

	bool mStopped;

public:
	CoreTimer();

	float TotalTime()const;
	float DeltatIme()const;

	void Reset();
	void Start();
	void Stop();
	void Tick();
};
#endif

