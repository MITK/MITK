#include "Step6.h"

#include <mitkProperties.h>
#include <mitkPointSet.h>

#include <itkCurvatureFlowImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>

#include <mitkImageAccessByItk.h>
#include <mitkDataTreeHelper.h>

template < typename TPixel, unsigned int VImageDimension >
void RegionGrowing( itk::Image<TPixel, VImageDimension>* itkImage, Step6* step6 )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;

  typedef float InternalPixelType;
  typedef itk::Image< InternalPixelType, VImageDimension > InternalImageType;

  mitk::Geometry3D* geometry = step6->m_FirstImage->GetGeometry();

  // create itk::CurvatureFlowImageFilter for smoothing and set itkImage as input
  typedef itk::CurvatureFlowImageFilter< ImageType, InternalImageType > 
    CurvatureFlowFilter;
  CurvatureFlowFilter::Pointer smoothingFilter = CurvatureFlowFilter::New();
  
  smoothingFilter->SetInput( itkImage );
  smoothingFilter->SetNumberOfIterations( 4 );
  smoothingFilter->SetTimeStep( 0.0625 );
  
 
  // create itk::ConnectedThresholdImageFilter and set filtered image as input
  typedef itk::ConnectedThresholdImageFilter< InternalImageType, ImageType > RegionGrowingFilterType;
  typedef typename RegionGrowingFilterType::IndexType IndexType;
  typename RegionGrowingFilterType::Pointer regGrowFilter = RegionGrowingFilterType::New();
  
  regGrowFilter->SetInput( smoothingFilter->GetOutput() );
  regGrowFilter->SetLower( step6->GetThresholdMin() );
  regGrowFilter->SetUpper( step6->GetThresholdMax() );
  
  // convert the points in the PointSet m_Seeds (in world-coordinates) to
  // "index" values, i.e. points in pixel coordinates, and add these as seeds
  // to the RegionGrower
  mitk::PointSet::PointsConstIterator pit, pend = step6->m_Seeds->GetPointSet()->GetPoints()->End();
  IndexType seedIndex;
  for(pit=step6->m_Seeds->GetPointSet()->GetPoints()->Begin();pit!=pend;++pit)
  {
    geometry->WorldToIndex(pit.Value(), seedIndex);
    regGrowFilter->AddSeed( seedIndex );
  }

  // add output of RegionGrower to tree
  mitk::DataTreePreOrderIterator it(step6->m_Tree);
  step6->m_ResultNode = mitk::DataTreeHelper::AddItkImageToDataTree(regGrowFilter->GetOutput(), &it, "segmentation");
  step6->m_ResultImage = static_cast<mitk::Image*>(step6->m_ResultNode->GetData());
  // set some additional properties
  step6->m_ResultNode->SetProperty("binary", mitk::BoolProperty::New(true));
  step6->m_ResultNode->SetProperty("color", mitk::ColorProperty::New(1.0,0.0,0.0));
  step6->m_ResultNode->SetProperty("volumerendering", mitk::BoolProperty::New(true));
  step6->m_ResultNode->SetProperty("layer", mitk::IntProperty::New(1));
}

/**
\example Step6RegionGrowing.txx
*/
