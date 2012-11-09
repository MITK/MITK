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
#include "mitkToFCameraMESASR4000Controller.h"
#include "mitkToFConfig.h"
#include <libMesaSR.h>

extern CMesaDevice* m_MESAHandle;

namespace mitk
{
  ToFCameraMESASR4000Controller::ToFCameraMESASR4000Controller()
  {
    this->m_Lambda = new float[MF_LAST];
    this->m_Frequency = new float[MF_LAST];
    this->m_Lambda[0] = 3.75f;
    this->m_Lambda[1] = 5.0f;
    this->m_Lambda[2] = 7.142857f;
    this->m_Lambda[3] = 7.5f;
    this->m_Lambda[4] = 7.894737f;
    this->m_Lambda[5] = 2.5f;
    this->m_Lambda[6] = 10.0f;
    this->m_Lambda[7] = 15.0f;
    this->m_Lambda[8] = 5.172414f;
    this->m_Lambda[9] = 4.838710f;
    this->m_Lambda[10] = 10.344828f;
    this->m_Lambda[11] = 9.677419f;
    this->m_Frequency[0] = 40.0f;
    this->m_Frequency[1] = 30.0f;
    this->m_Frequency[2] = 21.0f;
    this->m_Frequency[3] = 20.0f;
    this->m_Frequency[4] = 19.0f;
    this->m_Frequency[5] = 60.0f;
    this->m_Frequency[6] = 15.0f;
    this->m_Frequency[7] = 10.0f;
    this->m_Frequency[8] = 29.0f;
    this->m_Frequency[9] = 31.0f;
    this->m_Frequency[10] = 14.5f;
    this->m_Frequency[11] = 15.5f;
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

      //float lambda[MF_LAST]={3.75f, 5.f, 7.142857f, 7.5f, 7.894737f, 2.5f, 10.f, 15.f, 5.172414f, 4.838710f, 10.344828f, 9.677419f};//MF_40MHz,MF_30MHz,MF_21MHz,MF_20MHz,MF_19MHz,...
      //float frequency[MF_LAST]={40.00f, 30.00f, 21.00f, 20.00f, 19.00f, 60.00f, 15.00f, 10.00f, 29.00f, 31.00f, 14.50f, 15.50f};
      ModulationFrq frq = SR_GetModulationFrequency(m_MESAHandle);

      this->m_MaxRangeFactor = (this->m_Lambda[frq] * 1000.00) / (float)0xffff;

      unsigned char integrationTime8bit = SR_GetIntegrationTime(m_MESAHandle);
      float integrationTime = (0.3 + ((int)integrationTime8bit) * 0.1) * 1000; // for MESA4000

      char deviceText[1024];
      this->m_MESARes = SR_GetDeviceString(m_MESAHandle, deviceText, _countof(deviceText));//returns the device ID used in other calls
      MITK_INFO << "Device ID: " << deviceText <<std::endl;
      MITK_INFO << "Number of Images: " << this->m_NumImg <<std::endl;
      MITK_INFO << "Resolution: " << this->m_CaptureWidth << " x " << this->m_CaptureHeight <<std::endl;
      MITK_INFO << "Modulationfrequency: " << this->m_Frequency[frq] << " MHz" <<std::endl;
      MITK_INFO << "Max range: " << this->m_Lambda[frq] << " m" <<std::endl;
      MITK_INFO << "Integration time: " << integrationTime << " microsec" <<std::endl;
      return m_ConnectionCheck;
    }
    else return m_ConnectionCheck;
  }

  int ToFCameraMESASR4000Controller::SetIntegrationTime(unsigned int integrationTime)
  {
    float intTime = ((integrationTime / 1000.0) - 0.3) / 0.1;
    this->m_MESARes = SR_SetIntegrationTime(m_MESAHandle, intTime);
    MITK_INFO << "New integration time: " << integrationTime << " microsec" <<std::endl;
    return integrationTime;
  }

  int ToFCameraMESASR4000Controller::GetIntegrationTime()
  {
    unsigned char integrationTime8bit = SR_GetIntegrationTime(m_MESAHandle);
    float integrationTime = (0.3 + ((int)integrationTime8bit) * 0.1) * 1000;
    return (int)integrationTime;
  }

  int ToFCameraMESASR4000Controller::SetModulationFrequency(unsigned int modulationFrequency)
  {
    ModulationFrq frq;
    switch(modulationFrequency)
    {
    case 29: frq = MF_29MHz; break;
    case 30: frq = MF_30MHz; break;
    case 31: frq = MF_31MHz; break;
    default: frq = MF_30MHz;
      MITK_WARN << "Invalid modulation frequency: " << modulationFrequency << " MHz, reset to default (30MHz)" <<std::endl;
    }
    this->m_MESARes = SR_SetModulationFrequency (m_MESAHandle, frq);
    if (this->m_MESARes == 0)
    {
      this->m_MaxRangeFactor = (this->m_Lambda[frq] * 1000.00) / (float)0xffff;
      MITK_INFO << "New modulation frequency: " << this->m_Frequency[frq] << " MHz" <<std::endl;
      return modulationFrequency;
    }
    else
    {
      return this->m_MESARes;
    }
  }

  int ToFCameraMESASR4000Controller::GetModulationFrequency()
  {
    ModulationFrq frq = SR_GetModulationFrequency(m_MESAHandle);
    this->m_MaxRangeFactor = (this->m_Lambda[frq] * 1000.00) / (float)0xffff;
    float frequency = this->m_Frequency[frq];
    return (int)frequency; // TODO float!!
  }

  void ToFCameraMESASR4000Controller::SetFPN( bool fpn )
  {
    int acquireMode;
    acquireMode = SR_GetMode(m_MESAHandle);
    acquireMode &= ~AM_COR_FIX_PTRN;
    if (fpn)
    {
      acquireMode |= AM_COR_FIX_PTRN;
    }
    this->m_MESARes = SR_SetMode(m_MESAHandle, acquireMode);
  }

  void ToFCameraMESASR4000Controller::SetConvGray( bool ConvGray )
  {
    int acquireMode;
    acquireMode = SR_GetMode(m_MESAHandle);
    acquireMode &= ~AM_CONV_GRAY;
    if (ConvGray)
    {
      acquireMode |= AM_CONV_GRAY;
    }
    this->m_MESARes = SR_SetMode(m_MESAHandle, acquireMode);
  }

  void ToFCameraMESASR4000Controller::SetMedian( bool median )
  {
    int acquireMode;
    acquireMode = SR_GetMode(m_MESAHandle);
    acquireMode &= ~AM_MEDIAN;
    if (median)
    {
      acquireMode |= AM_MEDIAN;
    }
    this->m_MESARes = SR_SetMode(m_MESAHandle, acquireMode);
  }

  void ToFCameraMESASR4000Controller::SetANF( bool anf )
  {
    int acquireMode;
    acquireMode = SR_GetMode(m_MESAHandle);
    acquireMode &= ~AM_DENOISE_ANF;
    if (anf)
    {
      acquireMode |= AM_DENOISE_ANF;
    }
    this->m_MESARes = SR_SetMode(m_MESAHandle, acquireMode);
  }

}
