AM_CFLAGS = \
	-Wall -Werror -pedantic -std=c11 \
	-Iinclude \
	$(libevent_CFLAGS)

noinst_LIBRARIES = libmoodio.a
libmoodio_a_SOURCES = \
	server.c \
	session.c \
	setup.c \
	sigwrap.c \
	tcpsock.c \
	include/libmoodio/setup.h \
	include/libmoodio/session.h \
	include/libmoodio/tcpsock.h \
	include/libmoodio/sigwrap.h \
	include/libmoodio/server.h
