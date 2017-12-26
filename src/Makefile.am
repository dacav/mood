AM_CPPFLAGS = \
	-Wall -Werror -pedantic -std=c11 \
	-I$(top_srcdir)/libmoodio/include

LDADD = $(top_srcdir)/libmoodio/libmoodio.a $(libevent_LIBS)
AM_CFLAGS = $(libevent_CFLAGS)

bin_PROGRAMS = test
test_SOURCES = main.c
