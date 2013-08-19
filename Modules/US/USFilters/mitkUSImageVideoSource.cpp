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
#include "mitkCropOpenCVImageFilter.h"
#include "mitkConvertGrayscaleOpenCVImageFilter.h"

//OpenCV HEADER
#include <cv.h>
#include <highgui.h>

//Other
#include <stdio.h>


mitk::USImageVideoSource::USImageVideoSource()
: itk::Object(),
m_VideoCapture(new cv::VideoCapture()),
m_IsVideoReady(false),
m_IsGreyscale(false),
m_OpenCVToMitkFilter(mitk::OpenCVToMitkImageFilter::New()),
m_ResolutionOverrideWidth(0),
m_ResolutionOverrideHeight(0),
m_ResolutionOverride(false),
m_ImageFilter(0),
m_GrayscaleFilter(mitk::ConvertGrayscaleOpenCVImageFilter::New()),
m_CropFilter(mitk::CropOpenCVImageFilter::New())
{
  m_OpenCVToMitkFilter->SetCopyBuffer(false);
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

int mitk::USImageVideoSource::GetImageHeight()
{
if (m_VideoCapture) return m_VideoCapture->get(CV_CAP_PROP_FRAME_HEIGHT);
else return 0;
}

int mitk::USImageVideoSource::GetImageWidth()
{
if (m_VideoCapture) return m_VideoCapture->get(CV_CAP_PROP_FRAME_WIDTH);
else return 0;
}

bool mitk::USImageVideoSource::GetIsReady()
{
  if (!m_VideoCapture) return false;

  return m_VideoCapture->isOpened();
}

void mitk::USImageVideoSource::SetRegionOfInterest(int topLeftX, int topLeftY, int bottomRightX, int bottomRightY)
{
  m_CropFilter->SetCropRegion(topLeftX, topLeftY, bottomRightX, bottomRightY);
  m_IsCropped = true;
}

void mitk::USImageVideoSource::RemoveRegionOfInterest(){
  m_IsCropped = false;
}

mitk::USImage::Pointer mitk::USImageVideoSource::GetNextImage()
{
  // Loop video if necessary
  if (m_VideoCapture->get(CV_CAP_PROP_POS_AVI_RATIO) >= 0.99 )
    m_VideoCapture->set(CV_CAP_PROP_POS_AVI_RATIO, 0);

  // Setup pointers
  cv::Mat image;

  // Retrieve image
  *m_VideoCapture >> image; // get a new frame from camera

  // If region of interest was set, crop image
  if ( m_IsCropped ) { m_CropFilter->FilterImage(image); }

  // If this source is set to deliver greyscale images, convert it
  if ( m_IsGreyscale ) { m_GrayscaleFilter->FilterImage(image); }

  // Execute filter, if an additional filter is specified
  if ( m_ImageFilter.IsNotNull() ) { m_ImageFilter->FilterImage(image); }

  // Convert to MITK-Image
  IplImage ipl_img = image;

  this->m_OpenCVToMitkFilter->SetOpenCVImage(&ipl_img);
  this->m_OpenCVToMitkFilter->Update();

  // OpenCVToMitkImageFilter returns a standard mitk::image. We then transform it into an USImage
  mitk::USImage::Pointer result = mitk::USImage::New(this->m_OpenCVToMitkFilter->GetOutput());

  // Clean up
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

