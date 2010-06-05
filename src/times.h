/*
 * Copyright (c) 2003-2010, John Wiegley.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of New Artisans LLC nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @addtogroup util
 */

/**
 * @file   times.h
 * @author John Wiegley
 *
 * @ingroup util
 *
 * @brief datetime_t and date_t objects
 */
#ifndef _TIMES_H
#define _TIMES_H

#include "utils.h"

namespace ledger {

DECLARE_EXCEPTION(datetime_error, std::runtime_error);
DECLARE_EXCEPTION(date_error, std::runtime_error);

typedef boost::posix_time::ptime	datetime_t;
typedef datetime_t::time_duration_type	time_duration_t;

inline bool is_valid(const datetime_t& moment) {
  return ! moment.is_not_a_date_time();
}

typedef boost::gregorian::date          date_t;
typedef boost::gregorian::date_iterator date_iterator_t;

inline bool is_valid(const date_t& moment) {
  return ! moment.is_not_a_date();
}

extern optional<datetime_t> epoch;

#ifdef BOOST_DATE_TIME_HAS_HIGH_PRECISION_CLOCK
#define TRUE_CURRENT_TIME() (boost::posix_time::microsec_clock::universal_time())
#define CURRENT_TIME()      (epoch ? *epoch : TRUE_CURRENT_TIME())
#else
#define TRUE_CURRENT_TIME() (boost::posix_time::second_clock::universal_time())
#define CURRENT_TIME()      (epoch ? *epoch : TRUE_CURRENT_TIME())
#endif
#define CURRENT_DATE() \
  (epoch ? epoch->date() : boost::gregorian::day_clock::universal_day())

extern date_time::weekdays   start_of_week;

optional<date_time::weekdays>
string_to_day_of_week(const std::string& str);
optional<date_time::months_of_year>
string_to_month_of_year(const std::string& str);

typedef optional<date_t::year_type> optional_year;

datetime_t parse_datetime(const char * str, optional_year current_year = none);

inline datetime_t parse_datetime(const std::string& str,
				 optional_year current_year = none) {
  return parse_datetime(str.c_str(), current_year);
}

date_t parse_date(const char * str, optional_year current_year = none);

inline date_t parse_date(const std::string& str,
			 optional_year current_year = none) {
  return parse_date(str.c_str(), current_year);
}

enum format_type_t {
  FMT_WRITTEN, FMT_PRINTED, FMT_CUSTOM
};

std::string format_datetime(const datetime_t& when,
			    const format_type_t format_type = FMT_PRINTED,
			    const optional<const char *>& format = none);
void set_datetime_format(const char * format);

std::string format_date(const date_t& when,
			const format_type_t format_type = FMT_PRINTED,
			const optional<const char *>& format = none);
void set_date_format(const char * format);
void set_input_date_format(const char * format);

inline void to_xml(std::ostream& out, const datetime_t& when,
		   bool wrap = true)
{
  if (wrap) {
    push_xml x(out, "datetime");
    out << format_datetime(when, FMT_WRITTEN);
  } else {
    out << format_datetime(when, FMT_WRITTEN);
  }
}

inline void to_xml(std::ostream& out, const date_t& when,
		   bool wrap = true)
{
  if (wrap) {
    push_xml x(out, "date");
    out << format_date(when, FMT_WRITTEN);
  } else {
    out << format_date(when, FMT_WRITTEN);
  }
}

struct date_traits_t
{
  bool has_year;
  bool has_month;
  bool has_day;

  date_traits_t(bool _has_year  = false,
		bool _has_month = false,
		bool _has_day   = false)
    : has_year(_has_year), has_month(_has_month), has_day(_has_day) {
    TRACE_CTOR(date_traits_t, "bool, bool, bool");
  }
  date_traits_t(const date_traits_t& traits)
    : has_year(traits.has_year),
      has_month(traits.has_month),
      has_day(traits.has_day) {
    TRACE_CTOR(date_traits_t, "copy");
  }
  ~date_traits_t() throw() {
    TRACE_DTOR(date_traits_t);
  }

