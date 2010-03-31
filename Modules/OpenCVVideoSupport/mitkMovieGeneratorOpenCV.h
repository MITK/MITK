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

#ifndef MovieGeneratorOpenCV_H_HEADER_INCLUDED
#define MovieGeneratorOpenCV_H_HEADER_INCLUDED

#include "mitkMovieGenerator.h"
#include <mitkOpenCVVideoSupportExports.h>
#include <memory.h>
#include <string.h>

// OpenCV includes
#include "cv.h"
#include "highgui.h"


namespace mitk {


class MITK_OPENCVVIDEOSUPPORT_EXPORT MovieGeneratorOpenCV : public MovieGenerator
{

public:

  mitkClassMacro(MovieGeneratorOpenCV, MovieGenerator);
  itkNewMacro(Self);

  virtual void SetFileName( const char *fileName );

  void SetFourCCCodec(char* codec)
  {
    m_FourCCCodec = codec;
  }


protected:

  MovieGeneratorOpenCV();

  //! called directly before the first frame is added
  virtual bool InitGenerator();

  //! used to add a frame
  virtual bool AddFrame( void *data );

  //! called after the last frame is added
  virtual bool TerminateGenerator();

  //! name of output file
  const char * m_sFile;

  //! frame rate 
  int m_dwRate;

  
private:
  
  CvVideoWriter*    m_aviWriter;
  IplImage *        m_currentFrame;

  char *            m_FourCCCodec;

  //! frame counter
  long m_lFrame;
};

} // namespace mitk

#endif /* MovieGeneratorOpenCV_H_HEADER_INCLUDED */
