CFLAGS += -O2 -Wall -std=c99
CFLAGS += $(shell freetype-config --cflags)
LIBS += $(shell freetype-config --libs)

all: fontExtract

fontExtract: fontExtract.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	-rm -f *~ *.o fontExtract

