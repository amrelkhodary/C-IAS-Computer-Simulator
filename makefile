CC = gcc
DEBUG_FLAGS = -g -Wall -Werror

all: ./bin/ias

./bin/ias: ./bin/main.o ./bin/ias.o ./bin/program_loader.o ./bin/log.o
	${CC} ${DEBUG_FLAGS}  $^ -o $@ -lm

./bin/main.o: ./src/main.c
	${CC} ${DEBUG_FLAGS}  -c $^ -o $@ -lm

./bin/ias.o: ./src/ias.c
	${CC} ${DEBUG_FLAGS}  -c $^ -o $@ -lm

./bin/program_loader.o: ./src/program_loader.c
	${CC} ${DEBUG_FLAGS}  -c $^ -o $@ -lm

./bin/log.o: ./src/log.c 
	${CC} ${DEBUG_FLAGS}  -c $^ -o $@ -lm

clean:
	rm ./bin/*

.PHONY:
	all clean