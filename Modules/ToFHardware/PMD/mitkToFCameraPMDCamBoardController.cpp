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
#include "mitkToFCameraPMDCamBoardController.h"
#include "mitkToFConfig.h"
#include <pmdsdk2.h>
#include "mitkToFPMDConfig.h"

// vnl includes
#include "vnl/vnl_matrix.h"

//Plugin defines for CamBoard
#define SOURCE_PARAM ""
#define PROC_PARAM ""

extern PMDHandle m_PMDHandle; //TODO
extern PMDDataDescription m_DataDescription; //TODO

struct SourceDataStruct {
  PMDDataDescription dataDescription;
  char sourceData;
};

namespace mitk
{
  ToFCameraPMDCamBoardController::ToFCameraPMDCamBoardController(): m_InternalCaptureWidth(0),
    m_InternalCaptureHeight(0), m_InternalPixelNumber(0)
  {
    m_SourcePlugin = MITK_TOF_PMDCAMBOARD_SOURCE_PLUGIN;
    m_SourceParam = SOURCE_PARAM;
    m_ProcPlugin = MITK_TOF_PMDCAMBOARD_PROCESSING_PLUGIN;
    m_ProcParam = PROC_PARAM;
  }

  ToFCameraPMDCamBoardController::~ToFCameraPMDCamBoardController()
  {
  }

