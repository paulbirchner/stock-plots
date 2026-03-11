#include "config.h"

#include <cJSON.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>

char *get_file_path(const char *filename){

    const char *home_dir = getenv("HOME");

    if(!home_dir){
        struct passwd *pw = getpwuid(getuid());
        home_dir = pw->pw_dir;
    }

	int length = snprintf(NULL, 0, "%s/stock-plots", home_dir);
	char *folder_path = malloc(length + 1);
    snprintf(folder_path, length+1, "%s/stock-plots", home_dir);

    struct stat st = {0};
    if(stat(folder_path, &st) == -1){
        mkdir(folder_path, 0700); //lesen/schreiben
    }

	length = 0;

	length = snprintf(NULL, 0, "%s/%s", folder_path, filename);
    char *full_path = malloc(length + 1);
    snprintf(full_path, length+1, "%s/%s", folder_path, filename);
	free(folder_path);

    return full_path;
}


//speichert dateiinhalt in String
char *read_file(const char *file_path) {

	FILE *f = fopen(file_path, "r");
	if(!f) return NULL;
	fseek(f, 0, SEEK_END);
	long length = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *content = malloc(length + 1);
	if(content) {
		fread(content, length, 1, f);
		content[length] = 0;
	}
	fclose(f);
	return content;
}

//speichert neuen String in datei
int write_to_file(const char *file_path, const char *content) {

	FILE *f = fopen(file_path, "w");

	if(!f) {

		fprintf(stderr, "Failed to open file %s\n", file_path);
		perror(""); // gibt Systemfehler aus

		return -1;
	}

	fputs(content, f);
	fclose(f);
	return 0;
}


int add_stock(const char *symbol, const char *config_file_path, const char *category) {

	char *content = read_file(config_file_path);
	cJSON *root = NULL;

	if(content){
		root = cJSON_Parse(content);
		free(content);
	}
	if(!root){
		root = cJSON_CreateObject();
	}

	cJSON *categories = cJSON_GetObjectItem(root, "categories");
	if(!categories) {
		categories = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "categories", categories);
	}

	// keine kategorie -> default auf watchlist
	const char *target_category;
	if (!category) {
		target_category = "watchlist";
	} else {
		target_category = category;
	}

	cJSON *category_array = cJSON_GetObjectItem(categories, target_category);
	if (!category_array) {
		//Kategorie existiert nocht nicht -> neu erstellen
		category_array = cJSON_CreateArray();
		cJSON_AddItemToObject(categories, target_category, category_array);
	}

	//Prüfen ob Aktie bereits existiert
	int exists = 0;
	cJSON *item = NULL;
	cJSON_ArrayForEach(item, category_array) {
		if(cJSON_IsString(item) && strcmp(item->valuestring, symbol) == 0) {
			exists = 1;
			break;
		}
	}

	if (!exists) {
		cJSON_AddItemToArray(category_array, cJSON_CreateString(symbol));
		printf("Added '%s' to '%s\n", symbol, target_category);
	} else {
		printf("%s already exists\n", symbol);
	}

	char *new_json = cJSON_Print(root);
	if(write_to_file(config_file_path, new_json) != 0) {
		fprintf(stderr, "Error writing to config-file");
	}

	free(new_json);
	cJSON_Delete(root);

	return 0;
}

int delete_stock(const char *symbol, const char *config_file_path) {

	char *content = read_file(config_file_path);
	cJSON *root = NULL;

	if(content) {
		root = cJSON_Parse(content);
		free(content);
	}
	if(!root) {
		fprintf(stderr, "No config file found\n");
		return 0;
	}

	cJSON *categories = cJSON_GetObjectItem(root, "categories");
	if(!categories) {
		printf("No categories found\n");
		cJSON_Delete(root);
		return 0;
	}

	cJSON *current_category = NULL;
	//Loop durch alle Kategorien
	cJSON_ArrayForEach(current_category, categories) {

		cJSON *item = NULL;
		int delete_index = -1;
		int i = 0;
		//Loop durch Ketegorie Array
		cJSON_ArrayForEach(item, current_category) {
			if (cJSON_IsString(item) && strcmp(item->valuestring, symbol) == 0) {
				delete_index = i;
			}
			i++;
		}

		if (delete_index != -1) {
			cJSON_DeleteItemFromArray(current_category, delete_index);
			printf("%s deleted\n", symbol);
		}
	}

	char *new_json = cJSON_Print(root);
	if(write_to_file(config_file_path, new_json) != 0) {
		fprintf(stderr, "Error writing to config-file");
	}

	free(new_json);
	cJSON_Delete(root);

	return 0;
}


char *get_time_str_length(struct tm *tm, const char *format) {

	size_t size = 1;
	char *str_length = malloc(size);

	if (!str_length) return NULL;

	while (1) {

		size_t written = strftime(str_length, size, format, tm);

		if (written>0) {
			break;
		}

		size++;
		char *temp = realloc(str_length, size);
		if (!temp) {
			free(str_length);
			return NULL;
		}
		str_length = temp;
	}

	return str_length; //muss freigegeben werden!
}

