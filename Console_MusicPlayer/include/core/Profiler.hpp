#pragma once
#include <chrono>

namespace core
{
	class AutoProfile
	{
	public:
		using clock = std::chrono::high_resolution_clock;

		const char* name;
		clock::time_point startTp;

		AutoProfile(const char* name);
		~AutoProfile();
	};

	/** Writes all stored profiles to an .log file and overwrites its current data. */
	void logProfiles(const char* filepath);
}

/** Use this at the start of the function you want to profile. Each function can only have one function profile. */
#define PROFILE_FUNC core::AutoProfile _autoFuncProfile(__FUNCSIG__);