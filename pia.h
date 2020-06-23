/*
  pia.h

  calculate the area of intersection of a pair of polygons

  This code is a derived from aip.c by Norman Hardy, the original
  can be downoaded from

    http://www.cap-lore.com/MathPhys/IP/

  J.J. Green 2010, 2015
  -added license text / E.Bechet 2017
*/

/*
  The MIT License (MIT)

Copyright (c) 2010, 2015 J.J. Green

Derived, with permission, from the file aip.c by Norman Hardy available from

  http://www.cap-lore.com/MathPhys/IP/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#ifndef PIA_H
#define PIA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

  typedef struct { float x, y; } point_t;
  extern float pia_area(const point_t*, size_t, const point_t*, size_t);

#ifdef __cplusplus
}
#endif

#endif
