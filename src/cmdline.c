#include "cmdline.h"
#include "config.h"
#include "daemon.h"

#include <stdlib.h>
#include <argp.h>
#include <string.h>

const char* argp_program_version = "1.0";
const char *argp_program_bug_address = "paul.birchner@st.oth-regensburg.de";
static char doc[] = "program to view stocks";
static char args_doc[] = "";

struct argp_option options[] = {
    {"api-key", 'k', "API-KEY", 0, "Add your API key (replaces old api key, if already there)"},
	{"add", 'a', "ADD", 0, "Add stock symbol to config. Combine with -p to add to category if not will default to watchlist"},
	{"delete", 'd', "DELETE", 0, "Delete stock symbol"},
    {"log", 'l', 0, 0, "Start log daemon"},
	{"add-category", 'c', "ADD-CATEGORY", 0, "Add category"},
	{"delete-category", 'r', "DELETE-CATEGORY", 0, "Delete category"},
	{"category", 'p', "CATEGORY", 0, "category for add stock"},
    {0}
};

error_t parse_options(int key, char *arg, struct argp_state *state ) {

    struct arguments *arguments = state->input;
    char *log_file_path = NULL;
    char *pid_file_path = NULL;
	char *config_file_path = NULL;

    switch (key) {
        case 'k':
    		config_file_path = get_file_path(CONFIG_FILE);
            if(arguments->api_key) free(arguments->api_key);
            arguments->api_key = strdup(arg);
    		add_api_key(arguments->api_key, config_file_path);
            break;
		case 'a':
			config_file_path = get_file_path(CONFIG_FILE);
			if(arguments->add_symbol) free(arguments->add_symbol);
			arguments->add_symbol = strdup(arg);
			break;
		case 'd':
			config_file_path = get_file_path(CONFIG_FILE);
			if(arguments->delete_symbol) free(arguments->delete_symbol);
			arguments->delete_symbol = strdup(arg);
			delete_stock(arguments->delete_symbol, config_file_path);
			break;
    	case 'c':
    		config_file_path = get_file_path(CONFIG_FILE);
    		if(arguments->add_category) free(arguments->add_category);
    		arguments->add_category = strdup(arg);
    		add_category(arguments->add_category, config_file_path);
    		break;
		case 'r':
			config_file_path = get_file_path(CONFIG_FILE);
			if(arguments->delete_category) free(arguments->delete_category);
			arguments->delete_category = strdup(arg);
			delete_category(arguments->delete_category, config_file_path);
			break;
    	case 'p':
    		if (arguments->category) free(arguments->category);
    		arguments->category = strdup(arg);
    		break;
        case 'l':
			pid_file_path = get_file_path(PID_FILE);
			log_file_path = get_file_path(LOG_FILE);
			config_file_path = get_file_path(CONFIG_FILE);

            if(fopen(pid_file_path, "r") != NULL) {
                printf("Daemon already running\n");
            }else {
                unleash_daemon(pid_file_path, config_file_path, log_file_path);
            }
    		free(pid_file_path);
    		break;
        case ARGP_KEY_END:
    		if (arguments->add_symbol) {
    			config_file_path = get_file_path(CONFIG_FILE);
    			add_stock(arguments->add_symbol, config_file_path, arguments->category);
    		}
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    		break;
    }

	if (log_file_path) free(log_file_path);
	if (config_file_path) free(config_file_path);

    return 0;
}

static struct argp argp = { options, parse_options, args_doc, doc };

int parse_commandline(int argc, char **argv, struct arguments *arguments){
    arguments->api_key = NULL;
	arguments->add_symbol = NULL;
	arguments->delete_symbol = NULL;
	arguments->add_category = NULL;
	arguments->category = NULL;
return argp_parse(&argp, argc, argv, 0, 0, arguments);
}