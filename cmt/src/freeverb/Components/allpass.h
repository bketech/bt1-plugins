// Allpass filter declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _allpass_
#define _allpass_
#include "denormals.h"

#ifdef   qPluginArm
// Device builds
#define  qUseVfpAllFilter     1
#else
// Virtual builds
#define  qUseVfpAllFilter     0  // Must always be 0
#endif

#ifdef   qPluginArm
/*
r0	               filter buffer + size (end)
r1	               filter buffer + index
r2	               filter buffer at start of loop, temp
r4	               filter buffer (begin)
r5	               state ptr
r6 	            input & output buffer ptr
r7                size - index
r8	               count
s4	      d2       register save r0-r2
s5	            
s6	      d3    
s7	               feedback
s8-15    d4-7	   input samples
s16-23   d8-11	   buffer data
s24-31   d12-15   output samples
*/
inline void VfpAllProcess(float* InOutPtr, float* StatePtr, int Count, double* SaveArea)
{
   asm volatile
   (
      "MOV     r8, %[SaveArea]      \n\t"
      "FSTMIAX r8, {d2-d15}         \n\t"    // Save Vfp regs - Warning :  The clobbered list seems to not be reliable
      "FMSR    s4, r0               \n\t"    // Save r0-r2
      "FMSR    s5, r1               \n\t"
      "FMSR    s6, r2               \n\t"

      "FMRX    r8, FPSCR            \n\t"
      "BIC     r8, r8, #0x00370000  \n\t"    // Stride 1, vector length 0
      "ORR     r8, r8, #0x00070000  \n\t"    // Set vector length to 8
      "FMXR    FPSCR, r8            \n\t"

      "MOV     r6, %[InOutPtr]      \n\t"    // Load pointer to samples
      "MOV	   r5, %[StatePtr]      \n\t"    // Load pointer to filter state variables
      "MOV	   r8, %[Count]         \n\t"    // Load count
      "FMSR    s8, r8               \n\t"    // Save count temporarily
      "FLDS	   s7, [r5]             \n\t"    // Load feedback
      "LDR	   r4, [r5, #12]        \n\t"    // Load filter buffer pointer
      "LDR	   r8, [r5, #8]         \n\t"    // Load filter buffer size
      "MOV	   r7, r8               \n\t"    // Save a copy of the buffer size
      "ADD	   r0, r4, r8, LSL #2   \n\t"    // r0 = bufer pointer + size
      "LDR	   r8, [r5, #4]         \n\t"    // Load filter buffer index
      "SUB	   r7, r7, r8           \n\t"    // r7 = size - index
      "ADD	   r1, r4, r8, LSL #2   \n\t"    // r1 = buffer pointer + index
      "FMRS    r8, s8               \n\t"    // Restore count

      "CMP	   r7, #8               \n\t"    // If we have 8 or more samples until the end of the buffer
      "BPL	   8f                   \n\t"    // Go load a block of 8 the easy way

      "11:                          \n\t"    // The hard way, check for end of buffer after every load
      "MOV	   r2, r1               \n\t"    // Save current buffer pointer
      "CMP	   r0, r1               \n\t"    // Check for end of buffer
      "MOVEQ	r1, r4               \n\t"    // Reset pointer to beginning of buffer if we reached the end
      "FLDMIAS	r1!, {s16}           \n\t"    // Load buffer data, one sample at a time
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FLDMIAS	r1!, {s17}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FLDMIAS	r1!, {s18}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FLDMIAS	r1!, {s19}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FLDMIAS	r1!, {s20}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FLDMIAS	r1!, {s21}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FLDMIAS	r1!, {s22}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FLDMIAS	r1, {s23}            \n\t"
      "MOV	   r1, r2               \n\t"    // Restore current buffer pointer
      "B	      2f                   \n\t"    // Go process the 8 samples

      "8:                           \n\t"    // The easy way, block load 8 samples
      "FLDMIAS	r1, {s16-s23}        \n\t" 	// Load buffer data, 8 samples at once

      "2:                           \n\t"    // Process a block of samples
      "FLDMIAS	r6, {s8-s15}         \n\t" 	// Load input samples
      "FSUBS	s24, s16, s8         \n\t" 	// output = buffer data - input
      "FSTMIAS	r6!, {s24-s31}       \n\t" 	// Store output samples

      "FMACS   s8, s16, s7          \n\t"    // input += buffer data * feedback

      "CMP	   r7, #8               \n\t"    // If we have less than 8 samples to the end of the buffer...
      "BMI	   12f                  \n\t"    // ...do the store the hard way

      "FSTMIAS	r1!, {s8-s15}        \n\t"	   // Store buffer data, 8 samples at once

      "SUBS	   r8, r8, #8           \n\t"    // Count -= 8
      "BEQ	   90f                  \n\t"    // If we're at zero, we're done

      "SUB	   r7, r7, #8           \n\t"    // Distance until end of buffer -= 8
      "CMP     r7, #8               \n\t"    // If we still have 8 or more until the end of the buffer...
      "BPL	   8b                   \n\t"    // ...go do another block of 8 the easy way
      "B	      11b                  \n\t"    // ...otherwise do a block of 8 the hard way

      "12:                          \n\t"    // The hard way, check for end of buffer after every store
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FSTMIAS	r1!, {s8}            \n\t"	   // Store buffer data
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FSTMIAS	r1!, {s9}            \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FSTMIAS	r1!, {s10}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FSTMIAS	r1!, {s11}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FSTMIAS	r1!, {s12}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FSTMIAS	r1!, {s13}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FSTMIAS	r1!, {s14}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"
      "FSTMIAS	r1!, {s15}           \n\t"
      "CMP	   r0, r1               \n\t"
      "MOVEQ	r1, r4               \n\t"

      "SUB	   r7, r0, r1           \n\t"    // The buffer has wrapped, reset the distance until end
      "MOV	   r7, r7, LSR #2       \n\t"    // Convert from byte offset to count

      "SUBS	   r8, r8, #8           \n\t"    // Count -= 8
      "BNE	   8b                   \n\t"    // If we're not at zero yet, go do another block of 8 the easy way

      "90:                          \n\t"    // Exit
      "SUB     r1, r1, r4           \n\t"    // r1 = buffer index as a byte offset
      "MOV     r1, r1, LSR #2       \n\t"    // Convert from byte offset to count
      "STR     r1, [r5, #4]         \n\t"    // Update buffer index

      "FMRX    r8, FPSCR            \n\t"
      "BIC     r8, r8, #0x00370000  \n\t"    // Disable vector mode
      "FMXR    FPSCR, r8            \n\t"

      "FMRS    r0, s4               \n\t"    // Restore r0-r2
      "FMRS    r1, s5               \n\t"
      "FMRS    r2, s6               \n\t"
      "MOV     r8, %[SaveArea]      \n\t"
      "FLDMIAX r8, {d2-d15}         \n\t"    // Restore Vfp regs

      :
      :  [InOutPtr] "r" (InOutPtr), [StatePtr] "r" (StatePtr), [Count] "r" (Count), [SaveArea] "r" (SaveArea)
      :  "r4", "r5", "r6", "r7", "r8", "cc"
   );
}
#endif


class allpass
{
public:
					allpass();
			void	setbuffer(float *buf, int size);
 inline  void  process(float* inout, int count);
			void	mute();
 inline	void	setfeedback(float val);
 inline	float	getfeedback();
// private:
   // Do not add, remove, or change any of the data members below.
   // The assembler code assumes this order.
	float	feedback;
	int	bufidx;
	int	bufsize;
	float	*buffer;
   // End order restriction
};


inline void allpass::setfeedback(float val) 
{
	feedback = val;
}


inline float allpass::getfeedback() 
{
	return feedback;
}


// Big to inline - but crucial for speed

inline void allpass::process(float* inout, int count)
{
#if   qUseVfpAllFilter
   double   VfpSaveArea[17];
   VfpAllProcess(inout, &this->feedback, count, &VfpSaveArea[0]);
#else
   float input;
	float bufout;

   while (1)
   {
      input = *inout;

	   bufout = buffer[bufidx];
//	   undenormalise(bufout);

	   *inout = -input + bufout;
	
	   buffer[bufidx] = input + (bufout*feedback);

	   if (++bufidx >= bufsize)
         bufidx = 0;

      count--;
      if (0 == count)
         break;

      inout++;
   }
#endif
}

#endif//_allpass

