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

#include "vtkMitkLevelWindowFilter.h"
#include <vtkImageData.h>
#include <vtkImageIterator.h>
#include <vtkLookupTable.h>

//used for acos etc.
#include <cmath>

//used for PI
#include <itkMath.h>

#include <mitkLogMacros.h>

static const double PI = itk::Math::pi;

vtkMitkLevelWindowFilter::vtkMitkLevelWindowFilter():m_MinOqacity(0.0),m_MaxOpacity(255.0)
{
  //  SetNumberOfThreads(1);

  MITK_INFO << "mitk level/window filter uses " << GetNumberOfThreads() << " thread(s)";
}

vtkMitkLevelWindowFilter::~vtkMitkLevelWindowFilter()
{
}

unsigned long int vtkMitkLevelWindowFilter::GetMTime()
{
  unsigned long mTime=this->vtkObject::GetMTime();
  unsigned long time;

  if ( this->m_LookupTable != NULL )
  {
    time = this->m_LookupTable->GetMTime();
    mTime = ( time > mTime ? time : mTime );
  }

  return mTime;
}

void vtkMitkLevelWindowFilter::SetLookupTable(vtkScalarsToColors *lookupTable)
{
  if (m_LookupTable != lookupTable)
  {
    m_LookupTable = lookupTable;
    this->Modified();
  }
}

vtkScalarsToColors* vtkMitkLevelWindowFilter::GetLookupTable()
{
  return m_LookupTable;
}

//This code was copied from the iil. The template works only for float and double.
//Internal method which should never be used anywhere else and should not be in th header.
// Convert color pixels from (R,G,B) to (H,S,I).
// Reference: "Digital Image Processing, 2nd. edition", R. Gonzalez and R. Woods. Prentice Hall, 2002.
template<class T>
void RGBtoHSI(T* RGB, T* HSI)
{
  T R = RGB[0],
      G = RGB[1],
      B = RGB[2],
      nR = (R<0?0:(R>255?255:R))/255,
      nG = (G<0?0:(G>255?255:G))/255,
      nB = (B<0?0:(B>255?255:B))/255,
      m = nR<nG?(nR<nB?nR:nB):(nG<nB?nG:nB),
      theta = (T)(std::acos(0.5f*((nR-nG)+(nR-nB))/std::sqrt(std::pow(nR-nG,2)+(nR-nB)*(nG-nB)))*180/PI),
      sum = nR + nG + nB;
  T H = 0, S = 0, I = 0;
  if (theta>0) H = (nB<=nG)?theta:360-theta;
  if (sum>0) S = 1 - 3/sum*m;
  I = sum/3;
  HSI[0] = (T)H;
  HSI[1] = (T)S;
  HSI[2] = (T)I;
}

//This code was copied from the iil. The template works only for float and double.
//Internal method which should never be used anywhere else and should not be in th header.
// Convert color pixels from (H,S,I) to (R,G,B).
template<class T>
void HSItoRGB(T* HSI, T* RGB)
{
  T H = (T)HSI[0],
      S = (T)HSI[1],
      I = (T)HSI[2],
      a = I*(1-S),
      R = 0, G = 0, B = 0;
  if (H<120) {
    B = a;
    R = (T)(I*(1+S*std::cos(H*PI/180)/std::cos((60-H)*PI/180)));
    G = 3*I-(R+B);
  } else if (H<240) {
    H-=120;
    R = a;
    G = (T)(I*(1+S*std::cos(H*PI/180)/std::cos((60-H)*PI/180)));
    B = 3*I-(R+G);
  } else {
    H-=240;
    G = a;
    B = (T)(I*(1+S*std::cos(H*PI/180)/std::cos((60-H)*PI/180)));
    R = 3*I-(G+B);
  }
  R*=255; G*=255; B*=255;
  RGB[0] = (T)(R<0?0:(R>255?255:R));
  RGB[1] = (T)(G<0?0:(G>255?255:G));
  RGB[2] = (T)(B<0?0:(B>255?255:B));
}


