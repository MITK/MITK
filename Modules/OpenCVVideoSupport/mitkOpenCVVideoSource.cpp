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

#include "mitkOpenCVVideoSource.h"
#include <iostream>
#include <algorithm>

mitk::OpenCVVideoSource::OpenCVVideoSource()
: m_VideoCapture(0),
  m_CurrentImage(0),
  m_CurrentVideoTexture(0),
  m_PauseImage(0),
  m_CapturePaused(false),
  m_GrabbingDeviceNumber(-1),
  m_RepeatVideo(false),
  m_UseCVCAMLib(false),
  m_UndistortImage(false)
{
}

mitk::OpenCVVideoSource::~OpenCVVideoSource()
{
  this->Reset();  
}

void mitk::OpenCVVideoSource::SetVideoFileInput(const char * filename, bool repeatVideo, bool useCVCAMLib)
{
  this->Reset();
  m_VideoFileName = filename;

  m_VideoCapture = cvCaptureFromFile(filename);
  if(!m_VideoCapture)
    MITK_WARN << "Error in initializing video file input!";
 
  m_RepeatVideo  = repeatVideo;
  //m_CurrentImage        = cvCreateImage(cvSize(m_CaptureWidth,m_CaptureHeight),8,3);
    
}
void mitk::OpenCVVideoSource::SetVideoCameraInput(int cameraindex, bool useCVCAMLib)
{
  this->Reset();
 
  m_GrabbingDeviceNumber = cameraindex;
  m_VideoCapture = cvCaptureFromCAM(m_GrabbingDeviceNumber);
  if(!m_VideoCapture)
    MITK_ERROR << "Error in initializing CVHighGUI video camera!"<< std::endl;
  
}

double mitk::OpenCVVideoSource::GetVideoCaptureProperty(int property_id)
{
  return cvGetCaptureProperty(m_VideoCapture, property_id);
} 

int mitk::OpenCVVideoSource::SetVideoCaptureProperty(int property_id, double value)
{
  return cvSetCaptureProperty(m_VideoCapture, property_id, value);
}

//method extended for "static video feature" if enabled
unsigned char* mitk::OpenCVVideoSource::GetVideoTexture()
{ // Fetch Frame and return pointer to opengl texture  
  FetchFrame(); 

  if (m_RotationEnabled)
  { 
    //store pointer to release memory for the image after rotation
    IplImage * tmpImage = m_CurrentImage;  
    
    //rotate the image to get a static video
    m_CurrentImage = this->RotateImage(m_CurrentImage);
    
    // release memory   
    cvReleaseImage(&tmpImage); //
  }

  //transfer the image to a texture
  this->UpdateVideoTexture(); 
  return this->m_CurrentVideoTexture;
}


//image = cvCreateImage( cvSize( m_CurrentImage->width, m_CurrentImage->height )
                      //, m_CurrentImage->depth, m_CurrentImage->nChannels );

const IplImage * mitk::OpenCVVideoSource::GetCurrentFrame()
{
  return m_CurrentImage;
}

void mitk::OpenCVVideoSource::GetCurrentFrameAsOpenCVImage(IplImage * image)
{ // get last captured frame for processing the image data  
  if(m_CurrentImage)
  {
    if(image)
    {
      image->origin = m_CurrentImage->origin;
      memcpy(image->imageData,m_CurrentImage->imageData,m_CurrentImage->width*m_CurrentImage->height*m_CurrentImage->nChannels);  
    }
  }
}

void mitk::OpenCVVideoSource::FetchFrame()
{ // main procedure for updating video data
  if(m_CapturingInProcess)
  {
    if(m_VideoCapture) // we use highgui
    {
      if(!m_CapturePaused)
      {
        // release old image here
        m_CurrentImage = cvQueryFrame(m_VideoCapture);

        ++m_FrameCount;
      }
      else
        m_CurrentImage = m_PauseImage;
      if(m_CurrentImage == NULL) // do we need to repeat the video if it is from video file?
      {        
        double framePos = this->GetVideoCaptureProperty(CV_CAP_PROP_POS_AVI_RATIO);
        MITK_DEBUG << "End of video file found. framePos: " << framePos;
        if(m_RepeatVideo &&  framePos >= 0.99)
        {  
          MITK_DEBUG << "Restarting video file playback.";
          this->SetVideoCaptureProperty(CV_CAP_PROP_POS_AVI_RATIO, 0);
          m_CurrentImage = cvQueryFrame(m_VideoCapture);
        }
      }
      if(m_CaptureWidth == 0 || m_CaptureHeight == 0)
      {
        MITK_DEBUG << "Trying to set m_CaptureWidth & m_CaptureHeight.";
        m_CaptureWidth  = m_CurrentImage->width;
        m_CaptureHeight = m_CurrentImage->height;
        std::cout << "frame width: " << m_CaptureWidth << ", height: " << m_CaptureHeight << std::endl;
        m_CurrentImage->origin = 0;
      }
    }
  }
}

