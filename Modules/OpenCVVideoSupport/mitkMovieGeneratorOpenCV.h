/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMovieGeneratorOpenCV_h
#define mitkMovieGeneratorOpenCV_h

#include "mitkMovieGenerator.h"
#include <MitkOpenCVVideoSupportExports.h>
#include <memory.h>
#include <cstring>

// OpenCV includes
#include <opencv2/core/types_c.h>
#include <opencv2/videoio.hpp>


namespace mitk {


class MITKOPENCVVIDEOSUPPORT_EXPORT MovieGeneratorOpenCV : public MovieGenerator
{

public:

  mitkClassMacro(MovieGeneratorOpenCV, MovieGenerator);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  void SetFileName( const char *fileName ) override;

  void SetFourCCCodec(char* codec)
  {
    m_FourCCCodec = codec;
  }

  void SetFrameRate(unsigned int rate) override;

  ///
  /// if true the movie will be cutted by a 10 pixel margin
  /// in order to remove the standard mitk coloured borders
  /// default: true
  ///
  void SetRemoveColouredFrame(bool);
protected:

  MovieGeneratorOpenCV();

  //! called directly before the first frame is added
  bool InitGenerator() override;

  //! used to add a frame
  bool AddFrame( void *data ) override;

  //! called after the last frame is added
  bool TerminateGenerator() override;

  //! name of output file
  std::string m_sFile;

  //! frame rate
  int m_dwRate;


private:

  CvVideoWriter*    m_aviWriter;
  IplImage *        m_currentFrame;

  char *            m_FourCCCodec;

  bool m_RemoveColouredFrame;
};

} // namespace mitk

#endif
