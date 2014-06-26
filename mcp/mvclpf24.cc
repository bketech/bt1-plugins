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


#include <stdio.h>
#include <math.h>
#include "mvclpf24.h"


extern float exp2ap (float x);


void Ladspa_Moogvcf1::setport (unsigned long port, LADSPA_Data *data)
{
    _port [port] = data;
}


void Ladspa_Moogvcf1::active (bool act)
{
    _c1 = _c2 = _c3 = _c4 = _c5 = _w = _r = 0;
}


void Ladspa_Moogvcf1::runproc (unsigned long len, bool add)
{
    int   k;
   float *p0, *p1;//, *p2, *p3, *p4;
    float c1, c2, c3, c4, c5;
    float g0, g1, r, dr, w, dw, x, t;

    p0 = _port [0];
    p1 = _port [1];
   float p2 = 0.f;//_port [2] - 1;
   float p3 = 0.f;//_port [3] - 1;
   float p4 = 0.f;//_port [4] - 1;
    g0 = exp2ap (0.1661 * _port  [2][0]) / 4;
    g1 = exp2ap (0.1661 * _port [7][0]) * 4;
    if (add) g1 *= _gain;

    c1 = _c1 + 1e-6;
    c2 = _c2;
    c3 = _c3;
    c4 = _c4;
    c5 = _c5;
    w = _w; 
    r = _r;
 
    do
    {
        k = (len > 24) ? 16 : len;
        //p2 += k;
        //p3 += k;
        //p4 += k;
        len -= k;

        t = exp2ap (_port [4][0] * p3 + _port [3][0] + p2 + 10.82) / _fsam;
	if (t < 0.8) t *= 1 - 0.4 * t - 0.125 * t * t;
        else 
	{
            t *= 0.6; 
            if (t > 0.92) t = 0.92;
	}
        dw = (t - w) / k;

        t = _port [6][0] * p4 + _port [5][0];
        if (r > 1) r = 1;
        if (r < 0) r = 0;
        dr = (t - r) / k;  

        while (k--)
	{
            w += dw;                        
            r += dr;
	    x = -4.2 * r * c5 + *p0++ * g0 + 1e-10;
            t = c1 / (1 + fabs (c1));
            c1 += w * (x - t);
            x = c1 / (1 + fabs (c1));
            c2 += w * (x  - c2);
            c3 += w * (c2 - c3);
            c4 += w * (c3 - c4);
	    if (add) *p1++ += g1 * c4;
	    else     *p1++  = g1 * c4;
	    c5 += 0.5 * (c4 - c5);
	}
    }
    while (len);

    _c1 = c1;
    _c2 = c2;
    _c3 = c3;
    _c4 = c4;
    _c5 = c5;
    _w = w;
    _r = r;
}



void Ladspa_Moogvcf2::setport (unsigned long port, LADSPA_Data *data)
{
    _port [port] = data;
}


void Ladspa_Moogvcf2::active (bool act)
{
    _c1 = _c2 = _c3 = _c4 = _c5 = _w = _r = 0;
}


void Ladspa_Moogvcf2::runproc (unsigned long len, bool add)
{
//    int   k;
//    float *p0, *p1, *p2, *p3, *p4;
//    float c1, c2, c3, c4, c5;
//    float g0, g1, r, dr, w, dw, x, t;
//
//    p0 = _port [0];
//    p1 = _port [1];
//    p2 = _port [2] - 1;
//    p3 = _port [3] - 1;
//    p4 = _port [4] - 1;
//    g0 = exp2ap (0.1661 * _port  [5][0]) / 2;
//    g1 = exp2ap (0.1661 * _port [10][0]) * 2;
//    if (add) g1 *= _gain;
//
//    c1 = _c1 + 1e-6;
//    c2 = _c2;
//    c3 = _c3;
//    c4 = _c4;
//    c5 = _c5;
//    w = _w; 
//    r = _r;
//
//    do
//    {
//        k = (len > 24) ? 16 : len;
//        p2 += k;
//        p3 += k;
//        p4 += k;
//        len -= k;
//
//        t = exp2ap (_port [7][0] * *p3 + _port [6][0] + *p2 + 10.71) / _fsam;
//	if (t < 0.8) t *= 1 - 0.4 * t - 0.125 * t * t;
//        else 
//	{
//            t *= 0.6; 
//            if (t > 0.92) t = 0.92;
//	}
//        dw = (t - w) / k;
//
//        t = _port [9][0] * *p4 + _port [8][0];
//        if (t > 1) t = 1;
//        if (t < 0) t = 0;
//        dr = (t - r) / k;  
//
//        while (k--)
//	{
//            w += dw;                        
//            r += dr;
//
//	    x = -4.5 * r * c5 + *p0++ * g0 + 1e-10;
////	    x = tanh (x); 
//            x /= sqrt (1 + x * x);
//            c1 += w * (x  - c1) / (1 + c1 * c1);            
//            c2 += w * (c1 - c2) / (1 + c2 * c2);            
//            c3 += w * (c2 - c3) / (1 + c3 * c3);            
//            c4 += w * (c3 - c4) / (1 + c4 * c4);            
//
//	    if (add) *p1++ += g1 * (c4);
//	    else     *p1++  = g1 * (c4);
//	    c5 += 0.5 * (c4 - c5);
//	}
//    }
//    while (len);
//
//    _c1 = c1;
//    _c2 = c2;
//    _c3 = c3;
//    _c4 = c4;
//    _c5 = c5;
//    _w = w;
//    _r = r;
}



