#pragma once
#include <chrono>

using namespace std::chrono_literals;

namespace core
{
	using Nanoseconds  = std::chrono::nanoseconds; //< currently: std::chrono::duration<long long, std::nano>
	using Microseconds = std::chrono::microseconds; //< currently: std::chrono::duration<long long, std::micro>
	using Milliseconds = std::chrono::milliseconds; //< currently: std::chrono::duration<long long, std::milli>
	using Seconds      = std::chrono::seconds; //< currently: std::chrono::duration<long long>
	using Minutes      = std::chrono::minutes; //< currently: std::chrono::duration<int, std::ratio<60i64>>
	using Hours        = std::chrono::hours; //< currently: std::chrono::duration<int, std::ratio<3600i64>>

	/**
     * Type to store time:
     * The datatype long can display seconds as nanoseconds:
     * long = 2'147'483'647
     * 1s = 10^3ms = 10^6qs = 10^9ns
     * The datatype long long can display a whole week as nanoseconds:
     * long long = 9,223,372,036,854,775,807 (~10^18)
     * 1w = 7d = 168h = 10080min = 604800s = 0,6048*10^15ns
     * - long long vs long double:
     * long double is not precise when using large values. For example, if long double has a large
     * value, then either d + 1 == d or d - 1 == d can be true. Whereas integer are always precise.
     * Though long double can store larger values than long long.
     *
     * Chrono duration:
     * By default long long is used as type, which means that no floating point values can be stored and therefore you
     * can not convert from an more precise to less precise type (0.5min=30sec).
     * - Less precise to more precise type:
     * std::chrono::hours hour = std::chrono::hours(1); // or 1h
     * std::chrono::seconds seconds = std::chrono::seconds(hour);
     * std::cout << hours.count() << "h = " << second.count() << "s\n"; // 1h = 3600s
     * - More precise to less precise type:
     * This is possible if your less precise type can hold floating point values:
     * std::chrono::seconds second = std::chrono::seconds(1);
     * auto hours = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(second);
     * std::cout << hours.count() << "h = " << second.count() << "s\n"; // 0.000277778h = 1s
     * Note std::ratio is explained below - basically 3600 because 3600 seconds are 1 hour. And note that
     * std::chrono::hours can not be used instead of std::ratio<3600> - they are two different things.
     * - Structure:
     * A duration has this structure: std::chrono::duration<type, period>. type is just the datatype you want to use and
     * period is the unit. See:
     * std::chrono::duration<int, std::milli> milli = std::chrono::duration<int, std::milli>(1);
     * std::cout << milli.count() << "ms = " << std::chrono::duration<int, std::micro>(milli).count() << "qs\n"; // 1ms = 1000qs
     * Here milli stores just a 1 - it stores values in 10^3 steps like specified with std::milli. Note that if nothing
     * is specified it would be stored in single steps, which are seconds.
     * - Own types:
     * This is most like something you will not use, but you can define your one duration types:
     * using my5secondType = std::chrono::duration<int, std::ratio<5 / 1>>;
     * Note that ratio is a fracture with an numerator and an denominator (num / den) - std::ratio<num, denom=1>.
     * Example: my5secondType(10) = 50s
     */
	class Time
	{
	public:
		using Duration = std::chrono::duration<long long, std::nano>;

		Time();
		/** Usage: Time(20s) */
		Time(Duration time);
		~Time();

		long double asHours() const;
		long double asMinutes() const;
		long double asSeconds() const;
		long double asMilliseconds() const;
		long double asMicroSeconds() const;
		long long asNanoSeconds() const;
		Duration get() const;

		Time& operator+=(const Time& rhs);
		Time& operator-=(const Time& rhs);
		Time& operator+=(const Duration& rhs);
		Time& operator-=(const Duration& rhs);
		Time& operator=(Time rhs);
		Time& operator=(Duration rhs);
	private:
		Duration time;
	};
}

inline bool operator==(const core::Time& lhs, const core::Time& rhs) { return lhs.asNanoSeconds() == rhs.asNanoSeconds(); }
inline bool operator!=(const core::Time& lhs, const core::Time& rhs) { return !operator==(lhs, rhs); }
inline bool operator< (const core::Time& lhs, const core::Time& rhs) { return lhs.asNanoSeconds() < rhs.asNanoSeconds(); }
inline bool operator> (const core::Time& lhs, const core::Time& rhs) { return  operator< (rhs, lhs); }
inline bool operator<=(const core::Time& lhs, const core::Time& rhs) { return !operator> (lhs, rhs); }
inline bool operator>=(const core::Time& lhs, const core::Time& rhs) { return !operator< (lhs, rhs); }

inline bool operator==(const core::Time& lhs, const core::Time::Duration& rhs) { return lhs.get() == rhs; }
inline bool operator!=(const core::Time& lhs, const core::Time::Duration& rhs) { return !operator==(lhs, rhs); }
inline bool operator< (const core::Time& lhs, const core::Time::Duration& rhs) { return lhs.get() < rhs; }
inline bool operator> (const core::Time& lhs, const core::Time::Duration& rhs) { return  operator< (rhs, lhs); }
inline bool operator<=(const core::Time& lhs, const core::Time::Duration& rhs) { return !operator> (lhs, rhs); }
inline bool operator>=(const core::Time& lhs, const core::Time::Duration& rhs) { return !operator< (lhs, rhs); }

inline core::Time operator+(core::Time lhs, const core::Time& rhs) { lhs += rhs; return lhs; }
inline core::Time operator-(core::Time lhs, const core::Time& rhs) { lhs -= rhs; return lhs; }