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
#ifndef __itkSplitDWImageFilter_txx
#define __itkSplitDWImageFilter_txx

#include "itkSplitDWImageFilter.h"

template< class TInputImagePixelType,
          class TOutputImagePixelType>
itk::SplitDWImageFilter< TInputImagePixelType, TOutputImagePixelType >
::SplitDWImageFilter()
{
  this->SetNumberOfRequiredInputs(1);
}

template< class TInputImagePixelType,
          class TOutputImagePixelType>
void itk::SplitDWImageFilter< TInputImagePixelType, TOutputImagePixelType >
::CopyInformation( const DataObject* data)
{

}

template< class TInputImagePixelType,
          class TOutputImagePixelType>
void itk::SplitDWImageFilter<
TInputImagePixelType, TOutputImagePixelType >::GenerateOutputInformation()
{

}

template< class TInputImagePixelType,
          class TOutputImagePixelType>
void itk::SplitDWImageFilter<
TInputImagePixelType, TOutputImagePixelType >::GenerateData()
{

}


#endif // __itkSplitDWImageFilter_txx
