#pragma once
#include <vector>
#include "Time.hpp"

namespace core
{
	class Timer
	{
	public:
		using clock = std::chrono::high_resolution_clock;
		using Duration = std::chrono::duration<long long, std::nano>;

		explicit Timer();
		explicit Timer(const Timer&) = delete;
		Timer(Timer&&) = delete;
		~Timer();

		/** Restart the timer. This is the default behaviour when an Timer is created. */
		Time restart();
		/** Resume paused timer. */
		void resume();
		/** Pause timer, so that its elapsed time stored and it can be resumed. */
		void pause();
		/** Stop timer: Time is 0ns - use restart() to start timer. */
		void stop();
		/** Has no effect on stopped timer. */
		void add(Time time);
		Time getElapsedTime() const;
		bool isPaused() const;
		bool isStopped() const;
	private:
		Duration          stoppedTime;
		clock::time_point startTp;
	};
}