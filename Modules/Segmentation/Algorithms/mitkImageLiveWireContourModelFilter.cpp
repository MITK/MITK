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

#include "mitkImageLiveWireContourModelFilter.h"

#include <itkShortestPathCostFunctionLiveWire.h>
#include <itkImageRegionIterator.h>
#include <itkShortestPathImageFilter.h>


mitk::ImageLiveWireContourModelFilter::ImageLiveWireContourModelFilter()
{
  OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfOutputs( 1 );
  this->SetNthOutput(0, output.GetPointer());

}

mitk::ImageLiveWireContourModelFilter::~ImageLiveWireContourModelFilter()
{

}


mitk::ImageLiveWireContourModelFilter::OutputType* mitk::ImageLiveWireContourModelFilter::GetOutput()
{
  return Superclass::GetOutput();
}

void mitk::ImageLiveWireContourModelFilter::SetInput ( const mitk::ImageLiveWireContourModelFilter::InputType* input )
{
  this->SetInput( 0, input );
}

void mitk::ImageLiveWireContourModelFilter::SetInput ( unsigned int idx, const mitk::ImageLiveWireContourModelFilter::InputType* input )
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



const mitk::ImageLiveWireContourModelFilter::InputType* mitk::ImageLiveWireContourModelFilter::GetInput( void )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ImageLiveWireContourModelFilter::InputType*>(this->ProcessObject::GetInput(0));
}


const mitk::ImageLiveWireContourModelFilter::InputType* mitk::ImageLiveWireContourModelFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ImageLiveWireContourModelFilter::InputType*>(this->ProcessObject::GetInput(idx));
}


void mitk::ImageLiveWireContourModelFilter::GenerateData()
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

  //only start calculating if both indices are inside image geometry
  if( input->GetGeometry()->IsIndexInside(this->m_StartPointInIndex) && input->GetGeometry()->IsIndexInside(this->m_EndPointInIndex) )
  {
    AccessFixedDimensionByItk(input, ItkProcessImage, 2);
  }
}



template<typename TPixel, unsigned int VImageDimension>
void mitk::ImageLiveWireContourModelFilter::ItkProcessImage (itk::Image<TPixel, VImageDimension>* inputImage)
{
  typedef itk::Image< TPixel, VImageDimension >                              InputImageType;
  typedef itk::Image< float,  2 >                                          FloatImageType;

  typedef  itk::ShortestPathImageFilter< typename InputImageType, typename InputImageType >     ShortestPathImageFilterType;
  typedef  itk::ShortestPathCostFunctionLiveWire< itk::Image< TPixel, VImageDimension > >   CostFunctionType;

  typedef InputImageType::IndexType                                        IndexType;


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
  size[0] = abs( startPoint[0] - endPoint[0] );
  size[1] = abs( startPoint[1] - endPoint[1] );


  typename CostFunctionType::RegionType region;
  region.SetSize( size );
  region.SetIndex( startRegion );
  /*---------------------------------------------*/

  //inputImage->SetRequestedRegion(region);

  /* extracts features from image and calculates costs */
  typename CostFunctionType::Pointer costFunction = CostFunctionType::New();
  costFunction->SetImage(inputImage);
  costFunction->SetStartIndex(startPoint);
  costFunction->SetEndIndex(endPoint);
  costFunction->SetRequestedRegion(region);
  /*---------------------------------------------*/


  /* calculate shortest path between start and end point */
  ShortestPathImageFilterType::Pointer shortestPathFilter = ShortestPathImageFilterType::New();
  shortestPathFilter->SetFullNeighborsMode(true);
  shortestPathFilter->SetInput(inputImage);
  shortestPathFilter->SetMakeOutputImage(false);  

  //shortestPathFilter->SetCalcAllDistances(true);
  shortestPathFilter->SetStartIndex(startPoint);
  shortestPathFilter->SetEndIndex(endPoint);

  shortestPathFilter->SetCostFunction(costFunction);

  shortestPathFilter->Update();

  /*---------------------------------------------*/


  /* construct contour from path image */
  //get the shortest path as vector
  typename std::vector< ShortestPathImageFilterType::IndexType> shortestPath = shortestPathFilter->GetVectorPath();

  //fill the output contour with controll points from the path
  OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
  this->SetNthOutput(0, output.GetPointer());

  mitk::Image::ConstPointer input = dynamic_cast<const mitk::Image*>(this->GetInput());

  typename std::vector< ShortestPathImageFilterType::IndexType>::iterator pathIterator = shortestPath.begin();

  while(pathIterator != shortestPath.end())
  {
    mitk::Point3D currentPoint;
    currentPoint[0] = (*pathIterator)[0];
    currentPoint[1] = (*pathIterator)[1];
    currentPoint[2] = 0;


    input->GetGeometry()->IndexToWorld(currentPoint, currentPoint);
    output->AddVertex(currentPoint);
    
    pathIterator++;
  }
  /*---------------------------------------------*/

}