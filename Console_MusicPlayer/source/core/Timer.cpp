#include "core/Timer.hpp"

static const core::Timer::Duration STOP_DURATION = -1s;

core::Timer::Timer() :
	startTp(clock::now()),
	stoppedTime(0ns)
{

}

core::Timer::~Timer()
{

}

core::Time core::Timer::restart()
{
	Time time = getElapsedTime();
	startTp = clock::now();
	stoppedTime = 0ns;

	return time;
}

void core::Timer::resume()
{
	if (isPaused()) {
		startTp = clock::now() - stoppedTime;
		stoppedTime = 0ns;
	}
}

void core::Timer::pause()
{
	if (!isPaused() && !isStopped()) {
		stoppedTime = clock::now() - startTp;
	}
}

void core::Timer::stop()
{
	stoppedTime = STOP_DURATION;
}

void core::Timer::add(Time time)
{
	if (isStopped()) {
		return;
	}

	// Note you have to do "timePoint - seconds", because you have more time if the timepoint is smaller.
	if (isPaused()) {
		if (time.asNanoSeconds() < 0 && stoppedTime < Nanoseconds(abs(time.asNanoSeconds()))) {
			stoppedTime = 0ns;
		}
		else {
			stoppedTime -= time.get();
		}
	}
	else {
		startTp -= time.get();
	}
}

core::Time core::Timer::getElapsedTime() const
{
	if (isStopped()) return 0ns;
	if (isPaused())   return Time(stoppedTime);

	return Time(clock::now() - startTp);
}

bool core::Timer::isPaused() const
{ 
	return stoppedTime > 0ns;
}

bool core::Timer::isStopped() const
{
	return stoppedTime == STOP_DURATION;
}