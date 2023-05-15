/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkVideoRecorder.h>

#include <mitkBaseRenderer.h>
#include <mitkCoreServices.h>
#include <mitkExceptionMacro.h>
#include <mitkIOUtil.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkLogMacros.h>

#include <vtkImageResize.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>

#include <itksys/Process.h>

#include <iomanip>
#include <optional>
#include <sstream>

std::string mitk::VideoRecorder::GetFileExtension(OutputFormat format)
{
  switch (format)
  {
  case OutputFormat::WebM_VP9:
    return ".webm";

  case OutputFormat::MP4_H264:
    return ".mp4";

  default:
    break;
  }

  mitkThrow() << "Unknown output format for video recording.";
}

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.moviemaker");
  }

  class RecordingSession
  {
  public:
    RecordingSession(vtkRenderWindow* renderWindow, mitk::VideoRecorder::OutputFormat format)
      : m_FrameDir(mitk::IOUtil::CreateTemporaryDirectory("MITK_RecordingSession_XXXXXX")),
        m_NumberOfFrames(0)
    {
      m_WindowToImageFilter->SetInput(renderWindow);

      if (mitk::VideoRecorder::OutputFormat::MP4_H264 == format)
      {
        // H.264 only supports image dimensions that are a multiple of 2. Resize if necessary.

        auto* size = renderWindow->GetActualSize();

        if (size[0] & 1 || size[1] & 1)
        {
          m_ImageResize->SetInputConnection(m_WindowToImageFilter->GetOutputPort());
          m_ImageResize->SetOutputDimensions(size[0] & ~1, size[1] & ~1, -1);
          m_ImageResize->SetInterpolate(0);
          m_ImageResize->BorderOn();

          m_ImageWriter->SetInputConnection(m_ImageResize->GetOutputPort());
          return;
        }
      }

      m_ImageWriter->SetInputConnection(m_WindowToImageFilter->GetOutputPort());
    }

    ~RecordingSession()
    {
      std::error_code errorCode;
      std::filesystem::remove_all(m_FrameDir, errorCode);
    }

    RecordingSession(const RecordingSession&) = delete;
    RecordingSession& operator=(const RecordingSession&) = delete;

    std::filesystem::path GetFrameDir() const
    {
      return m_FrameDir;
    }

    void RecordFrame()
    {
      m_WindowToImageFilter->Modified();

      std::stringstream frameFilename;
      frameFilename << std::setw(6) << std::setfill('0') << m_NumberOfFrames << ".png";
      const auto framePath = m_FrameDir / frameFilename.str();

      m_ImageWriter->SetFileName(framePath.string().c_str());
      m_ImageWriter->Write();

      ++m_NumberOfFrames;
    }

  private:
    std::filesystem::path m_FrameDir;
    unsigned int m_NumberOfFrames;
    vtkNew<vtkWindowToImageFilter> m_WindowToImageFilter;
    vtkNew<vtkImageResize> m_ImageResize;
    vtkNew<vtkPNGWriter> m_ImageWriter;
  };
}

namespace mitk
{
  class VideoRecorder::Impl
  {
  public:
    Impl()
      : m_FrameRate(30)
    {
    }

    ~Impl() = default;

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    std::filesystem::path GetFFmpegPath() const
    {
      if (m_FFmpegPath)
        return m_FFmpegPath.value();

      auto* preferences = GetPreferences();

      if (nullptr != preferences)
      {
        auto ffmpegPath = preferences->Get("ffmpeg", "");

        if (!ffmpegPath.empty())
          return ffmpegPath;
      }

      return std::filesystem::path();
    }

    void SetFFmpegPath(const std::filesystem::path& path)
    {
      m_FFmpegPath = path;
    }

    std::filesystem::path GetOutputPath() const
    {
      return m_OutputPath;
    }

    void SetOutputPath(const std::filesystem::path& path)
    {
      m_OutputPath = path;
    }

    mitk::VideoRecorder::OutputFormat GetOutputFormat() const
    {
      if (m_OutputFormat)
        return m_OutputFormat.value();

      auto* preferences = GetPreferences();

      if (nullptr != preferences)
        return static_cast<OutputFormat>(preferences->GetInt("format", 0));

      return OutputFormat::WebM_VP9;
    }

    void SetOutputFormat(OutputFormat format)
    {
      m_OutputFormat = format;
    }

    std::string GetRenderWindowName() const
    {
      return m_RenderWindowName;
    }

    void SetRenderWindowName(const std::string& renderWindowName)
    {
      m_RenderWindowName = renderWindowName;
    }

    int GetFrameRate() const
    {
      return m_FrameRate;
    }

    void SetFrameRate(unsigned int fps)
    {
      m_FrameRate = fps;
    }

    bool OnAir() const
    {
      return nullptr != m_RecordingSession.get();
    }

    void StartRecording()
    {
      if (this->OnAir())
        mitkThrow() << "Recording session already running.";

      auto renderWindowName = this->GetRenderWindowName();

      if (renderWindowName.empty())
        mitkThrow() << "No render window specified for recording.";

      auto* renderWindow = BaseRenderer::GetRenderWindowByName(renderWindowName);

      if (nullptr == renderWindow)
        mitkThrow() << "\"" << renderWindowName << "\" references unknown render window for recording.";

      m_RecordingSession = std::make_unique<RecordingSession>(renderWindow, this->GetOutputFormat());
    }

