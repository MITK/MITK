/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVideoRecorder_h
#define mitkVideoRecorder_h

#include <mitkFileSystem.h>
#include <memory>
#include <string>

#include <MitkCoreExports.h>

namespace mitk
{
  /** \brief Record the contents of a render window as video using FFmpeg as external command-line application.
   *
   * Before recording, set the render window, the path to FFmpeg, the path to the video output file,
   * its format/codec, and frame rate.
   *
   * Most settings have decent defaults, e.g., the royalty-free and open VP9 video codec in a WebM container as
   * output format and a frame rate of 30 frames per second.
   *
   * If not set explicitly, the FFmpeg path and output format are queried from the preferences, if available.
   *
   * Call StartRecording() to begin a recording session, record each frame with RecordFrame(), and end the recording
   * session with a call to StopRecording(). StopRecording() is a blocking call that may take a long time to return
   * since it calls FFmpeg to encode the recorded frames into a video. Consider calling it from a separate thread.
   *
   * The VideoRecorder throws an Exception on any error. It is advised to use it within a try/catch block.
   */
  class MITKCORE_EXPORT VideoRecorder
  {
  public:
    enum class OutputFormat
    {
      WebM_VP9,
      MP4_H264
    };

    /** \brief Get the file extension corresponding to the specified video output format.
     *
     * \return A file extension string like ".webm" or ".mp4".
     */
    static std::string GetFileExtension(OutputFormat format);

    VideoRecorder();
    ~VideoRecorder();

    VideoRecorder(const VideoRecorder&) = delete;
    VideoRecorder& operator=(const VideoRecorder&) = delete;

    /**
     * \brief Get the path to the FFmpeg executable.
     *
     * If not set explicitly, the path is read from the user preferences.
     *
     * \return The path to FFmpeg, or an empty path if not configured.
     */
    fs::path GetFFmpegPath() const;

    /**
     * \brief Set the path to the FFmpeg executable.
     * \param[in] path The filesystem path to the FFmpeg binary.
     */
    void SetFFmpegPath(const fs::path& path);

    /**
     * \brief Get the path to the output video file.
     * \return The output file path.
     */
    fs::path GetOutputPath() const;

    /**
     * \brief Set the path for the output video file.
     * \param[in] path The filesystem path for the output video.
     */
    void SetOutputPath(const fs::path& path);

    /**
     * \brief Get the output format (codec/container).
     *
     * If not set explicitly, the format is read from the user preferences.
     * Defaults to WebM_VP9.
     *
     * \return The current output format.
     */
    OutputFormat GetOutputFormat() const;

    /**
     * \brief Set the output format (codec/container).
     * \param[in] format The desired output format.
     */
    void SetOutputFormat(OutputFormat format);

    /**
     * \brief Get the name of the render window being recorded.
     * \return The render window name.
     */
    std::string GetRenderWindowName() const;

    /**
     * \brief Set the name of the render window to record.
     * \param[in] renderWindowName The name of the MITK render window.
     */
    void SetRenderWindowName(const std::string& renderWindowName);

    /**
     * \brief Get the recording frame rate.
     * \return The frame rate in frames per second.
     */
    int GetFrameRate() const;

    /**
     * \brief Set the recording frame rate.
     * \param[in] fps The desired frame rate in frames per second.
     */
    void SetFrameRate(unsigned int fps);

    /**
     * \brief Begin a recording session.
     *
     * \pre A render window name must have been set.
     * \throw mitk::Exception if already recording, or if no valid render window is found.
     */
    void StartRecording();

    /**
     * \brief Record the current frame of the render window.
     *
     * Captures the current contents of the render window and stores
     * them as a PNG file in a temporary directory.
     *
     * \pre A recording session must be active (StartRecording() must have been called).
     * \throw mitk::Exception if no recording session is running.
     */
    void RecordFrame() const;

    /**
     * \brief Stop the recording session and encode the video.
     *
     * This is a blocking call that invokes FFmpeg to encode all captured
     * frames into the output video file. It may take a long time to return.
     * Consider calling from a separate thread.
     *
     * \pre A recording session must be active.
     * \pre FFmpeg path and output path must be set.
     * \return The FFmpeg process exit code (0 on success).
     * \throw mitk::Exception if no session is running, or paths are not set.
     */
    int StopRecording();

  private:
    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
