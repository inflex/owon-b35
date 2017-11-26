# 
# VERSION CHANGES
#

LOCATION=/usr/local
CFLAGS=-Wall -O2 
LIBS=

OBJ=owoncli
OFILES=
default: owoncli

.c.o:
	${CC} ${CFLAGS} $(COMPONENTS) -c $*.c

all: ${OBJ} 

owoncli: ${OFILES} owoncli.c 
#	ctags *.[ch]
#	clear
	${CC} ${CFLAGS} $(COMPONENTS) owoncli.c ${OFILES} -o owoncli ${LIBS}

install: ${OBJ}
	cp owoncli ${LOCATION}/bin/

clean:
	rm -f *.o *core ${OBJ}
