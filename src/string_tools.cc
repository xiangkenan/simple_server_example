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
    string date_now = get_year_mon_day_format(to_string(p.tm_year+1900) , to_string(p.tm_mon+1), to_string(p.tm_mday));
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

string get_now_hour_min_sec() {
    time_t timep;
    struct tm p;
    time(&timep);
    FastSecondToDate(timep, &p, 8);
    string date_now = to_string(p.tm_hour) + ":" + to_string(p.tm_min) + ":" + to_string(p.tm_sec);
    return date_now;
}

string get_add_del_date(long sec) {
    struct tm p;
    time_t cur_time = time(NULL) + sec;
    FastSecondToDate(cur_time, &p, 8);
    string date_now = get_year_mon_day_format(to_string(p.tm_year+1900) , to_string(p.tm_mon+1), to_string(p.tm_mday));
    return date_now;
}

string get_year_mon_day_format(string year, string month, string day) {
    string mm = month;
    string dd = day;
    if (month.length() == 1) {
        mm = "0"+month;
    }
    if (day.length() == 1) {
        dd = "0" + day;
    }

    return year+mm+dd;
}

string date_format_ymd(string date) {
    struct tm p,tmp_time;
    time_t cost;
    strptime(date.c_str(),"%Y-%m-%d %H:%M:%S", &tmp_time);
    cost = mktime(&tmp_time);
    FastSecondToDate(cost, &p, 8);
    string date_now = get_year_mon_day_format(to_string(p.tm_year+1900) , to_string(p.tm_mon+1), to_string(p.tm_mday));
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

bool DumpFile(const string& file_name, const unordered_map<long, vector<TimeRange>>& file_data) {
    string date = get_now_date();
    ofstream ofile;
    ofile.open(file_name);

    for (unordered_map<long, vector<TimeRange>>::const_iterator iter = file_data.begin();
            iter != file_data.end(); iter++) {
        ofile << iter->first << "\t";
        for (size_t i = 0; i < iter->second.size(); ++i) {
            ofile << (iter->second)[i].date << ":" << (iter->second)[i].num;
            if (i != iter->second.size() - 1) {
                ofile << ",";
            }
        }
        ofile << endl;
    }

    ofile.close();

    return true;
}

//flag:0 加载最初配置
//flag:1 更行每天增量
bool LoadRangeOriginConfig(string time_range_file, unordered_map<long, vector<TimeRange>>* time_range_origin) {
    ifstream fin(time_range_file);
    if (!fin) {
        LOG(ERROR) << "load "<< time_range_file << " failed!!";
        return false;
    }

    LOG(INFO) << "start loading file:" << time_range_file;
    string line;
    while (getline(fin, line)) {
        if ((line = Trim(line)).empty()) {
            continue;
        }

        vector<string> line_vec;
        Split(line, "\t", &line_vec);
        vector<string> vec;
        Split(line_vec[1], ",", &vec);
        vector<TimeRange> time_range_vec;
        for (size_t i = 0; i < vec.size(); ++i) {
            vector<string> item_vec;
            Split(vec[i], ":", &item_vec);
            if(item_vec.size() != 2)
                continue;

            TimeRange time_range;
            time_range.date = atoi(item_vec[0].c_str());
            time_range.num = atoi(item_vec[1].c_str());
            time_range_vec.push_back(time_range);
        }

        if (time_range_vec.empty())
            continue;

        sort(time_range_vec.begin(), time_range_vec.end(), time_rang_cmp);

        //if ((*time_range_origin).count(line_vec[0]) <= 0) {
        if (time_range_origin->find(atol(line_vec[0].c_str())) != time_range_origin->end()) {
            for (size_t i = 1; i < time_range_vec.size(); ++i) {
                time_range_vec[i].num = time_range_vec[i].num + time_range_vec[i-1].num;
            }

            time_range_origin->insert(make_pair(atol(line_vec[0].c_str()), time_range_vec));
        } else {
            vector<TimeRange> time_range_vec_last = (*time_range_origin)[atol(line_vec[0].c_str())];
            merge_vec(&time_range_vec_last, &time_range_vec);
            (*time_range_origin)[atol(line_vec[0].c_str())] = time_range_vec_last;
        }
    }

    fin.close();
    LOG(INFO) << "load file:" << time_range_file << "complete..";
    return true;
}

void merge_vec(vector<TimeRange>* time_range_vec_last, vector<TimeRange>* time_range_vec) {

    if (time_range_vec_last->size() == 0) {
        return;
    }

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
    int date_int = atoi(date.c_str());

    if (date_int < time_range_origin[l].date) {
        return 0;
    }

    if (date_int > time_range_origin[r-1].date) {
        return time_range_origin[r-1].num;
    }

    while(l < r-1) {
        int mid = (l + r) / 2;
        if (time_range_origin[mid].date <= date_int) {
            l = mid;
        } else {
            r = mid;
        }
    }

    if (time_range_origin[l].date == date_int) {
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

string& replace_all_distinct(string& str,const string& old_value,const string& new_value) {
    for(string::size_type pos(0); pos!=string::npos; pos+=new_value.length()) {
        if((pos=str.find(old_value,pos))!=string::npos ) {
            str.replace(pos,old_value.length(),new_value);
        } else {
            break;
        }
    }
    return str;
}
