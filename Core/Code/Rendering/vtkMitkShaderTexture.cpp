/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "vtkMitkShaderTexture.h"

#include "vtkHomogeneousTransform.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderer.h"
#include "vtkPointData.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkTransform.h"
#include "vtkPixelBufferObject.h"
#include "vtkOpenGL.h"
#include "vtkUnsignedShortArray.h"
#include "vtkFloatArray.h"
#include "vtkgl.h" // vtkgl namespace
#include "GL/glext.h"

#include <math.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkStandardNewMacro(vtkMitkShaderTexture);
#endif

void vtkMitkShaderTexture::SetLevelWindow(float lower, float upper)
{
  this->LevelWindowEnabled = true;
  this->LevelWindowLower = lower;
  this->LevelWindowUpper = upper;
  if(this->Index)
  {
    glBindTexture(GL_TEXTURE_2D, this->Index);
#ifdef MES_MOBILE_BUILD
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LW_LOWER, this->LevelWindowLower);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LW_UPPER, this->LevelWindowUpper);
#endif
  }
}


// ----------------------------------------------------------------------------
// Initializes an instance, generates a unique index.
vtkMitkShaderTexture::vtkMitkShaderTexture()
{
  this->Index = 0;
  this->RenderWindow = 0;
  this->LevelWindowEnabled = false;
  this->LevelWindowLower = 0.0f;
  this->LevelWindowUpper = 0.0f;

  this->Format16BIT = false;

  MapColorScalarsThroughLookupTable = 0;
}

// ----------------------------------------------------------------------------
vtkMitkShaderTexture::~vtkMitkShaderTexture()
{
  if (this->RenderWindow)
  {
    this->ReleaseGraphicsResources(this->RenderWindow);
  }
  this->RenderWindow = NULL;
}

// ----------------------------------------------------------------------------
void vtkMitkShaderTexture::Initialize(vtkRenderer * vtkNotUsed(ren))
{
}

// ----------------------------------------------------------------------------
// Release the graphics resources used by this texture.
void vtkMitkShaderTexture::ReleaseGraphicsResources(vtkWindow *renWin)
{
  if (this->Index && renWin && renWin->GetMapped())
  {
    static_cast<vtkRenderWindow *>(renWin)->MakeCurrent();

    // free any textures
    if (glIsTexture(static_cast<GLuint>(this->Index)))
    {
      GLuint tempIndex;
      tempIndex = this->Index;
      // NOTE: Sun's OpenGL seems to require disabling of texture before delete
      glDisable(GL_TEXTURE_2D);
      glDeleteTextures(1, &tempIndex);
    }
  }
  this->Index = 0;
  this->RenderWindow = NULL;
  this->Modified();
}

