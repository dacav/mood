AM_CPPFLAGS = -Wall -Werror -pedantic -std=c11 -g

bin_PROGRAMS = main funny

main_SOURCES = main.c
main_CFLAGS = \
	$(libevent_CFLAGS) \
	-I$(top_srcdir)/libmoodio/include
main_LDADD = \
	$(top_srcdir)/libmoodio/libmoodio.a \
	$(libevent_LIBS)

funny_SOURCES = funny.c
funny_CFLAGS = \
	-I$(top_srcdir)/libmoodgoph/include
funny_LDADD = \
	$(top_srcdir)/libmoodgoph/libmoodgoph.a