  date_traits_t& operator=(const date_traits_t& traits) {
    has_year	= traits.has_year;
    has_month = traits.has_month;
    has_day	= traits.has_day;
    return *this;
  }

  bool operator==(const date_traits_t& traits) const {
    return (has_year	== traits.has_year &&
	    has_month == traits.has_month &&
	    has_day	== traits.has_day);
  }

#if defined(HAVE_BOOST_SERIALIZATION)
private:
  /** Serialization. */

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & has_year;
    ar & has_month;
    ar & has_day;
  }
#endif // HAVE_BOOST_SERIALIZATION
};

struct date_duration_t
{
  enum skip_quantum_t {
    DAYS, WEEKS, MONTHS, QUARTERS, YEARS
  } quantum;
  int length;

  date_duration_t() : quantum(DAYS), length(0) {
    TRACE_CTOR(date_duration_t, "");
  }
  date_duration_t(skip_quantum_t _quantum, int _length)
    : quantum(_quantum), length(_length) {
    TRACE_CTOR(date_duration_t, "skip_quantum_t, int");
  }
  date_duration_t(const date_duration_t& dur)
    : quantum(dur.quantum), length(dur.length) {
    TRACE_CTOR(date_duration_t, "copy");
  }
  ~date_duration_t() throw() {
    TRACE_DTOR(date_duration_t);
  }      

  date_t add(const date_t& date) const {
    switch (quantum) {
    case DAYS:
      return date + gregorian::days(length);
    case WEEKS:
      return date + gregorian::weeks(length);
    case MONTHS:
      return date + gregorian::months(length);
    case QUARTERS:
      return date + gregorian::months(length * 3);
    case YEARS:
      return date + gregorian::years(length);
    default:
      assert(false); return date_t();
    }
  }

  date_t subtract(const date_t& date) const {
    switch (quantum) {
    case DAYS:
      return date - gregorian::days(length);
    case WEEKS:
      return date - gregorian::weeks(length);
    case MONTHS:
      return date - gregorian::months(length);
    case QUARTERS:
      return date - gregorian::months(length * 3);
    case YEARS:
      return date - gregorian::years(length);
    default:
      assert(false); return date_t();
    }
  }

  string to_string() const {
    std::ostringstream out;

    out << length << ' ';

    switch (quantum) {
    case DAYS:	   out << "day"; break;
    case WEEKS:	   out << "week"; break;
    case MONTHS:   out << "month"; break;
    case QUARTERS: out << "quarter"; break;
    case YEARS:	   out << "year"; break;
    default:
      assert(false);
      break;
    }

    if (length > 1)
      out << 's';

    return out.str();
  }

  static date_t find_nearest(const date_t& date, skip_quantum_t skip);

#if defined(HAVE_BOOST_SERIALIZATION)
private:
  /** Serialization. */

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & quantum;
    ar & length;
  }
#endif // HAVE_BOOST_SERIALIZATION
};

class date_specifier_t
{
  friend class date_parser_t;

#if 0
  typedef date_t::year_type	   year_type;
#else
  typedef unsigned short	   year_type;
#endif
  typedef date_t::month_type	   month_type;
  typedef date_t::day_type	   day_type;
  typedef date_t::day_of_week_type day_of_week_type;

  optional<year_type>	     year;
  optional<month_type>	     month;
  optional<day_type>	     day;
  optional<day_of_week_type> wday;

public:
  date_specifier_t(const optional<year_type>&        _year  = none,
		   const optional<month_type>&       _month = none,
		   const optional<day_type>&         _day   = none,
		   const optional<day_of_week_type>& _wday  = none)
    : year(_year), month(_month), day(_day), wday(_wday) {
    TRACE_CTOR(date_specifier_t,
	       "year_type, month_type, day_type, day_of_week_type");
  }
  date_specifier_t(const date_t& date,
		   const optional<date_traits_t>& traits = none) {
    TRACE_CTOR(date_specifier_t, "date_t, date_traits_t");
    if (! traits || traits->has_year)
      year = date.year();
    if (! traits || traits->has_month)
      month = date.month();
    if (! traits || traits->has_day)
      day = date.day();
  }
  date_specifier_t(const date_specifier_t& other)
    : year(other.year), month(other.month),
      day(other.day), wday(other.wday) {
    TRACE_CTOR(date_specifier_t, "copy");
  }
  ~date_specifier_t() throw() {
    TRACE_DTOR(date_specifier_t);
  }

