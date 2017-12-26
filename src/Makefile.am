AM_CPPFLAGS = \
	-Wall -Werror -pedantic -std=c11 \
	-I$(top_srcdir)/libmood/include

LDADD = $(top_srcdir)/libmood/libmood.a $(libevent_LIBS)
AM_CFLAGS = $(libevent_CFLAGS)

bin_PROGRAMS = test
test_SOURCES = main.c
