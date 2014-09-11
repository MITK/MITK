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


#include <itkImageRegionIterator.h>
#include <itkCastImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>

#include "mitkIOUtil.h"

mitk::ImageLiveWireContourModelFilter::ImageLiveWireContourModelFilter()
{
  OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfIndexedOutputs( 1 );
  this->SetNthOutput(0, output.GetPointer());
  m_CostFunction = CostFunctionType::New();
  m_ShortestPathFilter = ShortestPathImageFilterType::New();
  m_ShortestPathFilter->SetCostFunction(m_CostFunction);
  m_UseDynamicCostMap = false;
  m_TimeStep = 0;
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

    AccessFixedDimensionByItk(input, ItkPreProcessImage, 2);
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
      try
      {
        this->UpdateLiveWire();
      }
      catch( itk::ExceptionObject & e )
      {
        MITK_INFO << "Exception caught during live wiring calculation: " << e;
        return;
      }
  }
}


template<typename TPixel, unsigned int VImageDimension>
void mitk::ImageLiveWireContourModelFilter::ItkPreProcessImage (const itk::Image<TPixel, VImageDimension>* inputImage)
{
  typedef itk::Image< TPixel, VImageDimension >                      InputImageType;
  typedef itk::CastImageFilter< InputImageType, InternalImageType >  CastFilterType;

  typename CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(inputImage);
  castFilter->Update();
  m_InternalImage = castFilter->GetOutput();
  m_CostFunction->SetImage( m_InternalImage );
  m_ShortestPathFilter->SetInput( m_InternalImage );
}

void mitk::ImageLiveWireContourModelFilter::ClearRepulsivePoints()
{
    m_CostFunction->ClearRepulsivePoints();
}

void mitk::ImageLiveWireContourModelFilter::AddRepulsivePoint( const itk::Index<2>& idx )
{
    m_CostFunction->AddRepulsivePoint(idx);
}

void mitk::ImageLiveWireContourModelFilter::DumpMaskImage()
{
    mitk::Image::Pointer mask = mitk::Image::New();
    mask->InitializeByItk( this->m_CostFunction->GetMaskImage() );
    mask->SetVolume( this->m_CostFunction->GetMaskImage()->GetBufferPointer() );
    mitk::IOUtil::SaveImage(mask, "G:\\Data\\mask.nrrd");
/*
    mitk::Image::Pointer slice = mitk::Image::New();
    slice->InitializeByItk( this->m_CostFunction->m_MaskImage.GetPointer() );
    slice->SetVolume(this->m_CostFunction->m_MaskImage->GetBufferPointer());
    */

}

void mitk::ImageLiveWireContourModelFilter::RemoveRepulsivePoint( const itk::Index<2>& idx )
{
    m_CostFunction->RemoveRepulsivePoint(idx);
}

void mitk::ImageLiveWireContourModelFilter::SetRepulsivePoints(const ShortestPathType& points)
{
  m_CostFunction->ClearRepulsivePoints();

  ShortestPathType::const_iterator iter = points.begin();
  for (;iter != points.end(); iter++)
  {
      m_CostFunction->AddRepulsivePoint( (*iter) );
  }
}

