/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkToFCameraPMDO3Controller.h"
#include <pmdsdk2.h>
#include "mitkToFConfig.h"
#include "mitkToFPMDConfig.h"

#include "vnl/vnl_matrix.h"

// IP Adress of the Camera, change here if needed
#define CAMERA_ADDR "192.168.0.69"
#define SOURCE_PARAM "192.168.0.69"
#define PROC_PARAM ""

extern PMDHandle m_PMDHandle;

extern PMDDataDescription m_DataDescription;

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
      m_InternalCaptureWidth = m_CaptureWidth;
      m_InternalCaptureHeight = m_CaptureHeight;
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

  void ToFCameraPMDO3Controller::TransformCameraOutput(float* in, float* out, bool isDist)
  {
    vnl_matrix<float> inMat = vnl_matrix<float>(m_CaptureHeight,m_CaptureWidth);
    inMat.copy_in(in);
    vnl_matrix<float> outMat = vnl_matrix<float>(m_InternalCaptureHeight, m_InternalCaptureWidth);
    vnl_matrix<float> temp = vnl_matrix<float>(m_InternalCaptureHeight, m_InternalCaptureWidth);
    outMat = inMat.extract(m_InternalCaptureHeight, m_InternalCaptureWidth, 0,0);
    outMat.fliplr();
    if(isDist)
    {
      outMat*=1000;
    }
    outMat.copy_out(out);
    inMat.clear();
    outMat.clear();
    temp.clear();
  }
}
