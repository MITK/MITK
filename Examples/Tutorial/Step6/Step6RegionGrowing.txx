/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "Step6.h"

#include <mitkPointSet.h>
#include <mitkProperties.h>

#include <itkConnectedThresholdImageFilter.h>
#include <itkCurvatureFlowImageFilter.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkLevelWindowProperty.h>

template <typename TPixel, unsigned int VImageDimension>
void RegionGrowing(itk::Image<TPixel, VImageDimension> *itkImage, Step6 *step6)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;

  typedef float InternalPixelType;
  typedef itk::Image<InternalPixelType, VImageDimension> InternalImageType;

  mitk::BaseGeometry *geometry = step6->m_FirstImage->GetGeometry();

  // create itk::CurvatureFlowImageFilter for smoothing and set itkImage as input
  typedef itk::CurvatureFlowImageFilter<ImageType, InternalImageType> CurvatureFlowFilter;
  typename CurvatureFlowFilter::Pointer smoothingFilter = CurvatureFlowFilter::New();

  smoothingFilter->SetInput(itkImage);
  smoothingFilter->SetNumberOfIterations(4);
  smoothingFilter->SetTimeStep(0.0625);

  // create itk::ConnectedThresholdImageFilter and set filtered image as input
  typedef itk::ConnectedThresholdImageFilter<InternalImageType, ImageType> RegionGrowingFilterType;
  typedef typename RegionGrowingFilterType::IndexType IndexType;
  typename RegionGrowingFilterType::Pointer regGrowFilter = RegionGrowingFilterType::New();

  regGrowFilter->SetInput(smoothingFilter->GetOutput());
  regGrowFilter->SetLower(step6->GetThresholdMin());
  regGrowFilter->SetUpper(step6->GetThresholdMax());

  // convert the points in the PointSet m_Seeds (in world-coordinates) to
  // "index" values, i.e. points in pixel coordinates, and add these as seeds
  // to the RegionGrower
  mitk::PointSet::PointsConstIterator pit, pend = step6->m_Seeds->GetPointSet()->GetPoints()->End();
  IndexType seedIndex;
  for (pit = step6->m_Seeds->GetPointSet()->GetPoints()->Begin(); pit != pend; ++pit)
  {
    geometry->WorldToIndex(pit.Value(), seedIndex);
    regGrowFilter->AddSeed(seedIndex);
  }

  regGrowFilter->GetOutput()->Update();
  mitk::Image::Pointer mitkImage = mitk::Image::New();
  mitk::CastToMitkImage(regGrowFilter->GetOutput(), mitkImage);

  if (step6->m_ResultNode.IsNull())
  {
    step6->m_ResultNode = mitk::DataNode::New();
    step6->m_DataStorage->Add(step6->m_ResultNode);
  }
  step6->m_ResultNode->SetData(mitkImage);
  // set some additional properties
  step6->m_ResultNode->SetProperty("name", mitk::StringProperty::New("segmentation"));
  step6->m_ResultNode->SetProperty("binary", mitk::BoolProperty::New(true));
  step6->m_ResultNode->SetProperty("color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
  step6->m_ResultNode->SetProperty("volumerendering", mitk::BoolProperty::New(true));
  step6->m_ResultNode->SetProperty("layer", mitk::IntProperty::New(1));
  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto(mitkImage);
  levWinProp->SetLevelWindow(levelwindow);
  step6->m_ResultNode->SetProperty("levelwindow", levWinProp);

  step6->m_ResultImage = static_cast<mitk::Image *>(step6->m_ResultNode->GetData());
} //RegionGrowing()

/**
\example Step6RegionGrowing.txx
*/
