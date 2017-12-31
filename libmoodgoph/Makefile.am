SUBDIRS = . tests

AM_CFLAGS = \
	-Wall -Werror -pedantic -std=c11 -g \
	-Iinclude

noinst_LIBRARIES = libmoodgoph.a
libmoodgoph_a_SOURCES = \
	request.c \
	response.c \
	node_base.c \
	node_filesystem.c \
	include/libmoodgoph/request.h \
	include/libmoodgoph/response.h \
	include/libmoodgoph/node_base.h \
	include/libmoodgoph/node_filesystem.h
