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

#include "mitkShowSegmentationAsSurface.h"

#include "mitkManualSegmentationToSurfaceFilter.h"
#include "mitkVtkRepresentationProperty.h"
#include <mitkCoreObjectFactory.h>
#include <mitkImageCast.h>

#include <vtkPolyDataNormals.h>

#include <itkConstantPadImageFilter.h>

namespace mitk
{

ShowSegmentationAsSurface::ShowSegmentationAsSurface()
{
  m_ProgressAccumulator = itk::ProgressAccumulator::New();
}


ShowSegmentationAsSurface::~ShowSegmentationAsSurface()
{
}

void ShowSegmentationAsSurface::GenerateData()
{
  m_ProgressAccumulator->ResetProgress();
  m_ProgressAccumulator->SetMiniPipelineFilter(this);
  m_ProgressAccumulator->UnregisterAllFilters();

  // Pad image
  typedef itk::ConstantPadImageFilter<InputImageType, InputImageType> PadFilterType;
  typename PadFilterType::Pointer padFilter = PadFilterType::New();
  m_ProgressAccumulator->RegisterInternalFilter(padFilter, .02f);

  InputImageType::SizeType paddingSize;
  paddingSize.Fill(1);

  padFilter->SetInput(m_Input);
  padFilter->SetPadBound(paddingSize);
  padFilter->SetConstant(0);
  padFilter->Update();

  Image::Pointer mitkImage;
  CastToMitkImage<InputImageType>(padFilter->GetOutput(), mitkImage);

  // Move back by slice to adjust image position by padding
  mitk::Point3D point = mitkImage->GetGeometry()->GetOrigin();
  auto spacing = mitkImage->GetGeometry()->GetSpacing();
  auto directionMatrix = m_Input->GetDirection();
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      point[i] -= spacing[i] * directionMatrix[i][j];
    }
  }
  mitkImage->SetOrigin(point);

  ManualSegmentationToSurfaceFilter::Pointer surfaceFilter = ManualSegmentationToSurfaceFilter::New();
  m_ProgressAccumulator->RegisterInternalFilter(surfaceFilter, .98f);
  surfaceFilter->SetInput(mitkImage);
  surfaceFilter->SetThreshold(.5f); //expects binary image with zeros and ones

  surfaceFilter->SetUseGaussianImageSmooth(m_Args.smooth); // apply gaussian to thresholded image ?
  surfaceFilter->SetSmooth(m_Args.smooth);
  if (m_Args.smooth) {
    surfaceFilter->InterpolationOn();
    surfaceFilter->SetGaussianStandardDeviation(m_Args.gaussianSD);
  }

  surfaceFilter->SetMedianFilter3D(m_Args.applyMedian); // apply median to segmentation before marching cubes ?
  if (m_Args.applyMedian) {
    // Apply median to segmentation before marching cubes
    surfaceFilter->SetMedianKernelSize(m_Args.medianKernelSize, m_Args.medianKernelSize, m_Args.medianKernelSize);
  }

  //fix to avoid vtk warnings see bug #5390
  if (mitkImage->GetDimension() > 3) {
    m_Args.decimateMesh = false;
  }

  if (m_Args.decimateMesh) {
    surfaceFilter->SetDecimate( ImageToSurfaceFilter::QuadricDecimation );
    surfaceFilter->SetTargetReduction( m_Args.decimationRate );
  } else {
    surfaceFilter->SetDecimate( ImageToSurfaceFilter::NoDecimation );
  }

  surfaceFilter->UpdateLargestPossibleRegion();

  // calculate normals for nicer display
  Surface::Pointer surf = surfaceFilter->GetOutput();
  vtkPolyData* polyData = surf->GetVtkPolyData();

  if (!polyData) {
    throw std::logic_error("Could not create polygon model");
  }

  polyData->SetVerts(0);
  polyData->SetLines(0);

  if ( m_Args.smooth || m_Args.applyMedian || m_Args.decimateMesh) {
    vtkPolyDataNormals* normalsGen = vtkPolyDataNormals::New();

    normalsGen->SetInputData( polyData );
    normalsGen->Update();

    m_Output = normalsGen->GetOutput();

    normalsGen->Delete();
  } else {
    m_Output = surf->GetVtkPolyData();
  }

  m_ProgressAccumulator->SetMiniPipelineFilter(nullptr);
  m_ProgressAccumulator->UnregisterAllFilters();

  UpdateProgress(1.);
}

} // namespace

