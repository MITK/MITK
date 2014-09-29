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


#include <itkShortestPathCostFunctionLiveWire.h>
#include <itkImageRegionIterator.h>
#include <itkShortestPathImageFilter.h>



mitk::ImageToLiveWireContourFilter::ImageToLiveWireContourFilter()
{
  OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfIndexedOutputs( 1 );
  this->SetNthOutput(0, output.GetPointer());
}



mitk::ImageToLiveWireContourFilter::~ImageToLiveWireContourFilter()
{

}



void mitk::ImageToLiveWireContourFilter::SetInput ( const mitk::ImageToLiveWireContourFilter::InputType* input )
{
  this->SetInput( 0, input );
}

void mitk::ImageToLiveWireContourFilter::SetInput ( unsigned int idx, const mitk::ImageToLiveWireContourFilter::InputType* input )
{
  if ( idx + 1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs(idx + 1);
  }
  if ( input != static_cast<InputType*> ( this->ProcessObject::GetInput ( idx ) ) )
  {
    this->ProcessObject::SetNthInput ( idx, const_cast<InputType*> ( input ) );
    this->Modified();
  }
}



const mitk::ImageToLiveWireContourFilter::InputType* mitk::ImageToLiveWireContourFilter::GetInput( void )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ImageToLiveWireContourFilter::InputType*>(this->ProcessObject::GetInput(0));
}


const mitk::ImageToLiveWireContourFilter::InputType* mitk::ImageToLiveWireContourFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ImageToLiveWireContourFilter::InputType*>(this->ProcessObject::GetInput(idx));
}



void mitk::ImageToLiveWireContourFilter::GenerateData()
{
  mitk::Image::ConstPointer input = dynamic_cast<const mitk::Image*>(this->GetInput());

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
void mitk::ImageToLiveWireContourFilter::ItkProcessImage (const itk::Image<TPixel, VImageDimension>* inputImage)
{
  //typedef itk::Image< TPixel, VImageDimension >                              InputImageType;
  //typedef itk::Image< float,  2 >                                          FloatImageType;

  //typedef typename itk::ShortestPathImageFilter< InputImageType, InputImageType >     ShortestPathImageFilterType;
  //typedef typename itk::ShortestPathCostFunctionLiveWire< InputImageType >   CostFunctionType;

  //typedef InputImageType::IndexType                                        IndexType;


  ///* compute the requested region for itk filters */

  //typename IndexType startPoint, endPoint;
  //
  //startPoint[0] = m_StartPointInIndex[0];
  //startPoint[1] = m_StartPointInIndex[1];

  //endPoint[0] = m_EndPointInIndex[0];
  //endPoint[1] = m_EndPointInIndex[1];

  ////minimum value in each direction for startRegion
  //typename IndexType startRegion;
  //startRegion[0] = startPoint[0] < endPoint[0] ? startPoint[0] : endPoint[0];
  //startRegion[1] = startPoint[1] < endPoint[1] ? startPoint[1] : endPoint[1];

  ////maximum value in each direction for size
  //typename InputImageType::SizeType size;
  //size[0] = startPoint[0] > endPoint[0] ? startPoint[0] : endPoint[0];
  //size[1] = startPoint[1] > endPoint[1] ? startPoint[1] : endPoint[1];


  //typename InputImageType::RegionType region;
  //region.SetSize( size );
  //region.SetIndex( startRegion );
  ///*---------------------------------------------*/


  ///* extracts features from image and calculates costs */
  //typename CostFunctionType::Pointer costFunction = CostFunctionType::New();
  //costFunction->SetImage(inputImage);
  //costFunction->SetStartIndex(startPoint);
  //costFunction->SetEndIndex(endPoint);
  //costFunction->SetRequestedRegion(region);
  ///*---------------------------------------------*/


  ///* calculate shortest path between start and end point */
  //ShortestPathImageFilterType::Pointer shortestPathFilter = ShortestPathImageFilterType::New();
  //shortestPathFilter->SetFullNeighborsMode(true);
  //shortestPathFilter->SetInput(inputImage);
  //shortestPathFilter->SetMakeOutputImage(true);
  //shortestPathFilter->SetStoreVectorOrder(false);
  ////shortestPathFilter->SetActivateTimeOut(true);
  //shortestPathFilter->SetStartIndex(startPoint);
  //shortestPathFilter->SetEndIndex(endPoint);

  //shortestPathFilter->Update();

  ///*---------------------------------------------*/


  ///* construct contour from path image */
  ////get the shortest path as vector
  //std::vector< itk::Index<3> > shortestPath = shortestPathFilter->GetVectorPath();

  ////fill the output contour with controll points from the path
  //OutputType::Pointer outputContour = this->GetOutput();
  //mitk::Image::ConstPointer input = dynamic_cast<const mitk::Image*>(this->GetInput());

  //std::vector< itk::Index<3> >::iterator pathIterator = shortestPath.begin();

  //while(pathIterator != shortestPath.end())
  //{
  //  mitk::Point3D currentPoint;
  //  currentPoint[0] = (*pathIterator)[0];
  //  currentPoint[1] = (*pathIterator)[1];

  //  input->GetGeometry(0)->IndexToWorld(currentPoint, currentPoint);
  //  outputContour->AddVertex(currentPoint);
  //
  //  pathIterator++;
  //}
  /*---------------------------------------------*/

}
