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

#include <itkShortestPathCostFunctionLiveWire.h>
#include <itkImageRegionIterator.h>


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

  typedef itk::ShortestPathImageFilter<InputImageType, InputImageType>      ShortestPathImageFilterType;
  typedef itk::ShortestPathCostFunctionLiveWire<typename InputImageType> CostFunctionType;

  typedef InputImageType::IndexType IndexType;


  /* compute the requested region for itk filters */

  typename IndexType startPoint, endPoint;
  
  startPoint[0] = m_StartPointInIndex[0];
  startPoint[1] = m_StartPointInIndex[1];

  endPoint[0] = m_EndPointInIndex[0];
  endPoint[1] = m_EndPointInIndex[1];

  //minimum value in each direction for startRegion
  typename IndexType startRegion;
  startRegion[0] = startPoint[0] < endPoint[0] ? startPoint[0] : endPoint[0];
  startRegion[1] = startPoint[1] < endPoint[1] ? startPoint[1] : endPoint[1];

  //maximum value in each direction for size
  typename InputImageType::SizeType size;
  size[0] = startPoint[0] > endPoint[0] ? startPoint[0] : endPoint[0];
  size[1] = startPoint[1] > endPoint[1] ? startPoint[1] : endPoint[1];


  typename InputImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( startRegion );
  /*---------------------------------------------*/


  /* extracts features from image and calculates costs */
  typename CostFunctionType::Pointer costFunction = CostFunctionType::New();
  costFunction->SetImage(inputImage);
  costFunction->SetStartIndex(startPoint);
  costFunction->SetEndIndex(endPoint);
  /*---------------------------------------------*/


  /* calculate shortest path between start and end point */
  ShortestPathImageFilterType::Pointer shortestPathFilter = ShortestPathImageFilterType::New();
  shortestPathFilter->SetFullNeighborsMode(true);
  shortestPathFilter->SetInput(sourceImageItk);
  shortestPathFilter->SetMakeOutputImage(true);  
  shortestPathFilter->SetStoreVectorOrder(true);  
  //shortestPathFilter->SetActivateTimeOut(true); 
  shortestPathFilter->SetStartIndex(startPoint);
  shortestPathFilter->SetEndIndex(endPoint);

  shortestPathFilter->Update();

  mitk::Image::Pointer resultImageOrder;
  CastToMitkImage(shortestPathFilter->GetVectorOrderImage(), resultImageOrder);
  /*---------------------------------------------*/

}