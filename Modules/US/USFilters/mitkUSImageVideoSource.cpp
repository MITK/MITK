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
  : m_VideoCapture(new cv::VideoCapture()),
    m_IsVideoReady(false),
    m_IsGreyscale(false),
    m_ResolutionOverrideWidth(0),
    m_ResolutionOverrideHeight(0),
    m_ResolutionOverride(false),
    m_GrayscaleFilter(mitk::ConvertGrayscaleOpenCVImageFilter::New()),
    m_CropFilter(mitk::CropOpenCVImageFilter::New()),
    m_CombinationFilter(mitk::BasicCombinationOpenCVImageFilter::New())
{
  SetImageFilter(m_CombinationFilter.GetPointer());
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
  if (m_ResolutionOverride)
  {
    m_VideoCapture->set(CV_CAP_PROP_FRAME_WIDTH, this->m_ResolutionOverrideWidth);
    m_VideoCapture->set(CV_CAP_PROP_FRAME_HEIGHT, this->m_ResolutionOverrideHeight);
  }
}

void mitk::USImageVideoSource::SetColorOutput(bool isColor){
  if ( ! isColor && ! m_IsGreyscale )
  {
    m_CombinationFilter->PushFilter(m_GrayscaleFilter.GetPointer());
  }
  else if ( isColor && m_IsGreyscale )
  {
    m_CombinationFilter->RemoveFilter(m_GrayscaleFilter.GetPointer());
  }

  m_IsGreyscale = !isColor;
}

int mitk::USImageVideoSource::GetImageHeight()
{
  if (m_VideoCapture) { return m_VideoCapture->get(CV_CAP_PROP_FRAME_HEIGHT); }
  else { return 0; }
}

int mitk::USImageVideoSource::GetImageWidth()
{
  if (m_VideoCapture) { return m_VideoCapture->get(CV_CAP_PROP_FRAME_WIDTH); }
  else { return 0; }
}

bool mitk::USImageVideoSource::GetIsReady()
{
  if (!m_VideoCapture) { return false; }

  return m_VideoCapture->isOpened();
}

void mitk::USImageVideoSource::SetRegionOfInterest(int topLeftX, int topLeftY, int bottomRightX, int bottomRightY)
{
  m_CropFilter->SetCropRegion(topLeftX, topLeftY, bottomRightX, bottomRightY);

  if (! m_IsCropped ) { m_CombinationFilter->PushFilter(m_CropFilter.GetPointer()); }

  m_IsCropped = true;
}

void mitk::USImageVideoSource::RemoveRegionOfInterest()
{
  //itk::BaseData<>
  m_CombinationFilter->RemoveFilter(m_CropFilter.GetPointer());
  m_IsCropped = false;
}

void mitk::USImageVideoSource::GetNextRawImage( cv::Mat& image )
{
  // Loop video if necessary
  if (m_VideoCapture->get(CV_CAP_PROP_POS_AVI_RATIO) >= 0.99 )
  {
    m_VideoCapture->set(CV_CAP_PROP_POS_AVI_RATIO, 0);
  }

  // Retrieve image
  *m_VideoCapture >> image; // get a new frame from camera
}

void mitk::USImageVideoSource::GetNextRawImage( mitk::Image::Pointer image )
{
  cv::Mat cv_img;

  this->GetNextRawImage(cv_img);

  // Convert to MITK-Image
  IplImage ipl_img = cv_img;

  this->m_OpenCVToMitkFilter->SetOpenCVImage(&ipl_img);
  this->m_OpenCVToMitkFilter->Update();

  // OpenCVToMitkImageFilter returns a standard mitk::image. We then transform it into an USImage
  image = mitk::USImage::New(this->m_OpenCVToMitkFilter->GetOutput());

  // Clean up
  cv_img.release();
}

void mitk::USImageVideoSource::OverrideResolution(int width, int height)
{
  this->m_ResolutionOverrideHeight = height;
  this->m_ResolutionOverrideWidth = width;

  if (m_VideoCapture != 0)
  {
    m_VideoCapture->set(CV_CAP_PROP_FRAME_WIDTH, width);
    m_VideoCapture->set(CV_CAP_PROP_FRAME_HEIGHT, height);
  }
}
