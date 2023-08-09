
APPLICATION_NAME = enumPrinter
GIT_VERSION ?= $(shell git rev-parse HEAD | cut -c1-16)
GIT_CFLAGS += -DGIT_VERSION="\"$(GIT_VERSION)\"" -DAPPLICATION_NAME="\"$(APPLICATION_NAME)\""

INSTALL_DIR = /usr/bin

make: main.o
	g++ -o $(APPLICATION_NAME) main.o

main.o: main.cpp
	g++ -c -Wall $(GIT_CFLAGS) main.cpp

clean:
	rm *.o

install: clean make
	sudo cp $(APPLICATION_NAME) $(INSTALL_DIR)
