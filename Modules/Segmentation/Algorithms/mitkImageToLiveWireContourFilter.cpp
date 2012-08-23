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

#include "mitkImageToLiveWireContourFilter.h"
#include <mitkInteractionConst.h>

#include <itkGradientMagnitudeImageFilter.h>
#include <itkImageRegionIterator.h>
#include "itkShortestPathImageFilter.h"

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>


mitk::ImageToLiveWireContourFilter::ImageToLiveWireContourFilter()
{

}



mitk::ImageToLiveWireContourFilter::~ImageToLiveWireContourFilter()
{

}



void mitk::ImageToLiveWireContourFilter::GenerateData()
{
  mitk::Image::ConstPointer input = dynamic_cast<const mitk::Image*>(this->GetInput());

  ImageToContourModelFilter::OutputType::Pointer output = this->GetOutput();

  if(!input)
  {
    MITK_ERROR << "No input available.";
    itkExceptionMacro("mitk::ImageToLiveWireContourFilter: No input available. Please set the input!");
    return;
  }

  if( input->GetDimension() != 2 )
  {
    MITK_ERROR << "Filter is only working on 2D images.";
    itkExceptionMacro("mitk::ImageToLiveWireContourFilter: Filter is only working on 2D images.. Please make sure that the input is 2D!");
    return;
  }


  input->GetGeometry()->WorldToIndex(m_StartPoint, m_StartPointInIndex);
  input->GetGeometry()->WorldToIndex(m_EndPoint, m_EndPointInIndex);


  AccessFixedDimensionByItk(input, ItkProcessImage, 2);
}



template<typename TPixel, unsigned int VImageDimension>
void mitk::ImageToLiveWireContourFilter::ItkProcessImage (itk::Image<TPixel, VImageDimension>* inputImage)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef itk::Image< float,  2 >   FloatImageType;


  /* compute the requested region for itk filters */
  //minimum value in each direction for start
  typename InputImageType::IndexType start;
  start[0] = m_StartPointInIndex[0] < m_EndPointInIndex[0] ? m_StartPointInIndex[0] : m_EndPointInIndex[0];
  start[1] = m_StartPointInIndex[1] < m_EndPointInIndex[1] ? m_StartPointInIndex[1] : m_EndPointInIndex[1];


  typename InputImageType::SizeType size;
  //maximum value in each direction for size
  size[0] = m_StartPointInIndex[0] > m_EndPointInIndex[0] ? m_StartPointInIndex[0] : m_EndPointInIndex[0];
  size[1] = m_StartPointInIndex[1] > m_EndPointInIndex[1] ? m_StartPointInIndex[1] : m_EndPointInIndex[1];


  typename InputImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );
  /*---------------------------------------------*/


  /* extract features from image */

  //gradient magnitude
  itk::GradientMagnitudeImageFilter<typename InputImageType, FloatImageType>::Pointer gradientFilter = 
    itk::GradientMagnitudeImageFilter<typename InputImageType, FloatImageType>::New();

  gradientFilter->SetInput(inputImage);

  //apply filter only for specific region
  gradientFilter->GetOutput()->SetRequestedRegion(region);


  gradientFilter->Update();

  FloatImageType::Pointer gradientMagnImage = gradientFilter->GetOutput();
  /*---------------------------------------------*/


  /* transfer to costs */
  
  /*---------------------------------------------*/


  /* calculate shortest path between start and end point */

  /*---------------------------------------------*/

}