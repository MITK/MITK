#include "mitkMovieGeneratorOpenCV.h"
#include <GL/gl.h>
#include <iostream>


mitk::MovieGeneratorOpenCV::MovieGeneratorOpenCV()
{
  m_initialized = false;
  m_aviWriter   = NULL;
  m_dwRate = 20;
}


void mitk::MovieGeneratorOpenCV::SetFileName( const char *fileName )
{

  m_sFile = fileName;
}

bool mitk::MovieGeneratorOpenCV::InitGenerator()
{
  if (m_renderer) m_renderer->MakeCurrent();  // take the correct viewport!
  GLint viewport[4]; // Where The Viewport Values Will Be Stored
  glGetIntegerv( GL_VIEWPORT, viewport ); // Retrieves The Viewport Values (X, Y, Width, Height)
  m_width = viewport[2];  m_width -= m_width % 4;
  m_height = viewport[3];  m_height -= m_height % 4;
  
  m_currentFrame = cvCreateImage(cvSize(m_width,m_height),8,3); // creating image with widget size, 8 bit per pixel and 3 channel r,g,b
  m_currentFrame->origin = 1; // avoid building a video with bottom up
  
  /*
  m_sFile = Name of the output video file. 
  -1 = 4-character code of codec used to compress the frames. For example, CV_FOURCC('P','I','M','1') is MPEG-1 codec, 
  CV_FOURCC('M','J','P','G') is motion-jpeg codec etc.
  
  CV_FOURCC('P','I','M','1')    = MPEG-1 codec
  CV_FOURCC('M','J','P','G')    = motion-jpeg codec (does not work well)
  CV_FOURCC('M', 'P', '4', '2') = MPEG-4.2 codec
  CV_FOURCC('D', 'I', 'V', '3') = MPEG-4.3 codec
  CV_FOURCC('D', 'I', 'V', 'X') = MPEG-4 codec
  CV_FOURCC('U', '2', '6', '3') = H263 codec
  CV_FOURCC('I', '2', '6', '3') = H263I codec
  CV_FOURCC('F', 'L', 'V', '1') = FLV1 codec
  
  Under Win32 it is possible to pass -1 in order to choose compression 
  method and additional compression parameters from dialog. 
  m_dwRate = Framerate of the created video stream. 
  frame_size Size of video frames. 
  1 = If it is not zero, the encoder will expect and encode color frames, otherwise it will work with grayscale frames 
  (the flag is currently supported on Windows only).*/
  #ifdef WIN32
    m_aviWriter = cvCreateVideoWriter(m_sFile,-1,m_dwRate,cvSize(m_width,m_height),1); //initializing video writer
  #else
    m_aviWriter = cvCreateVideoWriter(m_sFile,CV_FOURCC('X','V','I','D'),m_dwRate,cvSize(m_width,m_height)); //initializing video writer
  #endif
  
  if(!m_aviWriter)
  {
    std::cout << "errors initializing video writer...correct video file path? on linux: ffmpeg must be included in OpenCV.";
    return false;
  }

  return true;
}


bool mitk::MovieGeneratorOpenCV::AddFrame( void *data )
{
  //cvSetImageData(m_currentFrame,data,m_width*3);
  memcpy(m_currentFrame->imageData,data,m_width*m_height*3);
  cvWriteFrame(m_aviWriter,m_currentFrame);
  return true;
}


bool mitk::MovieGeneratorOpenCV::TerminateGenerator()
{
  if (m_aviWriter) 
  {
    cvReleaseVideoWriter(&m_aviWriter);
  }
  return true;
}
