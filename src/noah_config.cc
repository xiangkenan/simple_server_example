#include "noah_config.h"

using namespace std;

NoahConfigRead::NoahConfigRead() {
    redis_field_map["userprofile.city"] = "1";
    redis_field_map["userprofile.competitor"] = "2";
    redis_field_map["userprofile.oauth"] = "3";
    redis_field_map["userprofile.bond"] = "4";
    redis_field_map["userprofile.recharge"] = "5";
    redis_field_map["userprofile.device"] = "6";
    redis_field_map["userprofile.reg_time"] = "7";
    redis_field_map["userprofile.auth_time"] = "8";
    redis_field_map["realtime.app.action"] = "20";
    redis_field_map["userprofile.first_order_time"] = "1008";
    redis_field_map["order.order"] = "1001";
    redis_field_map["order.repair_order"] = "1002";
    redis_field_map["order.free_order"] = "1003";
    redis_field_map["order.weekday_order"] = "1004";
    redis_field_map["order.peak_order"] = "1005";
    redis_field_map["offline.silence"] = "1009";

    type_map_operate["userprofile.city"] = 1;
    type_map_operate["userprofile.competitor"] = 1;
    type_map_operate["userprofile.oauth"] = 2;
    type_map_operate["userprofile.bond"] = 2;
    type_map_operate["userprofile.recharge"] = 2;
    type_map_operate["userprofile.device"] = 2;
    type_map_operate["userprofile.reg_time"] = 3;
    type_map_operate["userprofile.auth_time"] = 3;
    type_map_operate["userprofile.first_order_time"] = 3;
    type_map_operate["order.order"] = 4;
    type_map_operate["order.repair_order"] = 4;
    type_map_operate["order.free_order"] = 4;
    type_map_operate["order.weekday_order"] = 4;
    type_map_operate["order.peak_order"] = 4;
    type_map_operate["offline.silence"] = 1;
}

bool NoahConfigRead::is_include(const BaseConfig& config, string user_msg) {
    string not_contain;
    vector<string> user_msg_vec;
    user_msg_vec.push_back(user_msg);
    if (config.filter_id == "userprofile.city") {
        not_contain = ".NOT_IN.";
    } else if (config.filter_id == "userprofile.competitor") {
        not_contain = "BITMAP_NONE";
        user_msg_vec.clear();
        if (user_msg.find("摩拜单车") != string::npos) 
            user_msg_vec.push_back("1");
        if (user_msg.find("小蓝单车") != string::npos) 
            user_msg_vec.push_back("2");
        if (user_msg.find("一步单车") != string::npos) 
            user_msg_vec.push_back("4");
        if (user_msg.find("优拜单车") != string::npos) 
            user_msg_vec.push_back("32");
        if (user_msg.find("hello单车") != string::npos) 
            user_msg_vec.push_back("64");
        if (user_msg.find("小鸣单车") != string::npos) 
            user_msg_vec.push_back("2048");
        if (user_msg.find("永安行") != string::npos) 
            user_msg_vec.push_back("16384");
    } else if (config.filter_id == "offline.silence") {
        user_msg_vec.clear();
        user_msg = to_string(distance_time_now(user_msg)/86400);
        user_msg_vec.push_back(user_msg);
        not_contain = "jkashdlk";
    } else {
        not_contain = ".NOT_IN.";
    }
    
    int flag = 0;
    if (config.option_id.find(not_contain) != string::npos) {
        for (unsigned int i = 0; i < config.values.size(); ++i) {
            for (unsigned int j =0; j < user_msg_vec.size(); ++j) {
                if(config.values[i] == user_msg_vec[j])
                    return false;
            }
        }
        return true;
    } else {
        for (unsigned int i = 0; i < user_msg_vec.size(); ++i){
            for (unsigned int j = 0; j < config.values.size(); ++j) {
                string a = user_msg_vec[i];
                string b = config.values[j];
                if (user_msg_vec[i] == config.values[j]) {
                    flag = 1;
                }
            }
            if (flag != 1)
                return false;
            else
                flag = 0;
        }
        return true;
    }

    return true;
}

