AMALG = kabak.h kabak.c

#--------------------------------------
# Abstract targets
#--------------------------------------

all: $(AMALG)

check: test/kabak.so
	cd test && valgrind lua5.3 normalization.lua < NormalizationTest.txt

.PHONY: all check

#--------------------------------------
# Concrete targets
#--------------------------------------

kabak.h: src/api.h
	cp $< $@

kabak.c: $(wildcard src/*.h src/*.c src/*.ic)
	src/mkamalg.py src/*.c > $@

test/kabak.so: test/kabak.c $(AMALG)
	$(CC) $(CFLAGS) -fPIC -shared $< kabak.c -o $@
