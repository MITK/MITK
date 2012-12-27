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
#ifndef _mitk_VideoInput_Source_h_
#define _mitk_VideoInput_Source_h_

#include "mitkConfig.h"
#include "mitkOpenCVVideoSource.h"
#include "mitkUndistortCameraImage.h"
#include <mitkOpenCVVideoSupportExports.h>

#include "cv.h"       // open CV

class videoInput;

namespace mitk
{
  class MITK_OPENCVVIDEOSUPPORT_EXPORT VideoInputSource : public OpenCVVideoSource
  {
    public:
    mitkClassMacro( VideoInputSource, OpenCVVideoSource );
    itkNewMacro( Self );

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


