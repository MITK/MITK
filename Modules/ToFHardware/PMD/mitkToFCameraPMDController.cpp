/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkToFCameraPMDController.h"
#include <pmdsdk2.h>
#include <string.h>

// vnl includes
#include "vnl/vnl_matrix.h"

PMDHandle m_PMDHandle;
PMDDataDescription m_DataDescription;

struct SourceDataStruct {
   PMDDataDescription dataDescription;
   char sourceData;
};

namespace mitk
{
  ToFCameraPMDController::ToFCameraPMDController(): m_PMDRes(0), m_PixelNumber(40000), m_NumberOfBytes(0),
    m_CaptureWidth(200), m_CaptureHeight(200),m_InternalCaptureWidth(m_CaptureWidth),m_InternalCaptureHeight(m_CaptureHeight), m_SourceDataSize(0), m_SourceDataStructSize(0), m_ConnectionCheck(false),
    m_InputFileName("")
  {
  }

  ToFCameraPMDController::~ToFCameraPMDController()
  {
  }

  bool ToFCameraPMDController::CloseCameraConnection()
  {
    m_PMDRes = pmdClose(m_PMDHandle);
    m_ConnectionCheck = ErrorText(m_PMDRes);
    m_PMDHandle = 0;
    return m_ConnectionCheck;
  }

  bool ToFCameraPMDController::ErrorText(int error)
  {
    if(error != PMD_OK)
    {
      pmdGetLastError (m_PMDHandle, m_PMDError, 128);
      MITK_ERROR << "PMD Error " << m_PMDError;
      mitkThrow() << "PMD Error " << m_PMDError;
      return false;
    }
    else return true;
  }

  bool ToFCameraPMDController::UpdateCamera()
  {
    m_PMDRes = pmdUpdate(m_PMDHandle);
    return ErrorText(m_PMDRes);
  }

  bool ToFCameraPMDController::GetAmplitudes(float* amplitudeArray)
  {
    float* tempArray = new float[m_CaptureWidth*m_CaptureHeight];
    this->m_PMDRes = pmdGetAmplitudes(m_PMDHandle, tempArray, this->m_NumberOfBytes);
    TransformCameraOutput(tempArray, amplitudeArray, false);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDController::GetAmplitudes(char* sourceData, float* amplitudeArray)
  {
    float* tempArray = new float[m_CaptureWidth*m_CaptureHeight];
    this->m_PMDRes = pmdCalcAmplitudes(m_PMDHandle, tempArray, this->m_NumberOfBytes, m_DataDescription, &((SourceDataStruct*)sourceData)->sourceData);
    TransformCameraOutput(tempArray, amplitudeArray, false);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDController::GetIntensities(float* intensityArray)
  {
    float* tempArray = new float[m_CaptureWidth*m_CaptureHeight];
    this->m_PMDRes = pmdGetIntensities(m_PMDHandle, tempArray, this->m_NumberOfBytes);
    TransformCameraOutput(tempArray, intensityArray, false);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDController::GetIntensities(char* sourceData, float* intensityArray)
  {
    float* tempArray = new float[m_CaptureWidth*m_CaptureHeight];
    this->m_PMDRes = pmdCalcIntensities(m_PMDHandle, tempArray, this->m_NumberOfBytes, m_DataDescription, &((SourceDataStruct*)sourceData)->sourceData);
    TransformCameraOutput(tempArray, intensityArray, false);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDController::GetDistances(float* distanceArray)
  {
    float* tempArray = new float[m_CaptureWidth*m_CaptureHeight];
    this->m_PMDRes = pmdGetDistances(m_PMDHandle, tempArray, this->m_NumberOfBytes);
    TransformCameraOutput(tempArray, distanceArray, true);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDController::GetDistances(char* sourceData, float* distanceArray)
  {
    float* tempArray = new float[m_CaptureWidth*m_CaptureHeight];
    this->m_PMDRes = pmdCalcDistances(m_PMDHandle, tempArray, this->m_NumberOfBytes, m_DataDescription, &((SourceDataStruct*)sourceData)->sourceData);
    TransformCameraOutput(tempArray, distanceArray, true);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDController::GetSourceData(char* sourceDataArray)
  {
    this->m_PMDRes = pmdGetSourceDataDescription(m_PMDHandle, &m_DataDescription);
    if (!ErrorText(this->m_PMDRes))
    {
      return false;
    }
    memcpy(&((SourceDataStruct*)sourceDataArray)->dataDescription, &m_DataDescription, sizeof(m_DataDescription));
    this->m_PMDRes = pmdGetSourceData(m_PMDHandle, &((SourceDataStruct*)sourceDataArray)->sourceData, this->m_SourceDataSize);
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDController::GetShortSourceData( short* sourceData)
  {
     this->m_PMDRes = pmdGetSourceDataDescription(m_PMDHandle,&m_DataDescription);
    ErrorText( this->m_PMDRes);
     this->m_PMDRes = pmdGetSourceData(m_PMDHandle,sourceData,m_DataDescription.size);
    return ErrorText( this->m_PMDRes);
  }

  int ToFCameraPMDController::SetIntegrationTime(unsigned int integrationTime)
  {
    if(!m_ConnectionCheck)
    {
      return integrationTime;
    }
    unsigned int result;
    this->m_PMDRes = pmdGetValidIntegrationTime(m_PMDHandle, &result, 0, CloseTo, integrationTime);
    MITK_INFO << "Valid Integration Time = " << result;
    ErrorText(this->m_PMDRes);
    if (this->m_PMDRes != 0)
    {
      return 0;
    }
    this->m_PMDRes = pmdSetIntegrationTime(m_PMDHandle, 0, result);
    ErrorText(this->m_PMDRes);
    return result;
  }

  int ToFCameraPMDController::GetIntegrationTime()
  {
    unsigned int integrationTime = 0;
    this->m_PMDRes = pmdGetIntegrationTime(m_PMDHandle, &integrationTime, 0);
    ErrorText(this->m_PMDRes);
    return integrationTime;
  }

  int ToFCameraPMDController::SetModulationFrequency(unsigned int modulationFrequency)
  {
    if(!m_ConnectionCheck)
    {
      return modulationFrequency;
    }
    unsigned int result;
    this->m_PMDRes = pmdGetValidModulationFrequency(m_PMDHandle, &result, 0, AtLeast, (modulationFrequency*1000000));
    MITK_INFO << "Valid Modulation Frequency = " << result;
    ErrorText(this->m_PMDRes);
    if (this->m_PMDRes != 0)
    {
      return 0;
    }
    this->m_PMDRes = pmdSetModulationFrequency(m_PMDHandle, 0, result);
    ErrorText(this->m_PMDRes);
    return (result/1000000);;
  }

  int ToFCameraPMDController::GetModulationFrequency()
  {
    unsigned int modulationFrequency = 0;
    this->m_PMDRes = pmdGetModulationFrequency (m_PMDHandle, &modulationFrequency, 0);
    ErrorText(this->m_PMDRes);
    return (modulationFrequency/1000000);
  }

  void ToFCameraPMDController::SetInputFileName(std::string inputFileName)
  {
    this->m_InputFileName = inputFileName;
  }
}
