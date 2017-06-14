
CC=gcc
CFLAGS=-pthread
CLIBS=-I/usr/include/mysql
OBJ=bittwistb.o pktcheck.o
OUT=rss

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(CLIBS)

$(OUT): $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

listCtrl: listControl.c
	gcc -o listCtrl listControl.c -I /usr/include/mysql -l mysqlclient -L /usr/lib

clean :
	rm $(OBJ) $(OUT)
