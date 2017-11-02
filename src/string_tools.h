#ifndef _STRING_TOOLS_H_
#define _STRING_TOOLS_H_

#include <iostream>
#include <string>
#include <vector>
#include <jsoncpp/jsoncpp.h>
#include <glog/logging.h>
#include <fstream>
#include <algorithm>
#include <unordered_map>

#include "config_struct.h"

int FastSecondToDate(const time_t& unix_sec, struct tm* tm, int time_zone);
std::string get_now_date();
std::string get_now_hour();
std::string get_now_hour_min_sec();
std::string get_add_del_date(long sec);
int distance_time_now(std::string time_msg); //距离现在多少秒
void Split(const std::string& s, const std::string& delim, std::vector<std::string>* ret);
Json::Value get_url_json(char* buf);
std::string Trim(std::string s);
int time_rang_cmp(TimeRange time_range1, TimeRange time_range2);
bool LoadRangeOriginConfig(std::string time_range_file, std::unordered_map<long, std::vector<TimeRange>>* time_range_origin);
bool DumpFile(const std::string& file_name, const std::unordered_map<long, std::vector<TimeRange>>& file_data); //dump 文件
int get_range_order_num(const std::string& start, const std::string& end, const std::vector<TimeRange>& time_range_origin);
int find_two(const std::string& date, const std::vector<TimeRange>& time_range_origin, int flag);
std::string date_format_ymd(std::string date);
void merge_vec(std::vector<TimeRange>* time_range_vec_last, std::vector<TimeRange>* time_range_vec);
std::string get_year_mon_day_format(std::string year, std::string month, std::string day);
std::string& replace_all_distinct(std::string& str,const std::string& old_value,const std::string& new_value);

#endif