  date_t begin(const optional_year& current_year = none) const;
  date_t end(const optional_year& current_year = none) const;

  bool is_within(const date_t& date,
		 const optional_year& current_year = none) const {
    return date >= begin(current_year) && date < end(current_year);
  }

  optional<date_duration_t> implied_duration() const {
    if (day || wday)
      return date_duration_t(date_duration_t::DAYS, 1);
    else if (month)
      return date_duration_t(date_duration_t::MONTHS, 1);
    else if (year)
      return date_duration_t(date_duration_t::YEARS, 1);
    else
      return none;
  }

  string to_string() const {
    std::ostringstream out;

    if (year)
      out << " year " << *year;
    if (month)
      out << " month " << *month;
    if (day)
      out << " day " << *day;
    if (wday)
      out << " wday " << *wday;

    return out.str();
  }

#if defined(HAVE_BOOST_SERIALIZATION)
private:
  /** Serialization. */

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & year;
    ar & month;
    ar & day;
    ar & wday;
  }
#endif // HAVE_BOOST_SERIALIZATION
};

class date_range_t
{
  friend class date_parser_t;

  optional<date_specifier_t> range_begin;
  optional<date_specifier_t> range_end;

  bool end_inclusive;

public:
  date_range_t(const optional<date_specifier_t>& _range_begin = none,
	       const optional<date_specifier_t>& _range_end   = none)
    : range_begin(_range_begin), range_end(_range_end),
      end_inclusive(false) {
    TRACE_CTOR(date_range_t, "date_specifier_t, date_specifier_t");
  }
  date_range_t(const date_range_t& other)
    : range_begin(other.range_begin), range_end(other.range_end),
      end_inclusive(other.end_inclusive) {
    TRACE_CTOR(date_range_t, "date_range_t");
  }
  ~date_range_t() throw() {
    TRACE_DTOR(date_range_t);
  }

  optional<date_t> begin(const optional_year& current_year = none) const {
    if (range_begin)
      return range_begin->begin(current_year);
    else
      return none;
  }
  optional<date_t> end(const optional_year& current_year = none) const {
    if (range_end) {
      if (end_inclusive)
	return range_end->end(current_year);
      else
	return range_end->begin(current_year);
    } else {
      return none;
    }
  }

  bool is_within(const date_t& date,
		 const optional_year& current_year = none) const {
    optional<date_t> b = begin(current_year);
    optional<date_t> e = end(current_year);
    bool after_begin = b ? date >= *b : true;
    bool before_end  = e ? date <  *e : true;
    return after_begin && before_end;
  }

  string to_string() const {
    std::ostringstream out;

    if (range_begin)
      out << "from" << range_begin->to_string();
    if (range_end)
      out << " to" << range_end->to_string();
    
    return out.str();
  }

#if defined(HAVE_BOOST_SERIALIZATION)
private:
  /** Serialization. */

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & range_begin;
    ar & range_end;
    ar & end_inclusive;
  }
#endif // HAVE_BOOST_SERIALIZATION
};

class date_specifier_or_range_t
{
  typedef variant<int, date_specifier_t, date_range_t> value_type;

  value_type specifier_or_range;

public:
  date_specifier_or_range_t() {
    TRACE_CTOR(date_specifier_or_range_t, "");
  }
  date_specifier_or_range_t(const date_specifier_or_range_t& other)
    : specifier_or_range(other.specifier_or_range) {
    TRACE_CTOR(date_specifier_or_range_t, "copy");
  }
  date_specifier_or_range_t(const date_specifier_t& specifier)
    : specifier_or_range(specifier) {
    TRACE_CTOR(date_specifier_or_range_t, "date_specifier_t");
  }
  date_specifier_or_range_t(const date_range_t& range)
    : specifier_or_range(range) {
    TRACE_CTOR(date_specifier_or_range_t, "date_range_t");
  }
  ~date_specifier_or_range_t() throw() {
    TRACE_DTOR(date_specifier_or_range_t);
  }

