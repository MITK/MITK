
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

all = ipPicInfo.o ipPicType.o                           \
      ipPicAddT.o ipPicQueryT.o ipPicDelT.o ipPicGetT.o \
      ipPicOldGet.o ipPicOldGetH.o ipPicOldGetS.o       \
      ipPicGet.o ipPicGetH.o ipPicGetS.o                \
      ipPicPut.o ipPicPutS.o                            \
      ipPicGetMem.o ipPicPutMem.o                       \
      ipPicCopyS.o ipPicCopyH.o                         \
      ipPicNew.o ipPicClear.o ipPicFree.o ipPicClone.o  \
      ipEndian.o ipFRead.o ipFWrite.o                   \
      ipError.o                                         \
      ipPicSize.o ipPicElements.o                       \
      ipPicTSVSize.o ipPicTSVElements.o

DISTFILES = Makefile                                                     \
            ipPic.h ipTypes.h ipPicOldP.h ipPicAnnotation.h              \
            ipPicOldGet.c ipPicOldGetH.c ipPicOldGetS.c                  \
            ipPicGet.c ipPicGetH.c ipPicGetS.c                           \
            ipPicPut.c ipPicPutS.c                                       \
            ipPicGetMem.c ipPicPutMem.c                                  \
            ipPicCopyS.c ipPicCopyH.c                                    \
            ipPicNew.c ipPicClear.c ipPicFree.c ipPicClone.c             \
            ipPicAddT.c ipPicQueryT.c ipPicDelT.c ipPicGetT.c            \
            ipEndian.c ipFRead.c ipFWrite.c                              \
            ipError.c                                                    \
            ipPicInfo.c ipPicType.c                                      \
            ipPicSize.c ipPicElements.c ipPicTSVSize.c ipPicTSVElements.c\
            *.ps

picinfo	:libipPic.a picinfo.o 
	$(CC) $(LDFLAGS) -o $@ $@.o $(LIBS)

pic2seq	:libipPic.a pic2seq.o 
	$(CC) $(LDFLAGS) -o $@ $@.o $(LIBS)

test	:libipPic.a test.o
	$(CC) $(LDFLAGS) -o $@ $@.o $(LIBS)

libipPic.a	:$(all)
		ar rv libipPic.a $(all)
		-ranlib libipPic.a

install	:libipPic.a picinfo
	cp ipPic.h /usr/local/include/ipPic
	cp ipTypes.h /usr/local/include/ipPic
	cp libipPic.a /usr/local/lib
	cp picinfo /usr/local/bin
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

picinfo.o	:picinfo.c ipPic.h

pic2seq.o	:pic2seq.c ipPic.h

test.o	:test.c ipPic.h

ipPic.h	:ipTypes.h
	touch ipPic.h

ipPicInfo.o :ipPicInfo.c ipPic.h

ipPicType.o :ipPicType.c ipPic.h

ipEndian.o :ipEndian.c ipPic.h

ipPicGetT.o :ipPicGetT.c ipPic.h

ipPicAddT.o :ipPicAddT.c ipPic.h

ipPicQueryT.o :ipPicQueryT.c ipPic.h

ipPicDelT.o :ipPicDelT.c ipPic.h

ipPicOldGet.o :ipPicOldGet.c ipPic.h ipPicOldP.h

ipPicPut.o :ipPicPut.c ipPic.h

ipPicGet.o :ipPicGet.c ipPic.h

ipPicGetH.o :ipPicGetH.c ipPic.h

ipPicOldGetH.o :ipPicOldGetH.c ipPic.h ipPicOldP.h

ipPicPutS.o :ipPicPutS.c ipPic.h

ipPicGetS.o :ipPicGetS.c ipPic.h

ipPicOldGetS.o :ipPicOldGetS.c ipPic.h ipPicOldP.h

ipFRead.o :ipFRead.c ipPic.h

ipFWrite.o :ipFWrite.c ipPic.h

ipPicSize.o :ipPicSize.c ipPic.h

ipPicElements.o :ipPicElements.c ipPic.h

ipPicTSVSize.o :ipPicTSVSize.c ipPic.h

ipPicTSVElements.o :ipPicTSVElements.c ipPic.h

ipPicNew.o :ipPicNew.c ipPic.h

ipPicClear.o :ipPicClear.c ipPic.h

ipPicFree.o :ipPicFree.c ipPic.h

ipPicClone.o :ipPicClone.c ipPic.h

ipPicCopyS.o :ipPicCopyS.c ipPic.h

ipPicCopyH.o :ipPicCopyH.c ipPic.h

depend:
	for i in *.c;do $(CC) $(CFLAGS) -MM $$i;done > t

