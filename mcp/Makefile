CPPFLAGS += -I. -fPIC -D_REENTRANT -Wall -O4

CPPFLAGS += -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math -DPIC -g -msse -mfpmath=sse

LIBFLAGS = -dynamiclib



all: mvclpf24.dylib mvchpf24.dylib cs_phaser.dylib cs_chorus.dylib

mvclpf24.dylib:	mvclpf24.o mvclpf24_if.o exp2ap.o
	g++ $(LIBFLAGS)  mvclpf24.o mvclpf24_if.o exp2ap.o -o mvclpf24.dylib

mvclpf24.o:	ladspaplugin.h mvclpf24.h
mvclpf24_if.o:	ladspaplugin.h mvclpf24.h


mvchpf24.dylib:	mvchpf24.o mvchpf24_if.o exp2ap.o
	g++ $(LIBFLAGS) mvchpf24.o mvchpf24_if.o exp2ap.o -o mvchpf24.dylib

mvchpf24.o:	ladspaplugin.h mvchpf24.h
mvchpf24_if.o:	ladspaplugin.h mvchpf24.h


cs_phaser.dylib:	cs_phaser.o cs_phaser_if.o exp2ap.o
	g++ $(LIBFLAGS) cs_phaser.o cs_phaser_if.o exp2ap.o -o cs_phaser.dylib

cs_phaser.o:	ladspaplugin.h cs_phaser.h
cs_phaser_if.o:	ladspaplugin.h cs_phaser.h

cs_chorus.dylib:	cs_chorus.o cs_chorus_if.o exp2ap.o
	g++ $(LIBFLAGS)  cs_chorus.o cs_chorus_if.o exp2ap.o -o cs_chorus.dylib

cs_chorus.o:	ladspaplugin.h cs_chorus.h
cs_chorus_if.o:	ladspaplugin.h cs_chorus.h


install:
	cp  *.dylib /usr/lib/ladspa

installMac:
	cp *.dylib /Library/Audio/Plug-Ins/LADSPA

DIR := $(shell basename `pwd`)

archive:	clean
	cd ..; /bin/rm -f $(DIR).tar.bz2; tar cvf $(DIR).tar $(DIR); bzip2 $(DIR).tar

clean:
	/bin/rm -f *~ *.o *.dylib