// ----------------------------------------------------------------------------
// Implement base class method.
void vtkMitkShaderTexture::Load(vtkRenderer *ren)
{
  GLenum format = GL_LUMINANCE;
  vtkImageData *input = this->GetInput();

  this->Initialize(ren);

  // Need to reload the texture.
  // There used to be a check on the render window's mtime, but
  // this is too broad of a check (e.g. it would cause all textures
  // to load when only the desired update rate changed).
  // If a better check is required, check something more specific,
  // like the graphics context.
//  vtkOpenGLRenderWindow* renWin =
//  static_cast<vtkOpenGLRenderWindow*>(ren->GetRenderWindow());

  if(this->BlendingMode != VTK_TEXTURE_BLENDING_MODE_NONE
     && vtkgl::ActiveTexture)
  {
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, vtkgl::COMBINE);

    switch(this->BlendingMode)
    {
      case VTK_TEXTURE_BLENDING_MODE_REPLACE:
      {
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_RGB, GL_REPLACE);
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_ALPHA, GL_REPLACE);
        break;
      }
      case VTK_TEXTURE_BLENDING_MODE_MODULATE:
      {
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_RGB, GL_MODULATE);
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_ALPHA, GL_MODULATE);
        break;
      }
      case VTK_TEXTURE_BLENDING_MODE_ADD:
      {
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_RGB, GL_ADD);
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_ALPHA, GL_ADD);
        break;
      }
      case VTK_TEXTURE_BLENDING_MODE_ADD_SIGNED:
      {
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_RGB, vtkgl::ADD_SIGNED);
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_ALPHA, vtkgl::ADD_SIGNED);
        break;
      }
      case VTK_TEXTURE_BLENDING_MODE_INTERPOLATE:
      {
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_RGB, vtkgl::INTERPOLATE);
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_ALPHA, vtkgl::INTERPOLATE);
        break;
      }
      case VTK_TEXTURE_BLENDING_MODE_SUBTRACT:
      {
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_RGB, vtkgl::SUBTRACT);
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_ALPHA, vtkgl::SUBTRACT);
        break;
      }
      default:
      {
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_RGB, GL_ADD);
        glTexEnvf (GL_TEXTURE_ENV, vtkgl::COMBINE_ALPHA, GL_ADD);
      }
    }
  }

  if (this->GetMTime() > this->LoadTime.GetMTime() ||
      input->GetMTime() > this->LoadTime.GetMTime() ||
      (this->GetLookupTable() && this->GetLookupTable()->GetMTime () > this->LoadTime.GetMTime()) )
  {
    int numChannels;
    int channelByteSize;
    int size[3];
    vtkDataArray *scalars;
    unsigned char *dataPtr;
    float *floatPtr;
    float *floatData=NULL;
    unsigned char *resultData=NULL;
    int xsize, ysize;
    GLuint tempIndex=0;

    // Get the scalars the user choose to color with.
    scalars = this->GetInputArrayToProcess(0, input);

    // make sure scalars are non null
    if (!scalars)
    {
      vtkErrorMacro(<< "No scalar values found for texture input!");
      return;
    }

    // get some info
    input->GetDimensions(size);

    if (input->GetNumberOfCells() == scalars->GetNumberOfTuples())
    {
      // we are using cell scalars. Adjust image size for cells.
      for (int kk=0; kk < 3; kk++)
      {
        if (size[kk]>1)
        {
          size[kk]--;
        }
      }
    }

    numChannels = scalars->GetNumberOfComponents();

    this->Interpolate = 1;

    // make sure using unsigned char data of color scalars type
    if (scalars->GetDataType() == VTK_UNSIGNED_SHORT)
    {
//      std::cout << " ushort mode " << std::endl;
      dataPtr = reinterpret_cast<unsigned char *>(static_cast<vtkUnsignedShortArray *>(scalars)->GetPointer(0));
      channelByteSize = 2;
//      this->Interpolate = 0;
    }
    else if (scalars->GetDataType() == VTK_FLOAT)
    {
      floatData = new float[xsize*ysize];
      floatPtr = reinterpret_cast<float *>(static_cast<vtkFloatArray *>(scalars)->GetPointer(0));
      floatData = floatPtr;
    }
    else if (this->MapColorScalarsThroughLookupTable || scalars->GetDataType() != VTK_UNSIGNED_CHAR )
    {
      dataPtr = this->MapScalarsToColors (scalars);
      numChannels = 4;
      channelByteSize = 1;
    }
    else
    {
      channelByteSize = 1;
    }

    // we only support 2d texture maps right now
    // so one of the three sizes must be 1, but it
    // could be any of them, so lets find it
    if (size[0] == 1)
    {
      xsize = size[1]; ysize = size[2];
    }
    else
    {
      xsize = size[0];
      if (size[1] == 1)
      {
        ysize = size[2];
      }
      else
      {
        ysize = size[1];
        if (size[2] != 1)
        {
          vtkErrorMacro(<< "3D texture maps currently are not supported!");
          return;
        }
      }
    }


    // -- decide whether the texture needs to be resampled --

    GLint maxDimGL;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxDimGL);
    // if larger than permitted by the graphics library then must resample
    bool resampleNeeded=xsize > maxDimGL || ysize > maxDimGL;
    if(resampleNeeded)
    {
      vtkDebugMacro( "Texture too big for gl, maximum is " << maxDimGL);
    }

    if(resampleNeeded)
    {
      vtkDebugMacro(<< "Resampling texture to power of two for OpenGL");

      switch (channelByteSize)
      {
        case 1:
          resultData = this->ResampleToPowerOfTwo<unsigned char>(xsize, ysize, dataPtr, numChannels);
          break;

        case 2:
          resultData = reinterpret_cast<unsigned char *>(this->ResampleToPowerOfTwo<unsigned short>(xsize, ysize, reinterpret_cast<unsigned short *>(dataPtr), numChannels));
          break;

        default:
          vtkErrorMacro("resampling of component types with more than 2 bytes are not supported");
      }
    }

    if ( resultData == NULL )
    {
      resultData = dataPtr;
    }

    // free any old display lists (from the old context)
    this->RenderWindow = ren->GetRenderWindow();

    // make the new context current before we mess with opengl
    this->RenderWindow->MakeCurrent();

    // define a display list for this texture
    // get a unique display list id

    if(!this->Index)
    {
      glGenTextures(1, &tempIndex);
      this->Index = static_cast<long>(tempIndex);
    }
    glBindTexture(GL_TEXTURE_2D, this->Index);

    if (this->Interpolate)
    {
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                      GL_LINEAR );
    }
    else
    {
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }
    if (this->Repeat)
    {
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT );
    }
    else
    {
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        vtkgl::CLAMP_TO_EDGE );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        vtkgl::CLAMP_TO_EDGE );
    }
    int internalFormat = numChannels;
    switch (numChannels)
    {
      case 1: format = GL_LUMINANCE; break;
      case 2: format = GL_LUMINANCE_ALPHA; break;
      case 3: format = GL_RGB; break;
      case 4: format = GL_RGBA; break;
    }

    internalFormat = GL_LUMINANCE32F_ARB;
    format = GL_LUMINANCE;

    // blocking call
    if (channelByteSize == 2)
    {
      glTexImage2D( GL_TEXTURE_2D, 0 , internalFormat,
                   xsize, ysize, 0, format,
                   GL_FLOAT,
                   static_cast<const GLvoid *>(resultData) );

      this->Format16BIT = true;
    }
    else
    {
      glTexImage2D( GL_TEXTURE_2D, 0 , internalFormat,
                   xsize, ysize, 0, format,
                   GL_FLOAT,
                   static_cast<const GLvoid *>(floatData) );

      this->Format16BIT = false;
    }

    // modify the load time to the current time
    this->LoadTime.Modified();

    // free memory
    if (resultData != dataPtr)
    {
      delete [] resultData;
    }
  }

  // execute the display list that uses creates the texture
  glBindTexture(GL_TEXTURE_2D, this->Index);

  if (this->LevelWindowEnabled)
  {
#ifdef MES_MOBILE_BUILD
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LW_LOWER, this->LevelWindowLower);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LW_UPPER, this->LevelWindowUpper);
#endif
  }

  // don't accept fragments if they have zero opacity. this will stop the
  // zbuffer from be blocked by totally transparent texture fragments.
  glAlphaFunc (GL_GREATER, static_cast<GLclampf>(0));
  glEnable (GL_ALPHA_TEST);

  if (this->PremultipliedAlpha)
  {
    // save the blend function.
    glPushAttrib(GL_COLOR_BUFFER_BIT);

    // make the blend function correct for textures premultiplied by alpha.
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  }

  // now bind it
  glEnable(GL_TEXTURE_2D);

  // clear any texture transform
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();

  // build transformation
  if (this->Transform)
  {
    double *mat = this->Transform->GetMatrix()->Element[0];
    double mat2[16];
    mat2[0] = mat[0];
    mat2[1] = mat[4];
    mat2[2] = mat[8];
    mat2[3] = mat[12];
    mat2[4] = mat[1];
    mat2[5] = mat[5];
    mat2[6] = mat[9];
    mat2[7] = mat[13];
    mat2[8] = mat[2];
    mat2[9] = mat[6];
    mat2[10] = mat[10];
    mat2[11] = mat[14];
    mat2[12] = mat[3];
    mat2[13] = mat[7];
    mat2[14] = mat[11];
    mat2[15] = mat[15];

    // insert texture transformation
    glMultMatrixd(mat2);
  }
  glMatrixMode(GL_MODELVIEW);

}

