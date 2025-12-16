#pragma once

class TimeStep
{
public:
    constexpr TimeStep(const float time = 0.0f) :
        mTime(time) {}

    constexpr operator float() const { return mTime; }
    constexpr float GetSeconds() const { return mTime; }
    constexpr float GetMilliseconds() const { return mTime * 1000.0f; }

private:
    float mTime;
};