void mitk::OpenCVVideoSource::UpdateVideoTexture()
{  //write the grabbed frame into an opengl compatible array, that means flip it and swap channel order
  if(!m_CurrentImage)
    return;

  if(m_CurrentVideoTexture == NULL)
    m_CurrentVideoTexture = new unsigned char[m_CaptureWidth*m_CaptureHeight*3];

  // only undistort if not paused
  if(m_UndistortImage && !m_CapturePaused)
    m_UndistortCameraImage->UndistortImageFast(m_CurrentImage, 0);

  int width = m_CurrentImage->width;
  int height = m_CurrentImage->height;
  int widthStep = m_CurrentImage->widthStep;
  int nChannels = m_CurrentImage->nChannels;
  unsigned char* tex = m_CurrentVideoTexture;
  char* data = m_CurrentImage->imageData;
  char* currentData = m_CurrentImage->imageData;

  int hIndex=0;
  int wIndex=0;
  int iout,jout;

  for(int i=0;i<width*height*3;i+=3,++wIndex)
  {
    if(wIndex >= width)
    {
      wIndex=0;
      hIndex++;
    }

    // vertically flip the image
    iout = -hIndex+height-1;
    jout = wIndex;

    currentData = data + iout*widthStep;

    tex[i+2] = currentData[jout*nChannels + 0]; // B
    tex[i+1] = currentData[jout*nChannels + 1]; // G
    tex[i] = currentData[jout*nChannels + 2]; // R
  }

}

void mitk::OpenCVVideoSource::StartCapturing()
{
  if(m_VideoCapture != NULL)
    m_CapturingInProcess = true;
  else
    m_CapturingInProcess = false;
}

void mitk::OpenCVVideoSource::StopCapturing()
{
  m_CapturingInProcess = false;
}

void mitk::OpenCVVideoSource::PauseCapturing()
{
  m_CapturePaused = !m_CapturePaused;
  if(m_PauseImage)
  {
    cvReleaseImage( &m_PauseImage ); // release old pause image if necessary
    m_PauseImage = 0;
  }
  
  if(m_CapturePaused)
  {
    m_PauseImage = cvCloneImage(m_CurrentImage);

    // undistort this pause image if necessary
    if(m_UndistortImage)
      m_UndistortCameraImage->UndistortImageFast(m_PauseImage, 0);
  }
}

void mitk::OpenCVVideoSource::EnableOnlineImageUndistortion(mitk::Point3D focal, mitk::Point3D principal, mitk::Point4D distortion)
{
  // Initialize Undistortion
  m_UndistortImage = true;
  float kc[4];
  kc[0] = distortion[0]; kc[1] = distortion[1];
  kc[2] = distortion[2]; kc[3] = distortion[3];
  if(m_CaptureWidth == 0 || m_CaptureHeight == 0)
    FetchFrame();

  m_UndistortCameraImage = mitk::UndistortCameraImage::New();
  m_UndistortCameraImage->SetUndistortImageFastInfo(focal[0], focal[1], principal[0], principal[1],  kc, (float)m_CaptureWidth, (float)m_CaptureHeight);
}

void mitk::OpenCVVideoSource::DisableOnlineImageUndistortion()
{
  m_UndistortImage = false;
}

