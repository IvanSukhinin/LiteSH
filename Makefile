all:
	gcc src/rpc_client.c -o bin/rpc_client
	gcc src/daemon.c -o bin/daemon
	gcc src/file-app.c -o bin/file-app

	gcc src/systemlog.c -c -fPIC -o build/systemlog.o
	gcc -shared -o libserverlog.so build/systemlog.o

	gcc -c src/server.c -o build/server.o
	gcc build/server.o -L. -lserverlog -ldl -pthread -o bin/server