// ----------------------------------------------------------------------------
void vtkMitkShaderTexture::PostRender(vtkRenderer *vtkNotUsed(ren))
{
  if (this->GetInput() && this->PremultipliedAlpha)
  {
    // restore the blend function
    glPopAttrib();
  }
}

// ----------------------------------------------------------------------------
static int FindPowerOfTwo(int i)
{
  int size;

  for ( i--, size=1; i > 0; size*=2 )
  {
    i /= 2;
  }

  // [these lines added by Tim Hutton (implementing Joris Vanden Wyngaerd's
  // suggestions)]
  // limit the size of the texture to the maximum allowed by OpenGL
  // (slightly more graceful than texture failing but not ideal)
  GLint maxDimGL;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxDimGL);
  if ( size > maxDimGL )
  {
    size = maxDimGL ;
  }
  // end of Tim's additions

  return size;
}

// ----------------------------------------------------------------------------
// Creates resampled unsigned char texture map that is a power of two in both
// x and y.
template <typename T>
T *vtkMitkShaderTexture::ResampleToPowerOfTwo(int &xs,
                                             int &ys,
                                             T *dptr,
                                             int numChannels)
{
  T *tptr, *p, *p1, *p2, *p3, *p4;
  int xsize, ysize, i, j, k, jOffset, iIdx, jIdx;
  double pcoords[3], hx, hy, rm, sm, w0, w1, w2, w3;

  xsize = FindPowerOfTwo(xs);
  ysize = FindPowerOfTwo(ys);
  if (this->RestrictPowerOf2ImageSmaller)
  {
    if (xsize > xs)
    {
      xsize /= 2;
    }
    if (ysize > ys)
    {
      ysize /= 2;
    }
  }
  hx = xsize > 1 ? (xs - 1.0) / (xsize - 1.0) : 0;
  hy = ysize > 1 ? (ys - 1.0) / (ysize - 1.0) : 0;

  tptr = p = new T[xsize*ysize*numChannels];

  // Resample from the previous image. Compute parametric coordinates and
  // interpolate
  for (j=0; j < ysize; j++)
  {
    pcoords[1] = j*hy;

    jIdx = static_cast<int>(pcoords[1]);
    if ( jIdx >= (ys-1) ) //make sure to interpolate correctly at edge
    {
      jIdx = ys - 2;
      pcoords[1] = 1.0;
    }
    else
    {
      pcoords[1] = pcoords[1] - jIdx;
    }
    jOffset = jIdx*xs;
    sm = 1.0 - pcoords[1];

    for (i=0; i < xsize; i++)
    {
      pcoords[0] = i*hx;
      iIdx = static_cast<int>(pcoords[0]);
      if ( iIdx >= (xs-1) )
      {
        iIdx = xs - 2;
        pcoords[0] = 1.0;
      }
      else
      {
        pcoords[0] = pcoords[0] - iIdx;
      }
      rm = 1.0 - pcoords[0];

      // Get pointers to 4 surrounding pixels
      p1 = dptr + numChannels*(iIdx + jOffset);
      p2 = p1 + numChannels;
      p3 = p1 + numChannels*xs;
      p4 = p3 + numChannels;

      // Compute interpolation weights interpolate components
      w0 = rm*sm;
      w1 = pcoords[0]*sm;
      w2 = rm*pcoords[1];
      w3 = pcoords[0]*pcoords[1];
      for (k=0; k < numChannels; k++)
      {
        *p++ = static_cast<T>(p1[k]*w0 + p2[k]*w1 + p3[k]*w2
                              + p4[k]*w3);
      }
    }
  }

  xs = xsize;
  ys = ysize;

  return tptr;
}

// ----------------------------------------------------------------------------
void vtkMitkShaderTexture::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Index: " << this->Index << endl;
}
