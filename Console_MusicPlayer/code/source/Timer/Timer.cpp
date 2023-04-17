#include "Timer/Timer.hpp"

namespace core
{
	Timer::Timer() :
		startTp(clock::now()),
		stoppedTime(0ns)
	{

	}

	Timer::~Timer()
	{

	}


	void Timer::stop()
	{
		if (!isStopped()) {
			stoppedTime = clock::now() - startTp;
		}
	}

	void Timer::resume()
	{
		if (isStopped()) {
			startTp = clock::now() - stoppedTime;
			stoppedTime = 0ns;
		}
	}

	Time Timer::getElapsedTime()
	{
		if (isStopped())
			return Time(stoppedTime);
		
		return Time(clock::now() - startTp);
	}

	Time Timer::restart()
	{
		Time time = getElapsedTime();
		startTp = clock::now();
		stoppedTime = 0ns;

		return time;
	}

	void Timer::add(Time time)
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

	bool Timer::isStopped() const
	{ 
		return stoppedTime > 0ns;
	}
}