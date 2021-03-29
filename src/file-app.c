#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

long long summ = 0;

int dirwalk(char *dir, void (*fcn)(char *)) {
	char name[260];
	struct dirent *dp;
	DIR *dfd;

	if ((dfd = opendir(dir)) == NULL) {
	    fprintf(stderr, "Ошибка открытия папки %s\n", dir);
	    return -1;
	}
	while ((dp = readdir(dfd)) != NULL) {
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
			continue;
		}

		sprintf(name, "%s/%s", dir, dp->d_name);

		(*fcn) (name);
	}
	closedir(dfd);
	return 0;
}

void fsize(char *name) {
    struct stat stbuf;

    if (stat(name, &stbuf) == -1) {
        fprintf(stderr, "Нет доступа к %s\n", name);
        return;
    }
    if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
        dirwalk(name, fsize);
    }
    if ((stbuf.st_mode & S_IFMT) == S_IFREG) {
    	printf("%5ld %s\n", stbuf.st_size, name);
    	summ += stbuf.st_size;
    }
}


int get_file_size(char *filename) {
	int fp = open(filename, O_RDONLY);
	if (fp == -1) {
		printf("Ошибка открытия файла: %s\n", filename);
		return -1;
	}

	struct stat stbuf;

	stat(filename, &stbuf);

	printf("Размер %s: %ld байт\n", filename, stbuf.st_size);

	return 0;
}

int copy_file(int fp, char* copy_name) {
	int fp_copy = creat(copy_name, __S_IWRITE | __S_IREAD);
	if (fp_copy == -1) {
		printf("Ошибка открытия файла: %s\n", copy_name);
		return -1;
	}
	char buf[100];
	int n;
	while ((n = read(fp, buf, sizeof(buf))) != 0) {
		write(fp_copy, buf, n);
	}
	close(fp_copy);
	return 0;
}


int main(int argc, char *argv[]) {
	
	if (argv[1] == NULL) {
		printf("-h, --help - справка\n");
		return 0;
	}

	if (strcmp(argv[1], "-h\0") == 0 || strcmp(argv[1], "--help\0") == 0) {
		printf("-----Управление файловой системой-----\n");
		printf("Приложение позволяет совершать основные операции над файлами, вывести на экране все процессы из файловой системы procfs.\n");
		printf("-h,--help - справка [./main -h | --help]\n");
		printf("-c, --copy - копирование файла [./main -c | --copy path_to_filename]\n");
		printf("-r, --remove - удаление файла [./main -r | --remove path_to_filename]\n");
		printf("-m, --move - перемещение файла [./main -m | --move filename move_path]\n");
		printf("-s, --size - размер файла или директории [./main -s | --size filename-or-dirname]\n");
		printf("-v, --view -  отображение всех файлов в указанной директории [./main -v | --view dirname]\n");
		printf("-p, --proc - отображение всех процессов из файловой системы procfs [./main -p | --proc]\n");

		printf("Примеры запуска:\n");
		printf("./main --proc\n");
		printf("./main --remove myfile.dat\n");
		printf("./main --move myfile.dat /home/username/\n");

		return 0;
	}

	if (strcmp(argv[1], "-c\0") == 0 || strcmp(argv[1], "--copy\0") == 0) {

		int fp = open(argv[2], O_RDONLY);
		if (fp == -1) {
			printf("Ошибка открытия файла: %s\n", argv[2]);
			return -1;
		}
		int buf_size = strlen(argv[2]) + 6;
		char *copy_name = (char *)malloc(sizeof(char) * buf_size);
		strcpy(copy_name, "copy_\0");
		strncat(copy_name, argv[2], strlen(argv[2]));
		
		if (copy_file(fp, copy_name) == -1) {
			printf("Ошибка чтения из файла\n");
			return -1;
		}

		free(copy_name);
		close(fp);
		printf("Копирование успешно завершено.\n");
		return 0;
	}

	if (strcmp(argv[1], "-r\0") == 0 || strcmp(argv[1], "--remove\0") == 0) {
		if (remove(argv[2]) == -1) {
			printf("Ошибка удаления файла %s\n", argv[2]);
		}
		printf("Удаление завершено.\n");
		return 0;
	}

	if (strcmp(argv[1], "-m\0") == 0 || strcmp(argv[1], "--move\0") == 0) {
		
		if (argv[2] == NULL) {
			printf("./main -m | --move filename move_path\n");
			return -1;
		}

		int fp = open(argv[2], O_RDONLY);
		if (fp == -1) {
			printf("Ошибка открытия файла: %s\n", argv[2]);
			return -1;
		}

		strncat(argv[3], argv[2], strlen(argv[2]));
		printf("%s\n", argv[3]);

		if (copy_file(fp, argv[3]) == -1) {
			printf("Ошибка чтения из файла\n");
			return -1;
		}

		if (remove(argv[2]) == -1) {
			printf("Ошибка удаления файла %s\n", argv[2]);
			return -1;
		}

		close(fp);
		printf("Перемещение успешно завершено\n");
		return 0;
	}

	if (strcmp(argv[1], "-s\0") == 0 || strcmp(argv[1], "--size\0") == 0) {
		struct stat stbuf;

		stat(argv[2], &stbuf);

		if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
			fsize(argv[2]);
			printf("Всего: %lld байт\n", summ);
		} else {
			get_file_size(argv[2]);
		}

    	return 0;
	}

	if (strcmp(argv[1], "-v\0") == 0 || strcmp(argv[1], "--view\0") == 0) {
		if (argv[2] == NULL) {
			printf("./main -v | --view dirname\n");
			return -1;
		}
		struct dirent *pDirent;
		DIR *pDir = opendir(argv[2]);
		if (pDir == NULL) {
			printf("Ошибка открытия папки %s\n", argv[2]);
			return -1;
		}
		while ((pDirent = readdir(pDir)) != NULL) {
			printf("[%s]\n", pDirent->d_name);
		}
		closedir(pDir);
	}


	if (strcmp(argv[1], "-p\0") == 0 || strcmp(argv[1], "--proc\0") == 0) {

		struct dirent *pDirent;
		char fold_name[] = "/proc\0";
		DIR *pDir = opendir(fold_name);
		if (pDir == NULL) {
			printf("Ошибка открытия папки %s\n", fold_name);
			return -1;
		}
		char status_path[32];
		char buf;
		char *proc_name = (char *)malloc(sizeof(char) * 32);

		while ((pDirent = readdir(pDir)) != NULL) {
			if (isdigit(pDirent->d_name[0])) {
				strcpy(status_path, "/proc/");
				strncat(status_path, pDirent->d_name, strlen(pDirent->d_name));
				strncat(status_path, "/status\0", 8);

				int fp = open(status_path, O_RDONLY);
				if (fp == -1) {
					printf("Ошибка открытия файла: %s\n", status_path);
					continue;
				}
				int i;
				buf = '\0';
				for (i = 0; buf != '\n'; ++i) {
					read(fp, &buf, 1);
					proc_name[i] = buf;
				}
				proc_name[i - 1] = '\0';

				printf("%s --- PID: %s\n", proc_name, pDirent->d_name);
				close(fp);
			}
		}
		free(proc_name);
		closedir(pDir);
	}

	printf("unknown command\n");
	return 0;
}