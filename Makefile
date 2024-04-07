#Connor Tamme lrk312 11328286
#Mark Jia mij623 11271998


.PHONY: clean

CC = gcc
CFLAGS = -g
CPPFLAGS = -std=gnu90 -Wall -Wextra -pedantic



all: A3

A3: A3.o
	${CC} ${CFLAGS} -o A3 A3.o -L./ -lpthread
A3.o: A3.c protocol.h
	${CC} ${CPPFLAGS} ${CFLAGS} -o A3.o -c A3.c -I./


clean:
	rm -f *.o *.a A3
