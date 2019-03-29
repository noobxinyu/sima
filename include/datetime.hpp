/* 
 * File:   date.hpp
 * Author: xli
 *
 * Created on August 15, 2016, 6:33 PM
 */

#ifndef DATETIME_HPP
#define DATETIME_HPP

#include <ctime>
#include <boost/date_time/local_time/local_time.hpp>

static const int MS_PER_HOUR = 3600000;
static const int MS_PER_MIN = 60000;
static const int MS_PER_SEC = 1000;

static const int64_t US_PER_MS = 1000L;
static const int64_t US_PER_SEC = US_PER_MS * 1000;
static const int64_t US_PER_MIN = US_PER_SEC * 60;
static const int64_t US_PER_HOUR = US_PER_MIN * 60;

namespace utils
{
  namespace datetime
  {

    inline int
    next_day(int date, int n = 1)
    {
      tm broken_down;
      broken_down.tm_year = date / 10000 - 1900;
      broken_down.tm_mon = (date / 100) % 100 - 1;
      broken_down.tm_mday = date % 100 + n;
      broken_down.tm_hour = 12;  // avoid issues with summer time, etc.
      broken_down.tm_min = 0;
      broken_down.tm_sec = 0;
      mktime( &broken_down );
      return (broken_down.tm_year + 1900) * 10000
              +  (broken_down.tm_mon + 1) * 100
              +  broken_down.tm_mday;
    }

    inline int
    next_week_day(int date, int n = 1)
    {
      tm broken_down{};
      broken_down.tm_year = date / 10000 - 1900;
      broken_down.tm_mon = (date / 100) % 100 - 1;
      broken_down.tm_mday = date % 100 + n;
      broken_down.tm_hour = 12;  // avoid issues with summer time, etc.
      broken_down.tm_min = 0;
      broken_down.tm_sec = 0;
      mktime( &broken_down );
      if (0 == broken_down.tm_wday) {
        broken_down.tm_mday += n > 0 ? 1 : -2;
        mktime( &broken_down );
      } else if (6 == broken_down.tm_wday) {
        broken_down.tm_mday += n > 0 ? 2 : -1;
        mktime( &broken_down );
      }
      return (broken_down.tm_year + 1900) * 10000
              +  (broken_down.tm_mon + 1) * 100
              +  broken_down.tm_mday;
    }

    inline int
    get_ms_from_hhmm(int tm)
    {
      return tm / 100 * MS_PER_HOUR + tm % 100 * MS_PER_MIN;
    }

    inline int64_t
    get_us_from_hhmm(int64_t tm)
    {
      return tm / 100 * US_PER_HOUR + tm % 100 * US_PER_MIN;
    }

    inline int64_t
    get_us_from_hhmmss(int64_t tm)
    {
      return tm / 10000 * US_PER_HOUR + tm % 10000 / 100 * US_PER_MIN + tm % 100 * US_PER_SEC;
    }

    inline int64_t
    get_us_from_hhmmssMMM(int64_t tm)
    {
      auto hhmmss = tm / 1000;
      auto MMM = tm % 1000;

      return hhmmss / 10000 * US_PER_HOUR + hhmmss % 10000 / 100 * US_PER_MIN + hhmmss % 100 * US_PER_SEC + MMM * US_PER_MS;
    }

    inline int64_t
    get_us_from_hhmmssMMMMMM(int64_t tm)
    {
      auto hhmmss = tm / 1000000;
      auto MMMMMM = tm % 1000000;

      return hhmmss / 10000 * US_PER_HOUR + hhmmss % 10000 / 100 * US_PER_MIN + hhmmss % 100 * US_PER_SEC + MMMMMM;
    }

    inline const char*
    get_time_str_ms(int ms, char* out)
    {
      sprintf(out, "%02d:%02d:%02d.%03d", ms / MS_PER_HOUR, ms % MS_PER_HOUR / MS_PER_MIN, ms % MS_PER_MIN / 1000, ms % 1000);
      return out;
    }

