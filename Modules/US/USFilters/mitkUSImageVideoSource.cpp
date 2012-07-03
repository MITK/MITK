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

// MITK HEADER
#include "mitkUSImageVideoSource.h"
#include "mitkImage.h"

//OpenCV HEADER
#include <cv.h>
#include <highgui.h>

//Other
#include <stdio.h>  




mitk::USImageVideoSource::USImageVideoSource()
: itk::Object()
{
    m_IsVideoReady = false;
    m_IsGreyscale = true;
    this->m_OpenCVToMitkFilter = mitk::OpenCVToMitkImageFilter::New();
}


mitk::USImageVideoSource::~USImageVideoSource()
{
}


void mitk::USImageVideoSource::SetVideoFileInput(std::string path)
{
  m_OpenCVVideoSource = mitk::OpenCVVideoSource::New();

  m_OpenCVVideoSource->SetVideoFileInput(path.c_str(),true,false);
  m_OpenCVVideoSource->StartCapturing();
  m_OpenCVVideoSource->FetchFrame();
  
  // Let's see if we have been successful
  m_IsVideoReady = m_OpenCVVideoSource->IsCapturingEnabled();
}

    
void mitk::USImageVideoSource::SetCameraInput(int deviceID)
{
  m_OpenCVVideoSource = mitk::OpenCVVideoSource::New();
  m_OpenCVVideoSource->SetVideoCameraInput(deviceID);

  m_OpenCVVideoSource->StartCapturing();
  m_OpenCVVideoSource->FetchFrame();
  
  // Let's see if we have been successful
  m_IsVideoReady = m_OpenCVVideoSource->IsCapturingEnabled();
}

void mitk::USImageVideoSource::SetColorOutput(bool isColor){
  m_IsGreyscale = !isColor;
}

void mitk::USImageVideoSource::SetRegionOfInterest(int topLeftX, int topLeftY, int bottomRightX, int bottomRightY)
{
  // First, let's do some basic checks to make sure rectangle is inside of actual image
  if (topLeftX < 0) topLeftX = 0;
  if (topLeftY < 0) topLeftY = 0;

  if (bottomRightX >  m_OpenCVVideoSource->GetImageWidth()) bottomRightX = m_OpenCVVideoSource->GetImageWidth();
  if (bottomRightX >  m_OpenCVVideoSource->GetImageHeight()) bottomRightY = m_OpenCVVideoSource->GetImageHeight();

  if (topLeftX > bottomRightX) mitkThrow() << "Invalid boundaries supplied to USImageVideoSource::SetRegionOfInterest()";
  if (topLeftY > bottomRightY) mitkThrow() << "Invalid boundaries supplied to USImageVideoSource::SetRegionOfInterest()";

  m_CropRegion = cv::Rect(topLeftX, topLeftY, bottomRightX - topLeftX, bottomRightY - topLeftY);
}

void mitk::USImageVideoSource::RemoveRegionOfInterest(){
  m_CropRegion.width = 0;
  m_CropRegion.height = 0;
}

mitk::USImage::Pointer mitk::USImageVideoSource::GetNextImage()
{
  // Setup Pointers
  cv::Mat image;
  cv::Mat buffer;

  //Get dimensions and init rgb
  int width  = m_OpenCVVideoSource->GetImageWidth();
  int height = m_OpenCVVideoSource->GetImageHeight();

  // Get Frame from Source
  image = m_OpenCVVideoSource->GetImage();
  m_OpenCVVideoSource->FetchFrame();

  // if Region of interest is set, crop image
  if (m_CropRegion.width > 0){
    buffer = image(m_CropRegion);
    image = buffer;
  }
  // If this is a greyscale image, convert it
  if (m_IsGreyscale)
  {
    cv::cvtColor(image, buffer, CV_RGB2GRAY, 1);
    image = buffer;
  }
  IplImage ipl_img = image;
  this->m_OpenCVToMitkFilter->SetOpenCVImage(&ipl_img);

  this->m_OpenCVToMitkFilter->Update();

  // OpenCVToMitkImageFilter returns a standard mitk::image. We then transform it into an USImage
  mitk::USImage::Pointer result = mitk::USImage::New(this->m_OpenCVToMitkFilter->GetOutput(0));

  return result;
}
