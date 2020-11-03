/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkSpectroCamController_h
#define mitkSpectroCamController_h

#include <string>


#include <MitkSpectroCamExports.h>
#include <mitkImage.h>

namespace mitk
{
  // forward declaration of the implementation for the SpectroCam
  class SpectroCamController_pimpl;

  /**
  * @brief Controller for Pixelteq SpectroCam
  *
  *
  * @ingroup BiophotonicsHardware
  */
  class MITKSPECTROCAM_EXPORT SpectroCamController
  {
  public:
    SpectroCamController();
    ~SpectroCamController();

    bool Ini();
    int OpenCameraConnection();
    int CloseCameraConnection();
    bool isCameraRunning();

    /**
    Returns the current image stack. Is of VectorType, unsigned short
    */
    mitk::Image::Pointer GetCurrentImage();

    void SetCurrentImageAsWhiteBalance();

  private:
    SpectroCamController_pimpl* m_SpectroCamController_pimpl;
  };

}
#endif
