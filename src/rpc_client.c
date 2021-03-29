#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int main (int argc, char *argv[]) {

	if (argc < 2) {
		return 1;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(2019);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	connect(sock, (struct sockaddr*)&server, sizeof(server)); 

	char buf[4][20];

	strcpy(buf[0], argv[1]);

	strcpy(buf[1], "\0");
	strcpy(buf[2], "\0");
	strcpy(buf[3], "\0");

	if (argc > 2) {
		if (argv[2] != NULL) {
			strcpy(buf[1], argv[2]);
		}
	}

	if (argc > 3) {
		if (argv[3] != NULL) {
			strcpy(buf[2], argv[3]);
		}
	}

	if (argc > 4) {
		if (argv[4] != NULL) {
			strcpy(buf[3], argv[4]);
		}
	}

	send(sock, buf, sizeof(buf), 0);
	char recv_buf[2000];

	recv(sock, recv_buf, sizeof(recv_buf), 0);
	printf("%s\n", recv_buf);

	close(sock);

	return 0;
}