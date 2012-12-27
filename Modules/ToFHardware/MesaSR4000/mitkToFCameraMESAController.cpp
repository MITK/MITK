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
#include "mitkToFCameraMESAController.h"
#include <libMesaSR.h>
#include <string.h>

CMesaDevice* m_MESAHandle=0;

namespace mitk
{
  ToFCameraMESAController::ToFCameraMESAController(): m_MESARes(0), m_PixelNumber(40000), m_NumberOfBytes(0),
    m_CaptureWidth(0), m_CaptureHeight(0), m_NumImg(0), m_MaxRangeFactor(0.0), m_ConnectionCheck(false),
    m_InputFileName("")
  {
  }

  ToFCameraMESAController::~ToFCameraMESAController()
  {
  }

  bool ToFCameraMESAController::CloseCameraConnection()
  {
    m_MESARes = SR_Close(m_MESAHandle); //res=SR_Close(srCam);
    m_ConnectionCheck = ErrorText(m_MESARes);
    m_MESAHandle = 0;
    return m_ConnectionCheck;
  }

  bool ToFCameraMESAController::ErrorText(int error)
  {
    //if(error != MESA_OK)
    //{
    //  //pmdGetLastError (m_MESAHandle, m_MESAError, 128);
    //  //MITK_ERROR << "Camera Error " << m_MESAError;
    //  return false;
    //}
    //else return true;
    return true;
  }

  bool ToFCameraMESAController::UpdateCamera()
  {
    m_MESARes = SR_Acquire(m_MESAHandle); //res=SR_Acquire(srCam);
    //return ErrorText(m_MESARes);
    return true;
  }

  bool ToFCameraMESAController::GetAmplitudes(float* amplitudeArray)
  {
    unsigned short* data;
    data = (unsigned short*)SR_GetImage(m_MESAHandle, 1);
    for (int i=0; i<this->m_PixelNumber; i++)
    {
      if (data[i] & 0x8000) // bit 16 indicates saturation
      {
        amplitudeArray[i] = 0;
      }
      else
      {
        unsigned short value = data[i] & 0x7fff; // bit 16 indicates saturation
        unsigned short value2 = value >> 2; // bits 1 and 2 are reserved/unused
        amplitudeArray[i] = value2;
      }
    }
    return true;
  }

  bool ToFCameraMESAController::GetIntensities(float* intensityArray)
  {
    unsigned short* data;
    data = (unsigned short*)SR_GetImage(m_MESAHandle, 1);
    for (int i=0; i<this->m_PixelNumber; i++)
    {
      intensityArray[i] = data[i];
    }
    return true;
  }

  bool ToFCameraMESAController::GetDistances(float* distanceArray)
  {
    //this->m_MESARes = pmdGetDistances(m_MESAHandle, distanceArray, this->m_NumberOfBytes);
    //return ErrorText(this->m_MESARes);
    unsigned short* data;
    /*
    for(i=0; i<m_NumImg; i++)
    {
      data = SR_GetImage(m_MESAHandle, i);
    }
    */
    data = (unsigned short*)SR_GetImage(m_MESAHandle, 0);
    for (int i=0; i<this->m_PixelNumber; i++)
    {
      if (data[i] < 0xFFF8)
      {
        distanceArray[i] = data[i] * this->m_MaxRangeFactor;
      }
      else
      {
        distanceArray[i] = 0.0;
      }
    }

    return true;
  }

  void ToFCameraMESAController::SetInputFileName(std::string inputFileName)
  {
    this->m_InputFileName = inputFileName;
  }
}
