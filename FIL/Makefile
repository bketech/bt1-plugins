
CPPFLAGS += -I. -fPIC -D_REENTRANT -Wall -O3


all:	filters.dylib


filters.dylib:	filters.o filters_if.o exp2ap.o
	g++ -dynamiclib filters.o filters_if.o exp2ap.o -o filters.dylib

filters.o:	ladspaplugin.h filters.h
filters_if.o:	ladspaplugin.h filters.h


install:
	cp  *.so /usr/lib/ladspa


DIR := $(shell basename `pwd`)

archive:	clean
	cd ..; /bin/rm -f $(DIR).tar.bz2; tar cvf $(DIR).tar $(DIR); bzip2 $(DIR).tar


clean:
	/bin/rm -f *~ *.o *.so

