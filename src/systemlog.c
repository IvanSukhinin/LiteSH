#include <syslog.h>

void server_log(char *message) {
	openlog("localhost_log", LOG_PID, LOG_DAEMON);

	syslog(LOG_NOTICE, "%s", message);

    closelog();
}