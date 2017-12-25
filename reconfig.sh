#!/bin/sh

set -xe

aclocal
autoheader
autoconf
automake --add-missing