void mitk::ImageLiveWireContourModelFilter::UpdateLiveWire()
{
// compute the requested region for itk filters
  InternalImageType::IndexType startPoint, endPoint;

  startPoint[0] = m_StartPointInIndex[0];
  startPoint[1] = m_StartPointInIndex[1];

  endPoint[0] = m_EndPointInIndex[0];
  endPoint[1] = m_EndPointInIndex[1];

  // minimum value in each direction for startRegion
  InternalImageType::IndexType startRegion;
  startRegion[0] = startPoint[0] < endPoint[0] ? startPoint[0] : endPoint[0];
  startRegion[1] = startPoint[1] < endPoint[1] ? startPoint[1] : endPoint[1];

  // maximum value in each direction for size
  InternalImageType::SizeType size;
  size[0] = abs( startPoint[0] - endPoint[0] ) + 1;
  size[1] = abs( startPoint[1] - endPoint[1] ) + 1;

  CostFunctionType::RegionType region;
  region.SetSize( size );
  region.SetIndex( startRegion );

  //inputImage->SetRequestedRegion(region);

  // extracts features from image and calculates costs
  //m_CostFunction->SetImage(m_InternalImage);
  m_CostFunction->SetStartIndex(startPoint);
  m_CostFunction->SetEndIndex(endPoint);
  m_CostFunction->SetRequestedRegion(region);
  m_CostFunction->SetUseCostMap(m_UseDynamicCostMap);

  // calculate shortest path between start and end point
  m_ShortestPathFilter->SetFullNeighborsMode(true);
  //m_ShortestPathFilter->SetInput( m_CostFunction->SetImage(m_InternalImage) );
  m_ShortestPathFilter->SetMakeOutputImage(false);

  //m_ShortestPathFilter->SetCalcAllDistances(true);
  m_ShortestPathFilter->SetStartIndex(startPoint);
  m_ShortestPathFilter->SetEndIndex(endPoint);

  m_ShortestPathFilter->Update();

  // construct contour from path image
  //get the shortest path as vector
  ShortestPathType shortestPath = m_ShortestPathFilter->GetVectorPath();

  //fill the output contour with control points from the path
  OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
  this->SetNthOutput(0, output.GetPointer());

//  OutputType::Pointer output = dynamic_cast<OutputType*> ( this->GetOutput() );
  output->Expand(m_TimeStep+1);

//  output->Clear();

  mitk::Image::ConstPointer input = dynamic_cast<const mitk::Image*>(this->GetInput());

  ShortestPathType::const_iterator pathIterator = shortestPath.begin();

  while(pathIterator != shortestPath.end())
  {
    mitk::Point3D currentPoint;
    currentPoint[0] = static_cast<mitk::ScalarType>( (*pathIterator)[0] );
    currentPoint[1] = static_cast<mitk::ScalarType>( (*pathIterator)[1] );
    currentPoint[2] = 0.0;

    input->GetGeometry()->IndexToWorld(currentPoint, currentPoint);
    output->AddVertex(currentPoint, false, m_TimeStep);

    pathIterator++;
  }
}


