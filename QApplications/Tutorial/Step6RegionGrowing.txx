#include "Step6.h"

#include <mitkProperties.h>
#include <mitkPointSet.h>

#include <itkConfidenceConnectedImageFilter.h>
#include <mitkImageAccessByItk.h>
#include <mitkDataTreeHelper.h>

template < typename TPixel, unsigned int VImageDimension >
void RegionGrowing( itk::Image<TPixel, VImageDimension>* itkImage, Step6* step6)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;

  mitk::Geometry3D* geometry = step6->m_FirstImage->GetGeometry();

  // create itk::ConfidenceConnectedImageFilter and set itkImage as input
  typedef itk::ConfidenceConnectedImageFilter<ImageType, ImageType> ConfidenceConnectedFilterType;
  typedef typename ConfidenceConnectedFilterType::IndexType IndexType;
  typename ConfidenceConnectedFilterType::Pointer regGrowFilter = ConfidenceConnectedFilterType::New();
  regGrowFilter->SetInput(itkImage);

  // convert the points in the PointSet m_Seeds (in world-coordinates) to
  // "index" values, i.e. points in pixel coordinates, and add these as seeds
  // to the ConfidenceConnectedImageFilter
  mitk::PointSet::PointsConstIterator pit, pend = step6->m_Seeds->GetPointSet()->GetPoints()->End();
  IndexType seedIndex;
  for(pit=step6->m_Seeds->GetPointSet()->GetPoints()->Begin();pit!=pend;++pit)
  {
    geometry->WorldToIndex(pit.Value(), seedIndex);
    regGrowFilter->AddSeed( seedIndex );
  }

  // add output of ConfidenceConnectedImageFilter to tree
  mitk::DataTreePreOrderIterator it(step6->m_Tree);
  step6->m_ResultNode = mitk::DataTreeHelper::AddItkImageToDataTree(regGrowFilter->GetOutput(), &it, "segmentation");
  step6->m_ResultImage = static_cast<mitk::Image*>(step6->m_ResultNode->GetData());
  // set some additional properties
  step6->m_ResultNode->SetProperty("binary", new mitk::BoolProperty(true));
  step6->m_ResultNode->SetProperty("color", new mitk::ColorProperty(1.0,0.0,0.0));
  step6->m_ResultNode->SetProperty("volumerendering", new mitk::BoolProperty(true));
  step6->m_ResultNode->SetProperty("layer", new mitk::IntProperty(1));
}

/**
\example Step6RegionGrowing.txx
*/
