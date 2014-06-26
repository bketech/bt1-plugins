#   Copyright (C) 2004 Tom Szilagyi
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#   $Id: Makefile,v 1.16 2004/08/13 18:34:31 tszilagyi Exp $


#####################################################################
# PLEASE CHANGE THIS to your preferred installation location!
#
# Change this if you want to install somewhere else. In particular
# you may wish to remove the middle "local/" part of the path.

#INSTALL_PLUGINS_DIR	=	/usr/local/lib/ladspa/
INSTALL_PLUGINS_DIR	=	/Library/Audio/Plug-Ins/LADSPA/
INSTALL_LRDF_DIR	=	/usr/local/share/ladspa/rdf/

# NO EDITING below this line is required
# if all you want to do is install and use the plugins.
#####################################################################



# GENERAL

CC		=	gcc
CFLAGS		=	-I. -O3 -Wall -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math -c -fPIC -DPIC
LDFLAGS		=	-nostartfiles -dynamiclib -lc -lm

PLUGINS		=	tap_autopan.dylib \
			tap_chorusflanger.dylib \
			tap_deesser.dylib \
			tap_dynamics_m.dylib \
			tap_dynamics_st.dylib \
			tap_eq.dylib \
			tap_eqbw.dylib \
			tap_doubler.dylib \
			tap_pinknoise.dylib \
			tap_pitch.dylib \
			tap_reflector.dylib \
			tap_reverb.dylib \
			tap_rotspeak.dylib \
			tap_limiter.dylib \
			tap_sigmoid.dylib \
			tap_echo.dylib \
			tap_tremolo.dylib \
			tap_tubewarmth.dylib \
			tap_vibrato.dylib


all: $(PLUGINS)

# RULES TO BUILD PLUGINS FROM C CODE

tap_tremolo.dylib: tap_tremolo.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_tremolo.c -o tap_tremolo.o
	$(CC) $(LDFLAGS) -o tap_tremolo.dylib tap_tremolo.o

tap_eq.dylib: tap_eq.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_eq.c -o tap_eq.o
	$(CC) $(LDFLAGS) -o tap_eq.dylib tap_eq.o

tap_eqbw.dylib: tap_eqbw.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_eqbw.c -o tap_eqbw.o
	$(CC) $(LDFLAGS) -o tap_eqbw.dylib tap_eqbw.o

tap_echo.dylib: tap_echo.c init.cpp tap_utils.h ladspa.h
	g++ $(CFLAGS) tap_echo.c -o tap_echo.o
	g++ $(CFLAGS) init.cpp -o init.o
	g++ $(LDFLAGS) -o tap_echo.dylib tap_echo.o init.o

tap_reverb.dylib: tap_reverb.c tap_reverb.h tap_reverb_presets.h tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_reverb.c -o tap_reverb.o
	$(CC) $(LDFLAGS) -o tap_reverb.dylib tap_reverb.o

tap_limiter.dylib: tap_limiter.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_limiter.c -o tap_limiter.o
	$(CC) $(LDFLAGS) -o tap_limiter.dylib tap_limiter.o

tap_autopan.dylib: tap_autopan.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_autopan.c -o tap_autopan.o
	$(CC) $(LDFLAGS) -o tap_autopan.dylib tap_autopan.o

tap_deesser.dylib: tap_deesser.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_deesser.c -o tap_deesser.o
	$(CC) $(LDFLAGS) -o tap_deesser.dylib tap_deesser.o

tap_vibrato.dylib: tap_vibrato.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_vibrato.c -o tap_vibrato.o
	g++ $(CFLAGS) init.cpp -o init.o
	g++ $(LDFLAGS) -o tap_vibrato.dylib tap_vibrato.o init.o

tap_rotspeak.dylib: tap_rotspeak.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_rotspeak.c -o tap_rotspeak.o
	$(CC) $(LDFLAGS) -o tap_rotspeak.dylib tap_rotspeak.o

tap_pitch.dylib: tap_pitch.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_pitch.c -o tap_pitch.o
	g++ $(CFLAGS) init.cpp -o init.o
	g++ $(LDFLAGS) -o tap_pitch.dylib tap_pitch.o init.o

tap_dynamics_m.dylib: tap_dynamics_m.c tap_dynamics_presets.h tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_dynamics_m.c -o tap_dynamics_m.o
	$(CC) $(LDFLAGS) -o tap_dynamics_m.dylib tap_dynamics_m.o

tap_dynamics_st.dylib: tap_dynamics_st.c tap_dynamics_presets.h tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_dynamics_st.c -o tap_dynamics_st.o
	g++ $(CFLAGS) init.cpp -o init.o
	g++ $(LDFLAGS) -o tap_dynamics_st.dylib tap_dynamics_st.o init.o

tap_reflector.dylib: tap_reflector.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_reflector.c -o tap_reflector.o
	g++ $(CFLAGS) init.cpp -o init.o
	g++ $(LDFLAGS) -o tap_reflector.dylib tap_reflector.o init.o

tap_pinknoise.dylib: tap_pinknoise.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_pinknoise.c -o tap_pinknoise.o
	$(CC) $(LDFLAGS) -o tap_pinknoise.dylib tap_pinknoise.o

tap_doubler.dylib: tap_doubler.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_doubler.c -o tap_doubler.o
	$(CC) $(LDFLAGS) -o tap_doubler.dylib tap_doubler.o

tap_sigmoid.dylib: tap_sigmoid.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_sigmoid.c -o tap_sigmoid.o
	$(CC) $(LDFLAGS) -o tap_sigmoid.dylib tap_sigmoid.o

tap_tubewarmth.dylib: tap_tubewarmth.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_tubewarmth.c -o tap_tubewarmth.o
	$(CC) $(LDFLAGS) -o tap_tubewarmth.dylib tap_tubewarmth.o

tap_chorusflanger.dylib: tap_chorusflanger.c tap_utils.h ladspa.h
	$(CC) $(CFLAGS) tap_chorusflanger.c -o tap_chorusflanger.o
	g++ $(CFLAGS) init.cpp -o init.o
	g++ $(LDFLAGS) -o tap_chorusflanger.dylib tap_chorusflanger.o init.o


# OTHER TARGETS

install: targets
	-mkdir -p		$(INSTALL_PLUGINS_DIR)
	cp *.dylib 		$(INSTALL_PLUGINS_DIR)
	-mkdir -p		$(INSTALL_LRDF_DIR)
#	cp tap-plugins.rdf 	$(INSTALL_LRDF_DIR)
#	cp tap_reverb.rdf 	$(INSTALL_LRDF_DIR)

targets:	$(PLUGINS)

always:	

clean:
	-rm -f `find . -name "*.dylib"`
	-rm -f `find . -name "*.o"`
	-rm -f `find .. -name "*~"`