    inline std::string
    get_time_str_ms(int ms)
    {
      char out[256];
      get_time_str_ms(ms, out);
      return out;
    }

    inline const char*
    get_time_str_us(int64_t ms, char* out)
    {
      sprintf(out, "%02ld:%02ld:%02ld.%03ld", ms / US_PER_HOUR, ms % US_PER_HOUR / US_PER_MIN, ms % US_PER_MIN / 1000000, ms % 1000000);
      return out;
    }

    inline std::string
    get_time_str_us(int64_t ms)
    {
      char out[256];
      get_time_str_us(ms, out);
      return out;
    }

    inline time_t
    get_time_t(size_t t) // t in "hhmm" format
    {
      time_t rawtime;
      time(&rawtime);
      struct tm tm;
      localtime_r(&rawtime, &tm);
      tm.tm_hour =  t / 100;
      tm.tm_min = t % 100;
      tm.tm_sec = 0;
      return mktime(&tm);
    }

    inline int64_t
    get_time_us()
    {
      boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
      boost::posix_time::time_duration duration(time.time_of_day());
      return duration.total_microseconds();
    }

    inline int64_t
    get_time_epoch_us()
    {
      boost::posix_time::ptime const time_epoch(boost::gregorian::date(1970, 1, 1));
      return (boost::posix_time::microsec_clock::local_time() - time_epoch).total_microseconds();
    }

    inline int64_t
    get_time_epoch_ms()
    {
      boost::posix_time::ptime const time_epoch(boost::gregorian::date(1970, 1, 1));
      return (boost::posix_time::microsec_clock::local_time() - time_epoch).total_milliseconds();
    }

    inline int64_t
    get_time_epoch_sec()
    {
      boost::posix_time::ptime const time_epoch(boost::gregorian::date(1970, 1, 1));
      return (boost::posix_time::microsec_clock::local_time() - time_epoch).total_seconds();
    }

    inline int64_t
    get_time_epoch_from_date(int today)
    {
      struct tm t ={0};  // Initalize to all 0's
      t.tm_year = today / 10000 - 1900;  // This is year-1900, so 112 = 2012
      t.tm_mon = today % 10000 / 100 - 1;
      t.tm_mday = today % 100;
      return mktime(&t);
    }

    inline int64_t hhmm()
    {
      auto now = get_time_epoch_sec();
      auto h = now / 3600 % 24;
      auto m = now / 60 % 60;
      return h * 100 + m;
    }

    inline struct tm
    get_date(int date, int offset=0) // return tm from "YYYYmmdd" number
    {
      struct tm t;
      t.tm_year = date / 10000 - 1900;
      t.tm_mon = date % 10000 / 100 - 1;
      t.tm_mday = date % 100;
      t.tm_hour = 12;
      t.tm_min = 0;
      t.tm_sec = 0;
      if (offset) {
        boost::gregorian::date d = boost::gregorian::date_from_tm(t);
        t = boost::gregorian::to_tm(d + boost::gregorian::date_duration(offset));
      }
      return t;
    }

    inline void
    strftime(const char* pattern, char* out, int len, struct tm* timeinfo=NULL)
    {
      if (timeinfo) {
        ::strftime(out, len, pattern, timeinfo);
        return;
      }
      time_t t = time(NULL);
      struct tm today;
      localtime_r(&t, &today);
      ::strftime(out, len, pattern, &today);
    }

    inline int
    get_today()
    {
      char buf[256];
      strftime("%Y%m%d", buf, sizeof (buf));
      return atoi(buf);
    }

    inline std::string
    get_today_str(const char* fmt = "%Y%m%d")
    {
      char buf[16];
      strftime(fmt, buf, sizeof (buf));
      return std::string(buf);
    }

    inline std::string
    strftime(const std::string& src, int date = 0)
    {
      if (date == 0) {
        date = get_today();
      }
      struct tm t = get_date(date);
      char out[1024];
      ::strftime(out, sizeof (out), src.c_str(), &t);
      return out;
    }
  }
}

#endif /* DATE_HPP */

