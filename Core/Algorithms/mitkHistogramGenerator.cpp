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

#include "mitkHistogramGenerator.h"

#include "mitkImageAccessByItk.h"

#include "itkScalarImageToHistogramGenerator.h"


mitk::HistogramGenerator::HistogramGenerator() : m_Image(NULL), m_Size(256), m_Histogram(NULL)
{

}

mitk::HistogramGenerator::~HistogramGenerator()
{
}

template < typename TPixel, unsigned int VImageDimension > 
void InternalCompute(itk::Image< TPixel, VImageDimension >* itkImage, const mitk::HistogramGenerator* mitkHistoGenerator, mitk::HistogramGenerator::HistogramType& histogram)
{
  
  typedef itk::Statistics::ScalarImageToHistogramGenerator< 
                                              itk::Image< TPixel, VImageDimension >
                                                          >   HistogramGeneratorType;

  typename HistogramGeneratorType::Pointer histogramGenerator = HistogramGeneratorType::New();

  histogramGenerator->SetInput( itkImage );

  histogramGenerator->SetNumberOfBins( mitkHistoGenerator->GetSize() );
  histogramGenerator->SetMarginalScale( 10.0 );
  histogramGenerator->Compute();
  
  const typename HistogramGeneratorType::HistogramType* generatedHistogram = histogramGenerator->GetOutput();
  typename HistogramGeneratorType::HistogramType::SizeType size = generatedHistogram->GetSize();
  histogram.Initialize(size);
  unsigned int i, j;
  for ( i = 0 ; i < HistogramGeneratorType::HistogramType::MeasurementVectorSize ; i++)
  {
    for (j = 0; j < (size[i] - 1) ; ++j)
    {
      histogram.SetBinMin(0, j, generatedHistogram->GetBinMin(0, j));
      histogram.SetBinMax(0, j, generatedHistogram->GetBinMax(0, j));
    }
    for (j = 0; j < (size[i] - 1) ; ++j)
    {
      histogram.SetFrequency(j, generatedHistogram->GetFrequency(j));
    }
  }
}

void mitk::HistogramGenerator::ComputeHistogram()
{
  if((m_Histogram.IsNull()) || (m_Histogram->GetMTime() < m_Image->GetMTime()))
  {
    if(m_Histogram.IsNull())
      m_Histogram = HistogramType::New();
    const_cast<mitk::Image*>(m_Image.GetPointer())->SetRequestedRegionToLargestPossibleRegion(); //@todo without this, Image::GetScalarMin does not work for dim==3 (including sliceselector!)
    const_cast<mitk::Image*>(m_Image.GetPointer())->Update();
    AccessByItk_2(m_Image, InternalCompute, this, *m_Histogram);
    m_Histogram->Modified();
  }

// debug code
  /*
    std::cout << "Histogram modfied 1" << m_Histogram->GetMTime() << std::endl;
    m_Histogram->Modified();
    std::cout << "Histogram modfied 2" << m_Histogram->GetMTime() << std::endl;
    std::cout << "Image modfied" << m_Image->GetMTime() << std::endl;
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

float mitk::HistogramGenerator::CalculateMaximumFrequency(const HistogramType* histogram)
{  
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
