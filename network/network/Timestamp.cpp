#include "Timestamp.h"

std::string Timestamp::toString() const
{
	char buf[32] = { 0 };
	time_t seconds = static_cast<int64_t>(timeval_.tv_sec);
	int64_t microSeconds = static_cast<int64_t>(timeval_.tv_usec);
	snprintf(buf, sizeof(buf), "%ld.%06ld", seconds, microSeconds);
	return buf;
}

std::string Timestamp::toFormattedString(bool showMicroseconds ) const
{
	char buf[32] = { 0 };
	time_t seconds = static_cast<int64_t>(timeval_.tv_sec);
	struct tm t;
	gmtime(&seconds, &t);
	if (!showMicroseconds)
	{
		snprintf(buf, sizeof(buf), "%04d%02d%02d %02d:%02d:%02d", t.tm_year+1900,t.tm_mon+1,t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	}
	else
	{
		int64_t microSeconds = static_cast<int64_t>(timeval_.tv_usec);
		snprintf(buf, sizeof(buf), "%04d%02d%02d %02d:%02d:%02d.%06ld", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,microSeconds);
	}
	return buf;
}

Timestamp Timestamp::now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return Timestamp(tv);
}