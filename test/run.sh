#!/usr/bin/env sh

set -o errexit

VG="valgrind --leak-check=full --error-exitcode=1"

$VG lua5.3 normalization.lua < NormalizationTest.txt > ret.tmp
cmp ret.tmp expect.txt
rm ret.tmp
