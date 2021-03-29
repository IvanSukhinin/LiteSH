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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <sys/io.h>
#include <fcntl.h>


#define STDOUT 1

// cat /var/log/syslog | grep "localhost_log"

// Запись в файл и отправка клиенту.

void signalhandler(int signal) {
    char str[50] = "SIGUSR\0";
    switch (signal){
    case SIGUSR1:
        write(STDOUT, str, sizeof(str));
        break;
    case SIGUSR2:
        strcpy(str, "SIGUSR2\0");
        write(STDOUT, str, sizeof(str));
        break;
    case SIGINT:
        strcpy(str, "SIGINT\0");
        write(STDOUT, str, sizeof(str));
        exit(SIGINT);
    case SIGTERM:
        strcpy(str, "SIGTERM\0");
        write(STDOUT, str, sizeof(str));
        exit(SIGTERM);
    case SIGKILL:
        strcpy(str, "SIGKILL\0");
        write(STDOUT, str, sizeof(str));
        exit(SIGKILL);
    case SIGSTOP:
        strcpy(str, "SIGSTOP\0");
        write(STDOUT, str, sizeof(str));
        exit(SIGSTOP);
    }
}

int copy_file(int fp, char* copy_name);

int main(int argc, char *argv[]) {

    signal(SIGUSR1, signalhandler);
    signal(SIGUSR2, signalhandler);
    signal(SIGINT, signalhandler);
    signal(SIGTERM, signalhandler);
    signal(SIGKILL, signalhandler);
    signal(SIGSTOP, signalhandler);

    char *error;
    void *handle = dlopen("./libserverlog.so", RTLD_LAZY);
    if (!handle) {
        printf("%s\n", dlerror());
        exit(-1);
    }
    void (*mylog)(char *) = dlsym(handle, "server_log");
    if ((error = dlerror()) != NULL) {
        printf("%s\n", error);
        exit(-1);
    }


    char *cmd = (char *)malloc(sizeof(char) * 20);

    char *file_app_arg1 = (char *)malloc(sizeof(char) * 20);
    char *file_app_arg2 = (char *)malloc(sizeof(char) * 20);
    char *file_app_arg3 = (char *)malloc(sizeof(char) * 20);

    char glog[100];

    int status;

    struct sockaddr_in server, client;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int size;
    char buf[4][20];
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(2019);
    bind(sock, (struct sockaddr*)&server, sizeof(server));
    listen(sock, 5);


    int fd, oldstdout;
    while(1) {

        int newsock, clnlen;
        newsock = accept(sock, (struct sockaddr*)&client, &clnlen);

        write(STDOUT, " ", 1);

        strcpy(glog, inet_ntoa(client.sin_addr));

        strcat(glog, " : ");

        char check[20] = "ok\0";


        fd = creat("./data.dat", __S_IWRITE | __S_IREAD);
        oldstdout = dup(STDOUT);
        dup2(fd, STDOUT);
        close(fd);
        int ucmd_flag = 1;
        while ((size = recv(newsock, buf, sizeof(buf), 0)) != 0) {

            strcpy(cmd, buf[0]);
            strcpy(file_app_arg1, buf[1]);
            strcpy(file_app_arg2, buf[2]);
            strcpy(file_app_arg3, buf[3]);

            strcat(glog, cmd);
            strcat(glog, " ");
            strcat(glog, file_app_arg1);
            strcat(glog, " ");
            strcat(glog, file_app_arg2);
            strcat(glog, " ");
            strcat(glog, file_app_arg3);
            strcat(glog, " : ");

            if (strcmp(cmd, "h") == 0 || strcmp(cmd, "help") == 0) {
                ucmd_flag = 0;
                char str[] = "----- Работа с процессами в ОС Linux.-----\n\
h, help - справка\n\
d, daemon - запустить процесс в фоновом режиме\n\
c, call - запустить процесс\n\
file-app - запустить управление файловой системой\n\
Примеры использования:\n\
file-app -h\n\
call /bin/pwd/\n\
daemon file-app -c test.txt\0";

                dup2(oldstdout, STDOUT);  
                close(oldstdout);

                strcat(glog, check);
                (*mylog)(glog);

                send(newsock, str, sizeof(str), 0);
                continue;
            }

            if (strcmp(cmd, "file-app\0") == 0) {
                ucmd_flag = 0;
                char* conf[] = { "./bin/file-app", file_app_arg1, file_app_arg2, NULL };
                if (fork() == 0) {
                    execvp(conf[0], conf);
                    kill(getppid(), SIGUSR1);
                    strcpy(check, "SIGUSR1");
                    (*mylog)(glog);
                    return 1;
                } else {
                    waitpid(0, &status, 0);
                    if (status == 1) {
                        printf("что-то пошло не так..\n");
                        char msg[5] = "err\0";
                        strcat(glog, msg);
                        (*mylog)(glog);
                        send(newsock, msg, sizeof(msg), 0);
                        continue;
                    }
                }


                strcat(glog, check);

                dup2(oldstdout, STDOUT);

                char send_buf[2000];
                int n;
                fd = open("./data.dat", O_RDONLY); 
                n = read(fd, send_buf, 2000);
                close(fd);

                send_buf[n - 1] = '\0';

                send(newsock, send_buf, n, 0);
            
                close(oldstdout);
                (*mylog)(glog);
                continue;
            }


            if (strcmp(cmd, "c") == 0 || strcmp(cmd, "call") == 0) {
                ucmd_flag = 0;
                char* conf[] = { file_app_arg1, file_app_arg2, NULL };
                if (fork() == 0) {
                    execvp(conf[0], conf);
                    kill(getppid(), SIGUSR2);
                    return 1;
                } else {
                    waitpid(0, &status, 0);
                    if (status == 1) {
                        printf("что-то пошло не так..\n");
                        char msg[5] = "err\0";
                        strcat(glog, msg);
                        (*mylog)(glog);
                        send(newsock, msg, sizeof(msg), 0);
                        continue;
                    }
                
                }

                dup2(oldstdout,STDOUT);

                char send_buf[2000];
                int n;
                fd = open("./data.dat", O_RDONLY); 
                n = read(fd, send_buf, 2000);

                close(fd);

                if (n == 0) {
                    strcpy(send_buf, buf[1]);
                    strcat(send_buf, " не отправил статус завершения.\n");
                    n = sizeof(send_buf);
                }

                if (send_buf[0] == 'S' && send_buf[1] == 'I' && send_buf[2] == 'G') {
                    strcat(glog, send_buf);
                } else {
                    strcat(glog, "ok\0");
                }

                send_buf[n - 1] = '\0';

                send(newsock, send_buf, n, 0);


                close(oldstdout);
                (*mylog)(glog);
                
                continue;
            }

            if (strcmp(cmd, "d") == 0 || strcmp(cmd, "daemon") == 0) {
                ucmd_flag = 0;
                char* conf[] = { "./bin/daemon", file_app_arg1, file_app_arg2, file_app_arg3, NULL };
                write(STDOUT, "daemon works.\0", 14);
                if (fork() == 0) {
                    execvp(conf[0], conf);
                    kill(getppid(), SIGUSR1);
                    return 1;
                } else {
                    waitpid(0, &status, 0);
                    if (status == 1) {
                        printf("что-то пошло не так..\n");
                        char msg[5] = "err\0";
                        strcpy(check, msg);
                        strcat(glog, msg);
                        (*mylog)(glog);
                        send(newsock, msg, sizeof(msg), 0);
                        continue;
                    }
                }

                strcat(glog, "ok\0");
                (*mylog)(glog);

                dup2(oldstdout,STDOUT);

                write(STDOUT, "daemon terminated.\0", 20);

                char send_buf[2000];
                int n;
                fd = open("./data.dat", O_RDONLY);
                n = read(fd, send_buf, 2000);
                send_buf[n - 1] = '\0';
                close(fd);

                send(newsock, send_buf, n, 0);

            
                close(oldstdout);
                continue;
            }
            break;
        } // endwhile

        if (ucmd_flag) {
            char msg[] = "unknown command\0";
            strcat(glog, msg);
            (*mylog)(glog);
            send(newsock, msg, sizeof(msg), 0);
        }

        dup2(oldstdout, STDOUT);
        close(oldstdout);
    } // end inf loop



    free(cmd);
    free(file_app_arg1);
    free(file_app_arg2);
    free(file_app_arg3);

    return 0;
}
