
CC=gcc
CFLAGS=-pthread
CLIBS=-I/usr/include/mysql
OBJ=bittwistb.o pktcheck.o listControl.o pi2srv.o
OUT=rss

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(CLIBS)

$(OUT): $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean :
	rm $(OBJ) $(OUT)
