INPUT = main.c
LIBS = -lcaca -pthread
OPTIONS = -Wall -Wextra -Ofast -DHEIGHT=240 -DWIDTH=320

all:
	cc  $(INPUT) $(OPTIONS) $(LIBS) -o a.out
#cc $(LIBS) $(INPUT) $(OPTIONS) -o a.out

clean:
	rm a.out
