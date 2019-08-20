HEIGHT=240
WIDTH=320

INPUT=main.c
OPTIONS=-Wall -Wextra -O2
LIBS=-lcaca -lpthread
COMPILER=gcc
OUTPUT=mandelbrot

all:$(INPUT)
	$(COMPILER) -DWIDTH=$(WIDTH) -DHEIGHT=$(HEIGHT) $(OPTIONS) $(INPUT) -o $(OUTPUT) $(LIBS)
clean:$(OUTPUT)
	rm $(OUTPUT)
