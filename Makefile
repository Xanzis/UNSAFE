CC = gcc
RM = rm
CFLAGS  = -lm

VPATH = lib tests

all: unsafe-r tsts

unsafe-r: unsafe-r.c inutil-r.c matutil.c undefs.c visutil-2d.c
	$(CC) -o unsafe-r unsafe-r.c lib/inutil-r.c lib/matutil.c lib/undefs.c lib/visutil-2d.c $(CFLAGS)

tsts: tests.c matutil.c inutil-r.c
	$(CC) -o tsts tests/tests.c lib/matutil.c lib/inutil-r.c $(CFLAGS) 

clean:
	$(RM) unsafe-r
	$(RM) tsts