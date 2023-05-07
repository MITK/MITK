/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVideoRecorder_h
#define mitkVideoRecorder_h

#include <filesystem>
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

    std::filesystem::path GetFFmpegPath() const;
    void SetFFmpegPath(const std::filesystem::path& path);

    std::filesystem::path GetOutputPath() const;
    void SetOutputPath(const std::filesystem::path& path);

    OutputFormat GetOutputFormat() const;
    void SetOutputFormat(OutputFormat format);

    std::string GetRenderWindowName() const;
    void SetRenderWindowName(const std::string& renderWindowName);

    int GetFrameRate() const;
    void SetFrameRate(unsigned int fps);

    void StartRecording();
    void RecordFrame() const;
    int StopRecording();

  private:
    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
