# Autores : Rafael Dominguez Saez y Laura M Garcia Suarez
# NOTA: para ejecutar los programas que empiezan por Ejercicio basta con
# poner "make <ejercicio que se quiera ejecutar>" y "./<ejercicio que se 
# quiera ejecutar>"
#

CC=gcc -pthread -g
CFLAGS=-I.	-lrt
DEPS=pow.h miner.h register.h comprobador.h monitor.h mintocom.h

all: main_miner main_monitor

%.o: %.c $(DEPS)
	$(CC)	-c	-o	$@	$<	$(CFLAGS)

main_monitor: monitor.o comprobador.o main_monitor.o pow.o
	$(CC)	-o	monitor	monitor.o comprobador.o main_monitor.o pow.o $(CFLAGS)

main_miner: register.o miner.o main_miner.o pow.o
	$(CC)	-o	miner	miner.o register.o main_miner.o pow.o	$(CFLAGS)

compress:
	zip ../GR2262_04_3.zip -r ../

clean:
	rm *.o 
