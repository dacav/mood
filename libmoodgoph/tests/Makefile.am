AM_CFLAGS = \
	-Wall -Werror -pedantic -std=c11 -g \
	-I../include

LDADD = ../libmoodgoph.a

check_PROGRAMS = \
	request_parse \
	node_request

TESTS = $(check_PROGRAMS)

request_parse_SOURCES = request_parse.c
node_request_SOURCES = node_request.c
