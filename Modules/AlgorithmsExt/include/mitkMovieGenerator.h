/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMovieGenerator_h
#define mitkMovieGenerator_h

#include "MitkAlgorithmsExtExports.h"
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkStepper.h"

namespace mitk
{
  class MITKALGORITHMSEXT_EXPORT MovieGenerator : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(MovieGenerator, itk::LightObject);

    // delivers Win32 or Linux-versions of MovieGenerator
    static Pointer New(void);

    //!  stepper  used to  control  movie  generation
    virtual void SetStepper(Stepper *stepper) { m_stepper = stepper; }
    //!  renderer to record
    virtual void SetRenderer(BaseRenderer *renderer) { m_renderer = renderer; }
    //!  filename under which movie is  saved
    virtual void SetFileName(const char *fileName) { strcpy(m_fileName, fileName); }
    //!  uses given stepper and filename  to create a movie from the active OpenGL context
    virtual bool WriteMovie();

    //!  alternative way, which does not use a stepper;
    //   it adds a single frame to a movie each time the function is called
    //   Initialization is done with first function call; Renderer and Filename have to be set up properly before.
    virtual bool WriteCurrentFrameToMovie();
    //!  releases a movie writer after usage of WriteCurrentFrameToMovie()
    virtual void ReleaseMovieWriter();

    virtual void SetFrameRate(unsigned int rate);

    unsigned int GetFrameRate();

  protected:
    //!  default  constructor
    MovieGenerator();

    //!  called directly  before the first frame is  added, determines  movie  size from  renderer
    virtual bool InitGenerator() = 0;

    //!  used to  add  a  frame
    virtual bool AddFrame(void *data) = 0;

    //!  called after the last  frame  is added
    virtual bool TerminateGenerator() = 0;

    //!  stores the movie filename
    char m_fileName[1024];

    //!  stores the used  stepper
    Stepper *m_stepper;

    //! stores the uses renderer
    BaseRenderer *m_renderer;

    //!  InitGenerator()  stores movie size  in those variables
    int m_width, m_height;

    bool m_initialized;

    unsigned int m_FrameRate;
  };

} // namespace mitk

#endif
