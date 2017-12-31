AM_CFLAGS = \
	-Wall -Werror -pedantic -std=c11 -g \
	-I../include

LDADD = ../libmoodgoph.a

check_PROGRAMS = \
	request_parse \
	node_request \
	response_test

TESTS = $(check_PROGRAMS)

request_parse_SOURCES = request_parse.c
node_request_SOURCES = node_request.c
response_test_SOURCES = response_test.c
