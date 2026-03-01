#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FILE "config.json"
#define LOG_FILE "stock_logs.json"
#define PID_FILE "stock_daemon.pid"

#include <time.h>
#include <gtk/gtk.h>

typedef struct{
	char *category_name;
	char **symbols;
	int symbol_count;
}stock_category;

typedef struct {
	stock_category *categories;
	int category_count;
}stock_config;

typedef enum{
	RANGE_1D,
	RANGE_1W,
	RANGE_1M,
	RANGE_1Y,
}timerange;

typedef struct{
	double price;
	time_t timestamp;
}api_data;

typedef struct{
	time_t timestamp;
	double o;
	double h;
	double l;
	double c;
}candle;

typedef struct{
	candle *data;
	int count;
	int capacity; //für wieviele kerzen wurde speicher reserviert
}candle_series;

typedef struct {
	GtkWidget *flow_box;
	GList *charts;
	char *config_path;
	char *log_path;
	timerange active_range;
}AppState;

typedef struct{
	char *symbol;
	candle_series data;
	GtkWidget *drawing_area;
	GtkWidget *container;
	AppState *state;
}stock_chart;


//Helper
char *get_file_path(const char *filename);
int write_to_file(const char *file_path, const char *content);
char *get_time_str_length(struct tm *tm, const char *format);
char *read_file(const char *file_path);


//Add/remove
int add_stock(const char *symbol, const char *config_file_path, const char *category);
int delete_stock(const char *symbol, const char *config_file_path);
int add_category(const char *category, const char *config_file_path);
int delete_category(const char *category, const char *config_file_path);
int add_api_key(const char *api_key, const char *config_file_path);

//Read Config
char *get_api_key(const char *config_file_path);
stock_config get_stock_config(const char *config_file_path);
void free_stock_config(stock_config *config);

#endif

