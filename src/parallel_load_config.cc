#include <pthread.h>

#include "userquery.h"

using namespace std;

void* parallel_load_config(void* arg) {
    ParallelLoadConfig& parallel_conf = *(ParallelLoadConfig*)arg;

    unordered_map<long, vector<TimeRange>> base_vec;
    if (!LoadRangeOriginConfig("./data/"+parallel_conf.file_name+".txt", &base_vec)) {
        pthread_mutex_lock(&(parallel_conf.mutex));
        ofstream ofile;
        ofile.open("conf/parallel_load_config.txt", ios::app);
        ofile << "false";
        ofile.close();
        pthread_mutex_unlock(&(parallel_conf.mutex));
        return NULL;
    }

    parallel_conf.time_range_origin.insert(make_pair(parallel_conf.field_name, base_vec));

    return NULL;
}
