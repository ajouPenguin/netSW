listCtrl : listControl.c
	gcc -o listCtrl listControl.c -I /usr/include/mysql -l mysqlclient -L /usr/lib

clean : 
	rm *.o listCtrl
