#!/usr/bin/env sh

set -o errexit

VG="valgrind --leak-check=full --error-exitcode=1"
NORM_TEST="http://www.unicode.org/Public/8.0.0/ucd/NormalizationTest.txt"

curl $NORM_TEST | $VG lua5.3 normalization.lua > ret.tmp
cmp ret.tmp normalization.expect

$VG lua5.3 combinations.lua > ret.tmp
cmp ret.tmp combinations.expect

rm ret.tmp
