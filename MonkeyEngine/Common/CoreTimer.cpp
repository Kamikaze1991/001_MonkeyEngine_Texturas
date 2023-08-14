#include "CoreTimer.h"

CoreTimer::CoreTimer()
{
    mSecondPerCount = 0.0f;
    mDeltaTime = -1.0f;
    mBaseTime = 0;
    mPausedTime = 0;
    mPrevTime = 0;
    mCurrTime = 0;
    mStopped = false;
    mStopTime = 0;

    __int64 countsPerSec;
    QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
    mSecondPerCount = 1.0f / (double)countsPerSec;
}

float CoreTimer::TotalTime() const
{
    if (mStopped) {
        return (float)(((mStopTime - mPausedTime) - mBaseTime) * mSecondPerCount);
    }
    else {
        return (float)(((mCurrTime - mPausedTime) - mBaseTime) * mSecondPerCount);
    }
}

float CoreTimer::DeltatIme() const
{
    return (float)mDeltaTime;
}

void CoreTimer::Reset()
{
    __int64 currTime;
    QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
    mBaseTime = currTime;
    mPrevTime = currTime;
    mStopTime = 0;
    mStopped = false;
}

void CoreTimer::Start()
{
    __int64 startTime;
    QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
    if (mStopped) {
        mPausedTime += (startTime-mStopTime);
        mStopTime = 0;
        mStopped = false;
    }

}

void CoreTimer::Stop()
{
    if (!mStopped) {
        __int64 currTime;
        QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
        mStopTime = currTime;
        mStopped = true;
    }
}

void CoreTimer::Tick()
{
    if (mStopped)
    {
        mDeltaTime = 0.0;
        return;
    }
    __int64 currTime;
    QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
    mCurrTime = currTime;
    mDeltaTime = (mCurrTime - mPrevTime) * mSecondPerCount;
    mPrevTime = mCurrTime;
    if (mDeltaTime < 0.0)
    {
        mDeltaTime = 0.0;
    }
}
