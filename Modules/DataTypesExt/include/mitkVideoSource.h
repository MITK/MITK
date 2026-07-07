/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVideoSource_h
#define mitkVideoSource_h

#include <MitkDataTypesExtExports.h>
#include <mitkCommon.h>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkMessage.h>

namespace mitk
{
  /**
   * \brief Abstract base class for acquiring video frame data.
   *
   * Provides an interface for starting, stopping, and pausing video capture,
   * as well as fetching frames and retrieving video texture data. Subclasses
   * must implement GetVideoTexture() and typically override FetchFrame().
   *
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT VideoSource : virtual public itk::Object
  {
  public:
    mitkClassMacroItkParent(VideoSource, itk::Object);

    /**
     * \brief Fetch the next frame from the capture device.
     *
     * In this base implementation, only the frame counter is incremented.
     * Subclasses should override to perform actual frame acquisition.
     */
    virtual void FetchFrame();

    /**
     * \brief Get a pointer to the current video frame pixel data.
     *
     * The returned buffer is suitable for OpenGL texture upload.
     *
     * \return Pointer to the RGBA/RGB pixel data array.
     */
    virtual unsigned char *GetVideoTexture() = 0;

    /**
     * \brief Start video capturing.
     *
     * Sets m_CapturingInProcess to true and resets the frame count.
     * Subclasses overriding this method should call the base implementation.
     */
    virtual void StartCapturing();

    /**
     * \brief Stop video capturing.
     *
     * Sets m_CapturingInProcess to false and resets the frame count.
     * Subclasses overriding this method should call the base implementation.
     */
    virtual void StopCapturing();

    /**
     * \brief Query whether video capturing is currently active.
     * \return true if capturing is in progress.
     */
    virtual bool IsCapturingEnabled() const;

    /**
     * \brief Get the width of the current video frame.
     * \return The frame width in pixels, or 0 if unknown.
     */
    virtual int GetImageWidth();

    /**
     * \brief Get the height of the current video frame.
     * \return The frame height in pixels, or 0 if unknown.
     */
    virtual int GetImageHeight();

    /**
     * \brief Get the number of frames captured since the last start.
     * \return The frame count.
     */
    virtual unsigned long GetFrameCount() const;

    /**
     * \brief Query whether capturing is currently paused.
     * \return true if paused.
     */
    virtual bool GetCapturePaused() const;

    /**
     * \brief Toggle the capture pause state.
     *
     * While paused, FetchFrame() should provide the same frame repeatedly.
     * Subclasses overriding this method must toggle m_CapturePaused.
     */
    virtual void PauseCapturing();

  protected:
    ///
    /// init member
    ///
    VideoSource();
    ///
    /// deletes m_CurrentVideoTexture (if not 0)
    ///
    ~VideoSource() override;

  protected:
    ///
    /// finally this is what the video source must create: a video texture pointer
    /// default: 0
    ///
    unsigned char *m_CurrentVideoTexture;

    ///
    /// should be filled when the first frame is available
    /// default: 0
    ///
    int m_CaptureWidth;

    ///
    /// should be filled when the first frame is available
    /// default: 0
    ///
    int m_CaptureHeight;

    ///
    /// saves if capturing is in procress
    /// default: false
    ///
    bool m_CapturingInProcess;

    ///
    /// Saves the current frame count. Incremented in FetchFrame().
    /// Reset to 0 when StartCapturing() or StopCapturing() is called.
    /// default: 0
    ///
    unsigned long m_FrameCount;

    ///
    /// Saves if the capturing is currently paused, i.e. this
    /// will not fetch any further frames but provide the current
    /// frame as long as m_CapturePaused is true
    /// default: false
    ///
    bool m_CapturePaused;
  };
}
#endif
