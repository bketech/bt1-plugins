// Reverb model implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "revmodel.h"
#include <memory.h>

#define  kMaxBlockSize  256

#ifdef   qPluginArm
// Device builds
#define  qUseVfpInOut         0  // Assembler does not seem to be much faster...
#else
// Virtual builds
#define  qUseVfpInOut         0  // Must always be 0
#endif

#ifdef   qPluginArm
/*
r4	               input buffer L ptr
r5	               input buffer R ptr
r6 	            output buffer L ptr
r7                output buffer R ptr
r8	               count
s6	      d3       wet 1
s7	               wet 2
s8-11    d4-5	   input L samples
s12-15   d6-7	   input R samples
s16-19   d8-9     output L samples
s20-23   d10-11   output R samples
*/
inline void VfpOutProcess(float** InPtrs, float** OutPtrs, float* StatePtr, int Count, double* SaveArea)
{
   asm volatile
   (
      "MOV     r8, %[SaveArea]      \n\t"
      "FSTMIAX r8, {d3-d11}         \n\t"    // Save Vfp regs - Warning :  The clobbered list seems to not be reliable

      "FMRX    r8, FPSCR            \n\t"
      "BIC     r8, r8, #0x00370000  \n\t"    // Stride 1, vector length 0
      "ORR     r8, r8, #0x00030000  \n\t"    // Set vector length to 4
      "FMXR    FPSCR, r8            \n\t"

      "MOV     r8, %[InPtrs]        \n\t"    // Load pointer to input buffers
      "LDMIA   r8, {r4, r5}         \n\t"    // Load L & R input buffer pointers
      "MOV     r8, %[OutPtrs]       \n\t"    // Load pointer to output buffers
      "LDMIA   r8, {r6, r7}         \n\t"    // Load L & R output buffer pointers

      "MOV	   r8, %[StatePtr]      \n\t"    // Load pointer to state variables
      "FLDMIAS r8, {s6, s7}         \n\t"    // Load wet 1 & 2

      "MOV	   r8, %[Count]         \n\t"    // Load count

      "8:                           \n\t"
      "FLDMIAS	r6, {s16-s19}        \n\t" 	// Load output L data, 4 samples at once
      "FLDMIAS	r4!, {s8-s11}        \n\t" 	// Load input L data, 4 samples at once
      "FMACS   s16, s8, s6          \n\t"    // output L += input L * wet1
      "FLDMIAS	r5!, {s12-s15}       \n\t" 	// Load input R data, 4 samples at once
      "FMACS   s16, s12, s7         \n\t"    // output L += input R * wet2
      "FLDMIAS	r7, {s20-s23}        \n\t" 	// Load output R data, 4 samples at once
      "FMACS   s20, s8, s7          \n\t"    // output R += input L * wet2
      "FSTMIAS r6!, {s16-s19}       \n\t"    // Store output L data
      "FMACS   s20, s12, s6         \n\t"    // output R += input R * wet1
      "FSTMIAS r7!, {s20-s23}       \n\t"    // Store output R data

      "SUBS	   r8, r8, #4           \n\t"    // Count -= 4
      "BNE	   8b                   \n\t"    // If we're not at zero, do another block of 4

      "FMRX    r8, FPSCR            \n\t"
      "BIC     r8, r8, #0x00370000  \n\t"    // Disable vector mode
      "FMXR    FPSCR, r8            \n\t"

      "MOV     r8, %[SaveArea]      \n\t"
      "FLDMIAX r8, {d3-d11}         \n\t"    // Restore Vfp regs

      :
      :  [InPtrs] "r" (InPtrs), [OutPtrs] "r" (OutPtrs), [StatePtr] "r" (StatePtr), [Count] "r" (Count), [SaveArea] "r" (SaveArea)
      :  "r4", "r5", "r6", "r7", "r8", "cc"
   );
}
#endif




revmodel::revmodel()
{
   modified = true;

	// Tie the components to their buffers
	combL[0].setbuffer(bufcombL1,combtuningL1);
	combR[0].setbuffer(bufcombR1,combtuningR1);
	combL[1].setbuffer(bufcombL2,combtuningL2);
	combR[1].setbuffer(bufcombR2,combtuningR2);
	combL[2].setbuffer(bufcombL3,combtuningL3);
	combR[2].setbuffer(bufcombR3,combtuningR3);
	combL[3].setbuffer(bufcombL4,combtuningL4);
	combR[3].setbuffer(bufcombR4,combtuningR4);
	combL[4].setbuffer(bufcombL5,combtuningL5);
	combR[4].setbuffer(bufcombR5,combtuningR5);
	combL[5].setbuffer(bufcombL6,combtuningL6);
	combR[5].setbuffer(bufcombR6,combtuningR6);
	combL[6].setbuffer(bufcombL7,combtuningL7);
	combR[6].setbuffer(bufcombR7,combtuningR7);
	combL[7].setbuffer(bufcombL8,combtuningL8);
	combR[7].setbuffer(bufcombR8,combtuningR8);

	allpassL[0].setbuffer(bufallpassL1,allpasstuningL1);
	allpassR[0].setbuffer(bufallpassR1,allpasstuningR1);
	allpassL[1].setbuffer(bufallpassL2,allpasstuningL2);
	allpassR[1].setbuffer(bufallpassR2,allpasstuningR2);
	allpassL[2].setbuffer(bufallpassL3,allpasstuningL3);
	allpassR[2].setbuffer(bufallpassR3,allpasstuningR3);
	allpassL[3].setbuffer(bufallpassL4,allpasstuningL4);
	allpassR[3].setbuffer(bufallpassR4,allpasstuningR4);

	// Set default values
	allpassL[0].setfeedback(0.5f);
	allpassR[0].setfeedback(0.5f);
	allpassL[1].setfeedback(0.5f);
	allpassR[1].setfeedback(0.5f);
	allpassL[2].setfeedback(0.5f);
	allpassR[2].setfeedback(0.5f);
	allpassL[3].setfeedback(0.5f);
	allpassR[3].setfeedback(0.5f);

	setwet(initialwet);
	setroomsize(initialroom);
	setdry(initialdry);
	setdamp(initialdamp);
	setmode(initialmode);
	setwidth(initialwidth);

	// Buffer will be full of rubbish - so we MUST mute them
	mute();
}


