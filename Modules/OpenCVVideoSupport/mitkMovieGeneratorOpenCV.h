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

#ifndef MovieGeneratorOpenCV_H_HEADER_INCLUDED
#define MovieGeneratorOpenCV_H_HEADER_INCLUDED

#include "mitkMovieGenerator.h"
#include <MitkOpenCVVideoSupportExports.h>
#include <memory.h>
#include <string.h>

// OpenCV includes
#include "cv.h"
#include "highgui.h"


namespace mitk {


class MITKOPENCVVIDEOSUPPORT_EXPORT MovieGeneratorOpenCV : public MovieGenerator
{

public:

  mitkClassMacro(MovieGeneratorOpenCV, MovieGenerator);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual void SetFileName( const char *fileName ) override;

  void SetFourCCCodec(char* codec)
  {
    m_FourCCCodec = codec;
  }

  virtual void SetFrameRate(int rate);

  ///
  /// if true the movie will be cutted by a 10 pixel margin
  /// in order to remove the standard mitk coloured borders
  /// default: true
  ///
  void SetRemoveColouredFrame(bool);
protected:

  MovieGeneratorOpenCV();

  //! called directly before the first frame is added
  virtual bool InitGenerator() override;

  //! used to add a frame
  virtual bool AddFrame( void *data ) override;

  //! called after the last frame is added
  virtual bool TerminateGenerator() override;

  //! name of output file
  std::string m_sFile;

  //! frame rate
  int m_dwRate;


private:

  CvVideoWriter*    m_aviWriter;
  IplImage *        m_currentFrame;

  char *            m_FourCCCodec;

  //! frame counter
  long m_lFrame;

  bool m_RemoveColouredFrame;
};

} // namespace mitk

#endif /* MovieGeneratorOpenCV_H_HEADER_INCLUDED */
