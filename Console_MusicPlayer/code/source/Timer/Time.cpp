#include "Timer/Time.hpp"

namespace core
{
	Time::Time() :
		time(0ns)
	{

	}

	Time::Time(Duration time) :
		time(time)
	{

	}

	Time::~Time()
	{

	}

	long double Time::asHour() const
	{
		return static_cast<long double>(time.count()) / 1e9 / 60.0 / 60.0;
	}

	long double Time::asMinute() const
	{
		return static_cast<long double>(time.count()) / 1e9 / 60.f;
	}

	long double Time::asSeconds() const
	{
		return static_cast<long double>(time.count()) / 1e9;
	}
	long double Time::asMilliseconds() const
	{
		return static_cast<long double>(time.count()) / 1e6;
	}

	long double Time::asMicroSeconds() const
	{
		// Example: 500ns = 0.5ms
		return static_cast<long double>(time.count()) / 1e3;
	}

	long long Time::asNanoSeconds() const
	{
		return time.count();
	}

	Time::Duration Time::get() const
	{
		return time;
	}

	Time& Time::operator+=(const Time& rhs)
	{
		time += rhs.get();
		return *this;
	}

	Time& Time::operator-=(const Time& rhs)
	{
		time -= rhs.get();
		return *this;
	}

	Time& Time::operator+=(const Duration& rhs)
	{
		time += rhs;
		return *this;
	}

	Time& Time::operator-=(const Duration& rhs)
	{
		time -= rhs;
		return *this;
	}

	Time& Time::operator=(Time rhs)
	{
		std::swap(time, rhs.time);
		return *this;
	}

	Time& Time::operator=(Duration rhs)
	{
		std::swap(time, rhs);
		return *this;
	}
}