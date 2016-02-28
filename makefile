OBJS = main_pgsh.o pgsh.o processes.o pg_error.o pg_file.o pg_string.o pg_stdlib.o getline.o
DEBUG =
CFLAGS = -c $(DEBUG)
LFLAGS = 

pgsh : $(OBJS)
	gcc $(LFLAGS) $(OBJS) -o pgsh

main_pgsh.o : main_pgsh.c pg_error.h pgsh.h
	gcc $(CFLAGS) main_pgsh.c

pg_file.o : pg_file.c pg_file.h pg_error.h
	gcc $(CFLAGS) pg_file.c

processes.o : processes.c processes.h pg_error.h pg_file.h pg_string.h
	gcc $(CFLAGS) processes.c

pg_error.o : pg_error.c pg_error.h
	gcc $(CFLAGS) pg_error.c

pg_string.o	: pg_string.c pg_string.h pg_error.h
	gcc $(CFLAGS) pg_string.c

pgsh.o : pgsh.c processes.h pg_error.h pg_string.h pgsh.h
	gcc $(CFLAGS) pgsh.c

pg_stdlib.o : pg_stdlib.c pg_stdlib.h
	gcc $(CFLAGS) pg_stdlib.c

getline.o : lib/getline.c lib/getline.h
	gcc $(CFLAGS) lib/getline.c
	
clean :
	\rm *.o pgsh

clean.o :
	\rm *.o

all : pgsh

tar :
	tar cfv pgsh.tar *.c *.h makefile README.txt lib/*
	
tgz :
	tar czfv pgsh.tgz *.c *.h makefile README.txt lib/*
