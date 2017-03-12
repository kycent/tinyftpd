cc=gcc
CFLAG=-lpthread
EXE_NAME=tinyftpd
all: 
	$(cc) -c *.c
	$(cc) *.o -o $(EXE_NAME) $(CFLAG) 
.PHONY: clean
clean:
	rm $(EXE_NAME)
	rm *.o 
