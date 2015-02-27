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
#ifndef mitkSpectroCamController_h
#define mitkSpectroCamController_h

#include <ISpectroCam.h>
#include <Jai_Factory.h>
#include <string>

namespace mitk
{
  /**
  * @brief Controller for Pixelteq SpectroCam
  *
  *
  * @ingroup BiophotonicsHardware
  */

  class SpectroCamController
  {
  public:
    SpectroCamController();
    ~SpectroCamController();

    bool Ini();
    bool OpenCameraConnection();
    bool CloseCameraConnection();
    ISpectroCam* GetSpectroCam() const;
    J_tIMAGE_INFO GetImageInfo();

    std::string mode;
    std::string model;

  private:
    ISpectroCam* spectroCam;                        //SpectroCam var
    J_tIMAGE_INFO m_CnvImageInfo;                    //Image Info

    STREAM_HANDLE   m_hDS;                            // Handle to the data stream
    HANDLE          m_hStreamThread;                  // Handle to the image acquisition thread
    HANDLE          m_hStreamEvent;                    // Thread used to signal when image thread stops
    VIEW_HANDLE     m_hView;                          // View window handles

    uint32_t        m_iValidBuffers;                // Number of buffers allocated to image acquisition
    uint8_t*        m_pAquBuffer;                    // Buffers allocated to hold image data
    BUF_HANDLE      m_pAquBufferID;                  // Handles for all the image buffers
    HANDLE          m_hEventKill;                    // Event used for speeding up the termination of image capture
    bool            m_bEnableThread;                // Flag indicating if the image thread should run


    //Vars for Ini from file
    FastModeSettings fastSettings;
    SequenceModeSettings seqSettings;
    IndexModeSettings indexSettings;
  };

}
#endif
