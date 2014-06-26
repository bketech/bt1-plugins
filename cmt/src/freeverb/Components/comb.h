// Comb filter class declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _comb_
#define _comb_

#include "denormals.h"

#ifdef   qPluginArm
// Device builds
#define  qUseVfpCombFilter    1
#else
// Virtual builds
#define  qUseVfpCombFilter    0  // Must always be 0
#endif

#ifdef   qPluginArm
/*
r0	            filter buffer + size (end)
r1	            filter buffer + index
r2	            filter buffer at start of loop, temp
r3	            size - index
r4	            filter buffer (begin)
r5	            state ptr
r6	            input buffer ptr
r7	            output buffer ptr
r8	            count
s0-3     d0    register save r0-r3
s4	      d2    filterstore
s5	            damp 1
s6	      d3    damp 2
s7	            feedback
s8-15    d4-7	input samples, output samples
s16-23   d8-11	buffer data
*/
inline void VfpCombProcess(float* InPtr, float* OutPtr, float* StatePtr, int Count, double* SaveArea)
{
   asm volatile
   (
      "MOV     r8, %[SaveArea]      \n\t"
      "FSTMIAX r8, {d0-d11}         \n\t"    // Save Vfp regs - Warning :  The clobbered list seems to not be reliable
      "FMSR    s0, r0               \n\t"    // Save r0-r3
      "FMSR    s1, r1               \n\t"
      "FMSR    s2, r2               \n\t"
      "FMSR    s3, r3               \n\t"

      "FMRX    r8, FPSCR            \n\t"
      "BIC     r8, r8, #0x00370000  \n\t"    // Stride 1, vector length 0
      "ORR     r8, r8, #0x00070000  \n\t"    // Set vector length to 8
      "FMXR    FPSCR, r8            \n\t"

      "MOV     r6, %[InPtr]         \n\t"    // Load pointer to in samples
      "MOV	   r7, %[OutPtr]        \n\t"    // Load pointer to out samples
      "MOV	   r5, %[StatePtr]      \n\t"    // Load pointer to filter state variables
      "MOV	   r8, %[Count]         \n\t"    // Load count
      "FMSR    s8, r8               \n\t"    // Save count temporarily
      "FLDMIAS	r5, {s4-s7}          \n\t"    // Load filter state variables
      "LDR	   r4, [r5, #24]        \n\t"    // Load filter buffer pointer
      "LDR	   r8, [r5, #20]        \n\t"    // Load filter buffer size
      "MOV	   r3, r8               \n\t"    // Save a copy of the buffer size
      "ADD	   r0, r4, r8, LSL #2   \n\t"    // r0 = bufer pointer + size
      "LDR	   r8, [r5, #16]        \n\t"    // Load filter buffer index
      "SUB	   r3, r3, r8           \n\t"    // r3 = size - index
      "ADD	   r1, r4, r8, LSL #2   \n\t"    // r1 = buffer pointer + index
      "FMRS    r8, s8               \n\t"    // Restore count

      "CMP	   r3, #8               \n\t"    // If we have 8 or more samples until the end of the buffer
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
      "FLDMIAS	r7, {s8-s15}         \n\t" 	// Load output samples
      "FADDS	s8, s8, s16          \n\t" 	// Add buffer data to output samples
      "FSTMIAS	r7!, {s8-s15}        \n\t" 	// Store output samples

      "FMULS	s16, s16, s6         \n\t"	   // Multiply buffer data * damp2

      "FLDMIAS	r6!, {s8-s15}        \n\t"	   // Load input samples

      "FMULS	s4, s4, s5           \n\t" 	// filterstore *= damp1
      "FADDS	s4, s4, s16          \n\t"		// flterstore += first buffer data
      "FMRS	   r2, s4               \n\t"		// Move filterstore to first buffer data...
      "FMSR	   s16, r2              \n\t"    // ...via r2 to avoid vectorizing

      "FMACS	s17, s16, s5         \n\t"	   // buffer data [n] += buffer data [n-1] * damp1
      "FMSR	   s16, r2              \n\t"		// Fix first buffer data due to wrapping

      "FCPYS	s4, s23              \n\t"		// filterstore = last buffer data

      "FMACS	s8, s16, s7          \n\t"	   // input += filterstore * feedback

      "CMP	   r3, #8               \n\t"    // If we have less than 8 samples to the end of the buffer...
      "BMI	   12f                  \n\t"    // ...do the store the hard way

      "FSTMIAS	r1!, {s8-s15}        \n\t"	   // Store buffer data, 8 samples at once

      "SUBS	   r8, r8, #8           \n\t"    // Count -= 8
      "BEQ	   90f                  \n\t"    // If we're at zero, we're done

      "SUB	   r3, r3, #8           \n\t"    // Distance until end of buffer -= 8
      "CMP     r3, #8               \n\t"    // If we still have 8 or more until the end of the buffer...
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

      "SUB	   r3, r0, r1           \n\t"    // The buffer has wrapped, reset the distance until end
      "MOV	   r3, r3, LSR #2       \n\t"    // Convert from byte offset to count

      "SUBS	   r8, r8, #8           \n\t"    // Count -= 8
      "BNE	   8b                   \n\t"    // If we're not at zero yet, go do another block of 8 the easy way

      "90:                          \n\t"    // Exit
      "FSTS    s4, [r5]             \n\t"    // Update filterstore
      "SUB     r1, r1, r4           \n\t"    // r1 = buffer index as a byte offset
      "MOV     r1, r1, LSR #2       \n\t"    // Convert from byte offset to count
      "STR     r1, [r5, #16]        \n\t"    // Update buffer index

      "FMRX    r8, FPSCR            \n\t"
      "BIC     r8, r8, #0x00370000  \n\t"    // Disable vector mode
      "FMXR    FPSCR, r8            \n\t"

      "FMRS    r0, s0               \n\t"    // Restore r0-r3
      "FMRS    r1, s1               \n\t"
      "FMRS    r2, s2               \n\t"
      "FMRS    r3, s3               \n\t"
      "MOV     r8, %[SaveArea]      \n\t"
      "FLDMIAX r8, {d0-d11}         \n\t"    // Restore Vfp regs

      :
      :  [InPtr] "r" (InPtr), [OutPtr] "r" (OutPtr), [StatePtr] "r" (StatePtr), [Count] "r" (Count), [SaveArea] "r" (SaveArea)
      :  "r4", "r5", "r6", "r7", "r8", "cc"
   );
}
#endif


