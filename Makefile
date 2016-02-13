CFLAGS = -std=c11 -g -Wall -Werror -pedantic

AMALG = kabak.h kabak.c

#--------------------------------------
# Abstract targets
#--------------------------------------

all: $(AMALG)

check: $(AMALG) test/kabak.so
	cd test && ./run.sh

clean:
	rm -f test/kabak.so

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
