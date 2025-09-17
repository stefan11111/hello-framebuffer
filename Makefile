.POSIX:

XCFLAGS = ${CPPFLAGS} ${CFLAGS} -std=c99 -D_DEFAULT_SOURCE

XLDFLAGS = ${LDFLAGS}

OBJ = \
	fbdev.o \
	main.o

all: fbdev
	chown root:root fbdev
	chmod 4755 fbdev

.c.o:
	${CC} ${XCFLAGS} -c -o $@ $<

fbdev: ${OBJ}
	${CC} ${XCFLAGS} -o $@ ${OBJ} ${XLDFLAGS}

clean:
	rm -f fbdev ${OBJ}