class comb
{
public:
					comb();
			void	setbuffer(float *buf, int size);
 inline  void  process(float* input, float* output, int count);
			void	mute();
 inline	void	setdamp(float val);
 inline	float	getdamp();
 inline	void	setfeedback(float val);
 inline	float	getfeedback();
private:
   // Do not add, remove, or change any of the data members below.
   // The assembler code assumes this order.
	float	filterstore;
	float	damp1;
	float	damp2;
	float	feedback;
	int	bufidx;
	int	bufsize;
	float	*buffer;
   // End order restriction
};


inline void comb::setdamp(float val) 
{
	damp1 = val; 
	damp2 = 1-val;
}

inline float comb::getdamp() 
{
	return damp1;
}

inline void comb::setfeedback(float val) 
{
	feedback = val;
}

inline float comb::getfeedback() 
{
	return feedback;
}

// Big to inline - but crucial for speed

inline void comb::process(float* input, float* output, int count)
{
#if   qUseVfpCombFilter
   double   VfpSaveArea[17];
   VfpCombProcess(input, output, &this->filterstore, count, &VfpSaveArea[0]);
#else
	float temp;

   while (1)
   {
	   temp = buffer[bufidx];
//	   undenormalise(output);
	   *output += temp;

	   filterstore = (temp*damp2) + (filterstore*damp1);
//	   undenormalise(filterstore);

	   buffer[bufidx] = (*input) + (filterstore*feedback);

	   if (++bufidx >= bufsize)
         bufidx = 0;

      count--;
      if (0 == count)
         break;

      input++;
      output++;
   }
#endif
}

#endif //_comb_

