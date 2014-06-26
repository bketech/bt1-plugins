// Comb filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "comb.h"


comb::comb()
{
	filterstore = 0;
	bufidx = 0;
   feedback = 0.0f;
   damp1 = 0.0f;
   damp2 = 0.0f;
}

void comb::setbuffer(float *buf, int size) 
{
	buffer = buf; 
	bufsize = size;
}

void comb::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

// ends

