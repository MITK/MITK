/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mitkToFCameraMESASR4000Controller.h"
#include "mitkToFConfig.h"
#include <libMesaSR.h>

extern CMesaDevice* m_MESAHandle;

namespace mitk
{
  ToFCameraMESASR4000Controller::ToFCameraMESASR4000Controller()
  {
  }

  ToFCameraMESASR4000Controller::~ToFCameraMESASR4000Controller()
  {
  }

  bool ToFCameraMESASR4000Controller::OpenCameraConnection()
  {
    if(!m_ConnectionCheck)
    {
	/*
      m_PMDRes = pmdOpen (&m_PMDHandle , m_SourcePlugin , m_SourceParam , m_ProcPlugin , m_ProcParam );
      m_ConnectionCheck = ErrorText(m_PMDRes);
      if (!m_ConnectionCheck)
      {
        return m_ConnectionCheck;
      }

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

      char serial[16];
      m_PMDRes = pmdSourceCommand (m_PMDHandle, serial, 16, "GetSerialNumber");
      ErrorText(m_PMDRes);

      MITK_INFO << "Serial-No: " << serial <<std::endl;
      MITK_INFO << "Datasource size: " << this->m_SourceDataSize <<std::endl;
      MITK_INFO << "Integration Time: " << this->GetIntegrationTime();
      MITK_INFO << "Modulation Frequence: " << this->GetModulationFrequency();
      return m_ConnectionCheck;
    */
      //m_MESARes = SR_OpenDlg(&m_MESAHandle, 3, 0);
      this->m_MESARes = SR_OpenUSB(&m_MESAHandle, 0);
      this->m_ConnectionCheck = ErrorText(this->m_MESARes);
      if (m_MESARes <= 0)
      {
        this->m_ConnectionCheck = false;
        return this->m_ConnectionCheck;
      }

      this->m_MESARes = SR_GetRows(m_MESAHandle);
      this->m_CaptureHeight = this->m_MESARes;
      this->m_MESARes = SR_GetCols(m_MESAHandle); 
      this->m_CaptureWidth = this->m_MESARes;
      this->m_PixelNumber = this->m_CaptureWidth*this->m_CaptureHeight;
      this->m_NumberOfBytes = this->m_PixelNumber * sizeof(float);

      ImgEntry* imgEntryArray;
      this->m_NumImg = SR_GetImageList(m_MESAHandle, &imgEntryArray);

      float lambda[MF_LAST]={3.75f, 5.f, 7.142857f, 7.5f, 7.894737f, 2.5f, 10.f, 15.f, 5.172414f, 4.838710f, 10.344828f, 9.677419f};//MF_40MHz,MF_30MHz,MF_21MHz,MF_20MHz,MF_19MHz,...
      float frequency[MF_LAST]={40.00f, 30.00f, 21.00f, 20.00f, 19.00f, 60.00f, 15.00f, 10.00f, 29.00f, 31.00f, 14.50f, 15.50f};
      ModulationFrq frq = SR_GetModulationFrequency(m_MESAHandle);

      this->m_MaxRangeFactor = (lambda[frq] * 1000.00) / (float)0xffff;

      char deviceText[1024];
      this->m_MESARes = SR_GetDeviceString(m_MESAHandle, deviceText, _countof(deviceText));//returns the device ID used in other calls
      MITK_INFO << "Device ID: " << deviceText <<std::endl;
      MITK_INFO << "Number of Images: " << this->m_NumImg <<std::endl;
      MITK_INFO << "Resolution: " << this->m_CaptureWidth << " x " << this->m_CaptureHeight <<std::endl;
      MITK_INFO << "Modulationfrequency: " << frequency[frq] << " MHz" <<std::endl;
      MITK_INFO << "Max range: " << lambda[frq] << " m" <<std::endl;
      return m_ConnectionCheck;
    }
    else return m_ConnectionCheck;
  }
}
