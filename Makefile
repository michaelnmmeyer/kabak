PREFIX = /usr/local

CFLAGS = -std=c11 -g -Wall -Werror -pedantic
CFLAGS += -O2 -DNDEBUG -march=native -mtune=native -fomit-frame-pointer -s -flto

AMALG = kabak.h kabak.c
BINARY = kabak

#--------------------------------------
# Abstract targets
#--------------------------------------

all: $(AMALG) $(BINARY)

check: $(AMALG) test/kabak.so test/offset
	cd test && ./run.sh

clean:
	rm -f $(BINARY) test/kabak.so

install: $(BINARY)
	install -spm 0755 $< $(PREFIX)/bin/$(BINARY)

uninstall:
	rm -f $(PREFIX)/bin/$(BINARY)

.PHONY: all check clean install uninstall

#--------------------------------------
# Concrete targets
#--------------------------------------

cmd/%.ih: cmd/%.txt
	cmd/mkcstring.py < $< > $@

kabak.h: src/api.h
	cp $< $@

kabak.c: $(wildcard src/*.h src/*.c src/*.ic)
	src/mkamalg.py src/*.c > $@

$(BINARY): cmd/kabak.c cmd/kabak.ih cmd/cmd.c $(AMALG)
	$(CC) $(CFLAGS) cmd/kabak.c cmd/cmd.c kabak.c -o $@

test/kabak.so: test/kabak.c $(AMALG)
	$(CC) $(CFLAGS) -fPIC -shared $< kabak.c -o $@

test/%: test/%.c $(AMALG)
	$(CC) $(CFLAGS) $< kabak.c -o $@
