#ifndef DATE_H
#define DATE_H

#include <ctime>
#include <ctype.h>
#include <string>
#include <sstream>

class Date {
public:
	int year;
	int month;
	int day;

	Date() {
    year = month = day = 0;
  }

	Date(const int y, 
       const int m, 
       const int d) {
    year = y;
    month = m; 
    day = d;
  }

	Date(const Date& d) {
    year = d.year;
    month = d.month;
    day = d.day;
  }

  Date(const std::string& str) {
    std::string dStr, mStr, yStr;
    dStr = str.substr(0,2);
    mStr = str.substr(3,2);
    yStr = str.substr(6);

    auto isDigits = [](const std::string& s){
      for (auto c : s) {
        if (!isdigit(c)) 
          return false;
      }
      return true;
    };
    if (isDigits(dStr) && isDigits(mStr) && isDigits(yStr)) {
      day = stoi(str.substr(0, 2));
      month = stoi(str.substr(3, 2));
      year = stoi(str.substr(6));
    }
  }	

  std::string to_string() const {
    std::stringstream ss;
    if (this->day < 10) ss << '0';
    ss << this->day << '/';
    if (this->month < 10) ss << '0';
    ss << this->month << '/' << this->year;
    return ss.str();
  }

  static int difference(const Date& d1, 
                        const Date& d2) {
    auto make_tm = [](int y, int m, int d) {
      std::tm tm = {0};
      tm.tm_year = y - 1900;
      tm.tm_mon = m - 1;
      tm.tm_mday = d;
      return tm;
    };
    std::tm tm1 = make_tm(d1.year, d1.month, d1.day);
    std::tm tm2 = make_tm(d2.year, d2.month, d2.day); 

    std::time_t time1 = std::mktime(&tm1);
    std::time_t time2 = std::mktime(&tm2);

    const int secsPerDay = 60*60*24;
    std::time_t difference = (time1-time2) / secsPerDay;    

    return abs( int(std::difftime(time1, time2) / secsPerDay) );
  } 

  inline bool operator==(const Date& that) const {
    return (this->year  == that.year  && 
            this->month == that.month &&
            this->day   == that.day);
  }

  inline bool operator<(const Date& that) const {
    if      (this->year  < that.year)  return true;
    else if (this->year  > that.year)  return false;
    else if (this->month < that.month) return true;
    else if (this->month > that.month) return false;
    else if (this->day   < that.day)   return true;
    else if (this->day   > that.day)   return false;
    else                                return false;
  }

  friend std::ostream& operator<<(std::ostream& stream, 
                                  const Date& d) {
    stream << d.to_string();
    return stream;
  }
};

#endif