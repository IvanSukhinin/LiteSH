#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <sys/types.h>


void simple_daemon() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    pid = fork();
    
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    openlog("daemonlog", LOG_PID, LOG_DAEMON);
}

int main(int argc, char *argv[]) {

    simple_daemon();

    syslog(LOG_NOTICE, "daemon works.");

    char* conf[] = {argv[1], argv[2], argv[3], NULL };

    execvp(conf[0], conf);

    syslog(LOG_NOTICE, "daemon terminated.");

    closelog();
    
    return 0;
}