//Internal method which should never be used anywhere else and should not be in th header.
//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
void vtkApplyLookupTableOnRGBA(vtkMitkLevelWindowFilter* self,
                               vtkImageData* inData,
                               vtkImageData* outData,
                               int outExt[6],
                               vtkFloatingPointType* clippingBounds,
                               T*)
{
  vtkImageIterator<T> inputIt(inData, outExt);
  vtkImageIterator<T> outputIt(outData, outExt);
  vtkLookupTable* lookupTable;
  const int maxC = inData->GetNumberOfScalarComponents();

  double tableRange[2];

  clippingBounds ++;


  lookupTable = dynamic_cast<vtkLookupTable*>(self->GetLookupTable());

  lookupTable->GetTableRange(tableRange);

  //parameters for RGB level window
  double scale = (tableRange[1] -tableRange[0] > 0 ? 255.0 / (tableRange[1] - tableRange[0]) : 0.0);
  double bias = tableRange[0] * scale;

  //parameters for opaque level window
  double scaleOpac = (self->GetMaxOpacity() -self->GetMinOpacity() > 0 ? 255.0 / (self->GetMaxOpacity() - self->GetMinOpacity()) : 0.0);
  double biasOpac = self->GetMinOpacity() * scaleOpac;

  int y = outExt[2];

  // Loop through ouput pixels
  while (!outputIt.IsAtEnd())
  {
    T* inputSI = inputIt.BeginSpan();
    T* outputSI = outputIt.BeginSpan();
    T* outputSIEnd = outputIt.EndSpan();

    if( y >= clippingBounds[2] && y < clippingBounds[3] )
    {
      int x = outExt[0];

      while (outputSI != outputSIEnd)
      {
        if ( x >= clippingBounds[0] && x < clippingBounds[1])
        {
          double rgb[3], alpha, hsi[3];

          // level/window mechanism for intensity in HSI space
          rgb[0] = static_cast<double>(*inputSI); inputSI++;
          rgb[1] = static_cast<double>(*inputSI); inputSI++;
          rgb[2] = static_cast<double>(*inputSI); inputSI++;

          RGBtoHSI<double>(rgb,hsi);
          hsi[2] = hsi[2] * 255.0 * scale - bias;
          hsi[2] = (hsi[2] > 255.0 ? 255 : (hsi[2] < 0.0 ? 0 : hsi[2]));
          hsi[2] /= 255.0;
          HSItoRGB<double>(hsi,rgb);

          *outputSI = static_cast<T>(rgb[0]); outputSI++;
          *outputSI = static_cast<T>(rgb[1]); outputSI++;
          *outputSI = static_cast<T>(rgb[2]); outputSI++;

          unsigned char finalAlpha = 255;

          //RGBA case
          if(maxC >= 4)
          {
            // level/window mechanism for opacity
            alpha = static_cast<double>(*inputSI); inputSI++;
            alpha = alpha * scaleOpac - biasOpac;
            if(alpha > 255.0)
            {
              alpha = 255.0;
            }
            else if(alpha < 0.0)
            {
              alpha = 0.0;
            }
            finalAlpha = static_cast<T>(alpha);

            for( int c = 4; c < maxC; c++ )
              inputSI++;

          }

          *outputSI = static_cast<T>(finalAlpha); outputSI++;
        }
        else
        {
          inputSI+=maxC;
          *outputSI = 0; outputSI++;
          *outputSI = 0; outputSI++;
          *outputSI = 0; outputSI++;
          *outputSI = 0; outputSI++;
        }

        x++;
      }
    }
    else
    {
      while (outputSI != outputSIEnd)
      {
        *outputSI = 0; outputSI++;
        *outputSI = 0; outputSI++;
        *outputSI = 0; outputSI++;
        *outputSI = 0; outputSI++;
      }
    }
    inputIt.NextSpan();
    outputIt.NextSpan();
    y++;
  }
}

//Internal method which should never be used anywhere else and should not be in th header.
//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
void vtkApplyLookupTableOnScalarsFast(vtkMitkLevelWindowFilter *self,
                                  vtkImageData *inData,
                                  vtkImageData *outData,
                                  int outExt[6],
                                  T *)
{
  vtkImageIterator<T> inputIt(inData, outExt);
  vtkImageIterator<unsigned char> outputIt(outData, outExt);
  vtkLookupTable* lookupTable;

  double tableRange[2];

  lookupTable = dynamic_cast<vtkLookupTable*>(self->GetLookupTable());
  lookupTable->GetTableRange(tableRange);

  float scale,bias;

  int * realLookupTable = reinterpret_cast<int*>(lookupTable->GetTable()->GetPointer(0));
  int maxIndex = lookupTable->GetNumberOfColors() - 1;

  scale = (tableRange[1] -tableRange[0] > 0 ? (maxIndex + 1) / (tableRange[1] - tableRange[0]) : 0.0);
  bias = - tableRange[0] * scale;
  bias += 0.5f;


  // Loop through ouput pixels
  while (!outputIt.IsAtEnd())
  {
    unsigned char* outputSI = outputIt.BeginSpan();
    unsigned char* outputSIEnd = outputIt.EndSpan();

    T* inputSI = inputIt.BeginSpan();

    while (outputSI != outputSIEnd)
    {
      // map to an index
      int idx = static_cast<int>( *inputSI * scale + bias );

      if (idx < 0)
        idx = 0;
      else if (idx > maxIndex)
        idx = maxIndex;

      * reinterpret_cast<int*>(outputSI) = realLookupTable[idx];

      inputSI++;
      outputSI+=4;
    }

    inputIt.NextSpan();
    outputIt.NextSpan();
  }
}



