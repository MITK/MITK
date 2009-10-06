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

#if(_MSC_VER==1200)
#include <itkFixedCenterOfRotationAffineTransform.h>
#endif

#include "mitkHistogramGenerator.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageAccessByItk.h"


//
// The new ITK Statistics framework has
// a class with the same functionality as 
// MITKScalarImageToHistogramGenerator.h, but
// no longer has the classis the MITK class depends on.
#if !defined(ITK_USE_REVIEW_STATISTICS)
#include "itkMITKScalarImageToHistogramGenerator.h"
#else
#include "itkScalarImageToHistogramGenerator.h"
#endif

mitk::HistogramGenerator::HistogramGenerator() : m_Image(NULL), m_Size(256), m_Histogram(NULL)
{

}

mitk::HistogramGenerator::~HistogramGenerator()
{
}

template < typename TPixel, unsigned int VImageDimension > 
void InternalCompute(itk::Image< TPixel, VImageDimension >* itkImage, const mitk::HistogramGenerator* mitkHistoGenerator, mitk::HistogramGenerator::HistogramType::ConstPointer& histogram)
{
  
#if !defined(ITK_USE_REVIEW_STATISTICS)
  typedef itk::Statistics::MITKScalarImageToHistogramGenerator< 
                                              itk::Image< TPixel, VImageDimension >,
                                              double            >   HistogramGeneratorType;
#else
  typedef itk::Statistics::ScalarImageToHistogramGenerator< itk::Image< TPixel, VImageDimension > >
  HistogramGeneratorType;
                                               
#endif
  typename HistogramGeneratorType::Pointer histogramGenerator = HistogramGeneratorType::New();

  histogramGenerator->SetInput( itkImage );

  histogramGenerator->SetNumberOfBins( mitkHistoGenerator->GetSize() );
//  histogramGenerator->SetMarginalScale( 10.0 );
  histogramGenerator->Compute();
  
  histogram = histogramGenerator->GetOutput();
}

void mitk::HistogramGenerator::ComputeHistogram()
{
  if((m_Histogram.IsNull()) || (m_Histogram->GetMTime() < m_Image->GetMTime()))
  {
    const_cast<mitk::Image*>(m_Image.GetPointer())->SetRequestedRegionToLargestPossibleRegion(); //@todo without this, Image::GetScalarMin does not work for dim==3 (including sliceselector!)
    const_cast<mitk::Image*>(m_Image.GetPointer())->Update();
    mitk::ImageTimeSelector::Pointer timeSelector=mitk::ImageTimeSelector::New();
    timeSelector->SetInput(m_Image);
    timeSelector->SetTimeNr( 0 );
    timeSelector->UpdateLargestPossibleRegion();
    AccessByItk_2( timeSelector->GetOutput() , InternalCompute, this, m_Histogram);
  }

// debug code
  /*
    LOG_INFO << "Histogram modfied 1" << m_Histogram->GetMTime() << std::endl;
    m_Histogram->Modified();
    LOG_INFO << "Histogram modfied 2" << m_Histogram->GetMTime() << std::endl;
    LOG_INFO << "Image modfied" << m_Image->GetMTime() << std::endl;
  const unsigned int histogramSize = m_Histogram->Size();

  LOG_INFO << "Histogram size " << histogramSize << std::endl;
  
  HistogramType::ConstIterator itr = GetHistogram()->Begin();
  HistogramType::ConstIterator end = GetHistogram()->End();
  
  int bin = 0;
  while( itr != end )
    {
    LOG_INFO << "bin = " << GetHistogram()->GetBinMin(0,bin) << "--" << GetHistogram()->GetBinMax(0,bin) << " frequency = ";
    LOG_INFO << itr.GetFrequency() << std::endl;
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
    maxFreq = vnl_math_max(maxFreq,
                           // get rid of ambiguity with type signature
                           // for vnl_math_max
                           static_cast<float>(itr.GetFrequency()));
    ++itr;
    }
  return maxFreq;
};
