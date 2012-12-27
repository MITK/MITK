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
  m_VideoCapture = new cv::VideoCapture();
  m_IsVideoReady = false;
  m_IsGreyscale  = false;
  this->m_OpenCVToMitkFilter = mitk::OpenCVToMitkImageFilter::New();
  m_ResolutionOverrideWidth = 0;
  m_ResolutionOverrideHeight = 0;
  m_ResolutionOverride = false;
}

mitk::USImageVideoSource::~USImageVideoSource()
{
}

void mitk::USImageVideoSource::SetVideoFileInput(std::string path)
{
  m_VideoCapture->open(path.c_str());
  if(!m_VideoCapture->isOpened())  // check if we succeeded
    m_IsVideoReady = false;
  else
    m_IsVideoReady = true;

  // If Override is enabled, use it
  if (m_ResolutionOverride) {
    m_VideoCapture->set(CV_CAP_PROP_FRAME_WIDTH, this->m_ResolutionOverrideWidth);
    m_VideoCapture->set(CV_CAP_PROP_FRAME_HEIGHT, this->m_ResolutionOverrideHeight);
  }
}


void mitk::USImageVideoSource::SetCameraInput(int deviceID)
{
  m_VideoCapture->open(deviceID);
  if(!m_VideoCapture->isOpened())  // check if we succeeded
    m_IsVideoReady = false;
  else
    m_IsVideoReady = true;

  // If Override is enabled, use it
  if (m_ResolutionOverride) {
    m_VideoCapture->set(CV_CAP_PROP_FRAME_WIDTH, this->m_ResolutionOverrideWidth);
    m_VideoCapture->set(CV_CAP_PROP_FRAME_HEIGHT, this->m_ResolutionOverrideHeight);
  }
}

void mitk::USImageVideoSource::SetColorOutput(bool isColor){
  m_IsGreyscale = !isColor;
}

void mitk::USImageVideoSource::SetRegionOfInterest(int topLeftX, int topLeftY, int bottomRightX, int bottomRightY)
{
  // First, let's do some basic checks to make sure rectangle is inside of actual image
  if (topLeftX < 0) topLeftX = 0;
  if (topLeftY < 0) topLeftY = 0;

  // We can try and correct too large boundaries
  if (bottomRightX >  m_VideoCapture->get(CV_CAP_PROP_FRAME_WIDTH)) bottomRightX = m_VideoCapture->get(CV_CAP_PROP_FRAME_WIDTH);
  if (bottomRightX >  m_VideoCapture->get(CV_CAP_PROP_FRAME_HEIGHT)) bottomRightY = m_VideoCapture->get(CV_CAP_PROP_FRAME_HEIGHT);

  // Nothing to save, throw an exception
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
  // Loop video if necessary
  if (m_VideoCapture->get(CV_CAP_PROP_POS_AVI_RATIO) >= 0.99 )
    m_VideoCapture->set(CV_CAP_PROP_POS_AVI_RATIO, 0);

  // Setup pointers
  cv::Mat image;
  cv::Mat buffer;

  // Retrieve image
  *m_VideoCapture >> image; // get a new frame from camera

  // if Region of interest is set, crop image
  if (m_CropRegion.width > 0){
    buffer = image(m_CropRegion);
    image.release();
    image = buffer;
  }
  // If this source is set to deliver greyscale images, convert it
  if (m_IsGreyscale)
  {
    cv::cvtColor(image, buffer, CV_RGB2GRAY, 1);
    image.release();
    image = buffer;
  }

  // Convert to MITK-Image
  IplImage ipl_img = image;

  this->m_OpenCVToMitkFilter->SetOpenCVImage(&ipl_img);
  this->m_OpenCVToMitkFilter->Update();

  // OpenCVToMitkImageFilter returns a standard mitk::image. We then transform it into an USImage
  mitk::USImage::Pointer result = mitk::USImage::New(this->m_OpenCVToMitkFilter->GetOutput(0));

  // Clean up
  buffer.release();
  image.release();

  return result;
}

void mitk::USImageVideoSource::OverrideResolution(int width, int height){
  this->m_ResolutionOverrideHeight = height;
  this->m_ResolutionOverrideWidth = width;

  if (m_VideoCapture != 0)
  {
    m_VideoCapture->set(CV_CAP_PROP_FRAME_WIDTH, width);
    m_VideoCapture->set(CV_CAP_PROP_FRAME_HEIGHT, height);
  }
}

