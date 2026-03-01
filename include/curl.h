#ifndef CURL_H
#define CURL_H

#include <stdlib.h>

struct memory{
    char *response;
    size_t size;
};

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
char *fetch_api_data(const char* url);

#endif