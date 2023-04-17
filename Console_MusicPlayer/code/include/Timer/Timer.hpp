#pragma once
#include <vector>
#include "Timer/Time.hpp"

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

		void stop();
		void resume();
		Time getElapsedTime();
		// The clock will resume if it was stopped.
		Time restart();
		void add(Time time);

		bool isStopped() const;
	private:
		Duration          stoppedTime;
		clock::time_point startTp;
	};
}