void Ladspa_Moogvcf3::setport (unsigned long port, LADSPA_Data *data)
{
    _port [port] = data;
}


void Ladspa_Moogvcf3::active (bool act)
{
    _c1 = _c2 = _c3 = _c4 = _c5 = _w = _r = 0;
}


void Ladspa_Moogvcf3::runproc (unsigned long len, bool add)
{
 //   int   k;
//    float *p0, *p1, *p2, *p3, *p4;
//    float c1, c2, c3, c4, c5;
//    float g0, g1, r, dr, w, dw, x, t, d;
//
//    p0 = _port [0];
//    p1 = _port [1];
//    p2 = _port [2] - 1;
//    p3 = _port [3] - 1;
//    p4 = _port [4] - 1;
//    g0 = exp2ap (0.1661 * _port  [5][0]) / 2;
//    g1 = exp2ap (0.1661 * _port [10][0]) * 2;
//    if (add) g1 *= _gain;
//
//    c1 = _c1;
//    c2 = _c2;
//    c3 = _c3;
//    c4 = _c4;
//    c5 = _c5;
//    w = _w; 
//    r = _r;
//
//    do
//    {
//        k = (len > 24) ? 16 : len;
//        p2 += k;
//        p3 += k;
//        p4 += k;
//        len -= k;
//
//        t = exp2ap (_port [7][0] * *p3 + _port [6][0] + *p2 + 9.70) / _fsam;
//        if (t < 0.75) t *= 1.005 - t * (0.624 - t * (0.65 - t * 0.54));
//        else
//	{
//	    t *= 0.6748;
//            if (t > 0.82) t = 0.82;
//	}
//        dw = (t - w) / k;
//
//        t = _port [9][0] * *p4 + _port [8][0];
//        if (t > 1) t = 1;
//        if (t < 0) t = 0;
//        dr = (t - r) / k;  
//
//        while (k--)
//	{
//            w += dw;                        
//            r += dr;
//
//	    x = *p0 * g0 - (4.3 - 0.2 * w) * r * c5 + 1e-10; 
////            x = tanh (x);
//            x /= sqrt (1 + x * x);
//            d = w * (x  - c1) / (1 + c1 * c1);            
//            x = c1 + 0.77 * d;
//            c1 = x + 0.23 * d;        
//            d = w * (x  - c2) / (1 + c2 * c2);            
//            x = c2 + 0.77 * d;
//            c2 = x + 0.23 * d;        
//            d = w * (x  - c3) / (1 + c3 * c3);            
//            x = c3 + 0.77 * d;
//            c3 = x + 0.23 * d;        
//            d = w * (x  - c4);
//            x = c4 + 0.77 * d;
//            c4 = x + 0.23 * d;        
//            c5 += 0.85 * (c4 - c5);
//
//	    x = *p0++ * g0 -(4.3 - 0.2 * w) * r * c5;
////            x = tanh (x);
//            x /= sqrt (1 + x * x);
//            d = w * (x  - c1) / (1 + c1 * c1);            
//            x = c1 + 0.77 * d;
//            c1 = x + 0.23 * d;        
//            d = w * (x  - c2) / (1 + c2 * c2);            
//            x = c2 + 0.77 * d;
//            c2 = x + 0.23 * d;        
//            d = w * (x  - c3) / (1 + c3 * c3);            
//            x = c3 + 0.77 * d;
//            c3 = x + 0.23 * d;        
//            d = w * (x  - c4);
//            x = c4 + 0.77 * d;
//            c4 = x + 0.23 * d;        
//            c5 += 0.85 * (c4 - c5);
//
//	    if (add) *p1++ += g1 * c4;
//            else     *p1++  = g1 * c4;
//	}
//    }
//    while (len);
//
//    _c1 = c1;
//    _c2 = c2;
//    _c3 = c3;
//    _c4 = c4;
//    _c5 = c5;
//    _w = w;
//    _r = r;
}