// functions for compatibility with ITK segmentation only
void mitk::OpenCVVideoSource::GetCurrentFrameAsItkHSVPixelImage(HSVPixelImageType::Pointer &Image)
{
  FetchFrame();
   
  // Prepare iteration
  HSVConstIteratorType itImage( Image, Image->GetLargestPossibleRegion());
  itImage.Begin();
  HSVPixelType pixel;
  int rowsize = 3 * m_CaptureWidth;
   
  char* bufferend;
  char* picture;
  picture = this->m_CurrentImage->imageData;
  bufferend = this->m_CurrentImage->imageData + 3*(m_CaptureHeight*m_CaptureWidth);
  
  float r,g,b,h,s,v;
  try
  {
    // we have to flip the image 
    for(char* datapointer = bufferend - rowsize;datapointer >= picture; datapointer -= rowsize)
    {
      for(char* current = datapointer; current < datapointer + rowsize; current++)
      {
         
          b = *current; current++;
          g = *current; current++;
          r = *current;
          RGBtoHSV(r,g,b,h,s,v);
          pixel[0] = h;
          pixel[1] = s;
          pixel[2] = v;
                
          itImage.Set(pixel);
          ++itImage;                  
      }
    }
  }
  catch( ... )
  {
    std::cout << "Exception raised mitkOpenCVVideoSource: get hsv itk image conversion error." << std::endl;
  }
  
}

void mitk::OpenCVVideoSource::RGBtoHSV(float r, float g, float b, float &h, float &s, float &v) 
{ 
    if(r > 1.0)
      r = r/255;
    if(b > 1.0)
      b = b/255;
    if(g > 1.0)
      g = g/255;
    
    float mn=r,mx=r; 
    int maxVal=0; 
  
    if (g > mx){ mx=g;maxVal=1;} 
    if (b > mx){ mx=b;maxVal=2;}  
    if (g < mn) mn=g; 
    if (b < mn) mn=b;  

    float  delta = mx - mn; 
  
    v = mx;  
    if( mx != 0 ) 
      s = delta / mx;  
    else  
    { 
      s = 0; 
      h = 0; 
      return; 
    } 
    if (s==0.0f) 
    { 
      h=-1; 
      return; 
    } 
    else 
    {  
      switch (maxVal) 
      { 
      case 0:{h = ( g - b ) / delta;break;}         // yel < h < mag 
      case 1:{h = 2 + ( b - r ) / delta;break;}     // cyan < h < yel 
      case 2:{h = 4 + ( r - g ) / delta;break;}     // mag < h < cyan 
      } 
    } 
  
    h *= 60; 
    if( h < 0 ) h += 360; 
}

/*
* Rotate input image according to rotation angle around the viewing direction.
* Angle is supposed to be calculated in QmitkARRotationComponet in the update() method.
*/
IplImage* mitk::OpenCVVideoSource::RotateImage(IplImage* input)
{ 
  if(input == NULL)
  { //warn the user and quit
    std::cout<<"openCVVideoSource: Current video image is null! "<< std::endl;
    return input;
  }
  IplImage* dst = cvCloneImage( input );
  double angle =  this->GetRotationAngle(); //degree
  CvPoint2D32f centre;
  CvMat *translate = cvCreateMat(2, 3, CV_32FC1);
  cvSetZero(translate);
  centre.x =   m_CaptureWidth/2;
  centre.y = m_CaptureHeight/2;
  cv2DRotationMatrix(centre, angle, 1.0, translate);
  cvWarpAffine(input, dst, translate,CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS , cvScalarAll(0));
  cvReleaseMat(&translate);

  return dst; 
}

void mitk::OpenCVVideoSource::Reset()
{
  // set capturing to false
  this->StopCapturing();
  if(m_VideoCapture)
    cvReleaseCapture(&m_VideoCapture);
  m_VideoCapture = 0;
  m_CurrentImage = 0;
  m_CaptureWidth = 0;
  m_CaptureHeight = 0;
  delete m_CurrentVideoTexture;
  m_CurrentVideoTexture = 0;
  if(m_PauseImage)
    cvReleaseImage(&m_PauseImage);
  m_PauseImage = 0;
  m_CapturePaused = false;
  m_VideoFileName.clear();
  m_GrabbingDeviceNumber = -1;
  // do not touch repeat video
  //m_RepeatVideo = false;
  m_UseCVCAMLib = false;
  // do not touch undistort settings
  // bool m_UndistortImage;
}
