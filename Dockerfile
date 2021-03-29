FROM ubuntu:bionic

RUN apt-get update && apt-get install -y apt-utils 

RUN apt-get update && apt-get install -y make gcc

WORKDIR /app/

EXPOSE 2019

ADD ./ /app

RUN make

ENTRYPOINT ./bin/server
