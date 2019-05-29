#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__
#include<time.h>
#include<memory.h>
#include<string>

class Timestamp
{
public:
	Timestamp()
	{
		memset(&timeval_,0,sizeof(timeval_));
	}

	explicit Timestamp(const struct timeval& tv)
		:timeval_(tv)
	{

	}

	explicit Timestamp(int64_t microSecondsFromUnixEpoch)
	{
		timeval_.tv_sec = microSecondsFromUnixEpoch / kMicroSecondsPerSecond;
		timeval_.tv_usec = microSecondsFromUnixEpoch%kMicroSecondsPerSecond;
	}

	Timestamp(const Timestamp& rhs)
		:timeval_(rhs.timeval_)
	{

	}

	Timestamp& operator=(const Timestamp& rhs)
	{
		if (this!=&rhs)
		{
			timeval_ = rhs.timeval_;
		}
		return *this;
	}

	void swap(Timestamp& rhs)
	{
		std::swap(timeval_.tv_sec, rhs.timeval_.tv_sec);
		std::swap(timeval_.tv_usec, rhs.timeval_.tv_usec);
	}

	std::string toString() const;
	std::string toFormattedString(bool showMicroseconds = true) const;
	const struct timeval& toTimeval() const { return timeval_; }

	bool valid() const { return timeval_.tv_usec > 0; }

	time_t secondsFromUnixEpoch() const { return timeval_.tv_sec; }

	int64_t microSecondsFromUnixEpoch() const { return static_cast<int64_t>(timeval_.tv_sec*kMicroSecondsPerSecond + timeval_.tv_usec); }


	static Timestamp now();
	static Timestamp invalid()
	{
		return Timestamp();
	}

	static Timestamp fromUnixTime(time_t t)
	{
		struct timeval tv = { t,0 };
		return Timestamp(tv);
	}

private:
	struct timeval timeval_;

public:
	static const int kMicroSecondsPerSecond = 1000000;
};

inline bool operator<(const Timestamp& lhs, const Timestamp& rhs)
{
	return lhs.microSecondsFromUnixEpoch() < rhs.microSecondsFromUnixEpoch();
}

inline bool operator==(const Timestamp& lhs, const Timestamp& rhs)
{
	return lhs.microSecondsFromUnixEpoch() == rhs.microSecondsFromUnixEpoch();
}

inline int64_t timeDiff(const Timestamp& high, const Timestamp& low)
{
	return high.microSecondsFromUnixEpoch() - low.microSecondsFromUnixEpoch();
}

inline Timestamp addTime(const Timestamp& timestamp, double seconds)
{
	int64_t delta =static_cast<int64_t>(seconds*Timestamp::kMicroSecondsPerSecond);
	return Timestamp(timestamp.microSecondsFromUnixEpoch() + delta);
}

#endif

