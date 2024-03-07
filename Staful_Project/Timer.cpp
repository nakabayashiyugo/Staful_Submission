#include "Timer.h"
#include "Windows.h"
#include "Engine/Image.h"

Timer::Timer(int _limitTime):
	count_time_(0),
	current_time_(0),
	limit_time_(_limitTime),
	isTimeUpped_(false)
{
}

void Timer::Initialize()
{
}

void Timer::Update()
{
	count_time_++;
	current_time_ = count_time_ / FPS;

	if (count_time_ % FPS == 0)
	{
		std::string resStr = std::to_string(limit_time_ - current_time_) + '\n';
		OutputDebugString(resStr.c_str());
	}

	if (current_time_ == limit_time_)
	{
		isTimeUpped_ = true;
	}
}