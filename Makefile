HEIGHT=240
WIDTH=320

INPUT=main.c
OPTIONS=-Wall -Wextra -O2
LIBS=-lcaca -lpthread
COMPILER=gcc
OUTPUT=a.out

all:$(INPUT)
	$(COMPILER) -DWIDTH=$(WIDTH) -DHEIGHT=$(HEIGHT)  $(LIBS) $(OPTIONS) $(INPUT) -o $(OUTPUT)
clean:$(OUTPUT)
	rm $(OUTPUT)
