#include "daemon.h"
#include "datalogger.h"
#include "parse_json.h"
#include "curl.h"
#include "config.h"

#include <signal.h>
#include <curl/curl.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

//löscht pid file
void shutdown_daemon(){
    curl_global_cleanup();
    syslog(LOG_INFO, "Daemon shutting down...");
    unlink(get_file_path(PID_FILE));
    closelog();
    exit(EXIT_SUCCESS);
}


void unleash_daemon(const char *pid_file_path, const char *config_file_path, const char *log_file_path){

    pid_t child_pid = fork();
    if(child_pid < 0) exit(EXIT_FAILURE);
    if(child_pid > 0) exit(EXIT_SUCCESS);

    if(setsid() < 0) exit(EXIT_FAILURE);

    child_pid = fork();
    if(child_pid < 0) exit(EXIT_FAILURE);
    if(child_pid > 0) exit(EXIT_SUCCESS);

    umask(0);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    curl_global_init(CURL_GLOBAL_ALL);

    openlog("stock_daemon", LOG_PID, LOG_DAEMON);

    signal(SIGTERM, shutdown_daemon); // kill

                             //Fail if file exists | Create file if file doesnt exist | write only
    int pid_fd = open(pid_file_path, O_EXCL | O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if(pid_fd < 0){
        syslog(LOG_ERR, "Unable to open pid file: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int length = snprintf(NULL, 0, "%d\n", getpid());
    char *pid_str = malloc(length + 1);
    snprintf(pid_str, length+1, "%d\n", getpid());

    if(write(pid_fd, pid_str, strlen(pid_str)) == -1){
        syslog(LOG_ERR, "Unable to write to pid file: %s", strerror(errno));
        close(pid_fd);
        free(pid_str);
        exit(EXIT_FAILURE);
    }
    free(pid_str);
    close(pid_fd);

	char *api_key = get_api_key(config_file_path);

	if(!api_key){
		syslog(LOG_ERR, "No API key found. Aborting...");
		closelog();
		exit(EXIT_FAILURE);
	}

    syslog(LOG_INFO, "Successfully started Daemon");

    while(true){

        stock_config saved_stocks = get_stock_config(config_file_path);


        for(int i = 0; i < saved_stocks.category_count; i++){

			stock_category *category = &saved_stocks.categories[i];

			for(int j = 0; j < category->symbol_count; j++){

				char *symbol = category->symbols[j];

				int length = snprintf(NULL, 0, "https://finnhub.io/api/v1/quote?symbol=%s&token=%s", symbol, api_key);
           		char *url = malloc(length + 1);
            	snprintf(url, length+1, "https://finnhub.io/api/v1/quote?symbol=%s&token=%s", symbol, api_key);

				char *json_data = NULL;
				api_data response;

				json_data = fetch_api_data(url);

				if(!json_data){
					syslog(LOG_WARNING, "JSON content is NULL");
					free(url);
					continue;
				}

				response = parse_fin_api_response(json_data);
				if(response.timestamp>0){
					log_stock_data(symbol, response, log_file_path);
				} else {
					syslog(LOG_WARNING, "No data fetched");
					free(json_data);
					free(url);
					continue;
				}
				free(json_data);
				free(url);
			}
        }

        free_stock_config(&saved_stocks);
        sleep(60);
    }
}
