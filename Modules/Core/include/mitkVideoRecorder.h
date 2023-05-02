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
  class MITKCORE_EXPORT VideoRecorder
  {
  public:
    enum class OutputFormat
    {
      WebM_VP9,
      MP4_H264
    };

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
