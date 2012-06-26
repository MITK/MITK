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



  /* Old Code, this will probably not work
  m_OpenCVVideoSource = mitk::OpenCVVideoSource::New();

  m_OpenCVVideoSource->SetVideoCameraInput(deviceID);

  m_OpenCVVideoSource->StartCapturing();
  m_OpenCVVideoSource->FetchFrame();
  
  // Let's see if we have been successful
  m_IsVideoReady = m_OpenCVVideoSource->IsCapturingEnabled();
  */
}


mitk::USImage::Pointer mitk::USImageVideoSource::GetNextImage()
{
  IplImage *m_cvCurrentVideoFrame = NULL;
  CvCapture* capture = cvCaptureFromCAM( 1000 );
   if ( !capture ) {
     fprintf( stderr, "ERROR: capture is NULL \n" );
     getchar();
     return NULL;
   }
   // Show the image captured from the camera in the window and repeat
   
   // Get one frame
   m_cvCurrentVideoFrame  = cvQueryFrame( capture );
     
   

/// WORKING CODE


  /*IplImage *m_cvCurrentVideoFrame = NULL;
  int height = m_OpenCVVideoSource->GetImageHeight();
  int width = m_OpenCVVideoSource->GetImageWidth();
  m_cvCurrentVideoFrame = cvCreateImage(cvSize(width,height),8,3);
  m_OpenCVVideoSource->GetCurrentFrameAsOpenCVImage(m_cvCurrentVideoFrame);
  m_OpenCVVideoSource->FetchFrame();
  */  
  this->m_OpenCVToMitkFilter->SetOpenCVImage(m_cvCurrentVideoFrame);
  this->m_OpenCVToMitkFilter->Update();

  // OpenCVToMitkImageFilter returns a standard mit::image. We then transform it into an USImage
  mitk::USImage::Pointer result = mitk::USImage::New(this->m_OpenCVToMitkFilter->GetOutput(0));
  
  cvReleaseImage (&m_cvCurrentVideoFrame);

  return result;
}
