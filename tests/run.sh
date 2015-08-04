#!/bin/bash

TESTS="commitonce.sh"

for TEST in $TESTS; do
	echo "TEST: $TEST"
	mkdir TEST
	pushd TEST
	. ../$TEST
	popd
	rm -rf TEST
done
