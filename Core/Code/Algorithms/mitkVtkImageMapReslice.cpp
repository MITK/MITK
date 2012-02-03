/*=========================================================================

  Program:   Visualization Toolkit
  Module:    mitkVtkImageMapReslice.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "mitkVtkImageMapReslice.h"

#include "vtkImageData.h"
#include "vtkImageStencilData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTransform.h"
#include "vtkDataSetAttributes.h"
#include "vtkGarbageCollector.h"

#include "vtkTemplateAliasMacro.h"
// turn off 64-bit ints when templating over all types
# undef VTK_USE_INT64
# define VTK_USE_INT64 0
# undef VTK_USE_UINT64
# define VTK_USE_UINT64 0

#include <limits.h>
#include <float.h>
#include <math.h>

vtkStandardNewMacro(mitkVtkImageMapReslice);


//--------------------------------------------------------------------------
// The 'floor' function on x86 and mips is many times slower than these
// and is used a lot in this code, optimize for different CPU architectures
template<class F>
inline int vtkResliceFloor(double x, F &f)
{
#if defined mips || defined sparc || defined __ppc__
  x += 2147483648.0;
  unsigned int i = static_cast<unsigned int>(x);
  f = x - i;
  return static_cast<int>(i - 2147483648U);
#elif defined i386 || defined _M_IX86
  union { double d; unsigned short s[4]; unsigned int i[2]; } dual;
  dual.d = x + 103079215104.0;  // (2**(52-16))*1.5
  f = dual.s[0]*0.0000152587890625; // 2**(-16)
  return static_cast<int>((dual.i[1]<<16)|((dual.i[0])>>16));
#elif defined ia64 || defined __ia64__ || defined IA64
  x += 103079215104.0;
  long long i = static_cast<long long>(x);
  f = x - i;
  return static_cast<int>(i - 103079215104LL);
#else
  double y = floor(x);
  f = x - y;
  return static_cast<int>(y);
#endif
}

inline int vtkResliceRound(double x)
{
#if defined mips || defined sparc || defined __ppc__
  return static_cast<int>(static_cast<unsigned int>(x + 2147483648.5) - 2147483648U);
#elif defined i386 || defined _M_IX86
  union { double d; unsigned int i[2]; } dual;
  dual.d = x + 103079215104.5;  // (2**(52-16))*1.5
  return static_cast<int>((dual.i[1]<<16)|((dual.i[0])>>16));
#elif defined ia64 || defined __ia64__ || defined IA64
  x += 103079215104.5;
  long long i = static_cast<long long>(x);
  return static_cast<int>(i - 103079215104LL);
#else
  return static_cast<int>(floor(x+0.5));
#endif
}

//----------------------------------------------------------------------------
mitkVtkImageMapReslice::mitkVtkImageMapReslice()
{
}

//----------------------------------------------------------------------------
mitkVtkImageMapReslice::~mitkVtkImageMapReslice()
{
}




//----------------------------------------------------------------------------
//  Interpolation subroutines and associated code 
//----------------------------------------------------------------------------

// Three interpolation functions are supported: NearestNeighbor, Trilinear,
// and Tricubic.  These routines have the following signature:
//
//int interpolate(T *&outPtr,
//                const T *inPtr,
//                const int inExt[6],
//                const vtkIdType inInc[3],
//                int numscalars,
//                const F point[3],
//                int mode,
//                const T *background)
//
// where 'T' is any arithmetic type and 'F' is a float type
//
// The result of the interpolation is put in *outPtr, and outPtr is
// incremented.

//----------------------------------------------------------------------------
// constants for different boundary-handling modes

#define VTK_RESLICE_BACKGROUND 0   // use background if out-of-bounds
#define VTK_RESLICE_WRAP       1   // wrap to opposite side of image
#define VTK_RESLICE_MIRROR     2   // mirror off of the boundary 
#define VTK_RESLICE_BORDER     3   // use a half-voxel border
#define VTK_RESLICE_NULL       4   // do nothing to *outPtr if out-of-bounds

//----------------------------------------------------------------------------
// rounding functions for each type, where 'F' is a floating-point type

#if (VTK_USE_INT8 != 0)
template <class F>
inline void vtkResliceRound(F val, vtkTypeInt8& rnd)
{
  rnd = vtkResliceRound(val);
}
#endif

#if (VTK_USE_UINT8 != 0)
template <class F>
inline void vtkResliceRound(F val, vtkTypeUInt8& rnd)
{
  rnd = vtkResliceRound(val);
}
#endif

#if (VTK_USE_INT16 != 0)
template <class F>
inline void vtkResliceRound(F val, vtkTypeInt16& rnd)
{
  rnd = vtkResliceRound(val);
}
#endif

#if (VTK_USE_UINT16 != 0)
template <class F>
inline void vtkResliceRound(F val, vtkTypeUInt16& rnd)
{
  rnd = vtkResliceRound(val);
}
#endif

#if (VTK_USE_INT32 != 0)
template <class F>
inline void vtkResliceRound(F val, vtkTypeInt32& rnd)
{
  rnd = vtkResliceRound(val);
}
#endif

#if (VTK_USE_UINT32 != 0)
template <class F>
inline void vtkResliceRound(F val, vtkTypeUInt32& rnd)
{
  rnd = vtkResliceRound(val);
}
#endif

#if (VTK_USE_FLOAT32 != 0)
template <class F>
inline void vtkResliceRound(F val, vtkTypeFloat32& rnd)
{
  rnd = val;
}
#endif

#if (VTK_USE_FLOAT64 != 0)
template <class F>
inline void vtkResliceRound(F val, vtkTypeFloat64& rnd)
{
  rnd = val;
}
#endif

//----------------------------------------------------------------------------
// clamping functions for each type

#if (VTK_USE_INT8 != 0)
template <class F>
inline void vtkResliceClamp(F val, vtkTypeInt8& clamp)
{
  if (val < -128.0)
    { 
    val = -128.0;
    }
  if (val > 127.0)
    { 
    val = 127.0;
    }
  vtkResliceRound(val,clamp);
}
#endif

#if (VTK_USE_UINT8 != 0)
template <class F>
inline void vtkResliceClamp(F val, vtkTypeUInt8& clamp)
{
  if (val < 0)
    { 
    val = 0;
    }
  if (val > 255.0)
    { 
    val = 255.0;
    }
  vtkResliceRound(val,clamp);
}
#endif

#if (VTK_USE_INT16 != 0)
template <class F>
inline void vtkResliceClamp(F val, vtkTypeInt16& clamp)
{
  if (val < -32768.0)
    { 
    val = -32768.0;
    }
  if (val > 32767.0)
    { 
    val = 32767.0;
    }
  vtkResliceRound(val,clamp);
}
#endif

#if (VTK_USE_UINT16 != 0)
template <class F>
inline void vtkResliceClamp(F val, vtkTypeUInt16& clamp)
{
  if (val < 0)
    { 
    val = 0;
    }
  if (val > 65535.0)
    { 
    val = 65535.0;
    }
  vtkResliceRound(val,clamp);
}
#endif

#if (VTK_USE_INT32 != 0)
template <class F>
inline void vtkResliceClamp(F val, vtkTypeInt32& clamp)
{
  if (val < -2147483648.0) 
    {
    val = -2147483648.0;
    }
  if (val > 2147483647.0) 
    {
    val = 2147483647.0;
    }
  vtkResliceRound(val,clamp);
}
#endif

#if (VTK_USE_UINT32 != 0)
template <class F>
inline void vtkResliceClamp(F val, vtkTypeUInt32& clamp)
{
  if (val < 0)
    { 
    val = 0;
    }
  if (val > 4294967295.0)
    { 
    val = 4294967295.0;
    }
  vtkResliceRound(val,clamp);
}
#endif

#if (VTK_USE_FLOAT32 != 0)
template <class F>
inline void vtkResliceClamp(F val, vtkTypeFloat32& clamp)
{
  clamp = val;
}
#endif

#if (VTK_USE_FLOAT64 != 0)
template <class F>
inline void vtkResliceClamp(F val, vtkTypeFloat64& clamp)
{
  clamp = val;
}
#endif

//----------------------------------------------------------------------------
// Perform a wrap to limit an index to [0,range).
// Ensures correct behaviour when the index is negative.
 
inline int vtkInterpolateWrap(int num, int range)
{
  if ((num %= range) < 0)
    {
    num += range; // required for some % implementations
    } 
  return num;
}

//----------------------------------------------------------------------------
// Perform a mirror to limit an index to [0,range).
 
inline int vtkInterpolateMirror(int num, int range)
{
  if (num < 0)
    {
    num = -num - 1;
    }
  int count = num/range;
  num %= range;
  if (count & 0x1)
    {
    num = range - num - 1;
    }
  return num;
}

//----------------------------------------------------------------------------
// If the value is within one half voxel of the range [0,inExtX), then
// set it to "0" or "inExtX-1" as appropriate.

inline  int vtkInterpolateBorder(int &inIdX0, int &inIdX1, int inExtX,
                                 double fx)
{
  if (inIdX0 >= 0 && inIdX1 < inExtX)
    {
    return 0;
    }
  if (inIdX0 == -1 && fx >= 0.5)
    {
    inIdX1 = inIdX0 = 0;
    return 0;
    }
  if (inIdX0 == inExtX - 1 && fx < 0.5)
    {
    inIdX1 = inIdX0;
    return 0;
    }

  return 1;
}

inline  int vtkInterpolateBorderCheck(int inIdX0, int inIdX1, int inExtX,
                                      double fx)
{
  if ((inIdX0 >= 0 && inIdX1 < inExtX) ||
      (inIdX0 == -1 && fx >= 0.5) ||
      (inIdX0 == inExtX - 1 && fx < 0.5))
    {
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
// Do nearest-neighbor interpolation of the input data 'inPtr' of extent 
// 'inExt' at the 'point'.  The result is placed at 'outPtr'.  
// If the lookup data is beyond the extent 'inExt', set 'outPtr' to
// the background color 'background'.  
// The number of scalar components in the data is 'numscalars'
template <class F, class T>
int vtkNearestNeighborInterpolation(T *&outPtr, const T *inPtr,
                                    const int inExt[6],
                                    const vtkIdType inInc[3],
                                    int numscalars, const F point[3],
                                    int mode, const T *background)
{
  int inIdX0 = vtkResliceRound(point[0]) - inExt[0];
  int inIdY0 = vtkResliceRound(point[1]) - inExt[2];
  int inIdZ0 = vtkResliceRound(point[2]) - inExt[4];

  int inExtX = inExt[1] - inExt[0] + 1;
  int inExtY = inExt[3] - inExt[2] + 1;
  int inExtZ = inExt[5] - inExt[4] + 1;

  if (inIdX0 < 0 || inIdX0 >= inExtX ||
      inIdY0 < 0 || inIdY0 >= inExtY ||
      inIdZ0 < 0 || inIdZ0 >= inExtZ)
    {
    if (mode == VTK_RESLICE_WRAP)
      {
      inIdX0 = vtkInterpolateWrap(inIdX0, inExtX);
      inIdY0 = vtkInterpolateWrap(inIdY0, inExtY);
      inIdZ0 = vtkInterpolateWrap(inIdZ0, inExtZ);
      }
    else if (mode == VTK_RESLICE_MIRROR)
      {
      inIdX0 = vtkInterpolateMirror(inIdX0, inExtX);
      inIdY0 = vtkInterpolateMirror(inIdY0, inExtY);
      inIdZ0 = vtkInterpolateMirror(inIdZ0, inExtZ);
      }
    else if (mode == VTK_RESLICE_BACKGROUND ||
             mode == VTK_RESLICE_BORDER)
      {
      do
        {
        *outPtr++ = *background++;
        }
      while (--numscalars);
      return 0;
      }
    else
      {
      return 0;
      }
    }

  inPtr += inIdX0*inInc[0]+inIdY0*inInc[1]+inIdZ0*inInc[2];
  do
    {
    *outPtr++ = *inPtr++;
    }
  while (--numscalars);

  return 1;
} 

//----------------------------------------------------------------------------
// Do trilinear interpolation of the input data 'inPtr' of extent 'inExt'
// at the 'point'.  The result is placed at 'outPtr'.  
// If the lookup data is beyond the extent 'inExt', set 'outPtr' to
// the background color 'background'.  
// The number of scalar components in the data is 'numscalars'
template <class F, class T>
int vtkTrilinearInterpolation(T *&outPtr, const T *inPtr,
                              const int inExt[6], const vtkIdType inInc[3],
                              int numscalars, const F point[3],
                              int mode, const T *background)
{
  F fx, fy, fz;
  int floorX = vtkResliceFloor(point[0], fx);
  int floorY = vtkResliceFloor(point[1], fy);
  int floorZ = vtkResliceFloor(point[2], fz);

  int inIdX0 = floorX - inExt[0];
  int inIdY0 = floorY - inExt[2];
  int inIdZ0 = floorZ - inExt[4];

  int inIdX1 = inIdX0 + (fx != 0);
  int inIdY1 = inIdY0 + (fy != 0);
  int inIdZ1 = inIdZ0 + (fz != 0);

  int inExtX = inExt[1] - inExt[0] + 1;
  int inExtY = inExt[3] - inExt[2] + 1;
  int inExtZ = inExt[5] - inExt[4] + 1;

  if (inIdX0 < 0 || inIdX1 >= inExtX ||
      inIdY0 < 0 || inIdY1 >= inExtY ||
      inIdZ0 < 0 || inIdZ1 >= inExtZ)
    {
    if (mode == VTK_RESLICE_BORDER)
      {
      if (vtkInterpolateBorder(inIdX0, inIdX1, inExtX, fx) ||
          vtkInterpolateBorder(inIdY0, inIdY1, inExtY, fy) ||
          vtkInterpolateBorder(inIdZ0, inIdZ1, inExtZ, fz))
        {
        do
          {
          *outPtr++ = *background++;
          }
        while (--numscalars);
        return 0;
        }
      }
    else if (mode == VTK_RESLICE_WRAP)
      {
      inIdX0 = vtkInterpolateWrap(inIdX0, inExtX);
      inIdY0 = vtkInterpolateWrap(inIdY0, inExtY);
      inIdZ0 = vtkInterpolateWrap(inIdZ0, inExtZ);

      inIdX1 = vtkInterpolateWrap(inIdX1, inExtX);
      inIdY1 = vtkInterpolateWrap(inIdY1, inExtY);
      inIdZ1 = vtkInterpolateWrap(inIdZ1, inExtZ);
      }
    else if (mode == VTK_RESLICE_MIRROR)
      {
      inIdX0 = vtkInterpolateMirror(inIdX0, inExtX);
      inIdY0 = vtkInterpolateMirror(inIdY0, inExtY);
      inIdZ0 = vtkInterpolateMirror(inIdZ0, inExtZ);

      inIdX1 = vtkInterpolateMirror(inIdX1, inExtX);
      inIdY1 = vtkInterpolateMirror(inIdY1, inExtY);
      inIdZ1 = vtkInterpolateMirror(inIdZ1, inExtZ);
      }
    else if (mode == VTK_RESLICE_BACKGROUND)
      {
      do
        {
        *outPtr++ = *background++;
        }
      while (--numscalars);
      return 0;
      }
    else
      {
      return 0;
      }
    }

  vtkIdType factX0 = inIdX0*inInc[0];
  vtkIdType factX1 = inIdX1*inInc[0];
  vtkIdType factY0 = inIdY0*inInc[1];
  vtkIdType factY1 = inIdY1*inInc[1];
  vtkIdType factZ0 = inIdZ0*inInc[2];
  vtkIdType factZ1 = inIdZ1*inInc[2];

  vtkIdType i00 = factY0 + factZ0;
  vtkIdType i01 = factY0 + factZ1;
  vtkIdType i10 = factY1 + factZ0;
  vtkIdType i11 = factY1 + factZ1;

  F rx = 1 - fx;
  F ry = 1 - fy;
  F rz = 1 - fz;

  F ryrz = ry*rz;
  F fyrz = fy*rz;
  F ryfz = ry*fz;
  F fyfz = fy*fz;

  const T *inPtr0 = inPtr + factX0;
  const T *inPtr1 = inPtr + factX1;

  do
    {
    F result = (rx*(ryrz*inPtr0[i00] + ryfz*inPtr0[i01] +
                    fyrz*inPtr0[i10] + fyfz*inPtr0[i11]) +
                fx*(ryrz*inPtr1[i00] + ryfz*inPtr1[i01] +
                    fyrz*inPtr1[i10] + fyfz*inPtr1[i11]));

    vtkResliceRound(result, *outPtr++);
    inPtr0++;
    inPtr1++;
    }
  while (--numscalars);

  return 1;
}

//----------------------------------------------------------------------------
// Do tricubic interpolation of the input data 'inPtr' of extent 'inExt' 
// at the 'point'.  The result is placed at 'outPtr'.  
// The number of scalar components in the data is 'numscalars'
// The tricubic interpolation ensures that both the intensity and
// the first derivative of the intensity are smooth across the
// image.  The first derivative is estimated using a 
// centered-difference calculation.


// helper function: set up the lookup indices and the interpolation 
// coefficients

template <class T>
void vtkTricubicInterpCoeffs(T F[4], int l, int h, T f)
{
  static const T half = T(0.5);

  int order = h - l;

  if (order == 0)
    { // no interpolation
    F[0] = 0;
    F[1] = 1;
    F[2] = 0;
    F[3] = 0;
    return;
    }
  if (order == 3)
    { // cubic interpolation
    T fm1 = f - 1;
    T fd2 = f*half;
    T ft3 = f*3; 
    F[0] = -fd2*fm1*fm1;
    F[1] = ((ft3 - 2)*fd2 - 1)*fm1;
    F[2] = -((ft3 - 4)*f - 1)*fd2;
    F[3] = f*fd2*fm1;
    return;
    }
  if (order == 1)
    { // linear interpolation
    F[0] = 0;
    F[1] = 1 - f;
    F[2] = f;
    F[3] = 0;
    return;
    }
  if (l == 0)
    { // quadratic interpolation
    T fp1 = f + 1;
    T fm1 = f - 1; 
    T fd2 = f*half;
    F[0] = fd2*fm1;
    F[1] = -fp1*fm1;
    F[2] = fp1*fd2;
    F[3] = 0;
    return;
    }
  // else
    { // quadratic interpolation
    T fm1 = f - 1;
    T fm2 = fm1 - 1;
    T fm1d2 = fm1*half;
    F[0] = 0;
    F[1] = fm1d2*fm2;
    F[2] = -f*fm2;
    F[3] = f*fm1d2;
    return;
    }
}

// tricubic interpolation
template <class F, class T>
int vtkTricubicInterpolation(T *&outPtr, const T *inPtr,
                             const int inExt[6], const vtkIdType inInc[3],
                             int numscalars, const F point[3],
                             int mode, const T *background)
{
  F fx, fy, fz;
  int floorX = vtkResliceFloor(point[0], fx);
  int floorY = vtkResliceFloor(point[1], fy);
  int floorZ = vtkResliceFloor(point[2], fz);

  int fxIsNotZero = (fx != 0);
  int fyIsNotZero = (fy != 0);
  int fzIsNotZero = (fz != 0);

  int inIdX0 = floorX - inExt[0];
  int inIdY0 = floorY - inExt[2];
  int inIdZ0 = floorZ - inExt[4];

  int inIdX1 = inIdX0 + fxIsNotZero;
  int inIdY1 = inIdY0 + fyIsNotZero;
  int inIdZ1 = inIdZ0 + fzIsNotZero;

  int inExtX = inExt[1] - inExt[0] + 1;
  int inExtY = inExt[3] - inExt[2] + 1;
  int inExtZ = inExt[5] - inExt[4] + 1;

  vtkIdType inIncX = inInc[0];
  vtkIdType inIncY = inInc[1];
  vtkIdType inIncZ = inInc[2];

  vtkIdType factX[4], factY[4], factZ[4];

  if (inIdX0 < 0 || inIdX1 >= inExtX ||
      inIdY0 < 0 || inIdY1 >= inExtY ||
      inIdZ0 < 0 || inIdZ1 >= inExtZ)
    {
    if (mode == VTK_RESLICE_BORDER)
      {
      if (vtkInterpolateBorderCheck(inIdX0, inIdX1, inExtX, fx) ||
          vtkInterpolateBorderCheck(inIdY0, inIdY1, inExtY, fy) ||
          vtkInterpolateBorderCheck(inIdZ0, inIdZ1, inExtZ, fz))
        {
        do
          {
          *outPtr++ = *background++;
          }
        while (--numscalars);
        return 0;
        }
      }
    else if (mode != VTK_RESLICE_WRAP && mode != VTK_RESLICE_MIRROR)
      {
      if (mode == VTK_RESLICE_BACKGROUND)
        {
        do
          {
          *outPtr++ = *background++;
          }
        while (--numscalars);
        return 0;
        }
      else
        {
        return 0;
        }
      }
    }

  F fX[4], fY[4], fZ[4];
  int k1, k2, j1, j2, i1, i2;

  if (mode == VTK_RESLICE_WRAP || mode == VTK_RESLICE_MIRROR)
    {
    i1 = 0;
    i2 = 3;
    vtkTricubicInterpCoeffs(fX, i1, i2, fx);

    j1 = 1 - fyIsNotZero;
    j2 = 1 + (fyIsNotZero<<1);
    vtkTricubicInterpCoeffs(fY, j1, j2, fy);

    k1 = 1 - fzIsNotZero;
    k2 = 1 + (fzIsNotZero<<1);
    vtkTricubicInterpCoeffs(fZ, k1, k2, fz);

    if (mode == VTK_RESLICE_WRAP)
      {
      for (int i = 0; i < 4; i++)
        {
        factX[i] = vtkInterpolateWrap(inIdX0 + i - 1, inExtX)*inIncX;
        factY[i] = vtkInterpolateWrap(inIdY0 + i - 1, inExtY)*inIncY;
        factZ[i] = vtkInterpolateWrap(inIdZ0 + i - 1, inExtZ)*inIncZ;
        }
      }
    else
      {
      for (int i = 0; i < 4; i++)
        {
        factX[i] = vtkInterpolateMirror(inIdX0 + i - 1, inExtX)*inIncX;
        factY[i] = vtkInterpolateMirror(inIdY0 + i - 1, inExtY)*inIncY;
        factZ[i] = vtkInterpolateMirror(inIdZ0 + i - 1, inExtZ)*inIncZ;
        }
      }
    }
  else if (mode == VTK_RESLICE_BORDER)
    {
    // clamp to the border of the input extent
   
    i1 = 1 - fxIsNotZero;
    j1 = 1 - fyIsNotZero;
    k1 = 1 - fzIsNotZero;

    i2 = 1 + 2*fxIsNotZero;
    j2 = 1 + 2*fyIsNotZero;
    k2 = 1 + 2*fzIsNotZero;
    
    vtkTricubicInterpCoeffs(fX, i1, i2, fx);
    vtkTricubicInterpCoeffs(fY, j1, j2, fy);
    vtkTricubicInterpCoeffs(fZ, k1, k2, fz);

    int tmpExt = inExtX - 1;
    int tmpId = tmpExt - inIdX0 - 1;
    factX[0] = (inIdX0 - 1)*(inIdX0 - 1 >= 0)*inIncX;
    factX[1] = (inIdX0)*(inIdX0 >= 0)*inIncX;
    factX[2] = (tmpExt - (tmpId)*(tmpId >= 0))*inIncX;
    factX[3] = (tmpExt - (tmpId - 1)*(tmpId - 1 >= 0))*inIncX;

    tmpExt = inExtY - 1;
    tmpId = tmpExt - inIdY0 - 1;
    factY[0] = (inIdY0 - 1)*(inIdY0 - 1 >= 0)*inIncY;
    factY[1] = (inIdY0)*(inIdY0 >= 0)*inIncY;
    factY[2] = (tmpExt - (tmpId)*(tmpId >= 0))*inIncY;
    factY[3] = (tmpExt - (tmpId - 1)*(tmpId - 1 >= 0))*inIncY;

    tmpExt = inExtZ - 1;
    tmpId = tmpExt - inIdZ0 - 1;
    factZ[0] = (inIdZ0 - 1)*(inIdZ0 - 1 >= 0)*inIncZ;
    factZ[1] = (inIdZ0)*(inIdZ0 >= 0)*inIncZ;
    factZ[2] = (tmpExt - (tmpId)*(tmpId >= 0))*inIncZ;
    factZ[3] = (tmpExt - (tmpId - 1)*(tmpId - 1 >= 0))*inIncZ;
    }
  else
    {
    // depending on whether we are at the edge of the 
    // input extent, choose the appropriate interpolation
    // method to use

    i1 = 1 - (inIdX0 > 0)*fxIsNotZero;
    j1 = 1 - (inIdY0 > 0)*fyIsNotZero;
    k1 = 1 - (inIdZ0 > 0)*fzIsNotZero;

    i2 = 1 + (1 + (inIdX0 + 2 < inExtX))*fxIsNotZero;
    j2 = 1 + (1 + (inIdY0 + 2 < inExtY))*fyIsNotZero;
    k2 = 1 + (1 + (inIdZ0 + 2 < inExtZ))*fzIsNotZero;

    vtkTricubicInterpCoeffs(fX, i1, i2, fx);
    vtkTricubicInterpCoeffs(fY, j1, j2, fy);
    vtkTricubicInterpCoeffs(fZ, k1, k2, fz);

    factX[1] = inIdX0*inIncX;
    factX[0] = factX[1] - inIncX;
    factX[2] = factX[1] + inIncX;
    factX[3] = factX[2] + inIncX;

    factY[1] = inIdY0*inIncY;
    factY[0] = factY[1] - inIncY;
    factY[2] = factY[1] + inIncY;
    factY[3] = factY[2] + inIncY;
    
    factZ[1] = inIdZ0*inIncZ;
    factZ[0] = factZ[1] - inIncZ;
    factZ[2] = factZ[1] + inIncZ;
    factZ[3] = factZ[2] + inIncZ;

    // this little bit of wierdness allows us to unroll the x loop
    if (i1 > 0)
      {
      factX[0] = factX[1];
      }
    if (i2 < 3)
      {
      factX[3] = factX[1];
      if (i2 < 2)
        {
        factX[2] = factX[1];
        }
      }
    }
  
  do // loop over components
    {
    F val = 0;
    int k = k1;
    do // loop over z
      {
      F ifz = fZ[k];
      vtkIdType factz = factZ[k];
      int j = j1;
      do // loop over y
        {
        F ify = fY[j];
        F fzy = ifz*ify;
        vtkIdType factzy = factz + factY[j];
        const T *tmpPtr = inPtr + factzy;
        // loop over x is unrolled (significant performance boost)
        val += fzy*(fX[0]*tmpPtr[factX[0]] +
                    fX[1]*tmpPtr[factX[1]] +
                    fX[2]*tmpPtr[factX[2]] +
                    fX[3]*tmpPtr[factX[3]]);
        }
      while (++j <= j2);
      }
    while (++k <= k2);

    vtkResliceClamp(val, *outPtr++);
    inPtr++;
    }
  while (--numscalars);

  return 1;
}                   

//--------------------------------------------------------------------------
// get appropriate interpolation function according to interpolation mode
// and scalar type
template<class F>
void vtkGetResliceInterpFunc(mitkVtkImageMapReslice *self,
                             int (**interpolate)(void *&outPtr,
                                                 const void *inPtr,
                                                 const int inExt[6],
                                                 const vtkIdType inInc[3],
                                                 int numscalars,
                                                 const F point[3],
                                                 int mode,
                                                 const void *background))
{
  int dataType = self->GetOutput()->GetScalarType();
  int interpolationMode = self->GetInterpolationMode();
  
  switch (interpolationMode)
    {
    case VTK_RESLICE_NEAREST:
      switch (dataType)
        {
        vtkTemplateAliasMacro(*((int (**)(VTK_TT *&outPtr, const VTK_TT *inPtr,
                                     const int inExt[6],
                                     const vtkIdType inInc[3],
                                     int numscalars, const F point[3],
                                     int mode,
                                     const VTK_TT *background))interpolate) = \
                         &vtkNearestNeighborInterpolation);
        default:
          interpolate = 0;
        }
      break;
    case VTK_RESLICE_LINEAR:
    case VTK_RESLICE_RESERVED_2:
      switch (dataType)
        {
        vtkTemplateAliasMacro(*((int (**)(VTK_TT *&outPtr, const VTK_TT *inPtr,
                                     const int inExt[6],
                                     const vtkIdType inInc[3],
                                     int numscalars, const F point[3],
                                     int mode,
                                     const VTK_TT *background))interpolate) = \
                         &vtkTrilinearInterpolation);
        default:
          interpolate = 0;
        }
      break;
    case VTK_RESLICE_CUBIC:
      switch (dataType)
        {
        vtkTemplateAliasMacro(*((int (**)(VTK_TT *&outPtr, const VTK_TT *inPtr,
                                     const int inExt[6],
                                     const vtkIdType inInc[3],
                                     int numscalars, const F point[3],
                                     int mode,
                                     const VTK_TT *background))interpolate) = \
                         &vtkTricubicInterpolation);
        default:
          interpolate = 0;
        }
      break;
    default:
      interpolate = 0;
    }
}


//----------------------------------------------------------------------------
// Some helper functions for 'RequestData'
//----------------------------------------------------------------------------

//--------------------------------------------------------------------------
// pixel copy function, templated for different scalar types
template <class T>
struct vtkImageResliceSetPixels
{
static void Set(void *&outPtrV, const void *inPtrV, int numscalars, int n)
{
  const T* inPtr = static_cast<const T*>(inPtrV);
  T* outPtr = static_cast<T*>(outPtrV);
  for (int i = 0; i < n; i++)
    {
    const T *tmpPtr = inPtr;
    int m = numscalars;
    do
      {
      *outPtr++ = *tmpPtr++;
      }
    while (--m);
    }
  outPtrV = outPtr;
}

// optimized for 1 scalar components
static void Set1(void *&outPtrV, const void *inPtrV,
                          int vtkNotUsed(numscalars), int n)
{
  const T* inPtr = static_cast<const T*>(inPtrV);
  T* outPtr = static_cast<T*>(outPtrV);
  T val = *inPtr;
  for (int i = 0; i < n; i++)
    {
    *outPtr++ = val;
    }
  outPtrV = outPtr;
}
};

// get a pixel copy function that is appropriate for the data type
void vtkGetSetPixelsFunc(mitkVtkImageMapReslice *self,
                         void (**setpixels)(void *&out, const void *in,
                                            int numscalars, int n))
{
  int dataType = self->GetOutput()->GetScalarType();
  int numscalars = self->GetOutput()->GetNumberOfScalarComponents();

  switch (numscalars)
    {
    case 1:
      switch (dataType)
        {
        vtkTemplateAliasMacro(
          *setpixels = &vtkImageResliceSetPixels<VTK_TT>::Set1
          );
        default:
          setpixels = 0;
        }
    default:
      switch (dataType)
        {
        vtkTemplateAliasMacro(
          *setpixels = &vtkImageResliceSetPixels<VTK_TT>::Set
          );
        default:
          setpixels = 0;
        }
    }
}

//----------------------------------------------------------------------------
// Convert background color from float to appropriate type
template <class T>
void vtkAllocBackgroundPixelT(mitkVtkImageMapReslice *self,
                              T **background_ptr, int numComponents)
{
  *background_ptr = new T[numComponents];
  T *background = *background_ptr;

  for (int i = 0; i < numComponents; i++)
    {
    if (i < 4)
      {
      vtkResliceClamp(self->GetBackgroundColor()[i], background[i]);
      }
    else
      {
      background[i] = 0;
      }
    }
}

void vtkAllocBackgroundPixel(mitkVtkImageMapReslice *self, void **rval, 
                             int numComponents)
{
  switch (self->GetOutput()->GetScalarType())
    {
    vtkTemplateAliasMacro(vtkAllocBackgroundPixelT(self, (VTK_TT **)rval,
                                              numComponents));
    }
}      

void vtkFreeBackgroundPixel(mitkVtkImageMapReslice *self, void **rval)
{
  switch (self->GetOutput()->GetScalarType())
    {
    vtkTemplateAliasMacro(delete [] *((VTK_TT **)rval));
    }

  *rval = 0;
}      

//----------------------------------------------------------------------------
// helper function for clipping of the output with a stencil
int vtkResliceGetNextExtent(vtkImageStencilData *stencil,
                            int &r1, int &r2, int rmin, int rmax,
                            int yIdx, int zIdx, 
                            void *&outPtr, void *background, 
                            int numscalars,
                            void (*setpixels)(void *&out,
                                              const void *in,
                                              int numscalars,
                                              int n),
                            int &iter)
{
  // trivial case if stencil is not set
  if (!stencil)
    {
    if (iter++ == 0)
      {
      r1 = rmin;
      r2 = rmax;
      return 1;
      }
    return 0;
    }

  // for clearing, start at last r2 plus 1
  int clear1 = r2 + 1;
  if (iter == 0)
    { // if no 'last time', start at rmin
    clear1 = rmin;
    }

  int rval = stencil->GetNextExtent(r1, r2, rmin, rmax, yIdx, zIdx, iter);
  int clear2 = r1 - 1;
  if (rval == 0)
    {
    clear2 = rmax;
    }

  setpixels(outPtr, background, numscalars, clear2 - clear1 + 1);

  return rval;
}

//----------------------------------------------------------------------------
// This function simply clears the entire output to the background color,
// for cases where the transformation places the output extent completely
// outside of the input extent.
void vtkImageResliceClearExecute(mitkVtkImageMapReslice *self,
                                 vtkImageData *, void *,
                                 vtkImageData *outData, void *outPtr,
                                 int outExt[6], int id)
{
  int numscalars;
  int idY, idZ;
  vtkIdType outIncX, outIncY, outIncZ;
  int scalarSize;
  unsigned long count = 0;
  unsigned long target;
  void *background;
  void (*setpixels)(void *&out, const void *in, int numscalars, int n);

  // for the progress meter
  target = static_cast<unsigned long>
    ((outExt[5]-outExt[4]+1)*(outExt[3]-outExt[2]+1)/50.0);
  target++;
  
  // Get Increments to march through data 
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  scalarSize = outData->GetScalarSize();
  numscalars = outData->GetNumberOfScalarComponents();

  // allocate a voxel to copy into the background (out-of-bounds) regions
  vtkAllocBackgroundPixel(self, &background, numscalars);
  // get the appropriate function for pixel copying
  vtkGetSetPixelsFunc(self, &setpixels);

  // Loop through output voxels
  for (idZ = outExt[4]; idZ <= outExt[5]; idZ++)
    {
    for (idY = outExt[2]; idY <= outExt[3]; idY++)
      {
      if (id == 0) 
        { // update the progress if this is the main thread
        if (!(count%target)) 
          {
          self->UpdateProgress(count/(50.0*target));
          }
        count++;
        }
      // clear the pixels to background color and go to next row
      setpixels(outPtr, background, numscalars, outExt[1]-outExt[0]+1);
      outPtr = static_cast<void *>(
        static_cast<char *>(outPtr) + outIncY*scalarSize);
      }
    outPtr = static_cast<void *>(
      static_cast<char *>(outPtr) + outIncZ*scalarSize);
    }

  vtkFreeBackgroundPixel(self, &background);
}

//----------------------------------------------------------------------------
// This function executes the filter for any type of data.  It is much simpler
// in structure than vtkImageResliceOptimizedExecute.
void vtkImageResliceExecute(mitkVtkImageMapReslice *self,
                            vtkImageData *inData, void *inPtr,
                            vtkImageData *outData, void *outPtr,
                            int outExt[6], int id)
{
  int numscalars;
  int idX, idY, idZ;
  int idXmin, idXmax, iter;
  vtkIdType outIncX, outIncY, outIncZ;
  int scalarSize;
  int inExt[6];
  vtkIdType inInc[3];
  unsigned long count = 0;
  unsigned long target;
  double point[4];
  double f;
  double *inSpacing, *inOrigin, *outSpacing, *outOrigin, inInvSpacing[3];
  void *background;
  int (*interpolate)(void *&outPtr, const void *inPtr,
                     const int inExt[6], const vtkIdType inInc[3],
                     int numscalars, const double point[3],
                     int mode, const void *background);
  void (*setpixels)(void *&out, const void *in, int numscalars, int n);

  // the 'mode' species what to do with the 'pad' (out-of-bounds) area
  int mode = VTK_RESLICE_BACKGROUND;
  if (self->GetMirror())
    {
    mode = VTK_RESLICE_MIRROR;
    }
  else if (self->GetWrap())
    {
    mode = VTK_RESLICE_WRAP;
    }
  else if (self->GetBorder())
    {
    mode = VTK_RESLICE_BORDER;
    }

  // the transformation to apply to the data
  vtkAbstractTransform *transform = self->GetResliceTransform();
  vtkMatrix4x4 *matrix = self->GetResliceAxes();

  // for conversion to data coordinates
  inOrigin = inData->GetOrigin();
  inSpacing = inData->GetSpacing();
  outOrigin = outData->GetOrigin();
  outSpacing = outData->GetSpacing();

  // save effor later: invert inSpacing
  inInvSpacing[0] = 1.0/inSpacing[0];
  inInvSpacing[1] = 1.0/inSpacing[1];
  inInvSpacing[2] = 1.0/inSpacing[2];

  // find maximum input range
  inData->GetExtent(inExt);
  
  // for the progress meter
  target = static_cast<unsigned long>
    ((outExt[5]-outExt[4]+1)*(outExt[3]-outExt[2]+1)/50.0);
  target++;
  
  // Get Increments to march through data 
  inData->GetIncrements(inInc);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  scalarSize = outData->GetScalarSize();
  numscalars = inData->GetNumberOfScalarComponents();

  // allocate a voxel to copy into the background (out-of-bounds) regions
  vtkAllocBackgroundPixel(self, &background, numscalars);

  // get the appropriate functions for interpolation and pixel copying
  vtkGetResliceInterpFunc(self, &interpolate);
  vtkGetSetPixelsFunc(self, &setpixels);

  // get the stencil
  vtkImageStencilData *stencil = self->GetStencil();

  // Loop through output voxels
  for (idZ = outExt[4]; idZ <= outExt[5]; idZ++)
    {
    for (idY = outExt[2]; idY <= outExt[3]; idY++)
      {
      if (id == 0) 
        { // update the progress if this is the main thread
        if (!(count%target)) 
          {
          self->UpdateProgress(count/(50.0*target));
          }
        count++;
        }
      
      iter = 0; // if there is a stencil, it is applied here
      while (vtkResliceGetNextExtent(stencil, idXmin, idXmax,
                                     outExt[0], outExt[1], idY, idZ,
                                     outPtr, background, numscalars, 
                                     setpixels, iter))
        {
        for (idX = idXmin; idX <= idXmax; idX++)
          {
          // convert to data coordinates 
          point[0] = idX*outSpacing[0] + outOrigin[0];
          point[1] = idY*outSpacing[1] + outOrigin[1];
          point[2] = idZ*outSpacing[2] + outOrigin[2];

          // apply ResliceAxes matrix
          if (matrix)
            {
            point[3] = 1.0;
            matrix->MultiplyPoint(point, point);
            f = 1.0/point[3];
            point[0] *= f;
            point[1] *= f;
            point[2] *= f;
            }

          // apply ResliceTransform
          if (transform)
            {
            transform->InternalTransformPoint(point, point);
            }
          
          // convert back to voxel indices
          point[0] = (point[0] - inOrigin[0])*inInvSpacing[0];
          point[1] = (point[1] - inOrigin[1])*inInvSpacing[1];
          point[2] = (point[2] - inOrigin[2])*inInvSpacing[2];

          // interpolate output voxel from input data set
          interpolate(outPtr, inPtr, inExt, inInc, numscalars,
                      point, mode, background);
          } 
        }
      outPtr = static_cast<void *>(
        static_cast<char *>(outPtr) + outIncY*scalarSize);
      }
    outPtr = static_cast<void *>(
      static_cast<char *>(outPtr) + outIncZ*scalarSize);
    }

  vtkFreeBackgroundPixel(self, &background);
}




//----------------------------------------------------------------------------
// This method is passed a input and output region, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the regions data types.
void mitkVtkImageMapReslice::ThreadedRequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector),
  vtkImageData ***inData,
  vtkImageData **outData,
  int outExt[6], int id)
{

  vtkDebugMacro(<< "Execute: inData = " << inData[0][0]
                      << ", outData = " << outData[0]);

  // this filter expects that input is the same type as output.
  if (inData[0][0]->GetScalarType() != outData[0]->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, "
                  << inData[0][0]->GetScalarType()
                  << ", must match out ScalarType "
                  << outData[0]->GetScalarType());
    return;
    }

  int inExt[6];
  inData[0][0]->GetExtent(inExt);
  // check for empty input extent
  if (inExt[1] < inExt[0] ||
      inExt[3] < inExt[2] ||
      inExt[5] < inExt[4])
    {
    return;
    }

  // Get the output pointer
  void *outPtr = outData[0]->GetScalarPointerForExtent(outExt);

  if (this->HitInputExtent == 0)
    {
    vtkImageResliceClearExecute(this, inData[0][0], 0, outData[0], outPtr,
                                outExt, id);
    return;
    }
  
  // Now that we know that we need the input, get the input pointer
  void *inPtr = inData[0][0]->GetScalarPointerForExtent(inExt);

  
  vtkImageResliceExecute(this, inData[0][0], inPtr, outData[0], outPtr,
                           outExt, id);
}