bool NoahConfigRead::is_confirm(const BaseConfig& config, string user_msg) {
    string not_contain;
    if (config.filter_id == "userprofile.oauth")
        not_contain = "NOT_EQUAL_TO";
    else if (config.filter_id == "userprofile.bond")
        not_contain = "EQUAL_TO";
    else if (config.filter_id == "userprofile.recharge")
        not_contain = "EQUAL_TO";
    else if (config.filter_id == "userprofile.device")
        not_contain = "FILTER..device..EQUAL_TO.INT32.OPTION_PARA.2";
    else
        not_contain = "NOT_EQUAL_TO";
    if (config.option_id.find(not_contain) != string::npos) {
        if (user_msg == "1")
            return false;
    } else {
        if(user_msg == "0")
            return false;
    }

    return true;
}

bool NoahConfigRead::is_time_range(const BaseConfig& config, string user_msg) {
    double cost = distance_time_now(user_msg);
    if (config.option_id.find("-7D.") != string::npos) {
        if (cost > 604800)
            return false;
    } else if (config.option_id.find("-14D.") != string::npos) {
        if (cost > 1209600)
            return false;
    } else if (config.option_id.find("-30D.") != string::npos) {
        if (cost > 2592000)
            return false;
    } else if (config.option_id.find("OPTION_TIME_RANGE") != string::npos) {
        if (cost > 604800)
            return false;
    }

    return true;
}

bool NoahConfigRead::is_time_range_value(const BaseConfig& config, string user_msg) {

    if (config.value_id.find("GREATER") != string::npos) {
        if (atoi(user_msg.c_str()) <= atoi(config.values[0].c_str()))
            return false;
    } else if (config.value_id.find("LESS") != string::npos) {
        if (atoi(user_msg.c_str()) >= atoi(config.values[0].c_str()))
            return false;
    } else if (config.value_id.find("BETWEEN") != string::npos) {
        if (atoi(user_msg.c_str()) < atoi(config.values[0].c_str()) || atoi(user_msg.c_str()) > atoi(config.values[1].c_str()) )
            return false;
    }

    return true;
}

bool NoahConfigRead::is_range_value(const BaseConfig& config, string user_msg) {
    if (config.option_id.find("GREATER") != string::npos) {
        if (atoi(user_msg.c_str()) <= atoi(config.values[0].c_str()))
            return false;
    } else if (config.value_id.find("LESS") != string::npos) {
        if (atoi(user_msg.c_str()) >= atoi(config.values[0].c_str()))
            return false;
    } else if (config.value_id.find("BETWEEN") != string::npos) {
        if (atoi(user_msg.c_str()) <= atoi(config.values[0].c_str()))
            return false;
    }
    return true;
}

bool NoahConfigRead::write_log(const BaseConfig& config, bool flag, KafkaData* kafka_data) {
    string msg = redis_field_map[config.filter_id];
    if (flag == true) {
        kafka_data->log_str += "&" + msg;
    }
    else {
        kafka_data->log_str += "&no" + msg;
    }
    return flag;
}

bool NoahConfigRead::is_satisfied_value(const BaseConfig& config, string user_msg) {
    if (config.filter_id == "realtime.app.action") {
        if (config.option_id == "app.action.appon") {
            if (user_msg == "appscan")
                return false;
        } else {
            if (user_msg == "appstart")
                return false;
        }
    }
    return true;
}

/* flag参数解释
 * 1：包含， 不包含
 * 2:是， 否
 * 3:当前时间在 某个范围内
 * 4:规定范围内，订单是否满足条件（目前只判断是否满足条件）
 * 5:满足大于，小于，范围
 * 6:是否符合其中一种情况
*/
bool NoahConfigRead::data_core_operate(const BaseConfig& config, int flag, KafkaData* kafka_data) {
    string user_msg = kafka_data->offline_data_json["rv"][redis_field_map[config.filter_id]].asString();

    bool ret;
    switch (flag) {
        case 1:
            ret = is_include(config, user_msg);
            return write_log(config, ret, kafka_data);
        case 2:
            ret = is_confirm(config, user_msg);
            return write_log(config, ret, kafka_data);
        case 3:
            ret = is_time_range(config, user_msg);
            return write_log(config, ret, kafka_data);
        case 4:
            ret = is_time_range_value(config, user_msg);
            return write_log(config, ret, kafka_data);
        case 5:
            ret = is_range_value(config, user_msg);
            return write_log(config, ret, kafka_data);
        case 6:
            ret = is_satisfied_value(config, user_msg);
            return write_log(config, ret, kafka_data);
        default:
            kafka_data->log_str += " 未知字段未满足";
            return false;
    }

    return true;
}

bool NoahConfigRead::Run(const BaseConfig& config, KafkaData* kafka_data) {
    return data_core_operate(config, type_map_operate[config.filter_id], kafka_data);
}
