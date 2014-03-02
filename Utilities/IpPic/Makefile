
CC = cc -fullwarn -woff 1009,1174
CC = cc -mips2
CC = gcc -Wall -pedantic -ansi -Wno-comment

DEBUG = -O2
DEBUG = -g
COPTIONS = #-DDMALLOC_FUNC_CHECK
INCPATHS = -I. -I.. -I/usr/local/include
LIBPATHS = -L. -L/usr/local/lib

CFLAGS = $(DEBUG) $(COPTIONS) $(INCPATHS)

LDFLAGS = $(DEBUG) $(LIBPATHS)

LIBS = -lipPic -ldmalloc
LIBS = -lipPic -lz

.o:.c

all = mitkIpPicInfo.o mitkIpPicType.o                           \
      mitkIpPicAddT.o mitkIpPicQueryT.o mitkIpPicDelT.o mitkIpPicGetT.o \
      mitkIpPicOldGet.o mitkIpPicOldGetH.o mitkIpPicOldGetS.o       \
      mitkIpPicGet.o mitkIpPicGetH.o mitkIpPicGetS.o                \
      mitkIpPicPut.o mitkIpPicPutS.o                            \
      mitkIpPicGetMem.o mitkIpPicPutMem.o                       \
      mitkIpPicCopyS.o mitkIpPicCopyH.o                         \
      mitkIpPicNew.o mitkIpPicClear.o mitkIpPicFree.o mitkIpPicClone.o  \
      ipEndian.o ipFRead.o ipFWrite.o                   \
      ipError.o                                         \
      mitkIpPicSize.o mitkIpPicElements.o                       \
      mitkIpPicTSVSize.o mitkIpPicTSVElements.o

DISTFILES = Makefile                                                     \
            mitkIpPic.h mitkIpTypes.h mitkIpPicOldP.h mitkIpPicAnnotation.h              \
            mitkIpPicOldGet.c mitkIpPicOldGetH.c mitkIpPicOldGetS.c                  \
            mitkIpPicGet.c mitkIpPicGetH.c mitkIpPicGetS.c                           \
            mitkIpPicPut.c mitkIpPicPutS.c                                       \
            mitkIpPicGetMem.c mitkIpPicPutMem.c                                  \
            mitkIpPicCopyS.c mitkIpPicCopyH.c                                    \
            mitkIpPicNew.c mitkIpPicClear.c mitkIpPicFree.c mitkIpPicClone.c             \
            mitkIpPicAddT.c mitkIpPicQueryT.c mitkIpPicDelT.c mitkIpPicGetT.c            \
            ipEndian.c ipFRead.c ipFWrite.c                              \
            ipError.c                                                    \
            mitkIpPicInfo.c mitkIpPicType.c                                      \
            mitkIpPicSize.c mitkIpPicElements.c mitkIpPicTSVSize.c mitkIpPicTSVElements.c\
            *.ps

all:	libipPic.a picinfo pic2seq

picinfo	:libipPic.a picinfo.o 
	$(CC) $(LDFLAGS) -o $@ $@.o $(LIBS)

pic2seq	:libipPic.a pic2seq.o 
	$(CC) $(LDFLAGS) -o $@ $@.o $(LIBS)

test	:libipPic.a test.o
	$(CC) $(LDFLAGS) -o $@ $@.o $(LIBS)

libipPic.a	:$(all)
		ar rv libipPic.a $(all)
		-ranlib libipPic.a

install	:libipPic.a picinfo pic2seq
	cp mitkIpPic.h /usr/local/include/ipPic
	cp mitkIpTypes.h /usr/local/include/ipPic
	cp libipPic.a /usr/local/lib
	cp picinfo /usr/local/bin
	cp pic2seq /usr/local/bin
	( cd /usr/local ; rdist )

clean:
	-rm *.o *.a core picinfo pic2seq test


#distname: version.c
#	@echo ipPic-`sed -e '/version_string/!d' \
#	  -e 's/[^0-9.]*\([0-9.a-z]*\).*/\1/' -e q version.c` > $@-tmp
#	@mv $@-tmp $@
#distdir = `cat distname`
#dist:$(DISTFILES) distname

distdir=ipPic
dist:$(DISTFILES)
	@rm -rf $(distdir)
	@mkdir $(distdir)
	@for file in $(DISTFILES);                                          \
          do                                                                \
	    ln $$file $(distdir)                                            \
	      || { echo copying $$file instead; cp -p $$file $(distdir); }; \
 	  done
#	@tar --gzip -chvf $(distdir).tar.gz $(distdir)
	@tar -chvf $(distdir).tar $(distdir)
	@gzip $(distdir).tar
	@rm -rf $(distdir)

picinfo.o	:picinfo.c mitkIpPic.h

pic2seq.o	:pic2seq.c mitkIpPic.h

test.o	:test.c mitkIpPic.h

mitkIpPic.h	:mitkIpTypes.h
	touch mitkIpPic.h

mitkIpPicInfo.o :mitkIpPicInfo.c mitkIpPic.h

mitkIpPicType.o :mitkIpPicType.c mitkIpPic.h

ipEndian.o :ipEndian.c mitkIpPic.h

mitkIpPicGetT.o :mitkIpPicGetT.c mitkIpPic.h

mitkIpPicAddT.o :mitkIpPicAddT.c mitkIpPic.h

mitkIpPicQueryT.o :mitkIpPicQueryT.c mitkIpPic.h

mitkIpPicDelT.o :mitkIpPicDelT.c mitkIpPic.h

mitkIpPicOldGet.o :mitkIpPicOldGet.c mitkIpPic.h mitkIpPicOldP.h

mitkIpPicPut.o :mitkIpPicPut.c mitkIpPic.h

mitkIpPicGet.o :mitkIpPicGet.c mitkIpPic.h

mitkIpPicGetH.o :mitkIpPicGetH.c mitkIpPic.h

mitkIpPicOldGetH.o :mitkIpPicOldGetH.c mitkIpPic.h mitkIpPicOldP.h

mitkIpPicPutS.o :mitkIpPicPutS.c mitkIpPic.h

mitkIpPicGetS.o :mitkIpPicGetS.c mitkIpPic.h

mitkIpPicOldGetS.o :mitkIpPicOldGetS.c mitkIpPic.h mitkIpPicOldP.h

ipFRead.o :ipFRead.c mitkIpPic.h

ipFWrite.o :ipFWrite.c mitkIpPic.h

mitkIpPicSize.o :mitkIpPicSize.c mitkIpPic.h

mitkIpPicElements.o :mitkIpPicElements.c mitkIpPic.h

mitkIpPicTSVSize.o :mitkIpPicTSVSize.c mitkIpPic.h

mitkIpPicTSVElements.o :mitkIpPicTSVElements.c mitkIpPic.h

mitkIpPicNew.o :mitkIpPicNew.c mitkIpPic.h

mitkIpPicClear.o :mitkIpPicClear.c mitkIpPic.h

mitkIpPicFree.o :mitkIpPicFree.c mitkIpPic.h

mitkIpPicClone.o :mitkIpPicClone.c mitkIpPic.h

mitkIpPicCopyS.o :mitkIpPicCopyS.c mitkIpPic.h

mitkIpPicCopyH.o :mitkIpPicCopyH.c mitkIpPic.h

depend:
	for i in *.c;do $(CC) $(CFLAGS) -MM $$i;done > t