  optional<date_t> begin(const optional_year& current_year = none) const {
    if (specifier_or_range.type() == typeid(date_specifier_t))
      return boost::get<date_specifier_t>(specifier_or_range).begin(current_year);
    else if (specifier_or_range.type() == typeid(date_range_t))
      return boost::get<date_range_t>(specifier_or_range).begin(current_year);
    else
      return none;
  }
  optional<date_t> end(const optional_year& current_year = none) const {
    if (specifier_or_range.type() == typeid(date_specifier_t))
      return boost::get<date_specifier_t>(specifier_or_range).end(current_year);
    else if (specifier_or_range.type() == typeid(date_range_t))
      return boost::get<date_range_t>(specifier_or_range).end(current_year);
    else
      return none;
  }


  string to_string() const {
    std::ostringstream out;

    if (specifier_or_range.type() == typeid(date_specifier_t))
      out << "in" << boost::get<date_specifier_t>(specifier_or_range).to_string();
    else if (specifier_or_range.type() == typeid(date_range_t))
      out << boost::get<date_range_t>(specifier_or_range).to_string();

    return out.str();
  }

#if defined(HAVE_BOOST_SERIALIZATION)
private:
  /** Serialization. */

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & specifier_or_range;
  }
#endif // HAVE_BOOST_SERIALIZATION
};

class date_interval_t : public equality_comparable<date_interval_t>
{
public:
  static date_t add_duration(const date_t&	    date,
			     const date_duration_t& duration);
  static date_t subtract_duration(const date_t&		 date,
				  const date_duration_t& duration);

  optional<date_specifier_or_range_t> range;

  optional<date_t>	    start;  // the real start, after adjustment
  optional<date_t>	    finish; // the real end, likewise
  bool			    aligned;
  optional<date_t>	    next;
  optional<date_duration_t> duration;
  optional<date_t>	    end_of_duration;

  explicit date_interval_t() : aligned(false) {
    TRACE_CTOR(date_interval_t, "");
  }
  date_interval_t(const string& str) : aligned(false) {
    TRACE_CTOR(date_interval_t, "const string&");
    parse(str);
  }
  date_interval_t(const date_interval_t& other)
    : range(other.range),
      start(other.start),
      finish(other.finish),
      aligned(other.aligned),
      next(other.next),
      duration(other.duration),
      end_of_duration(other.end_of_duration) {
    TRACE_CTOR(date_interval_t, "copy");
  }
  ~date_interval_t() throw() {
    TRACE_DTOR(date_interval_t);
  }

  bool operator==(const date_interval_t& other) const {
    return (start == other.start &&
	    (! start || *start == *other.start));
  }

  operator bool() const {
    return is_valid();
  }

  optional<date_t> begin(const optional_year& current_year = none) const {
    return start ? start : (range ? range->begin(current_year) : none);
  }
  optional<date_t> end(const optional_year& current_year = none) const {
    return finish ? finish : (range ? range->end(current_year) : none);
  }

  void   parse(const string& str);

  void	 resolve_end();
  void	 stabilize(const optional<date_t>& date = none);

  bool   is_valid() const {
    return start;
  }

  /** Find the current or next period containing date.  Returns true if the
      date_interval_t object has been altered to reflect the interval
      containing date, or false if no such period can be found. */
  bool find_period(const date_t& date);

  optional<date_t> inclusive_end() const {
    if (end_of_duration)
      return *end_of_duration - gregorian::days(1);
    else
      return none;
  }

  date_interval_t& operator++();

  void dump(std::ostream& out, optional_year current_year = none);

#if defined(HAVE_BOOST_SERIALIZATION)
private:
  /** Serialization. */

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & range;
    ar & start;
    ar & finish;
    ar & aligned;
    ar & next;
    ar & duration;
    ar & end_of_duration;
  }
#endif // HAVE_BOOST_SERIALIZATION
};

void times_initialize();
void times_shutdown();

void show_period_tokens(std::ostream& out, const string& arg);

std::ostream& operator<<(std::ostream& out, const date_duration_t& duration);

} // namespace ledger

#endif // _TIMES_H