#!/bin/sh

for i in aclocal autoconf; do
    echo "$i"
    $i
    if [ $? -ne 0 ]; then
	echo "Error $? in $i"
	exit 1
    fi
done

echo "./configure $@"
./configure $@
if [ $? -ne 0 ]; then
    echo "Error $? in ./configure"
    exit 1
fi
