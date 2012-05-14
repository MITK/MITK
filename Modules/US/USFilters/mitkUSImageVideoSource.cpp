/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkUSImageVideoSource.h"
#include "mitkImage.h"
#include <cv.h>




mitk::USImageVideoSource::USImageVideoSource()
: itk::Object()
{
    m_IsVideoReady = false;
    m_IsMetadataReady = false;
    m_IsGeometryReady = false;
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
  m_OpenCVVideoSource->SetVideoCameraInput(deviceID);

  m_OpenCVVideoSource->StartCapturing();
  m_OpenCVVideoSource->FetchFrame();
  
  // Let's see if we have been successful
  m_IsVideoReady = m_OpenCVVideoSource->IsCapturingEnabled();
}


mitk::USImage::Pointer mitk::USImageVideoSource::GetNextImage()
{
  m_OpenCVVideoSource->FetchFrame();

  // This is a bit of a workaround: We only need to initialize an OpenCV Image. Actual
  // Initialization happens inside the OpenCVVideoSource
  IplImage* iplImage = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3); 
  m_OpenCVVideoSource->GetCurrentFrameAsOpenCVImage(iplImage);
  
  this->m_OpenCVToMitkFilter->SetOpenCVImage(iplImage);
  this->m_OpenCVToMitkFilter->Update();

  // OpenCVToMitkImageFilter returns a standard mit::image. We then transform it into an USImage
  mitk::USImage::Pointer result = mitk::USImage::New(this->m_OpenCVToMitkFilter->GetOutput(0));
  
  cvReleaseImage (&iplImage);

  return result;
}