int add_api_key(const char *api_key, const char *config_file_path) {

	char *content = read_file(config_file_path);
	cJSON *root = NULL;

	if(content) {
		root = cJSON_Parse(content);
		free(content);
	}
	if(!root) {
		root = cJSON_CreateObject();
	}

	cJSON *key = cJSON_GetObjectItem(root, "api_key");
	if(key) {
		cJSON_SetValuestring(key, api_key); //alten key ersetzen
	} else {
		cJSON_AddStringToObject(root, "api_key", api_key);
	}

	char *new_json = cJSON_Print(root);
	if(write_to_file(config_file_path, new_json) != 0) {
		fprintf(stderr, "Error writing to config-file\n");
		free(new_json);
		cJSON_Delete(root);
		return -1;
	}

	printf("API Key updated successfully\n");

	free(new_json);
	cJSON_Delete(root);

	return 0;
}

int add_category(const char *category, const char *config_file_path) {

	char *content = read_file(config_file_path);
	cJSON *root = NULL;

	if(content) {
		root = cJSON_Parse(content);
		free(content);
	}
	if(!root) {
		root = cJSON_CreateObject();
	}

	cJSON *categories = cJSON_GetObjectItem(root, "categories");
	if(!categories) {
		categories = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "categories", categories);
	}

	if (cJSON_HasObjectItem(categories, category)) {
		printf("Category '%s' already exists\n", category);
	} else {
		cJSON_AddItemToObject(categories, category, cJSON_CreateArray());
		printf("Category '%s' added\n", category);
	}

	char *new_json = cJSON_Print(root);
	if(write_to_file(config_file_path, new_json) != 0) {
		fprintf(stderr, "Error writing to config-file");
	}

	free(new_json);
	cJSON_Delete(root);
	return 0;
}

int delete_category(const char *category, const char *config_file_path) {

	char* content = read_file(config_file_path);
	cJSON *root = NULL;

	if(content) {
		root = cJSON_Parse(content);
		free(content);
	}

	if(!root) {
		fprintf(stderr, "Config file invalid\n");
		return -1;
	}

	cJSON *categories = cJSON_GetObjectItem(root, "categories");
	if(!categories) {
		printf("No categories found\n");
		cJSON_Delete(root);
		return 0;
	}

	if(cJSON_HasObjectItem(categories, category)) {
		cJSON_DeleteItemFromObject(categories, category);
		printf("Category '%s' deleted\n", category);

		char *new_json = cJSON_Print(root);
		if(write_to_file(config_file_path, new_json) != 0) {
			fprintf(stderr, "Error writing to config-file\n");
		}
		free(new_json);
	}else {
		printf("Category '%s' not found\n", category);
	}

	cJSON_Delete(root);
	return 0;
}


char *get_api_key(const char *config_file_path) {

	char *content = read_file(config_file_path);
	cJSON *root = NULL;

	if(content) {
		root = cJSON_Parse(content);
		free(content);
	}
	if(!root) {
		syslog(LOG_ERR, "Config file empty or invalid");
		return NULL;
	}

	cJSON *api_key = cJSON_GetObjectItem(root, "api_key");

	if(!api_key || !cJSON_IsString(api_key) || api_key->valuestring == NULL) {
		syslog(LOG_ERR, "No API Key found");
		cJSON_Delete(root);
		return NULL;
	}

	char *key = strdup(api_key->valuestring);

	cJSON_Delete(root);

	return key;
}

stock_config get_stock_config(const char *config_file_path) {

	stock_config config = {0};

	char *content = read_file(config_file_path);
	if(!content) return config;

	cJSON *root = cJSON_Parse(content);
	if(!root) {
		free(content);
		return config;
	}

	cJSON *categories = cJSON_GetObjectItem(root, "categories");
	if(categories && cJSON_IsObject(categories)) {

		int categories_count = cJSON_GetArraySize(categories);
		config.category_count = categories_count;

		if(categories_count>0) {
			config.categories = malloc(sizeof(stock_category) * categories_count);
		}

		int i=0;
		cJSON *current_category = NULL;

		cJSON_ArrayForEach(current_category, categories) {
			stock_category *category = &config.categories[i];

			category->category_name = strdup(current_category->string);

			int stock_count = cJSON_GetArraySize(current_category);
			category->symbol_count = stock_count;

			if (stock_count>0) {
				category->symbols = malloc(sizeof(char *) * stock_count);
			} else {
				category->symbols = NULL;
			}

			int j=0;
			cJSON *current_symbol = NULL;
			cJSON_ArrayForEach(current_symbol, current_category) {

				if (cJSON_IsString(current_symbol)) {
					category->symbols[j] = strdup(current_symbol->valuestring);
					j++;
				}
			}
			i++;
		}
	}

	cJSON_Delete(root);
	free(content);

	return config;
}

void free_stock_config(stock_config *config) {

	if (!config || config->categories) return;

	for (int i = 0; i < config->category_count; i++) {

		stock_category *current_category = &config->categories[i];

		if (current_category->category_name) free(current_category->category_name);

		if (current_category->symbols) {
			for (int j = 0; j < current_category->symbol_count; j++) {

				if (current_category->symbols[j]) free(current_category->symbols[j]);
			}
			free(current_category->symbols);
		}

	}
	free(config->categories);

	config->categories = NULL;
	config->category_count = 0;
}