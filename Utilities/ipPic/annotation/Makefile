CC = gcc  -DSHAPE

DEBUG = -g

CFLAGS = $(DEBUG) -I../.. -L../../ipPic -L../../Xip

LIBS =  ../../pic/Pic.o -lEditres -lXip -lXm -lXmu -lXt -lXext -lX11 -lipPic -lm

.o:.c

all = main.o Annotation.o 

main	:$(all)
	$(CC) $(CFLAGS) -o $@ $(all) $(LIBS)

Annotation.o	:Annotation.c Annotation.h Annotation.h icon_text.xbm

main.o	:main.c Annotation.h ad.h

ad.h	:main.ad
	ad2c main.ad >$@
