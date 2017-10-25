#include "string_tools.h"

using namespace std;

int FastSecondToDate(const time_t& unix_sec, struct tm* tm, int time_zone) {
    static const int kHoursInDay = 24;
    static const int kMinutesInHour = 60;
    static const int kDaysFromUnixTime = 2472632;
    static const int kDaysFromYear = 153;
    static const int kMagicUnkonwnFirst = 146097;
    static const int kMagicUnkonwnSec = 1461;
    tm->tm_sec  =  unix_sec % kMinutesInHour;
    int i      = (unix_sec/kMinutesInHour);
    tm->tm_min  = i % kMinutesInHour; //nn
    i /= kMinutesInHour;
    tm->tm_hour = (i + time_zone) % kHoursInDay; // hh
    tm->tm_mday = (i + time_zone) / kHoursInDay;
    int a = tm->tm_mday + kDaysFromUnixTime;
    int b = (a*4  + 3)/kMagicUnkonwnFirst;
    int c = (-b*kMagicUnkonwnFirst)/4 + a;
    int d =((c*4 + 3) / kMagicUnkonwnSec);
    int e = -d * kMagicUnkonwnSec;
    e = e/4 + c;
    int m = (5*e + 2)/kDaysFromYear;
    tm->tm_mday = -(kDaysFromYear * m + 2)/5 + e + 1;
    tm->tm_mon = (-m/10)*12 + m + 2;
    tm->tm_year = b*100 + d  - 6700 + (m/10);
    return 0;
}


string get_now_date() {
    time_t timep;
    struct tm p;
    time(&timep);
    FastSecondToDate(timep, &p, 8);
    string date_now = to_string(p.tm_year+1900) + to_string(p.tm_mon+1) + to_string(p.tm_mday);
    return date_now;
}

int distance_time_now(std::string time_msg) {
    struct tm tmp_time;
    strptime(time_msg.c_str(),"%Y-%m-%d %H:%M:%S",&tmp_time);
    time_t user_time, cur_time;
    user_time = mktime(&tmp_time);
    time(&cur_time);
    int cost = difftime(cur_time, user_time);
    return cost;
}

void Split(const std::string& s, const std::string& delim, std::vector<std::string>* ret) {
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    while (index != std::string::npos) {
        ret->push_back(s.substr(last, index - last));
        last = index + 1;
        index = s.find_first_of(delim, last);
    }
    if (index - last > 0) {
        ret->push_back(s.substr(last, index - last));
    }
}

Json::Value get_url_json(char* buf) {
    Json::Reader reader;
    Json::Value result;
    char * ret_begin = buf + sizeof (uint32_t);
    char * ret_end = buf + sizeof (uint32_t) + *((uint32_t *) buf);
    *ret_end = '\0';

    bool rt = reader.parse(ret_begin, ret_end, result);
    if (!rt) {
        LOG(WARNING) << "murl_get_url parse failed url";
        return false;
    }

    return result;
}
