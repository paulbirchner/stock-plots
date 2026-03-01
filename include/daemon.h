#ifndef DAEMON_H
#define DAEMON_H


void unleash_daemon(const char *pid_file_path, const char *config_file_path, const char *log_file_path);
void shutdown_daemon();

#endif