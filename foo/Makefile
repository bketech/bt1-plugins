CC=gcc
CFLAGS=-O3 -Wall -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math -fPIC -DPIC -g -msse -mfpmath=sse

LIBFLAGS=-nostartfiles -dynamiclib -Wl

PLUGIN_LIBS=foo_limiter.dylib foo_transients.dylib foo_transients_mono.dylib foo_driver.dylib
PLUGIN_SOURCES=foo_limiter.c foo_transients.c foo_transients_mono.c init.cpp foo_driver.dylib

all: plugins tester

%.dylib : %.o rms.o init.o
	g++ $(LIBFLAGS) rms.o init.o $< -o $@

%.c : %.xml makestub.pl
	./makestub.pl $< > $@


plugins: $(PLUGIN_LIBS)

tester.o: foo_limiter.xml foo_transients.xml

tester: tester.o rms.o $(PLUGIN_SOURCES)
	gcc tester.o rms.o -o tester -lm 

tester.o: $(PLUGIN_SOURCES)

$(PLUGIN_SOURCES): utils.h rms.h

rms.o: rms.h

install: plugins
	cp $(PLUGIN_LIBS) /usr/lib/ladspa/

clean:
	rm -f *.o *.so $(PLUGIN_SOURCES) tester plot/*


#