bool mitk::ImageLiveWireContourModelFilter::CreateDynamicCostMap(mitk::ContourModel* path)
{
  mitk::Image::ConstPointer input = dynamic_cast<const mitk::Image*>(this->GetInput());
  if(!input) return false;

  try
  {
    AccessFixedDimensionByItk_1(input,CreateDynamicCostMapByITK, 2, path);
  }
  catch( itk::ExceptionObject & e )
  {
    MITK_INFO << "Exception caught during dynamic cost map alculation: " << e;
    return false;
  }

  return true;
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::ImageLiveWireContourModelFilter::CreateDynamicCostMapByITK( const itk::Image<TPixel, VImageDimension>* inputImage, mitk::ContourModel* path )
{
  /*++++++++++ create dynamic cost transfer map ++++++++++*/

  /* Compute  the costs of the gradient magnitude dynamically.
  * using a map of the histogram of gradient magnitude image.
  * Use the histogram gradient map to interpolate the costs
  * with gaussing function including next two bins right and left
  * to current position x. With the histogram gradient costs are interpolated
  * with a gaussing function summation of next two bins right and left
  * to current position x.
  */
  std::vector< itk::Index<VImageDimension> > shortestPath;

  mitk::Image::ConstPointer input = dynamic_cast<const mitk::Image*>(this->GetInput());
  if(path == NULL)
  {
    OutputType::Pointer output = this->GetOutput();
    mitk::ContourModel::VertexIterator it = output->IteratorBegin();
    while( it != output->IteratorEnd() )
    {
      itk::Index<VImageDimension> cur;
      mitk::Point3D c = (*it)->Coordinates;
      input->GetGeometry()->WorldToIndex(c, c);
      cur[0] = c[0];
      cur[1] = c[1];

      shortestPath.push_back( cur);
      it++;
    }
  }
  else
  {

    mitk::ContourModel::VertexIterator it = path->IteratorBegin();
    while( it != path->IteratorEnd() )
    {
      itk::Index<VImageDimension> cur;
      mitk::Point3D c = (*it)->Coordinates;
      input->GetGeometry()->WorldToIndex(c, c);
      cur[0] = c[0];
      cur[1] = c[1];

      shortestPath.push_back( cur);
      it++;
    }

  }

  // filter image gradient magnitude
  typedef  itk::GradientMagnitudeImageFilter< itk::Image<TPixel, VImageDimension>,  itk::Image<TPixel, VImageDimension> > GradientMagnitudeFilterType;
  typename GradientMagnitudeFilterType::Pointer gradientFilter = GradientMagnitudeFilterType::New();
  gradientFilter->SetInput(inputImage);
  gradientFilter->Update();
  typename itk::Image<TPixel, VImageDimension>::Pointer gradientMagnImage = gradientFilter->GetOutput();

  //get the path

  //iterator of path
  typename std::vector< itk::Index<VImageDimension> >::iterator pathIterator = shortestPath.begin();

  std::map< int, int > histogram;

  //create histogram within path
  while(pathIterator != shortestPath.end())
  {
    //count pixel values
    //use scale factor to avoid mapping gradients between 0.0 and 1.0 to same bin
    histogram[ static_cast<int>( gradientMagnImage->GetPixel((*pathIterator)) * ImageLiveWireContourModelFilter::CostFunctionType::MAPSCALEFACTOR ) ] += 1;

    pathIterator++;
  }

  double max = 1.0;

  if( !histogram.empty() )
  {
    std::map< int, int >::iterator itMAX;

    //get max of histogramm
    int currentMaxValue = 0;
    std::map< int, int >::iterator it = histogram.begin();
    while( it != histogram.end())
    {
      if((*it).second > currentMaxValue)
      {
        itMAX = it;
        currentMaxValue = (*it).second;
      }
      it++;
    }

    std::map< int, int >::key_type keyOfMax = itMAX->first;

    // compute the to max of gaussian summation
    std::map< int, int >::iterator end = histogram.end();
    std::map< int, int >::iterator last = --(histogram.end());

    std::map< int, int >::iterator left2;
    std::map< int, int >::iterator left1;
    std::map< int, int >::iterator right1;
    std::map< int, int >::iterator right2;

    right1 = itMAX;

    if(right1 == end || right1 == last )
    {
      right2 = end;
    }
    else//( right1 <= last )
    {
      std::map< int, int >::iterator temp = right1;
      right2 = ++right1;//rght1 + 1
      right1 = temp;
    }

    if( right1 == histogram.begin() )
    {
      left1 = end;
      left2 = end;
    }
    else if( right1 == (++(histogram.begin())) )
    {
      std::map< int, int >::iterator temp = right1;
      left1  = --right1;//rght1 - 1
      right1 = temp;
      left2  = end;
    }
    else
    {
      std::map< int, int >::iterator temp = right1;
      left1 = --right1;//rght1 - 1
      left2 = --right1;//rght1 - 2
      right1 = temp;
    }

    double partRight1, partRight2, partLeft1, partLeft2;
    partRight1 = partRight2 = partLeft1 = partLeft2 = 0.0;

    /*
    f(x) = v(bin) * e^ ( -1/2 * (|x-k(bin)| / sigma)^2 )

    gaussian approximation

    where
    v(bin) is the value in the map
    k(bin) is the key
    */

    if( left2 != end )
    {
      partLeft2 = ImageLiveWireContourModelFilter::CostFunctionType::Gaussian(keyOfMax, left2->first, left2->second);
    }

    if( left1 != end )
    {
      partLeft1 = ImageLiveWireContourModelFilter::CostFunctionType::Gaussian(keyOfMax, left1->first, left1->second);
    }

    if( right1 != end )
    {
      partRight1 = ImageLiveWireContourModelFilter::CostFunctionType::Gaussian(keyOfMax, right1->first, right1->second);
    }

    if( right2 != end )
    {
      partRight2 = ImageLiveWireContourModelFilter::CostFunctionType::Gaussian(keyOfMax, right2->first, right2->second);
    }

    max = (partRight1 + partRight2 + partLeft1 + partLeft2);

  }

  this->m_CostFunction->SetDynamicCostMap(histogram);
  this->m_CostFunction->SetCostMapMaximum(max);
}
