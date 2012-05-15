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

    
void mitk::USImageVideoSource::SetCameraInput(int deviceID){
  m_OpenCVVideoSource->SetVideoCameraInput(deviceID);

  m_OpenCVVideoSource->StartCapturing();
  m_OpenCVVideoSource->FetchFrame();
  
  // Let's see if we have been successful
  m_IsVideoReady = m_OpenCVVideoSource->IsCapturingEnabled();
}


mitk::USImage::Pointer mitk::USImageVideoSource::GetNextImage(){
 // MITK_INFO << "GetNextImage called in USVideoSource!";
  mitk::USImage::Pointer result;
  mitk::Image::Pointer normalImage;
  
  m_OpenCVVideoSource->FetchFrame();

  IplImage* iplImage = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3); 
  m_OpenCVVideoSource->GetCurrentFrameAsOpenCVImage(iplImage);
  
  this->m_OpenCVToMitkFilter->SetOpenCVImage(iplImage);
  this->m_OpenCVToMitkFilter->Update();

  normalImage = this->m_OpenCVToMitkFilter->GetOutput(0);
  result = mitk::USImage::New(normalImage);
  
  
  cvReleaseImage (&iplImage);
 // MITK_INFO << "GetNextImage completed in USVideoSource!";
  return result;
}