  bool ToFCameraPMDCamBoardController::OpenCameraConnection()
  {
    if(!m_ConnectionCheck)
    {
      m_PMDRes = pmdOpen (&m_PMDHandle , m_SourcePlugin , m_SourceParam , m_ProcPlugin , m_ProcParam );
      m_ConnectionCheck = ErrorText(m_PMDRes);
      if (!m_ConnectionCheck)
      {
        return m_ConnectionCheck;
      }
      // get image properties from camera
      this->UpdateCamera();
      this->m_PMDRes = pmdGetSourceDataDescription(m_PMDHandle, &m_DataDescription);
      ErrorText(m_PMDRes);
      this->m_InternalCaptureWidth = m_DataDescription.img.numColumns;
      this->m_CaptureWidth = 200;
      this->m_InternalCaptureHeight = m_DataDescription.img.numRows;
      this->m_CaptureHeight = 200;
      this->m_InternalPixelNumber = m_InternalCaptureWidth*m_InternalCaptureHeight;
      this->m_PixelNumber = m_CaptureWidth*m_CaptureHeight;

      this->m_NumberOfBytes = m_InternalPixelNumber * sizeof(float);
      this->m_SourceDataSize = m_DataDescription.size;
      this->m_SourceDataStructSize = m_DataDescription.size + sizeof(PMDDataDescription);
      MITK_INFO << "Datasource size: " << this->m_SourceDataSize <<std::endl;
      MITK_INFO << "Integration Time: " << this->GetIntegrationTime();
      MITK_INFO << "Modulation Frequency: " << this->GetModulationFrequency();
      return m_ConnectionCheck;
    }
    else return m_ConnectionCheck;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetDistanceOffset( float offset )
  {
    std::stringstream command;
    command<<"SetSoftOffset "<<offset;
    this->m_PMDRes = pmdSourceCommand(m_PMDHandle,0,0,command.str().c_str());
    return ErrorText(this->m_PMDRes);
  }

  float mitk::ToFCameraPMDCamBoardController::GetDistanceOffset()
  {
    char offset[16];
    this->m_PMDRes = pmdSourceCommand(m_PMDHandle, offset, 16, "GetSoftOffset");
    ErrorText(this->m_PMDRes);
    return atof(offset);
  }

  bool mitk::ToFCameraPMDCamBoardController::SetRegionOfInterest( unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height )
  {
    // CAVE: This function does not work properly, don't use unless you know what you're doing!!
    // check if leftUpperCornerX and width are divisible by 3 otherwise round to the next value divisible by 3
    unsigned int factor = leftUpperCornerX/3;
    leftUpperCornerX = 3*factor;
    factor = width/3;
    width = 3*factor;
    std::stringstream command;
    command<<"SetROI "<<leftUpperCornerX<<" "<<leftUpperCornerY<<" "<<width<<" "<<height;
    this->m_PMDRes = pmdSourceCommand(m_PMDHandle,0,0,command.str().c_str());
    return ErrorText(this->m_PMDRes);
  }

  bool mitk::ToFCameraPMDCamBoardController::SetRegionOfInterest( unsigned int roi[4] )
  {
    return this->SetRegionOfInterest(roi[0],roi[1],roi[2],roi[3]);
  }

  unsigned int* mitk::ToFCameraPMDCamBoardController::GetRegionOfInterest()
  {
    /*
    char result[64];
    this->m_PMDRes = pmdSourceCommand(m_PMDHandle, result, 64, "GetROI");
    ErrorText(this->m_PMDRes);
    // convert char array to uint array
    unsigned int roi[4];
    std::stringstream currentValue;
    int counter = 0;
    std::string resultString = result;
    char blank = ' ';
    for (int i=0; i<64; i++)
    {
      if (result[i]!=blank)
      {
        currentValue<<result[i];
      }
      else
      {
        if (counter<4)
        {
          roi[counter] = atoi(currentValue.str().c_str());
          counter++;
          // empty the stream
          currentValue.str("");
          currentValue.clear();
        }
      }
    }
    return roi;
    */
    return NULL;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetExposureMode( int mode )
  {
    if (mode==0) // normal mode
    {
      this->m_PMDRes = pmdSourceCommand(m_PMDHandle, 0, 0, "SetExposureMode Normal");
      return ErrorText(this->m_PMDRes);
    }
    else if (mode==1) // SMB mode
    {
      this->m_PMDRes = pmdSourceCommand(m_PMDHandle, 0, 0, "SetExposureMode SMB");
      return ErrorText(this->m_PMDRes);
    }
    else
    {
      MITK_ERROR<<"Specified exposure mode not supported. Exposure mode must be 0 (Normal) or 1 (SMB)";
      return false;
    }
  }

  bool mitk::ToFCameraPMDCamBoardController::SetFieldOfView( float fov )
  {
    std::stringstream commandStream;
    commandStream<<"SetFOV "<<fov;
    this->m_PMDRes = pmdProcessingCommand(m_PMDHandle, 0, 0, commandStream.str().c_str());
    return ErrorText(this->m_PMDRes);
  }

  bool mitk::ToFCameraPMDCamBoardController::SetFPNCalibration( bool on )
  {
    if(on)
    {
      this->m_PMDRes=pmdSourceCommand(m_PMDHandle,0,0,"SetFPNCalibration On");
      return this->ErrorText(this->m_PMDRes);
    }
    else
    {
      this->m_PMDRes=pmdSourceCommand(m_PMDHandle,0,0,"SetFPNCalibration Off");
      return this->ErrorText(this->m_PMDRes);
    }
  }

  bool mitk::ToFCameraPMDCamBoardController::SetFPPNCalibration( bool on )
  {
    if(on)
    {
      this->m_PMDRes=pmdProcessingCommand(m_PMDHandle,0,0,"SetFPPNCalibration On");
      return this->ErrorText(this->m_PMDRes);
    }
    else
    {
      this->m_PMDRes=pmdProcessingCommand(m_PMDHandle,0,0,"SetFPPNCalibration Off");
      return this->ErrorText(this->m_PMDRes);
    }
  }

  bool mitk::ToFCameraPMDCamBoardController::SetLinearityCalibration( bool on )
  {
    if(on)
    {
      this->m_PMDRes=pmdProcessingCommand(m_PMDHandle,0,0,"SetLinearityCalibration On");
      return this->ErrorText(this->m_PMDRes);
    }
    else
    {
      this->m_PMDRes=pmdProcessingCommand(m_PMDHandle,0,0,"SetLinearityCalibration Off");
      return this->ErrorText(this->m_PMDRes);
    }
  }

  bool mitk::ToFCameraPMDCamBoardController::SetLensCalibration( bool on )
  {
    if (on)
    {
      this->m_PMDRes = pmdProcessingCommand(m_PMDHandle, 0, 0, "SetLensCalibration On");
      return ErrorText(this->m_PMDRes);
    }
    else
    {
      this->m_PMDRes = pmdProcessingCommand(m_PMDHandle, 0, 0, "SetLensCalibration Off");
      return ErrorText(this->m_PMDRes);
    }
  }

  bool ToFCameraPMDCamBoardController::GetAmplitudes(float* amplitudeArray)
  {
    float* tempArray = new float[m_InternalCaptureWidth*m_InternalCaptureHeight];
    this->m_PMDRes = pmdGetAmplitudes(m_PMDHandle, tempArray, this->m_NumberOfBytes);
    this->TransformCameraOutput(tempArray, amplitudeArray, false);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDCamBoardController::GetAmplitudes(char* sourceData, float* amplitudeArray)
  {
    float* tempArray = new float[m_InternalCaptureWidth*m_InternalCaptureHeight];
    this->m_PMDRes = pmdCalcAmplitudes(m_PMDHandle, tempArray, this->m_NumberOfBytes, m_DataDescription, &((SourceDataStruct*)sourceData)->sourceData);
    this->TransformCameraOutput(tempArray, amplitudeArray, false);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDCamBoardController::GetIntensities(float* intensityArray)
  {
    float* tempArray = new float[m_InternalCaptureWidth*m_InternalCaptureHeight];
    this->m_PMDRes = pmdGetIntensities(m_PMDHandle, tempArray, this->m_NumberOfBytes);
    this->TransformCameraOutput(tempArray, intensityArray, false);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDCamBoardController::GetIntensities(char* sourceData, float* intensityArray)
  {
    float* tempArray = new float[m_InternalCaptureWidth*m_InternalCaptureHeight];
    this->m_PMDRes = pmdCalcIntensities(m_PMDHandle, tempArray, this->m_NumberOfBytes, m_DataDescription, &((SourceDataStruct*)sourceData)->sourceData);
    this->TransformCameraOutput(tempArray, intensityArray, false);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDCamBoardController::GetDistances(float* distanceArray)
  {
    float* tempArray = new float[m_InternalCaptureWidth*m_InternalCaptureHeight];
    this->m_PMDRes = pmdGetDistances(m_PMDHandle, tempArray, this->m_NumberOfBytes);
    this->TransformCameraOutput(tempArray, distanceArray, true);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  bool ToFCameraPMDCamBoardController::GetDistances(char* sourceData, float* distanceArray)
  {
    float* tempArray = new float[m_InternalCaptureWidth*m_InternalCaptureHeight];
    this->m_PMDRes = pmdCalcDistances(m_PMDHandle, tempArray, this->m_NumberOfBytes, m_DataDescription, &((SourceDataStruct*)sourceData)->sourceData);
    this->TransformCameraOutput(tempArray, distanceArray, true);
    delete[] tempArray;
    return ErrorText(this->m_PMDRes);
  }

  void ToFCameraPMDCamBoardController::TransformCameraOutput( float* in, float* out, bool isDist )
  {
    vnl_matrix<float> inMat = vnl_matrix<float>(m_InternalCaptureHeight,m_InternalCaptureWidth);
    inMat.copy_in(in);
    vnl_matrix<float> outMat = vnl_matrix<float>(m_CaptureHeight, m_CaptureWidth);
    vnl_matrix<float> temp = vnl_matrix<float>(m_CaptureHeight, m_CaptureWidth);
    temp = inMat.extract(m_CaptureHeight, m_CaptureWidth, 0,1);
    outMat = temp.transpose();
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
