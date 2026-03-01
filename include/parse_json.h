#ifndef PARSE_JSON_H
#define PARSE_JSON_H

#include "config.h"

#include <stdio.h>
#include <time.h>

#define INITIAL_CAPACITY 64;

api_data parse_fin_api_response(char *json_data);
candle_series resample_stock_data(char *json_data, const char *log_file_path, int interval, long range);
int get_interval(timerange range);
long get_cutoff(timerange range);

#endif