#include "core/Timer.hpp"

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
	if (isStopped()) {
		startTp = clock::now() - stoppedTime;
		stoppedTime = 0ns;
	}
}

void core::Timer::stop()
{
	if (!isStopped()) {
		stoppedTime = clock::now() - startTp;
	}
}

void core::Timer::add(Time time)
{
	// Note you have to do "timePoint - seconds", because you have more time if the timepoint is smaller.
	if (isStopped()) {
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
	if (isStopped())
		return Time(stoppedTime);

	return Time(clock::now() - startTp);
}

bool core::Timer::isStopped() const
{ 
	return stoppedTime > 0ns;
}