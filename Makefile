CC=gcc
CFLAGS = -g 
# uncomment this for SunOS
# LIBS = -lsocket -lnsl

all: transmitter receiver

transmitter: transmitter.o 
	$(CC) -o transmitter transmitter.o $(LIBS)

receiver: receiver.o 
	$(CC) -o receiver receiver.o $(LIBS)

transmitter.o: transmitter.c port.h

receiver.o: receiver.c port.h

clean:
	rm -f transmitter receiver transmitter.o receiver.o 
