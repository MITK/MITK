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

#ifndef MITKCOMPAREIMAGEFILTER_H
#define MITKCOMPAREIMAGEFILTER_H

#include "itkAbsoluteValueDifferenceImageFilter.h"
#include "mitkImageToImageFilter.h"
#include "mitkImage.h"
#include "itkImage.h"

namespace mitk{

class MITK_CORE_EXPORT CompareImageFilter : public ImageToImageFilter
{
public:

  CompareImageFilter();

  void SetInputImage1( mitk::Image::Pointer image);
  void SetInputImage2( mitk::Image::Pointer image);

  float GetMaxValue() const;

  float GetMinValue() const;

private:

  virtual void GenerateData();

  template< typename TPixel1, unsigned int VImageDimension1,
            typename TPixel2, unsigned int VImageDimension2>
  void EstimateValueDifference( itk::Image< TPixel1, VImageDimension1>* itkImage1,
                                itk::Image< TPixel2, VImageDimension2>* itkImage2);


  mitk::Image::Pointer m_InputImage1;
  mitk::Image::Pointer m_InputImage2;

  float m_MaxValue;
  float m_MinValue;
  float m_ValueSum;
  float m_ValueVariance;

};

} // end namespace mitk

#endif // ITKCOMPAREIMAGEFILTER_H
