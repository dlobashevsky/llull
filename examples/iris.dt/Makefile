

PREFIX=irisdt
OUTPUT=generated
GENERATED=$(OUTPUT)/$(PREFIX).h $(OUTPUT)/$(PREFIX).c $(OUTPUT)/$(PREFIX)__proto.h
PROG=irisdt
DATA=data/iris.data
GESRC=$(PREFIX).llull
SRC=$(wildcard *.c) $(OUTPUT)/$(PREFIX).c


#CFLAGS=-O3 -Wall -Wno-parentheses -Wno-switch -Wno-pointer-sign -Wno-trampolines -Wno-unused-result -std=gnu17 -funroll-loops -D_GNU_SOURCE -I. -I./generated
CFLAGS=-ggdb3 -Wall -Wno-parentheses -Wno-switch -Wno-pointer-sign -Wno-trampolines -Wno-unused-result -std=gnu17 -D_GNU_SOURCE -I. -I./generated
LDFLAGS=-lm

.PHONY: all clean

all: $(DATA) $(PROG)

$(GENERATED):  $(GESRC)
	../../src/llull -d g.dump -o $(OUTPUT) -i $<

$(DATA):
	mkdir data
	wget -nv -c http://archive.ics.uci.edu/ml/machine-learning-databases/iris/iris.data -O $(DATA)

$(PROG): $(SRC)
	gcc $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -fR $(OUTPUT) $(PROG) test.out g.dump data
