.PHONY: all clean

all: init pipe sharedmemory

init:
	mkdir -p build

pipe:
	gcc -o build/timep pipes/timepipe.c

sharedmemory:
	gcc -o build/timesm shared_memory/timeshmm.c

clean:
	rm build/*
