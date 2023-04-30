#include "core/Time.hpp"

core::Time::Time() :
	time(0ns)
{

}

core::Time::Time(Duration time) :
	time(time)
{

}

core::Time::~Time()
{

}

long double core::Time::asHours() const
{
	return static_cast<long double>(time.count()) / 1e9 / 60.0 / 60.0;
}

long double core::Time::asMinutes() const
{
	return static_cast<long double>(time.count()) / 1e9 / 60.f;
}

long double core::Time::asSeconds() const
{
	return static_cast<long double>(time.count()) / 1e9;
}
long double core::Time::asMilliseconds() const
{
	return static_cast<long double>(time.count()) / 1e6;
}

long double core::Time::asMicroSeconds() const
{
	// Example: 500ns = 0.5ms
	return static_cast<long double>(time.count()) / 1e3;
}

long long core::Time::asNanoSeconds() const
{
	return time.count();
}

core::Time::Duration core::Time::get() const
{
	return time;
}

core::Time& core::Time::operator+=(const Time& rhs)
{
	time += rhs.get();
	return *this;
}

core::Time& core::Time::operator-=(const Time& rhs)
{
	time -= rhs.get();
	return *this;
}

core::Time& core::Time::operator+=(const Duration& rhs)
{
	time += rhs;
	return *this;
}

core::Time& core::Time::operator-=(const Duration& rhs)
{
	time -= rhs;
	return *this;
}

core::Time& core::Time::operator=(Time rhs)
{
	std::swap(time, rhs.time);
	return *this;
}

core::Time& core::Time::operator=(Duration rhs)
{
	std::swap(time, rhs);
	return *this;
}