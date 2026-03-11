#include "curl.h"

#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>



size_t write_callback(char *contents, size_t size, size_t nmemb, void *userdata){

    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory*)userdata;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if(ptr == NULL){
        return 0;
    }
    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

return realsize;
}

char *fetch_api_data(const char *url){

    CURL *curl_handle = curl_easy_init();
    CURLcode res;

    struct memory chunk;
    chunk.response = malloc(1);
    chunk.size = 0;

    if(!curl_handle){
        curl_easy_cleanup(curl_handle);
        return NULL;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url); //setze url
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback); //rufe callback auf, nicht console
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk); // übergib daten an chunk struct

    res = curl_easy_perform(curl_handle);

        if(res != CURLE_OK){
            free(chunk.response);
            curl_easy_cleanup(curl_handle);
            return NULL;
        }

    curl_easy_cleanup(curl_handle);

    return chunk.response;
}