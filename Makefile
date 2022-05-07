# Autores : Rafael Dominguez Saez y Laura M Garcia Suarez
# NOTA: para ejecutar los programas que empiezan por Ejercicio basta con
# poner "make <ejercicio que se quiera ejecutar>" y "./<ejercicio que se 
# quiera ejecutar>"
#

CC=gcc -pthread -g
CFLAGS=-I.	-lrt
DEPS=pow.h miner.h register.h comprobador.h monitor.h

all: main_miner

%.o: %.c $(DEPS)
	$(CC)	-c	-o	$@	$<	$(CFLAGS)

#monitor: main.o monitor.o	pow.o	comprobador.o
#	$(CC)	-o	monitor	main.o	monitor.o	comprobador.o	pow.o $(CFLAGS)

miner: miner.o pow.o
	$(CC)	-o	mineraux	miner.o	pow.o	$(CFLAGS)

register: register.o miner.o
	$(CC)	-o	register	miner.o	register.o	$(CFLAGS)

main_miner: register.o miner.o main_miner.o pow.o
	$(CC)	-o	miner	miner.o register.o main_miner.o pow.o	$(CFLAGS)

compress:
	zip ../GR2262_04_3.zip -r ../

clean:
	rm *.o 