void Ladspa_Moogvcf4::setport (unsigned long port, LADSPA_Data *data)
{
    _port [port] = data;
}


void Ladspa_Moogvcf4::active (bool act)
{
    _c1 = _c2 = _c3 = _c4 = _c5 = _w = _r = 0;
}


void Ladspa_Moogvcf4::runproc (unsigned long len, bool add)
{
    int   k, op;
    float *p0, *p1, *p2, *p3, *p4;
    float c1, c2, c3, c4, c5;
    float g0, g1, r, dr, w, dw, x, t, d, y;

    p0 = _port [0];
    p1 = _port [1];
    p2 = _port [2] - 1;
    p3 = _port [3] - 1;
    p4 = _port [4] - 1;
    g0 = exp2ap (0.1661 * _port  [5][0]) / 2;
    g1 = exp2ap (0.1661 * _port [11][0]) * 2;
    op = (int)(floor (_port [10][0] + 0.5));
    if (add) g1 *= _gain;

    c1 = _c1 + 1e-6;
    c2 = _c2;
    c3 = _c3;
    c4 = _c4;
    c5 = _c5;
    w = _w; 
    r = _r;

    do
    {
        k = (len > 24) ? 16 : len;
        p2 += k;
        p3 += k;
        p4 += k;
        len -= k;

        t = exp2ap (_port [7][0] * *p3 + _port [6][0] + *p2 + 9.70) / _fsam;
        if (t < 0.75) t *= 1.005 - t * (0.624 - t * (0.65 - t * 0.54));
        else
	{
	    t *= 0.6748;
            if (t > 0.82) t = 0.82;
	}
        dw = (t - w) / k;

        t = _port [9][0] * *p4 + _port [8][0];
        if (t > 1) t = 1;
        if (t < 0) t = 0;
        dr = (t - r) / k;  

        while (k--)
	{
            w += dw;                        
            r += dr;

	    x = *p0 * g0 - (4.3 - 0.2 * w) * r * c5 + 1e-10; 
//            x = tanh (x);
            x /= sqrt (1 + x * x);
	    d = w * (x  - c1) / (1 + c1 * c1);           
            x = c1 + 0.77 * d;
            c1 = x + 0.23 * d;        
            d = w * (x  - c2) / (1 + c2 * c2);            
            x = c2 + 0.77 * d;
            c2 = x + 0.23 * d;        
            d = w * (x  - c3) / (1 + c3 * c3);            
            x = c3 + 0.77 * d;
            c3 = x + 0.23 * d;        
            d = w * (x  - c4);
            x = c4 + 0.77 * d;
            c4 = x + 0.23 * d;        
            c5 += 0.85 * (c4 - c5);

	    x = y = *p0++ * g0 -(4.3 - 0.2 * w) * r * c5;
//            x = tanh (x);
            x /= sqrt (1 + x * x);
            d = w * (x  - c1) / (1 + c1 * c1);            
            x = c1 + 0.77 * d;
            c1 = x + 0.23 * d;        
            d = w * (x  - c2) / (1 + c2 * c2);            
            x = c2 + 0.77 * d;
            c2 = x + 0.23 * d;        
            d = w * (x  - c3) / (1 + c3 * c3);            
            x = c3 + 0.77 * d;
            c3 = x + 0.23 * d;        
            d = w * (x  - c4);
            x = c4 + 0.77 * d;
            c4 = x + 0.23 * d;        
            c5 += 0.85 * (c4 - c5);

            switch (op)
	    {
            case 1: y = c1; break;
            case 2: y = c2; break;
            case 3: y = c3; break;
            case 4: y = c4; break;
	    }

	    if (add) *p1++ += g1 * y;
            else     *p1++  = g1 * y;
	}
    }
    while (len);

    _c1 = c1;
    _c2 = c2;
    _c3 = c3;
    _c4 = c4;
    _c5 = c5;
    _w = w;
    _r = r;
}


