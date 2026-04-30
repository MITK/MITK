/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMovieGenerator_h
#define mitkMovieGenerator_h

#include <MitkAlgorithmsExtExports.h>
#include <mitkBaseRenderer.h>
#include <mitkCommon.h>
#include <mitkStepper.h>

namespace mitk
{
  /**
   * \brief Abstract base class for recording movies from OpenGL render windows.
   *
   * MovieGenerator provides the infrastructure for capturing frames from a
   * BaseRenderer and assembling them into a movie file. It supports two modes:
   * - Stepper-driven: Iterates through all steps of a Stepper and captures each frame.
   * - Frame-by-frame: Each call to WriteCurrentFrameToMovie() captures a single frame.
   *
   * Platform-specific subclasses (e.g., MovieGeneratorWin32) implement the
   * actual video encoding via platform APIs.
   *
   * \sa MovieGeneratorWin32
   * \sa Stepper
   * \sa BaseRenderer
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT MovieGenerator : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(MovieGenerator, itk::LightObject);

    /**
     * \brief Factory method returning a platform-specific MovieGenerator.
     * \return Smart pointer to a MovieGenerator instance (Win32 or Linux).
     */
    static Pointer New(void);

    /**
     * \brief Set the stepper used to control movie generation.
     * \param[in] stepper The Stepper that drives frame iteration.
     */
    virtual void SetStepper(Stepper *stepper) { m_stepper = stepper; }

    /**
     * \brief Set the renderer whose render window will be captured.
     * \param[in] renderer The BaseRenderer to record from.
     */
    virtual void SetRenderer(BaseRenderer *renderer) { m_renderer = renderer; }

    /**
     * \brief Set the output filename for the movie.
     * \param[in] fileName Path to the output movie file.
     */
    virtual void SetFileName(const char *fileName) { strcpy(m_fileName, fileName); }

    /**
     * \brief Generate a movie by iterating through all stepper steps.
     *
     * Uses the configured Stepper and Renderer to capture frames from the
     * active OpenGL context and assemble them into a movie file.
     *
     * \pre Stepper and Renderer must be set.
     * \return True if the movie was successfully written.
     */
    virtual bool WriteMovie();

    /**
     * \brief Capture a single frame and add it to the movie.
     *
     * This method does not use a Stepper. Initialization occurs on the
     * first call. Renderer and FileName must be set before calling this method.
     *
     * \return True if the frame was successfully added.
     */
    virtual bool WriteCurrentFrameToMovie();

    /**
     * \brief Release the movie writer resources after using WriteCurrentFrameToMovie().
     */
    virtual void ReleaseMovieWriter();

    /**
     * \brief Set the frame rate of the output movie.
     * \param[in] rate Frames per second (default: 20).
     */
    virtual void SetFrameRate(unsigned int rate);

    /**
     * \brief Get the frame rate of the output movie.
     * \return The frame rate in frames per second.
     */
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
