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
#include "mitkToFCameraPMDPlayerController.h"
#include <mitkToFConfig.h>
#include <mitkToFPMDConfig.h>
#include <pmdsdk2.h>

//Plugin defines for Camcube 2.0
#define PROC_PARAM ""

extern PMDHandle m_PMDHandle; //TODO

extern PMDDataDescription m_DataDescription; //TODO

namespace mitk
{
  ToFCameraPMDPlayerController::ToFCameraPMDPlayerController()
  {
    m_SourcePlugin = MITK_TOF_PMDFILE_SOURCE_PLUGIN;
    m_ProcPlugin = MITK_TOF_PMDCAMCUBE_PROCESSING_PLUGIN;
    m_ProcParam = PROC_PARAM;

    this->m_NumOfFrames = 0;
  }

  ToFCameraPMDPlayerController::~ToFCameraPMDPlayerController()
  {
  }

  bool ToFCameraPMDPlayerController::OpenCameraConnection()
  {
    bool ok = false;

    if( m_PMDFileName == "" )
    {
      return ok;
    }

    this->m_PMDRes = pmdOpen(&m_PMDHandle , this->m_SourcePlugin , this->m_PMDFileName.c_str(), this->m_ProcPlugin, this->m_ProcParam );
    ok = ErrorText(this->m_PMDRes);

    if (ok)
    {
      this->m_PMDRes = pmdUpdate(m_PMDHandle);
      ok = ErrorText(this->m_PMDRes);

      this->m_PMDRes = pmdGetSourceDataDescription(m_PMDHandle, &m_DataDescription);
      ok = ErrorText(m_PMDRes);

      if (ok)
      {
        this->m_CaptureWidth = m_DataDescription.img.numColumns;
        this->m_CaptureHeight = m_DataDescription.img.numRows;
        this->m_PixelNumber = this->m_CaptureWidth * this->m_CaptureHeight;
        this->m_NumberOfBytes = this->m_PixelNumber * sizeof(float);
        this->m_SourceDataSize = m_DataDescription.size;
        this->m_SourceDataStructSize = m_DataDescription.size + sizeof(PMDDataDescription);

        MITK_INFO << "Open PMD file: " << m_InputFileName.c_str();
        MITK_INFO << "Datasource size: " << this->m_SourceDataSize <<std::endl;
        MITK_INFO << "Integration Time: " << this->GetIntegrationTime();
        MITK_INFO << "Modulation Frequence: " << this->GetModulationFrequency();
        MITK_INFO << "NumOfFrames: " << this->m_NumOfFrames;
      }
    }

    return ok;
  }

  int ToFCameraPMDPlayerController::SetIntegrationTime(unsigned int integrationTime)
  {
    return m_DataDescription.img.integrationTime[0];;
  }

  int ToFCameraPMDPlayerController::SetModulationFrequency(unsigned int modulationFrequency)
  {
    return m_DataDescription.img.modulationFrequency[0];;
  }

}
