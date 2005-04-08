/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#if(_MSC_VER==1200)
#include <itkFixedCenterOfRotationAffineTransform.h>
#endif

#include "mitkImageAccessByItk.h"

#include "itkScalarImageToHistogramGenerator.h"

#include "mitkHistogramGenerator.h"

mitk::HistogramGenerator::HistogramGenerator() : m_Image(NULL), m_Size(256), m_Histogram(NULL)
{
}

mitk::HistogramGenerator::~HistogramGenerator()
{
}

template < typename TPixel, unsigned int VImageDimension > 
void InternalCompute(itk::Image< TPixel, VImageDimension >* itkImage, const mitk::HistogramGenerator* mitkHistoGenerator, mitk::HistogramGenerator::HistogramType::ConstPointer& histogram)
{
  
  typedef itk::Statistics::ScalarImageToHistogramGenerator< 
                                              itk::Image< TPixel, VImageDimension >
                                                          >   HistogramGeneratorType;

  typename HistogramGeneratorType::Pointer histogramGenerator = HistogramGeneratorType::New();

  histogramGenerator->SetInput( itkImage );

  histogramGenerator->SetNumberOfBins( mitkHistoGenerator->GetSize() );
  histogramGenerator->SetMarginalScale( 10.0 );
  histogramGenerator->Compute();
  
  histogram = (mitk::HistogramGenerator::HistogramType*)(histogramGenerator->GetOutput());
}

void mitk::HistogramGenerator::ComputeHistogram()
{
  AccessByItk_2(m_Image, InternalCompute, this, m_Histogram);

// debug code
  /*
  const unsigned int histogramSize = m_Histogram->Size();

  std::cout << "Histogram size " << histogramSize << std::endl;
  
  HistogramType::ConstIterator itr = GetHistogram()->Begin();
  HistogramType::ConstIterator end = GetHistogram()->End();
  
  int bin = 0;
  while( itr != end )
    {
    std::cout << "bin = " << GetHistogram()->GetBinMin(0,bin) << "--" << GetHistogram()->GetBinMax(0,bin) << " frequency = ";
    std::cout << itr.GetFrequency() << std::endl;
    ++itr;
    ++bin;
    }
    */ 
}

float mitk::HistogramGenerator::GetMaximumFrequency() const {
  return CalculateMaximumFrequency(this->m_Histogram);
};

float mitk::HistogramGenerator::CalculateMaximumFrequency(HistogramType::ConstPointer histogram) {
  
  HistogramType::ConstIterator itr = histogram->Begin();
  HistogramType::ConstIterator end = histogram->End();
  
  float maxFreq = 0;
  while( itr != end )
    {
    maxFreq = vnl_math_max(maxFreq,itr.GetFrequency());
    ++itr;
    }
  return maxFreq;
};
