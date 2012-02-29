/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkMitkLevelWindowFilter.h"
#include <vtkImageData.h>
#include <vtkImageIterator.h>
#include <vtkLookupTable.h>

//used for acos etc.
#include <cmath>

//used for PI
#include <itkMath.h>

static const double PI = itk::Math::pi;

vtkMitkLevelWindowFilter::vtkMitkLevelWindowFilter():m_MinOqacity(0.0),m_MaxOpacity(255.0)
{
}

vtkMitkLevelWindowFilter::~vtkMitkLevelWindowFilter()
{
}

void vtkMitkLevelWindowFilter::SetLookupTable(vtkScalarsToColors *lookupTable)
{
  m_LookupTable = lookupTable;
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
    void vtkCalculateIntensityFromLookupTable2(vtkMitkLevelWindowFilter *self,
                                              vtkImageData *inData,
                                              vtkImageData *outData,
                                              int outExt[6], T *)
{
  vtkImageIterator<T> inputIt(inData, outExt);
  vtkImageIterator<unsigned char> outputIt(outData, outExt);
  vtkLookupTable* lookupTable;
  int maxC;
  int indexComponents = 3; //RGB case
  double imgRange[2];
  double tableRange[2];

  lookupTable = dynamic_cast<vtkLookupTable*>(self->GetLookupTable());

  double scale = (tableRange[1] -tableRange[0] > 0 ? 255.0 / (tableRange[1] - tableRange[0]) : 0.0);
  double bias = - tableRange[0] * scale;

  /*
  lookupTable->GetTableRange(tableRange);
  inData->GetScalarRange(imgRange);
  */


  // Loop through ouput pixels
  while (!outputIt.IsAtEnd())
  {
    T* inputSI = inputIt.BeginSpan();
    unsigned char* outputSI = outputIt.BeginSpan();
    unsigned char* outputSIEnd = outputIt.EndSpan();
    while (outputSI != outputSIEnd)
    {
      /*
         TODO
         - input: grauwert-bild, bounds
         - output rgba

         r = g = b = lookuptable(grauwert * l/w)
         a = outOfImage ? 0 : 1
      */
      double grayValue, alpha;

      // level/window mechanism for intensity in HSI space
      grayValue = static_cast<double>(*inputSI); inputSI++;

      grayValue *= scale;
      grayValue += bias;

      // double levelWindowedGrayValue = ...
      // double lookupTableTransformedGrayValue = ..

      *outputSI = static_cast<unsigned char>(grayValue); outputSI++;
      *outputSI = static_cast<unsigned char>(grayValue); outputSI++;
      *outputSI = static_cast<unsigned char>(grayValue); outputSI++;
      *outputSI = static_cast<unsigned char>(255); outputSI++;

    }
    inputIt.NextSpan();
    outputIt.NextSpan();
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
  switch (inData->GetScalarType())
  {
    vtkTemplateMacro(
        vtkCalculateIntensityFromLookupTable2( this,
                                              inData,
                                              outData,
                                              extent,
                                              static_cast<VTK_TT *>(0)));
  default:
    vtkErrorMacro(<< "Execute: Unknown ScalarType");
    return;
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
