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
#include "mitkToFCameraPMDO3Controller.h"
#include <pmdsdk2.h>
#include "mitkToFConfig.h"
#include "mitkToFPMDConfig.h"

// IP Adress of the Camera, change here if needed
#define CAMERA_ADDR "192.168.0.69"
#define SOURCE_PARAM "192.168.0.69"
#define PROC_PARAM ""

extern PMDHandle m_PMDHandle; //TODO

extern PMDDataDescription m_DataDescription; //TODO

namespace mitk
{
  ToFCameraPMDO3Controller::ToFCameraPMDO3Controller()
  {
    m_IPAddress = CAMERA_ADDR;
    m_SourcePlugin = MITK_TOF_PMDO3_SOURCE_PLUGIN;
    m_SourceParam = SOURCE_PARAM;
    m_ProcPlugin = MITK_TOF_PMDO3_PROCESSING_PLUGIN;
    m_ProcParam = PROC_PARAM;
  }

  ToFCameraPMDO3Controller::~ToFCameraPMDO3Controller()
  {
  }

  bool ToFCameraPMDO3Controller::OpenCameraConnection()
  {
    if(!m_ConnectionCheck)
    {
      m_PMDRes = pmdOpen (&m_PMDHandle , m_SourcePlugin , m_SourceParam , m_ProcPlugin , m_ProcParam );
      m_ConnectionCheck = ErrorText(m_PMDRes);

      // get image properties from camera
      this->UpdateCamera();
      m_PMDRes = pmdGetSourceDataDescription(m_PMDHandle, &m_DataDescription);
      ErrorText(m_PMDRes);
      m_CaptureWidth = m_DataDescription.img.numColumns;
      m_CaptureHeight = m_DataDescription.img.numRows;
      m_PixelNumber = m_CaptureWidth*m_CaptureHeight;
      m_NumberOfBytes = m_PixelNumber * sizeof(float);
      m_SourceDataSize = m_DataDescription.size;
      m_SourceDataStructSize = m_DataDescription.size + sizeof(PMDDataDescription);
      MITK_INFO << "Datasource size: " << m_SourceDataSize <<std::endl;
      MITK_INFO << "Integration Time: " << this->GetIntegrationTime();
      MITK_INFO << "Modulation Frequence: " << this->GetModulationFrequency();

      return m_ConnectionCheck;
    }
    else return m_ConnectionCheck;
  }

}
