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
    m_IsMetadataReady = false;
    m_IsGeometryReady = false;
    m_IsGreyscale = true;
    this->m_OpenCVToMitkFilter = mitk::OpenCVToMitkImageFilter::New();
}


mitk::USImageVideoSource::~USImageVideoSource()
{
}


void mitk::USImageVideoSource::SetVideoFileInput(std::string path)
{
  m_OpenCVVideoSource = mitk::OpenCVVideoSource::New();

  // Example: "C:\\Users\\maerz\\Videos\\Debut\\us.avi"
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


mitk::USImage::Pointer mitk::USImageVideoSource::GetNextImage()
{
  // Setup Pointers
  IplImage *rgbImage = NULL;
  IplImage *greyImage = NULL;

  //Get Dimensions and init rgb
  int height = m_OpenCVVideoSource->GetImageHeight();
  int width = m_OpenCVVideoSource->GetImageWidth();
  rgbImage = cvCreateImage(cvSize(width,height),8,3);

  // Get Frame from Source
  m_OpenCVVideoSource->GetCurrentFrameAsOpenCVImage(rgbImage);
  m_OpenCVVideoSource->FetchFrame();

  // If this is a greyscale image, convert it and put into the filter.
  if (m_IsGreyscale){
    greyImage = cvCreateImage(cvSize(width,height),8,1);
    cvCvtColor( rgbImage, greyImage, CV_RGB2GRAY );
    this->m_OpenCVToMitkFilter->SetOpenCVImage(greyImage);
  } else {
    this->m_OpenCVToMitkFilter->SetOpenCVImage(rgbImage);
  }
  this->m_OpenCVToMitkFilter->Update();

  // OpenCVToMitkImageFilter returns a standard mit::image. We then transform it into an USImage
  mitk::USImage::Pointer result = mitk::USImage::New(this->m_OpenCVToMitkFilter->GetOutput(0));
  
  cvReleaseImage (&rgbImage);
  cvReleaseImage (&greyImage);

  return result;
}
