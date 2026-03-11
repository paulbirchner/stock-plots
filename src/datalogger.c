#include "datalogger.h"
#include "config.h"
#include "parse_json.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <cJSON.h>

static int is_market_open(int tm_wday, int tm_hour, int tm_min){
    //Sonntag = 0
    if(tm_wday == 0 || tm_wday == 6 ){
        return 0;
    }

    if(tm_hour < 15 || (tm_hour == 15 && tm_min <30)){
        return 0;
    }

    if(tm_hour >= 22){
        return 0;
    }

    return 1;
}

int log_stock_data(const char *symbol, const api_data data, const char *log_file_path){

    int market_open = 0; //closed
    struct tm *info = localtime(&data.timestamp);
    market_open = is_market_open(info->tm_wday, info->tm_hour, info->tm_min);

    if(market_open == 0){
        return 0;
    }

    char *content = read_file(log_file_path);
    cJSON *root = NULL;

    if(content){
        root = cJSON_Parse(content);
        free(content);
    }

    if(!root || !cJSON_IsObject(root)){
        if(root) cJSON_Delete(root);
        root = cJSON_CreateObject();
    }

    cJSON *symbol_array = cJSON_GetObjectItem(root, symbol);
    if(!symbol_array){
        //Falls Symbol noch nicht existiert, erstelle neues Array und füge es zum Object hinzu
        symbol_array = cJSON_CreateArray();
        cJSON_AddItemToObject(root, symbol, symbol_array);
    }

    cJSON *new_entry = cJSON_CreateObject();
    cJSON_AddItemToObject(new_entry, "timestamp",  cJSON_CreateNumber((double)data.timestamp));
    cJSON_AddItemToObject(new_entry, "price",  cJSON_CreateNumber((double)data.price));

    cJSON_AddItemToArray(symbol_array, new_entry);

    char *new_log_string = cJSON_PrintUnformatted(root);

    if(write_to_file(log_file_path, new_log_string)!=0){
        printf("Error writing to log file\n");
    }

    cJSON_Delete(root);
    free(new_log_string);

return 0;
}