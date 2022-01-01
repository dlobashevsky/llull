
CFLAGS=-ggdb3 -Wall -Wno-parentheses -std=gnu11 -D_GNU_SOURCE
LDFLAGS=-luuid -lm
PROGRAM=llull

.PHONY: install clean all

all:	$(PROGRAM)

lex.yy.c:  llulldef.l
	flex  $<

$(PROGRAM): llulldef.tab.c lex.yy.c llull.c codegen.c
	gcc $(CFLAGS) $^ $(LDFLAGS) -o $@

llulldef.tab.c llulldef.tab.h: llulldef.y
	bison -d $<

install:
	install -D -m 0755 llull /usr/local/bin/llull

clean:
	rm -f $(PROGRAM) llulldef.tab.* lex.yy.c
