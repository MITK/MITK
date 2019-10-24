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

#include <mitkImageVtkAccessor.h>

#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkImageShiftScale.h>

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
  mitk::Image * image    =  (mitk::Image*)GetInput();
  mitk::Image::RegionType outputRegion = image->GetRequestedRegion();

  int tstart=outputRegion.GetIndex(3);
  int tmax=tstart+outputRegion.GetSize(3); //GetSize()==1 - will aber 0 haben, wenn nicht zeitaufgeloest

  ScalarType thresholdExpanded = this->m_Threshold;

  m_ProgressWeight = 1.f / ((float)m_MedianFilter3D
    + (float)m_Interpolation
    + (float)m_UseGaussianImageSmooth * 2
    // Filters from ImageToSurface (used when call CreateSurface())
    + 1 // Skin Extractor
    + (float)m_Smooth
    + (float)(m_Decimate == DecimatePro) + (float)(m_Decimate == QuadricDecimation)
    + 1 // Normals
    + 1); // Clean poly data
  
  m_CurrentProgress = 0.f;
  m_CurrentlyProgressingBuilder = this;

  ImageVtkAccessor accessor(image);
  ImageAccessLock lock(&accessor);
  for( int t=tstart; t<tmax; ++t )
  {
    vtkSmartPointer<vtkImageData> vtkimage = accessor.getVtkImageData(t);

    // Median -->smooth 3D
    MITK_INFO << (m_MedianFilter3D ? "Applying median..." : "No median filtering");
    if(m_MedianFilter3D)
    {
      vtkImageMedian3D *median = vtkImageMedian3D::New();
      median->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
      median->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
      median->SetInputData(vtkimage); //RC++ (VTK < 5.0)
      median->SetKernelSize(m_MedianKernelSizeX,m_MedianKernelSizeY,m_MedianKernelSizeZ);//Std: 3x3x3
      median->ReleaseDataFlagOn();
      median->UpdateInformation();
      median->Update();
      vtkimage = median->GetOutput(); //->Out
      median->Delete();
    }

    //Interpolate image spacing
    MITK_INFO << (m_Interpolation ? "Resampling..." : "No resampling");
    if(m_Interpolation)
    {
      vtkImageResample * imageresample = vtkImageResample::New();
      imageresample->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
      imageresample->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
      imageresample->SetInputData(vtkimage);

      //Set Spacing Manual to 1mm in each direction (Original spacing is lost during image processing)
      imageresample->SetAxisOutputSpacing(0, m_InterpolationX);
      imageresample->SetAxisOutputSpacing(1, m_InterpolationY);
      imageresample->SetAxisOutputSpacing(2, m_InterpolationZ);
      imageresample->UpdateInformation();
      imageresample->Update();
      vtkimage=imageresample->GetOutput();//->Output
      imageresample->Delete();
    }

    MITK_INFO << (m_UseGaussianImageSmooth ? "Applying gaussian smoothing..." : "No gaussian smoothing");
    if(m_UseGaussianImageSmooth)//gauss
    {
      vtkImageShiftScale* scalefilter = vtkImageShiftScale::New();
      scalefilter->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
      scalefilter->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
      scalefilter->SetScale(100);
      scalefilter->SetInputData(vtkimage);
      scalefilter->Update();

      vtkImageGaussianSmooth *gaussian = vtkImageGaussianSmooth::New();
      gaussian->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
      gaussian->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
      gaussian->SetInputConnection(scalefilter->GetOutputPort());
      gaussian->SetDimensionality(3);
      gaussian->SetRadiusFactor(0.49);
      gaussian->SetStandardDeviation( m_GaussianStandardDeviation );
      gaussian->ReleaseDataFlagOn();
      gaussian->UpdateInformation();
      gaussian->Update();

      vtkimage=scalefilter->GetOutput();

      double range[2];
      vtkimage->GetScalarRange(range);

      MITK_DEBUG << "Current scalar max is: " << range[1];
      if (range[1]!=0) //too little slices, image smoothing eliminates all segmentation pixels
      {
        vtkimage = gaussian->GetOutput(); //->Out
      }
      else
      {
        MITK_INFO<<"Smoothing removes all pixels of the segmentation. Use unsmoothed result";
      }
      gaussian->Delete();
      scalefilter->Delete();
    }

    // Create surface for t-Slice
    CreateSurface(t, vtkimage, surface, thresholdExpanded);
  }

  MITK_INFO << "Updating Time Geometry to ensure right timely displaying";
  // Fixing wrong time geometry
  TimeGeometry* surfaceTG = surface->GetTimeGeometry();
  ProportionalTimeGeometry* surfacePTG = dynamic_cast<ProportionalTimeGeometry*>(surfaceTG);
  TimeGeometry* imageTG = image->GetTimeGeometry();
  ProportionalTimeGeometry* imagePTG = dynamic_cast<ProportionalTimeGeometry*>(imageTG);
  // Requires ProportionalTimeGeometries to work. May not be available for all steps.
  assert(surfacePTG != nullptr);
  assert(imagePTG != nullptr);
  if ((surfacePTG != nullptr) && (imagePTG != nullptr))
  {
    TimePointType firstTime = imagePTG->GetFirstTimePoint();
    TimePointType duration = imagePTG->GetStepDuration();
    surfacePTG->SetFirstTimePoint(firstTime);
    surfacePTG->SetStepDuration(duration);
    MITK_INFO << "First Time Point: " << firstTime << "  Duration: " << duration;
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
