#!/bin/bash
echo $COVERAGE

if [ "$COVERAGE" = "true" ]; then
    echo expression evaluated as true
    ./configure --with-check --enable-gcov && make && make check
    coveralls --exclude-pattern ".+?\.h" --exclude-pattern "\w+/test" --gcov-options '\-lp'
fi