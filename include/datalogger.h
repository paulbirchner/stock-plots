#ifndef DATALOGGER_H
#define DATALOGGER_H

#include "parse_json.h"

int log_stock_data(const char *symbol, const api_data data, const char *log_file_path);

#endif