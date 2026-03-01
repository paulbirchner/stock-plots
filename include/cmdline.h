#ifndef CMDLINE_H
#define CMDLINE_H

#include <stdbool.h>

struct arguments{
	char *add_symbol;
	char *delete_symbol;
	char *add_category;
	char *delete_category;
	char *api_key;
	char *category;
};

int parse_commandline(int argc, char **argv, struct arguments *arguments);

#endif