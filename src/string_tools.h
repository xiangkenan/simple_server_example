#ifndef _STRING_TOOLS_H_
#define _STRING_TOOLS_H_

#include <iostream>
#include <string>
#include <vector>

int FastSecondToDate(const time_t& unix_sec, struct tm* tm, int time_zone);
std::string get_now_date();
int distance_time_now(std::string time_msg); //距离现在多少秒
void Split(const std::string& s, const std::string& delim, std::vector<std::string>* ret);

#endif
