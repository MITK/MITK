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

#include "mitkMovieGeneratorOpenCV.h"
#include <GL/gl.h>
#include <iostream>


mitk::MovieGeneratorOpenCV::MovieGeneratorOpenCV()
{
  m_initialized = false;
  m_aviWriter   = NULL;
  m_dwRate = 20;

  m_FourCCCodec = NULL;
}


void mitk::MovieGeneratorOpenCV::SetFileName( const char *fileName )
{

  m_sFile = fileName;
}

bool mitk::MovieGeneratorOpenCV::InitGenerator()
{
  m_width = m_renderer->GetRenderWindow()->GetSize()[0];  // changed from glGetIntegerv( GL_VIEWPORT, viewport );
  m_height = m_renderer->GetRenderWindow()->GetSize()[1]; // due to sometimes strange dimensions
  
  m_width  -= 10;  //remove colored boarders around renderwindows 
  m_height -= 10;
  
  m_width -= m_width % 4; // some video codecs have prerequisites to the image dimensions
  m_height -= m_height % 4;


  
  m_currentFrame = cvCreateImage(cvSize(m_width,m_height),8,3); // creating image with widget size, 8 bit per pixel and 3 channel r,g,b
  m_currentFrame->origin = 1; // avoid building a video with bottom up
  
  /*
  m_sFile = Name of the output video file. 
  CV_FOURCC = 4-character code of codec used to compress the frames. For example, CV_FOURCC('P','I','M','1') is MPEG-1 codec, 
  CV_FOURCC('M','J','P','G') is motion-jpeg codec etc.
   CV_FOURCC('P','I','M','1')    = MPEG-1 codec
  CV_FOURCC('M','J','P','G')    = motion-jpeg codec (does not work well)
  CV_FOURCC('M', 'P', '4', '2') = MPEG-4.2 codec
  CV_FOURCC('D', 'I', 'V', '3') = MPEG-4.3 codec
  CV_FOURCC('D', 'I', 'V', 'X') = MPEG-4 codec
  CV_FOURCC('U', '2', '6', '3') = H263 codec
  CV_FOURCC('I', '2', '6', '3') = H263I codec
  CV_FOURCC('F', 'L', 'V', '1') = FLV1 codec

  List of FOURCC codes is available at http://msdn2.microsoft.com/en-us/library/ms867195.aspx
  
  Under Win32 it is possible to pass -1 in order to choose compression 
  method and additional compression parameters from dialog. 
  m_dwRate = Framerate of the created video stream. 
  frame_size Size of video frames. 
  1 = If it is not zero, the encoder will expect and encode color frames, otherwise it will work with grayscale frames 
  (the flag is currently supported on Windows only).*/
  
  if(m_FourCCCodec != NULL)
  {
    #ifdef WIN32
      m_aviWriter = cvCreateVideoWriter(m_sFile,CV_FOURCC(m_FourCCCodec[0],m_FourCCCodec[1],m_FourCCCodec[2],
                                        m_FourCCCodec[3]),m_dwRate,cvSize(m_width,m_height),1); //initializing video writer
    #else
      m_aviWriter = cvCreateVideoWriter(m_sFile,CV_FOURCC(m_FourCCCodec[0],m_FourCCCodec[1],m_FourCCCodec[2],
                                        m_FourCCCodec[3]),m_dwRate,cvSize(m_width,m_height)); //initializing video writer
    #endif
  }
  else
  {
    #ifdef WIN32
      m_aviWriter = cvCreateVideoWriter(m_sFile,-1,m_dwRate,cvSize(m_width,m_height),1); //initializing video writer
    #else
      m_aviWriter = cvCreateVideoWriter(m_sFile,CV_FOURCC('X','V','I','D'),m_dwRate,cvSize(m_width,m_height)); //initializing video writer
    #endif
  }
 
  
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
