/*=========================================================================

  Program:   Visualization Toolkit
  Module:    mitkVtkImageOverwrite.cpp

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "mitkVtkImageOverwrite.h"

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


vtkStandardNewMacro(mitkVtkImageOverwrite);




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
mitkVtkImageOverwrite::mitkVtkImageOverwrite()
{
  m_Overwrite_Mode = false;
  this->GetOutput()->SetScalarTypeToUnsignedInt();
}

//----------------------------------------------------------------------------
mitkVtkImageOverwrite::~mitkVtkImageOverwrite()
{
}




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
static int vtkNearestNeighborInterpolation(T *&outPtr, const T *inPtr,
                                    const int inExt[6],
                                    const vtkIdType inInc[3],
                                    int numscalars, const F point[3],
                                    int mode, const T *background,
                                    mitkVtkImageOverwrite *self)
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

      if(!self->IsOverwriteMode())
      {
        //just copy from input to output
        *outPtr++ = *inPtr++;
      }
      else
      {
        //copy from output to input in overwrite mode
        *(const_cast<T*>(inPtr)) = *outPtr++;
        inPtr++;
      }
    }
  while (--numscalars);

  return 1;
}


//--------------------------------------------------------------------------
// get appropriate interpolation function according to scalar type
template<class F>
static void vtkGetResliceInterpFunc(mitkVtkImageOverwrite *self,
                             int (**interpolate)(void *&outPtr,
                                                 const void *inPtr,
                                                 const int inExt[6],
                                                 const vtkIdType inInc[3],
                                                 int numscalars,
                                                 const F point[3],
                                                 int mode,
                                                 const void *background,
                                                 mitkVtkImageOverwrite *self))
{
  int dataType = self->GetOutput()->GetScalarType();

  switch (dataType)
  {
    vtkTemplateAliasMacro(*((int (**)(VTK_TT *&outPtr, const VTK_TT *inPtr,
      const int inExt[6],
      const vtkIdType inInc[3],
      int numscalars, const F point[3],
      int mode,
      const VTK_TT *background,
      mitkVtkImageOverwrite *self))interpolate) = \
      &vtkNearestNeighborInterpolation);
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
static void vtkGetSetPixelsFunc(mitkVtkImageOverwrite *self,
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
static void vtkAllocBackgroundPixelT(mitkVtkImageOverwrite *self,
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

static void vtkAllocBackgroundPixel(mitkVtkImageOverwrite *self, void **rval,
                             int numComponents)
{
  switch (self->GetOutput()->GetScalarType())
    {
    vtkTemplateAliasMacro(vtkAllocBackgroundPixelT(self, (VTK_TT **)rval,
                                              numComponents));
    }
}

static void vtkFreeBackgroundPixel(mitkVtkImageOverwrite *self, void **rval)
{
  switch (self->GetOutput()->GetScalarType())
    {
    vtkTemplateAliasMacro(delete [] *((VTK_TT **)rval));
    }

  *rval = 0;
}

//----------------------------------------------------------------------------
// helper function for clipping of the output with a stencil
static int vtkResliceGetNextExtent(vtkImageStencilData *stencil,
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
static void vtkImageResliceClearExecute(mitkVtkImageOverwrite *self,
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
static void vtkImageResliceExecute(mitkVtkImageOverwrite *self,
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
                     int mode, const void *background, mitkVtkImageOverwrite *self);
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
                      point, mode, background, self);
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


void mitkVtkImageOverwrite::SetOverwriteMode(bool b){
  m_Overwrite_Mode = b;
}


void mitkVtkImageOverwrite::SetInputSlice(vtkImageData* slice){
  //set the output as input
  this->SetOutput(slice);
}




//----------------------------------------------------------------------------
// This method is passed a input and output region, and executes the filter
// algorithm to fill the output from the input or vice versa.
void mitkVtkImageOverwrite::ThreadedRequestData(
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


