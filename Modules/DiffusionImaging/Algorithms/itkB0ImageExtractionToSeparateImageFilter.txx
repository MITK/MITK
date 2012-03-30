/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkB0ImageExtractionToSeparateImageFilter_txx
#define __itkB0ImageExtractionToSeparateImageFilter_txx

#include "itkB0ImageExtractionToSeparateImageFilter.h"

template< class TInputImagePixelType,
          class TOutputImagePixelType>
  itk::B0ImageExtractionToSeparateImageFilter< TInputImagePixelType, TOutputImagePixelType >
  ::B0ImageExtractionToSeparateImageFilter()
    : B0ImageExtractionImageFilter< TInputImagePixelType, TOutputImagePixelType >()
  {

  }

template< class TInputImagePixelType,
          class TOutputImagePixelType>
  void itk::B0ImageExtractionToSeparateImageFilter<
  TInputImagePixelType, TOutputImagePixelType >::GenerateData()
  {

  }

#endif // ifndef __itkB0ImageExtractionToSeparateImageFilter_txx
