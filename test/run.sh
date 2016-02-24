#!/usr/bin/env sh

set -o errexit

VG="valgrind --leak-check=full --error-exitcode=1"
NORM_TEST="http://www.unicode.org/Public/8.0.0/ucd/NormalizationTest.txt"

$VG ./offset

$VG lua5.3 io.lua

cat NormalizationTest.txt | $VG lua5.3 normalization.lua > ret.tmp
cmp ret.tmp normalization.expect

rm ret.tmp
