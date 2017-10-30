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

string get_now_hour() {
    time_t timep;
    struct tm p;
    time(&timep);
    FastSecondToDate(timep, &p, 8);
    string date_now = to_string(p.tm_hour);
    return date_now;
}

string get_add_del_date(long sec) {
    struct tm p;
    time_t cur_time = time(NULL) + sec;
    FastSecondToDate(cur_time, &p, 8);
    string date_now = to_string(p.tm_year+1900) + to_string(p.tm_mon+1) + to_string(p.tm_mday);
    return date_now;
}

string date_format_ymd(string date) {
    struct tm p,tmp_time;
    time_t cost;
    strptime(date.c_str(),"%Y-%m-%d %H:%M:%S", &tmp_time);
    cost = mktime(&tmp_time);
    FastSecondToDate(cost, &p, 8);
    string date_now = to_string(p.tm_year+1900) + to_string(p.tm_mon+1) + to_string(p.tm_mday);
    return date_now;
}

int distance_time_now(string time_msg) {
    if (time_msg == "")
        return 0;
    struct tm tmp_time;
    strptime(time_msg.c_str(),"%Y-%m-%d %H:%M:%S",&tmp_time);
    time_t user_time, cur_time;
    user_time = mktime(&tmp_time);
    time(&cur_time);
    int cost = difftime(cur_time, user_time);
    return cost;
}

void Split(const string& s, const string& delim, vector<string>* ret) {
    if (ret == NULL)
        return;
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    while (index != string::npos) {
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

string Trim(string s) {
    if (s.empty()) {
        return s;
    }

    s.erase(0, s.find_first_not_of(" \t\n"));
    s.erase(s.find_last_not_of(" \t\n") + 1);
    return s;
}

int time_rang_cmp(TimeRange time_range1, TimeRange time_range2) {
    return time_range1.date < time_range2.date;
}

//flag:0 加载最初配置
//flag:1 更行每天增量
bool LoadRangeOriginConfig(string time_range_file, unordered_map<string, vector<TimeRange>>* time_range_origin) {
    ifstream fin(time_range_file);
    if (!fin) {
        LOG(ERROR) << "load "<< time_range_file << " failed!!";
        return false;
    }

    string line;
    while (getline(fin, line)) {
        if ((line = Trim(line)).empty()) {
            continue;
        }

        vector<string> line_vec;
        Split(line, "\t", &line_vec);
        vector<string> vec;
        cout << "kenan: " << line_vec[1] << endl;
        Split(line_vec[1], ",", &vec);
        vector<TimeRange> time_range_vec;
        for (size_t i = 0; i < vec.size(); ++i) {
            vector<string> item_vec;
            Split(vec[i], ":", &item_vec);
            if(item_vec.size() != 2)
                continue;

            TimeRange time_range;
            time_range.date = item_vec[0];
            time_range.num = atoi(item_vec[1].c_str());
            time_range_vec.push_back(time_range);
        }

        if (time_range_vec.empty())
            continue;

        sort(time_range_vec.begin(), time_range_vec.end(), time_rang_cmp);

        for (size_t i = 1; i < time_range_vec.size(); ++i) {
            time_range_vec[i].num = time_range_vec[i].num + time_range_vec[i-1].num;
        }
        
        if ((*time_range_origin).count(line_vec[0]) <= 0) {
            time_range_origin->insert(pair<string, vector<TimeRange>>(line_vec[0], time_range_vec));
        } else {
            vector<TimeRange> time_range_vec_last = (*time_range_origin)[line_vec[0]];
            merge_vec(&time_range_vec_last, &time_range_vec);
        }
    }

    fin.close();
    return true;
}

void merge_vec(vector<TimeRange>* time_range_vec_last, vector<TimeRange>* time_range_vec) {

    if (time_range_vec_last->size() == 0) {
        return;
    }
    cout << "asd" << endl;

    for (size_t i = 0; i < time_range_vec->size(); ++i) {
        if ((*time_range_vec)[i].date <= (*time_range_vec_last)[time_range_vec_last->size()-1].date) {
            continue;
        }

        (*time_range_vec)[i].num += (*time_range_vec_last)[time_range_vec_last->size()-1].num;
        time_range_vec_last->push_back((*time_range_vec)[i]);
    }
}

int find_two(const string& date, const vector<TimeRange>& time_range_origin, int flag) {
    int l = 0;
    int r = time_range_origin.size();

    if (date < time_range_origin[l].date) {
        return 0;
    }

    if (date > time_range_origin[r-1].date) {
        return time_range_origin[r-1].num;
    }

    while(l < r-1) {
        int mid = (l + r) / 2;
        if (time_range_origin[mid].date <= date) {
            l = mid;
        } else {
            r = mid;
        }
    }

    if (time_range_origin[l].date == date) {
        if (flag == 0) {
            if (l == 0) return 0;
            return time_range_origin[l-1].num;
        } else {
            return time_range_origin[l].num;
        }
    } else {
        return time_range_origin[l].num; 
    }
}

int get_range_order_num(const string& start, const string& end, const vector<TimeRange>& time_range_origin) {
    if (time_range_origin.size() == 0) {
        return 0;
    }
    int start_num = find_two(start, time_range_origin, 0);
    int end_num = find_two(end, time_range_origin, 1);
    return end_num - start_num;
}
