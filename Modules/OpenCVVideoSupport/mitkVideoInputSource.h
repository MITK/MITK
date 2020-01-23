/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef _mitk_VideoInput_Source_h_
#define _mitk_VideoInput_Source_h_

#include "mitkConfig.h"
#include "mitkOpenCVVideoSource.h"
#include "mitkUndistortCameraImage.h"
#include <MitkOpenCVVideoSupportExports.h>


class videoInput;

namespace mitk
{
  class MITKOPENCVVIDEOSUPPORT_EXPORT VideoInputSource : public OpenCVVideoSource
  {
    public:
    mitkClassMacro( VideoInputSource, OpenCVVideoSource );
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    ////##Documentation
    ////## @brief All functions behave like OpenCVVideoSource to the outside.
    //void GetCurrentFrameAsOpenCVImage(IplImage * image);

    void FetchFrame();
    ////##Documentation
    ////## @brief returns a pointer to the image data array for opengl rendering.
    //unsigned char * GetVideoTexture();

    void StartCapturing();

    void StopCapturing();

    void SetVideoCameraInput(int cameraindex, bool useCVCAMLib);
    void SetVideoFileInput(const char * filename, bool repeatVideo, bool useCVCAMLib);

    itkGetConstMacro(ShowSettingsWindow, bool);
    itkSetMacro(ShowSettingsWindow, bool);

    itkGetMacro(VideoInput, bool);

  protected:
    VideoInputSource();
    virtual ~VideoInputSource();

    videoInput* m_VideoInput;
    int m_DeviceNumber;
    int m_CaptureSize;

    // current Video image
    unsigned char* m_CurrentVideoTexture;

    ///
    /// Show vendor specific settings window?
    ///
    bool m_ShowSettingsWindow;

  };
}
#endif // Header


