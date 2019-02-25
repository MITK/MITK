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

#include <mitkManualSegmentationToSurfaceFilter.h>

#include <vtkImageShiftScale.h>
#include <vtkSmartPointer.h>

#include "mitkProgressBar.h"

mitk::ManualSegmentationToSurfaceFilter::ManualSegmentationToSurfaceFilter()
{
  m_MedianFilter3D = false;
  m_MedianKernelSizeX = 3;
  m_MedianKernelSizeY = 3;
  m_MedianKernelSizeZ = 3;
  m_UseGaussianImageSmooth = false;
  m_GaussianStandardDeviation = 1.5;
  m_Interpolation = false;
  m_InterpolationX = 1.0f;
  m_InterpolationY = 1.0f;
  m_InterpolationZ = 1.0f;
};

mitk::ManualSegmentationToSurfaceFilter::~ManualSegmentationToSurfaceFilter(){};

void mitk::ManualSegmentationToSurfaceFilter::GenerateData()
{
  mitk::Surface *surface = this->GetOutput();
  auto *image = (mitk::Image *)GetInput();
  mitk::Image::RegionType outputRegion = image->GetRequestedRegion();

  int tstart = outputRegion.GetIndex(3);
  int tmax = tstart + outputRegion.GetSize(3); // GetSize()==1 - will aber 0 haben, wenn nicht zeitaufgeloest

  ScalarType thresholdExpanded = this->m_Threshold;

  if ((tmax - tstart) > 0)
  {
    ProgressBar::GetInstance()->AddStepsToDo(4 * (tmax - tstart));
  }
  else
  {
    ProgressBar::GetInstance()->AddStepsToDo(4);
  }

  for (int t = tstart; t < tmax; ++t)
  {
    vtkSmartPointer<vtkImageData> vtkimage = image->GetVtkImageData(t);

    // Median -->smooth 3D
    // MITK_INFO << (m_MedianFilter3D ? "Applying median..." : "No median filtering");
    if (m_MedianFilter3D)
    {
      vtkImageMedian3D *median = vtkImageMedian3D::New();
      median->SetInputData(vtkimage);                                                       // RC++ (VTK < 5.0)
      median->SetKernelSize(m_MedianKernelSizeX, m_MedianKernelSizeY, m_MedianKernelSizeZ); // Std: 3x3x3
      median->ReleaseDataFlagOn();
      median->UpdateInformation();
      median->Update();
      vtkimage = median->GetOutput(); //->Out
      median->Delete();
    }
    ProgressBar::GetInstance()->Progress();

    // Interpolate image spacing
    // MITK_INFO << (m_Interpolation ? "Resampling..." : "No resampling");
    if (m_Interpolation)
    {
      vtkImageResample *imageresample = vtkImageResample::New();
      imageresample->SetInputData(vtkimage);

      // Set Spacing Manual to 1mm in each direction (Original spacing is lost during image processing)
      imageresample->SetAxisOutputSpacing(0, m_InterpolationX);
      imageresample->SetAxisOutputSpacing(1, m_InterpolationY);
      imageresample->SetAxisOutputSpacing(2, m_InterpolationZ);
      imageresample->UpdateInformation();
      imageresample->Update();
      vtkimage = imageresample->GetOutput(); //->Output
      imageresample->Delete();
    }
    ProgressBar::GetInstance()->Progress();

    // MITK_INFO << (m_UseGaussianImageSmooth ? "Applying gaussian smoothing..." : "No gaussian smoothing");
    if (m_UseGaussianImageSmooth) // gauss
    {
      vtkImageShiftScale *scalefilter = vtkImageShiftScale::New();
      scalefilter->SetScale(100);
      scalefilter->SetInputData(vtkimage);
      scalefilter->Update();

      vtkImageGaussianSmooth *gaussian = vtkImageGaussianSmooth::New();
      gaussian->SetInputConnection(scalefilter->GetOutputPort());
      gaussian->SetDimensionality(3);
      gaussian->SetRadiusFactor(0.49);
      gaussian->SetStandardDeviation(m_GaussianStandardDeviation);
      gaussian->ReleaseDataFlagOn();
      gaussian->UpdateInformation();
      gaussian->Update();

      vtkimage = scalefilter->GetOutput();

      double range[2];
      vtkimage->GetScalarRange(range);

      if (range[1] != 0) // too little slices, image smoothing eliminates all segmentation pixels
      {
        vtkimage = gaussian->GetOutput(); //->Out
      }
      else
      {
        MITK_INFO << "Smoothing would remove all pixels of the segmentation. Use unsmoothed result instead.";
      }
      gaussian->Delete();
      scalefilter->Delete();
    }
    ProgressBar::GetInstance()->Progress();

    // Create surface for t-Slice
    CreateSurface(t, vtkimage, surface, thresholdExpanded);
    ProgressBar::GetInstance()->Progress();
  }

  // MITK_INFO << "Updating Time Geometry to ensure right timely displaying";
  // Fixing wrong time geometry
  TimeGeometry *surfaceTG = surface->GetTimeGeometry();
  auto *surfacePTG = dynamic_cast<ProportionalTimeGeometry *>(surfaceTG);
  TimeGeometry *imageTG = image->GetTimeGeometry();
  auto *imagePTG = dynamic_cast<ProportionalTimeGeometry *>(imageTG);
  // Requires ProportionalTimeGeometries to work. May not be available for all steps.
  assert(surfacePTG != nullptr);
  assert(imagePTG != nullptr);
  if ((surfacePTG != nullptr) && (imagePTG != nullptr))
  {
    TimePointType firstTime = imagePTG->GetFirstTimePoint();
    TimePointType duration = imagePTG->GetStepDuration();
    surfacePTG->SetFirstTimePoint(firstTime);
    surfacePTG->SetStepDuration(duration);
    // MITK_INFO << "First Time Point: " << firstTime << "  Duration: " << duration;
  }
};

void mitk::ManualSegmentationToSurfaceFilter::SetMedianKernelSize(int x, int y, int z)
{
  m_MedianKernelSizeX = x;
  m_MedianKernelSizeY = y;
  m_MedianKernelSizeZ = z;
}

void mitk::ManualSegmentationToSurfaceFilter::SetInterpolation(vtkDouble x, vtkDouble y, vtkDouble z)
{
  m_InterpolationX = x;
  m_InterpolationY = y;
  m_InterpolationZ = z;
}