    void RecordFrame()
    {
      if (!this->OnAir())
        mitkThrow() << "Cannot record frame. No recording session running.";

      m_RecordingSession->RecordFrame();
    }

    std::string GetFFmpegCommandLine() const
    {
      bool vp9 = OutputFormat::WebM_VP9 == this->GetOutputFormat();

      std::stringstream stream;
      stream << this->GetFFmpegPath() << ' '
        << "-y" << ' '
        << "-r " << std::to_string(this->GetFrameRate()) << ' '
        << "-i %6d.png" << ' '
        << "-c:v " << (vp9 ? "libvpx-vp9" : "libx264") << ' '
        << "-crf " << (vp9 ? "31" : "23") << ' '
        << "-pix_fmt yuv420p" << ' '
        << "-b:v 0" << ' '
        << this->GetOutputPath();

      return stream.str();
    }

    int ExecuteFFmpeg() const
    {
      auto commandLine = this->GetFFmpegCommandLine();
      auto commandLineCStr = commandLine.c_str();

      auto workingDirectory = m_RecordingSession->GetFrameDir().string();

      auto* ffmpeg = itksysProcess_New();

      itksysProcess_SetOption(ffmpeg, itksysProcess_Option_Verbatim, 1);
      itksysProcess_SetCommand(ffmpeg, &commandLineCStr);
      itksysProcess_SetWorkingDirectory(ffmpeg, workingDirectory.c_str());

      itksysProcess_Execute(ffmpeg);
      itksysProcess_WaitForExit(ffmpeg, nullptr);

      auto state = itksysProcess_GetState(ffmpeg);

      if (itksysProcess_State_Exited != state)
      {
        std::stringstream message;
        message << "FFmpeg process did not exit as expected: ";

        if (itksysProcess_State_Error == state)
        {
          message << itksysProcess_GetErrorString(ffmpeg);
        }
        else if (itksysProcess_State_Exception == state)
        {
          message << itksysProcess_GetExceptionString(ffmpeg);
        }

        message << "\n  Command: " << commandLineCStr;
        message << "\n  Working directory: " << workingDirectory.c_str();

        itksysProcess_Delete(ffmpeg);

        mitkThrow() << message.str();
      }

      auto exitCode = itksysProcess_GetExitValue(ffmpeg);

      itksysProcess_Delete(ffmpeg);

      return exitCode;
    }

    int StopRecording()
    {
      if (!this->OnAir())
        mitkThrow() << "No recording session running.";

      if (this->GetFFmpegPath().empty())
        mitkThrow() << "Path to FFmpeg not set.";

      if (this->GetOutputPath().empty())
        mitkThrow() << "Path to output video file not set.";

      auto exitCode = this->ExecuteFFmpeg();

      m_RecordingSession = nullptr;

      return exitCode;
    }

  private:
    std::optional<std::filesystem::path> m_FFmpegPath;
    std::filesystem::path m_OutputPath;
    std::optional<OutputFormat> m_OutputFormat;
    std::string m_RenderWindowName;
    unsigned int m_FrameRate;
    std::unique_ptr<RecordingSession> m_RecordingSession;
  };
}

mitk::VideoRecorder::VideoRecorder()
  : m_Impl(std::make_unique<Impl>())
{
}

mitk::VideoRecorder::~VideoRecorder()
{
}

std::filesystem::path mitk::VideoRecorder::GetFFmpegPath() const
{
  return m_Impl->GetFFmpegPath();
}

void mitk::VideoRecorder::SetFFmpegPath(const std::filesystem::path& path)
{
  m_Impl->SetFFmpegPath(path);
}

std::filesystem::path mitk::VideoRecorder::GetOutputPath() const
{
  return m_Impl->GetOutputPath();
}

void mitk::VideoRecorder::SetOutputPath(const std::filesystem::path& path)
{
  m_Impl->SetOutputPath(path);
}

mitk::VideoRecorder::OutputFormat mitk::VideoRecorder::GetOutputFormat() const
{
  return m_Impl->GetOutputFormat();
}

void mitk::VideoRecorder::SetOutputFormat(OutputFormat format)
{
  m_Impl->SetOutputFormat(format);
}

std::string mitk::VideoRecorder::GetRenderWindowName() const
{
  return m_Impl->GetRenderWindowName();
}

void mitk::VideoRecorder::SetRenderWindowName(const std::string& renderWindowName)
{
  m_Impl->SetRenderWindowName(renderWindowName);
}

int mitk::VideoRecorder::GetFrameRate() const
{
  return m_Impl->GetFrameRate();
}

void mitk::VideoRecorder::SetFrameRate(unsigned int fps)
{
  m_Impl->SetFrameRate(fps);
}

void mitk::VideoRecorder::StartRecording()
{
  m_Impl->StartRecording();
}

void mitk::VideoRecorder::RecordFrame() const
{
  m_Impl->RecordFrame();
}

int mitk::VideoRecorder::StopRecording()
{
  return m_Impl->StopRecording();
}