//Internal method which should never be used anywhere else and should not be in th header.
//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
void vtkApplyLookupTableOnScalars(vtkMitkLevelWindowFilter *self,
                                  vtkImageData *inData,
                                  vtkImageData *outData,
                                  int outExt[6],
                                  vtkFloatingPointType* clippingBounds,
                                  T *)
{
  vtkImageIterator<T> inputIt(inData, outExt);
  vtkImageIterator<unsigned char> outputIt(outData, outExt);
  vtkLookupTable* lookupTable;

  lookupTable = dynamic_cast<vtkLookupTable*>(self->GetLookupTable());

  int y = outExt[2];

  // Loop through ouput pixels
  while (!outputIt.IsAtEnd())
  {
    unsigned char* outputSI = outputIt.BeginSpan();
    unsigned char* outputSIEnd = outputIt.EndSpan();

    if( y >= clippingBounds[2] && y < clippingBounds[3] )
    {
      T* inputSI = inputIt.BeginSpan();

      int x= outExt[0];

      while (outputSI != outputSIEnd)
      {
        if ( x >= clippingBounds[0] && x < clippingBounds[1])
        {

          // fetching original value
          double grayValue = static_cast<double>(*inputSI);

          // applying lookuptable
          int *RGBA = reinterpret_cast<int *>(lookupTable->MapValue( grayValue ));

          // storing
          * reinterpret_cast<int*>(outputSI) = *RGBA;

          outputSI+=4;
        }
        else
        {
          *outputSI = 0; outputSI++;
          *outputSI = 0; outputSI++;
          *outputSI = 0; outputSI++;
          *outputSI = 0; outputSI++;
        }

        inputSI++;
        x++;
      }

    }
    else
    {
      while (outputSI != outputSIEnd)
      {
        *outputSI = 0; outputSI++;
        *outputSI = 0; outputSI++;
        *outputSI = 0; outputSI++;
        *outputSI = 0; outputSI++;
      }
    }

    inputIt.NextSpan();
    outputIt.NextSpan();
    y++;
  }
}


void vtkMitkLevelWindowFilter::ExecuteInformation()
{
  vtkImageData *input = this->GetInput();
  vtkImageData *output = this->GetOutput();

  if (!input)
  {
    vtkErrorMacro(<< "Input not set.");
    return;
  }
  output->CopyTypeSpecificInformation( input );

  // TODO make output RGBA
  output->SetScalarTypeToUnsignedChar();
  output->SetNumberOfScalarComponents(4);

  int extent[6];
  input->GetWholeExtent(extent);
  output->SetExtent(extent);
  output->SetWholeExtent(extent);
  output->SetUpdateExtent(extent);
  output->AllocateScalars();
}

//Method to run the filter in different threads.
void vtkMitkLevelWindowFilter::ThreadedExecute(vtkImageData *inData,
                                               vtkImageData *outData,
                                               int extent[6], int /*id*/)
{
  if(inData->GetNumberOfScalarComponents() > 2)
  {
    switch (inData->GetScalarType())
    {
      vtkTemplateMacro(
            vtkApplyLookupTableOnRGBA( this,
                                       inData,
                                       outData,
                                       extent,
                                       m_ClippingBounds,
                                       static_cast<VTK_TT *>(0)));
      default:
        vtkErrorMacro(<< "Execute: Unknown ScalarType");
        return;
    }
  }
  else
  {
    bool dontClip =
           extent[2] >= m_ClippingBounds[2]
        && extent[3] <= m_ClippingBounds[3]
        && extent[0] >= m_ClippingBounds[0]
        && extent[1] <= m_ClippingBounds[1];

    bool linearLookupTable = dynamic_cast<vtkLookupTable*>(this->GetLookupTable())->GetScale() == VTK_SCALE_LINEAR;

    bool useFast = dontClip && linearLookupTable;

    if(useFast)
      switch (inData->GetScalarType())
      {
        vtkTemplateMacro(
              vtkApplyLookupTableOnScalarsFast( this,
                                                inData,
                                                outData,
                                                extent,
                                                static_cast<VTK_TT *>(0)));
        default:
          vtkErrorMacro(<< "Execute: Unknown ScalarType");
          return;
      }
    else
      switch (inData->GetScalarType())
      {
        vtkTemplateMacro(
              vtkApplyLookupTableOnScalars( this,
                                            inData,
                                            outData,
                                            extent,
                                            m_ClippingBounds,
                                            static_cast<VTK_TT *>(0)));
        default:
          vtkErrorMacro(<< "Execute: Unknown ScalarType");
          return;
      }
  }
}

void vtkMitkLevelWindowFilter::ExecuteInformation(
    vtkImageData *vtkNotUsed(inData), vtkImageData *vtkNotUsed(outData))
{
}

void vtkMitkLevelWindowFilter::SetMinOpacity(double minOpacity)
{
  m_MinOqacity = minOpacity;
}

inline double vtkMitkLevelWindowFilter::GetMinOpacity() const
{
  return m_MinOqacity;
}

void vtkMitkLevelWindowFilter::SetMaxOpacity(double maxOpacity)
{
  m_MaxOpacity = maxOpacity;
}

inline double vtkMitkLevelWindowFilter::GetMaxOpacity() const
{
  return m_MaxOpacity;
}

void vtkMitkLevelWindowFilter::SetClippingBounds(vtkFloatingPointType* bounds) // TODO does vtkFloatingPointType[4] work??
{
  for (unsigned int i = 0 ; i < 4; ++i)
    m_ClippingBounds[i] = bounds[i];
}
