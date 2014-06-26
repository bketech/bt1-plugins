// Reverb model declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _revmodel_
#define _revmodel_

#include "comb.h"
#include "allpass.h"
#include "tuning.h"

class revmodel
{
public:
					revmodel();
			void	mute();
			void	processmix(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples /*, int skip*/);
			void	processreplace(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples /*, int skip*/);
         inline void  update()
         {
         // Recalculate internal values after parameter change

	         wet1 = wet*(width*0.5f + 0.5f);
	         wet2 = wet*((1-width)*0.5f);

	         if (mode >= freezemode)
	         {
		         roomsize1 = 1;
		         damp1 = 0;
		         gain = muted;
	         }
	         else
	         {
		         roomsize1 = roomsize;
		         damp1 = damp;
		         gain = fixedgain;
	         }

	         int i;

	         for(i=0; i<numcombs; i++)
	         {
		         combL[i].setfeedback(roomsize1);
		         combR[i].setfeedback(roomsize1);
	         }

	         for(i=0; i<numcombs; i++)
	         {
		         combL[i].setdamp(damp1);
		         combR[i].setdamp(damp1);
	         }
         }

         inline void setroomsize(float value)
         {
            float newroomsize;

	         newroomsize = (value*scaleroom) + offsetroom;
            if (newroomsize != roomsize)
            {
               roomsize = newroomsize;
               modified = true;
            }
         //	update();
         }

         inline float getroomsize()
         {
	         return (roomsize-offsetroom)/scaleroom;
         }

         inline void setdamp(float value)
         {
            float newdamp;

	         newdamp = value*scaledamp;
            if (newdamp != damp)
            {
               damp = newdamp;
               modified = true;
            }
         //	update();
         }

         inline float getdamp()
         {
	         return damp/scaledamp;
         }

         inline void setwet(float value)
         {
            float newwet;

	         newwet = value*scalewet;
            if (newwet != wet)
            {
               wet = newwet;
               modified = true;
            }
         //	update();
         }

         inline float getwet()
         {
	         return wet/scalewet;
         }

         inline void setdry(float value)
         {
            float newdry;

	         newdry = value*scaledry;
            if (newdry != dry)
            {
               dry = newdry;
               modified = true;
            }
         }

         inline float getdry()
         {
	         return dry/scaledry;
         }

         inline void setwidth(float value)
         {
            if (width != value)
            {
            	width = value;
               modified = true;
            }
            if (modified)
            {
               modified = false;
            	update();
            }
         }

         inline float getwidth()
         {
	         return width;
         }

         inline void setmode(float value)
         {
            if (mode != value)
            {
            	mode = value;
               modified = true;
            }
         //	update();
         }

         inline float getmode()
         {
	         if (mode >= freezemode)
		         return 1;
	         else
		         return 0;
         }

private:
   bool  modified;

	float	gain;
	float	roomsize,roomsize1;
	float	damp,damp1;
	float	wet;
   // Do not add, remove, or change any of the data members below.
   // The assembler code assumes this order.
   float wet1;
   float wet2;
   // End order restriction
	float	dry;
	float	width;
	float	mode;

	// The following are all declared inline 
	// to remove the need for dynamic allocation
	// with its subsequent error-checking messiness

	// Comb filters
	comb	combL[numcombs];
	comb	combR[numcombs];

	// Allpass filters
	allpass	allpassL[numallpasses];
	allpass	allpassR[numallpasses];

	// Buffers for the combs
	float	bufcombL1[combtuningL1];
	float	bufcombR1[combtuningR1];
	float	bufcombL2[combtuningL2];
	float	bufcombR2[combtuningR2];
	float	bufcombL3[combtuningL3];
	float	bufcombR3[combtuningR3];
	float	bufcombL4[combtuningL4];
	float	bufcombR4[combtuningR4];
	float	bufcombL5[combtuningL5];
	float	bufcombR5[combtuningR5];
	float	bufcombL6[combtuningL6];
	float	bufcombR6[combtuningR6];
	float	bufcombL7[combtuningL7];
	float	bufcombR7[combtuningR7];
	float	bufcombL8[combtuningL8];
	float	bufcombR8[combtuningR8];

	// Buffers for the allpasses
	float	bufallpassL1[allpasstuningL1];
	float	bufallpassR1[allpasstuningR1];
	float	bufallpassL2[allpasstuningL2];
	float	bufallpassR2[allpasstuningR2];
	float	bufallpassL3[allpasstuningL3];
	float	bufallpassR3[allpasstuningR3];
	float	bufallpassL4[allpasstuningL4];
	float	bufallpassR4[allpasstuningR4];
};

#endif//_revmodel_

//ends

