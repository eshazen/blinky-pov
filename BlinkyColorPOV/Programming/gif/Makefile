
all: blinky main

clean:	
	rm -f main blinky *.o *~

main: main.o AnimatedGifSaver.o
	c++ -o main main.o AnimatedGifSaver.o -lgif

blinky: blinky.o AnimatedGifSaver.o
	c++ -o blinky blinky.o AnimatedGifSaver.o -lgif

blinky.o: blinky.cpp

AnimatedGifSaver.o: AnimatedGifSaver.cpp

