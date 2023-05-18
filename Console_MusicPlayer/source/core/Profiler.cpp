#include "core/Profiler.hpp"
#include <map>
#include <fstream>

using namespace std::chrono_literals;
using Duration = std::chrono::duration<long long, std::nano>;

struct Profile
{
	Duration  min{ 0s };
	Duration  max{ 0s };
	long long sampleCount{ 0 };
};

static struct cmp_str {
	bool operator()(char const* a, char const* b) const {
		return std::strcmp(a, b) < 0;
	}
};

static std::map<const char*, Profile, cmp_str> profiles;

core::AutoProfile::AutoProfile(const char* name) :
	name(name),
	startTp(clock::now())
{

}

core::AutoProfile::~AutoProfile()
{
	clock::duration elapsedTime = clock::now() - startTp;
	if (profiles.count(name)) {
		profiles[name].min = profiles[name].min > elapsedTime ? elapsedTime : profiles[name].min;
		profiles[name].max = profiles[name].max < elapsedTime ? elapsedTime : profiles[name].max;
	}
	else {
		profiles[name] = { elapsedTime, elapsedTime };
	}
	++profiles[name].sampleCount;
}

void core::logProfiles(const char* filepath)
{
	std::ofstream ofs(filepath, std::ios::out);
	for (auto& [name, value] : profiles) {
		ofs << name << ": " << (int)(value.min.count() / 1e6) << " - " 
			<< (int)(value.max.count() / 1e6) 
			<< " ms (samples: " << value.sampleCount << ")\n";
	}
	ofs.close();
}