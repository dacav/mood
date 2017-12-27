AM_CFLAGS = \
	-Wall -Werror -pedantic -std=c11 -g

check_PROGRAMS = \
	request_parse

TESTS = $(check_PROGRAMS)

request_parse_SOURCES = request_parse.c
request_parse_CFLAGS = -I../include
request_parse_LDADD = ../libmoodgoph.a
