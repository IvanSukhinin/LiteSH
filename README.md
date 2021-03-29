[![Build Status](https://travis-ci.org/ssshazam/LiteSH.svg?branch=master)](https://travis-ci.org/ssshazam/LiteSH)
# LiteSH
### ----- Работа с процессами в ОС Linux -----
#### Команды:

- h, help - справка
- d, daemon - запустить процесс в фоновом режиме
- c, call - запустить процесс
- file-app - запустить управление файловой системой

#### Примеры использования:
```sh
$ ./rpc_client h
$ ./rpc_client file-app -h
$ ./rpc_client call /bin/pwd/
$ ./rpc_client daemon file-app -c test.txt
```
