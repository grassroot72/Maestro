# $@ - left side of rule
# $^ - right side of rule
# $< - first prerequisite (usually the source file)

CC = gcc
#CFLAGS = -O0 -g -Wall -pedantic -Isvc -I.
CFLAGS = -O2 -Wall -pedantic -Isvc -I.
LDFLAGS = -lpthread
OBJS = base64.o \
       deflate.o \
       thpool.o \
       linkedlist.o \
       io.o \
       util.o \
       jsmn.o \
       http_msg.o \
       http_parser.o \
       http_cache.o \
       http_get.o \
       http_post.o \
       http_conn.o \
       svc/dml_obj.o \
       svc/identity.o \
       maestro.o
EXES = maestro

all: ${EXES}

${EXES}: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	${CC} -o $@ -c $< $(CFLAGS)

clean:
	$(RM) *.o svc/*.o $(EXES)
