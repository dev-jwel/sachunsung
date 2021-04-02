headers = $(wildcard src/*.h)
sources = $(wildcard src/*.c)
objects = $(patsubst src/%.c, bin/%.o, $(sources))

all: bin sachunsung

bin:
	mkdir bin

sachunsung: $(objects)
	gcc -o sachunsung $(objects)

bin/%.o: src/%.c
	gcc -c -o $@ $<

src/%.c: $(headers)

clean:
	rm -f bin/* sachunsung save.dat log.txt
