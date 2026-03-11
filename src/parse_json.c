#include "plotter.h"
#include "config.h"
#include "parse_json.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cJSON.h>

api_data parse_fin_api_response(char *json_data){

    api_data data = {0};
    cJSON *root = cJSON_Parse(json_data);

    if(!root){
        return data;
    }

    cJSON *price_json = cJSON_GetObjectItemCaseSensitive(root, "c");
    if(cJSON_IsNumber(price_json)) data.price = price_json->valuedouble;

    cJSON *time_json = cJSON_GetObjectItemCaseSensitive(root, "t");
    if(cJSON_IsNumber(time_json)) data.timestamp = (time_t)time_json->valuedouble;

    cJSON_Delete(root);

    return data;
}


candle_series resample_stock_data(char *symbol, const char *log_file_path, int interval, long range){

	time_t start_time = time(NULL)-range; // range von aktueller zeit abziehen und timestamp damit vergleichen
	candle_series series = {0};

	char *json_data = read_file(log_file_path);
	if(!json_data){
		fprintf(stderr, "ERROR: could not read file '%s'\n", log_file_path);
		return series;
	}

	cJSON *root = cJSON_Parse(json_data);
	if(!root){
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr){
			fprintf(stderr, "cJSON_Parse Error before: ...%.20s...\n", error_ptr);
		}
		fprintf(stderr, "ERROR: could not parse file '%s'\n", log_file_path);
		free(json_data);
		return series;
	}

	cJSON *stock_array = cJSON_GetObjectItemCaseSensitive(root, symbol);
	if(!stock_array || !cJSON_IsArray(stock_array)){
		fprintf(stderr, "ERROR: Symbol '%s' not found in JSON or is not array\n", symbol);
		cJSON_Delete(root);
		free(json_data);
		return series;
	}

	int array_size = cJSON_GetArraySize(stock_array);
	series.capacity = INITIAL_CAPACITY;
	series.data = malloc(series.capacity * sizeof(candle));

	if(!series.data){
		free(json_data);
		cJSON_Delete(root);
		return series;
	}

	int candle_index = -1;
	long current_bucket = -1;

	cJSON *item = NULL;
	cJSON_ArrayForEach(item, stock_array){

		cJSON *timestamp_json = cJSON_GetObjectItem(item, "timestamp");
		cJSON *price_json = cJSON_GetObjectItem(item, "price");

		if(!cJSON_IsNumber(timestamp_json) || !cJSON_IsNumber(price_json)) continue;
		//GetObjectItem gibt NULL zurück falls object nicht existiert => seg fault verhindern
		//zum nächsten datenpunkt gehen

		long long ts = (long long) timestamp_json->valuedouble;
		double price = price_json->valuedouble;

		if(ts < start_time) continue; //überspringe alles bis zum ersten ts innerhalb der range

		long bucket = ts / interval; //Daten in buckets einteilen

		if(bucket == current_bucket && candle_index >= 0){ //gleiche Kerze
			if(price > series.data[candle_index].h) series.data[candle_index].h = price;
			if(price < series.data[candle_index].l) series.data[candle_index].l = price;
			series.data[candle_index].c = price;
		} else { //neue Kerze
			candle_index++;
			if(candle_index >= series.capacity){
				int new_capacity = series.capacity * 2;
				//temp pointer falls realloc failed
				candle *temp = realloc(series.data, new_capacity * sizeof(candle));

				if(!temp){
					fprintf(stderr, "realloc failed\n");
					break;
				}
				series.data = temp;
				series.capacity = new_capacity;
			}
		current_bucket = bucket;
		series.data[candle_index].timestamp = bucket*interval;
		series.data[candle_index].o = price;
		series.data[candle_index].h = price;
		series.data[candle_index].l = price;
		series.data[candle_index].c = price;
		}
	}
series.count = candle_index+1;

if(series.count > 0 && series.count < series.capacity){
	candle *mem = realloc(series.data, series.count * sizeof(candle));
	if(mem){
		series.data = mem;
		series.capacity = series.count;
	}
}

cJSON_Delete(root);
free(json_data);

return series;
}


int get_interval(timerange range){

	switch(range){
		case RANGE_1D:
			return 300; //5min
			break;
		case RANGE_1W:
			return 1500; //25min
			break;
		case RANGE_1M:
			return 6000; //100min
		case RANGE_1Y:
			return 86400; //1d
			break;
	default:
		return 300;
		break;
	}
}

long get_cutoff(timerange range){

	switch(range){
		case RANGE_1D:
			return 86400;
			break;
		case RANGE_1W:
			return 604800;
			break;
		case RANGE_1M:
			return 2629743;
		case RANGE_1Y:
			return 31556926;
			break;
	default:
		return 86400;
		break;
	}
}