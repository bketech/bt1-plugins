/*
  Copyright (C) 2003 Fons Adriaensen
    
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

//-----------------------------------------------------------------------------------
// Common definitions


#include "cs_chorus.h"

#define IDENT 1944
#define NMODS 2

static const char* maker = "Fons Adriaensen <fons.adriaensen@alcatel.be>";
static const char* copyr = "(C) 2003 Fons Adriaensen - GNU General Public License version 2 applies";


static void pconnect (LADSPA_Handle H, unsigned long port, LADSPA_Data *data)
{
    ((LadspaPlugin *)H)->setport (port, data);
}

static void activate (LADSPA_Handle H)
{
    ((LadspaPlugin *)H)->active (true);
}

static void runplugin (LADSPA_Handle H, unsigned long k)
{
    ((LadspaPlugin *)H)->runproc (k, false);
}

static void runadding (LADSPA_Handle H, unsigned long k)
{
    ((LadspaPlugin *)H)->runproc (k, true);
}

static void setadding (LADSPA_Handle H, LADSPA_Data gain)
{
    ((LadspaPlugin *)H)->setgain (gain);
}

static void deactivate (LADSPA_Handle H)
{
    ((LadspaPlugin *)H)->active (false);
}

static void cleanup (LADSPA_Handle H)
{
    delete (LadspaPlugin *) H;
}

//-----------------------------------------------------------------------------------
// Plugin definitions


static const char* name1 = "Chorus1 - Based on CSound orchestra by Sean Costello";
static const char* label1  = "Chorus1";

static LADSPA_Handle instant1 (const struct _LADSPA_Descriptor *desc, unsigned long rate)
{
    return new Ladspa_CS_chorus1 (rate);
}


static const char* name2 = "Chorus2 - Based on CSound orchestra by Sean Costello";
static const char* label2  = "Chorus2";

static LADSPA_Handle instant2 (const struct _LADSPA_Descriptor *desc, unsigned long rate)
{
    return new Ladspa_CS_chorus2 (rate);
}


static const LADSPA_PortDescriptor pdesc1 [Ladspa_CS_chorus1::NPORT] = 
{
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL
};

static const char * const pname1  [Ladspa_CS_chorus1::NPORT] = 
{
    "Input",
    "Output",
    "Delay (ms)",
    "Mod Frequency 1 (Hz)",
    "Mod Amplitude 1 (ms)",
    "Mod Frequency 2 (Hz)",
    "Mod Amplitude 2 (ms)"
};

static const LADSPA_PortRangeHint phint1 [Ladspa_CS_chorus1::NPORT] = 
{
    { 0, 0, 0 },
    { 0, 0, 0 },
    { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0, 0, 30 },
    { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_LOGARITHMIC, 0.003, 10 },
    { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0, 0, 10 },
    { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_LOGARITHMIC, 0.01, 30 },
    { LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0, 0, 3 }
};


static const LADSPA_Descriptor moddescr [NMODS] =
{
    {
	IDENT+0,
	label1,
	LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE,
	name1,
	maker,
	copyr,
	Ladspa_CS_chorus1::NPORT,
	pdesc1,
	pname1,
	phint1,
	0,
	instant1,
	pconnect,
	activate,
	runplugin,
	runadding,
	setadding,
	deactivate,
	cleanup
    },  
    {
	IDENT+1,
	label2,
	LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE,
	name2,
	maker,
	copyr,
	Ladspa_CS_chorus2::NPORT,
	pdesc1,
	pname1,
	phint1,
	0,
	instant2,
	pconnect,
	activate,
	runplugin,
	runadding,
	setadding,
	deactivate,
	cleanup
    }  
};

extern "C"
#ifdef   WIN32
__declspec(dllexport)
#endif
const LADSPA_Descriptor *ladspa_descriptor (unsigned long i)
{
    if (i >= NMODS) return 0;
    return moddescr + i;
}

//-----------------------------------------------------------------------------------