void revmodel::mute()
{
	int i;

	if (getmode() >= freezemode)
		return;

	for (i=0;i<numcombs;i++)
	{
		combL[i].mute();
		combR[i].mute();
	}
	for (i=0;i<numallpasses;i++)
	{
		allpassL[i].mute();
		allpassR[i].mute();
	}
}


void revmodel::processreplace(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples /*, int skip*/)
{
#if 0
   // Original code for reference only
	float outL,outR,input;

	while(numsamples-- > 0)
	{
		outL = outR = 0;
		input = (*inputL + *inputR) * gain;

   	int i;

		// Accumulate comb filters in parallel
		for(i=0; i<numcombs; i++)
		{
			outL += combL[i].process(input);
			outR += combR[i].process(input);
		}

		// Feed through allpasses in series
		for(i=0; i<numallpasses; i++)
		{
			outL = allpassL[i].process(outL);
			outR = allpassR[i].process(outR);
		}

		// Calculate output REPLACING anything already there
		*outputL = outL*wet1 + outR*wet2 /*+ *inputL*dry*/;   // We always use this fully wet, with no dry signal.
		*outputR = outR*wet1 + outL*wet2 /*+ *inputR*dry*/;

		// Increment sample pointers, allowing for interleave (if any)
		inputL += 1 /*skip*/;
		inputR += 1 /*skip*/;
		outputL += 1 /*skip*/;
		outputR += 1 /*skip*/;
	}
#else
   float input[kMaxBlockSize];
   float outL[kMaxBlockSize];
   float outR[kMaxBlockSize];
   int   count;
   int   i;

   while (1)
   {
      if (numsamples < kMaxBlockSize)
         count = numsamples;
      else
         count = kMaxBlockSize;
      numsamples -= count;

      memset(&outL[0], 0, sizeof(float) * count);
      memset(&outR[0], 0, sizeof(float) * count);
      i = 0;
      while (1)
      {
         input[i] = ((*inputL) + (*inputR)) * gain;
         inputL++;
         inputR++;
         i++;
         if (i >= count)
            break;
      }

		// Accumulate comb filters in parallel
		for(i=0; i < numcombs; i++)
		{
			combL[i].process(&input[0], &outL[0], count);
		}

		for(i=0; i < numcombs; i++)
		{
			combR[i].process(&input[0], &outR[0], count);
		}

		// Feed through allpasses in series
		for(i=0; i < numallpasses; i++)
		{
			allpassL[i].process(&outL[0], count);
		}

		for(i=0; i < numallpasses; i++)
		{
			allpassR[i].process(&outR[0], count);
		}

      i = 0;
      while (1)
      {
		   *outputL = (outL[i]*wet1 + outR[i]*wet2) /*+ *inputL*dry*/;   // We always use this fully wet, with no dry signal.
		   *outputR = (outR[i]*wet1 + outL[i]*wet2) /*+ *inputR*dry*/;
         outputL++;
         outputR++;
         i++;
         if (i >= count)
            break;
      }

      if (0 == numsamples)
         break;
   }
#endif
}


void revmodel::processmix(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples /*, int skip*/)
{
   float    input[kMaxBlockSize];
   float    outL[kMaxBlockSize];
   float    outR[kMaxBlockSize];
#if   qUseVfpInOut
   float*   FXs[2] = {&outL[0], &outR[0]};
   float*   Outs[2] = {outputL, outputR};
#endif
   int      count;
   int      i;

   while (1)
   {
      if (numsamples < kMaxBlockSize)
         count = numsamples;
      else
         count = kMaxBlockSize;
      numsamples -= count;

      memset(&outL[0], 0, sizeof(float) * count);
      memset(&outR[0], 0, sizeof(float) * count);
      i = 0;
      while (1)
      {
         input[i] = ((*inputL) + (*inputR)) * gain;
         inputL++;
         inputR++;
         i++;
         if (i >= count)
            break;
      }

		// Accumulate comb filters in parallel
		for(i=0; i < numcombs; i++)
		{
			combL[i].process(&input[0], &outL[0], count);
		}

		for(i=0; i < numcombs; i++)
		{
			combR[i].process(&input[0], &outR[0], count);
		}

		// Feed through allpasses in series
		for(i=0; i < numallpasses; i++)
		{
			allpassL[i].process(&outL[0], count);
		}

		for(i=0; i < numallpasses; i++)
		{
			allpassR[i].process(&outR[0], count);
		}

#if   qUseVfpInOut
      double   VfpSaveArea[17];
      VfpOutProcess(&FXs[0], &Outs[0], &this->wet1, count, &VfpSaveArea[0]);
#else
      i = 0;
      while (1)
      {
		   *outputL += (outL[i]*wet1 + outR[i]*wet2) /*+ *inputL*dry*/;   // We always use this fully wet, with no dry signal.
		   *outputR += (outR[i]*wet1 + outL[i]*wet2) /*+ *inputR*dry*/;
         outputL++;
         outputR++;
         i++;
         if (i >= count)
            break;
      }
#endif

      if (0 == numsamples)
         break;
   }
}

//ends

