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

		// The clock will resume if it was stopped.
		Time restart();
		void resume();
		void stop();
		void add(Time time);
		Time getElapsedTime() const;
		bool isStopped() const;
	private:
		Duration          stoppedTime;
		clock::time_point startTp;
	};
}