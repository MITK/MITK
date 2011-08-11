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

#include "vtkMitkApplyLevelWindowToRGBFilter.h"
#include <vtkImageRGBToHSI.h>
#include <vtkImageData.h>
#include <vtkImageRGBToHSI.h>
#include <vtkImageIterator.h>
#include <vtkImageProgressIterator.h>
#include <vtkLookupTable.h>

vtkMitkApplyLevelWindowToRGBFilter::vtkMitkApplyLevelWindowToRGBFilter()
{

}

vtkMitkApplyLevelWindowToRGBFilter::~vtkMitkApplyLevelWindowToRGBFilter()
{

}

void vtkMitkApplyLevelWindowToRGBFilter::SetLookupTable(vtkScalarsToColors *lookupTable)
{
  m_LookupTable = lookupTable;
}

vtkScalarsToColors* vtkMitkApplyLevelWindowToRGBFilter::GetLookupTable()
{
  return m_LookupTable;
}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
    void vtkMitkApplyLevelWindowToRGBFilter::vtkCalculateIntensityFromLookupTable(
                             vtkImageData *inData,
                             vtkImageData *outData,
                             int outExt[6], T *)
{
  vtkImageIterator<T> inIt(inData, outExt);
  vtkImageIterator<T> outIt(outData, outExt);
  vtkLookupTable* lookupTable;
  int idxC, maxC;
  double H, S, I, Imin, Imax;
  double imgRange[2];
  double tableRange[2];

  // find the region to loop over
  maxC = inData->GetNumberOfScalarComponents()-1;
  Imin = 255;
  Imax = 0;
  // Loop through ouput pixels
  while (!outIt.IsAtEnd())
    {
    T* inSI = inIt.BeginSpan();
    T* outSI = outIt.BeginSpan();
    T* outSIEnd = outIt.EndSpan();
    while (outSI != outSIEnd)
      {
      // Pixel operation
      H = static_cast<double>(*inSI); inSI++;
      S = static_cast<double>(*inSI); inSI++;
      I = static_cast<double>(*inSI); inSI++;

      lookupTable = dynamic_cast<vtkLookupTable*>(this->GetLookupTable());

      lookupTable->GetTableRange(tableRange);
      inData->GetScalarRange(imgRange);

      I = ((I - tableRange[0]) / ( tableRange[1] - tableRange[0] )) * imgRange[1];
      I = (I < imgRange[0] ? imgRange[0] : I);
      I = (I > imgRange[1] ? imgRange[1] : I);

      // assign output.
      *outSI = static_cast<T>(H); outSI++;
      *outSI = static_cast<T>(S); outSI++;
      *outSI = static_cast<T>(I); outSI++;

      for (idxC = 3; idxC <= maxC; idxC++)
        {
        *outSI++ = *inSI++;
        }
      }
    inIt.NextSpan();
    outIt.NextSpan();
    }
}

void vtkMitkApplyLevelWindowToRGBFilter::ExecuteInformation()
{
  vtkImageData *input = this->GetInput();
  vtkImageData *output = this->GetOutput();

  if (!input)
  {
    vtkErrorMacro(<< "Input not set.");
    return;
  }
  output->CopyTypeSpecificInformation( input );

  int extent[6];
  input->GetWholeExtent(extent);
  output->SetExtent(extent);
  output->SetWholeExtent(extent);
  output->SetUpdateExtent(extent);
  output->AllocateScalars();

  switch (input->GetScalarType())
    {
    vtkTemplateMacro(
      vtkCalculateIntensityFromLookupTable( input,
                               output, extent,
                               static_cast<VTK_TT *>(0)));
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}

void vtkMitkApplyLevelWindowToRGBFilter::ExecuteInformation(
           vtkImageData *vtkNotUsed(inData), vtkImageData *vtkNotUsed(outData))
{
}
