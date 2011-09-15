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
#include "mitkToFCameraPMDMITKPlayerController.h"
#include <pmdsdk2.h>

#include <mitkPicFileReader.h>
#include <mitkInstantiateAccessFunctions.h>
#include <mitkToFConfig.h>

#include <itksys/SystemTools.hxx>

//Plugin defines for Camcube 2.0
#define PROC_PARAM ""

extern PMDHandle m_PMDHandle; //TODO

extern PMDDataDescription m_DataDescription; //TODO

struct SourceDataStruct {
   PMDDataDescription dataDescription;
   char sourceData;
};

namespace mitk
{
  ToFCameraPMDMITKPlayerController::ToFCameraPMDMITKPlayerController()
  {
    m_ProcPlugin = MITK_TOF_PMDCAMCUBE_PROCESSING_PLUGIN; //MITK_TOF_PMDFILE_SOURCE_PLUGIN; //
    m_ProcParam = PROC_PARAM;

    this->m_CurrentFrame = -1;
    this->m_NumOfFrames = 0;
  }

  ToFCameraPMDMITKPlayerController::~ToFCameraPMDMITKPlayerController()
  {
  }

  bool ToFCameraPMDMITKPlayerController::OpenMITKFile()
  {
    if(this->m_InputFileName == "")
    {
      return false;
    }
    mitk::PicFileReader::Pointer reader = mitk::PicFileReader::New();
    reader->SetFileName ( this->m_InputFileName.c_str() );
    reader->Update();
    if ( reader->GetOutput() == NULL )
    {
      MITK_ERROR << "File " << this->m_InputFileName.c_str() << " could not be read!";
      return false;
    }
    this->m_MitkImage = reader->GetOutput();
    unsigned int dim = this->m_MitkImage->GetDimension();
    unsigned int *dims = this->m_MitkImage->GetDimensions();
//TODO
    if (/*dim != 4 ||*/ dims[1] != 1 || dims[2] != 1) {
      MITK_ERROR << "File " << this->m_InputFileName.c_str() << " is not a ToF format!";
      return false;
    }
    this->m_NumOfFrames = dims[3];
    return true;
  }

  bool ToFCameraPMDMITKPlayerController::ReadFile()
  {
    this->m_PMDRes = pmdOpenProcessingPlugin (&m_PMDHandle , this->m_ProcPlugin , this->m_ProcParam );
    bool ok = ErrorText(this->m_PMDRes);

    if (!ok)
    {
      return false;
    }

    ok = this->OpenMITKFile();
    if (!ok)
    {
      this->CloseCameraConnection();
      return ok;
    }

    try
    {
      SourceDataStruct* sourceDataStruct = (SourceDataStruct*) this->m_MitkImage->GetSliceData(0, 0, 0)->GetData();
      memcpy(&m_DataDescription, &(sourceDataStruct->dataDescription), sizeof(m_DataDescription));

      this->m_CaptureWidth = m_DataDescription.img.numColumns;
      this->m_CaptureHeight = m_DataDescription.img.numRows;

      if ( this->m_CaptureWidth > 10000 || this->m_CaptureHeight > 10000 ) {
        MITK_ERROR << "File " << this->m_InputFileName.c_str() << " is not a ToF format!";
        return false;
      }

      this->m_PixelNumber = this->m_CaptureWidth * this->m_CaptureHeight;
      this->m_NumberOfBytes = this->m_PixelNumber * sizeof(float);
      this->m_SourceDataSize = m_DataDescription.size;
      this->m_SourceDataStructSize = m_DataDescription.size + sizeof(PMDDataDescription);
    }
    catch (...)
    {
      MITK_ERROR << "File " << this->m_InputFileName.c_str() << " is not a ToF format!";
      return false;
    }
    MITK_INFO << "Open MITK file: " << this->m_InputFileName.c_str();
    return true;
  }

  bool ToFCameraPMDMITKPlayerController::OpenCameraConnection()
  {
    bool ok = false;
    ok = ReadFile();
    if (ok)
    {
      MITK_INFO << "Datasource size: " << this->m_SourceDataSize <<std::endl;
      MITK_INFO << "Integration Time: " << this->GetIntegrationTime();
      MITK_INFO << "Modulation Frequence: " << this->GetModulationFrequency();
      MITK_INFO << "NumOfFrames: " << this->m_NumOfFrames;
    }
    return ok;
  }

  bool ToFCameraPMDMITKPlayerController::CloseCameraConnection()
  {
    if(m_MitkImage.IsNotNull())
    {
      this->m_MitkImage->ReleaseData();
    }
    this->m_PMDRes = pmdClose(m_PMDHandle);
    m_ConnectionCheck = ErrorText(this->m_PMDRes);
    m_PMDHandle = 0;
    return m_ConnectionCheck;
  }

  bool ToFCameraPMDMITKPlayerController::UpdateCamera()
  {
    this->m_CurrentFrame = (this->m_CurrentFrame + 1) % this->m_NumOfFrames;
    itksys::SystemTools::Delay(40);
    return true;
  }

  bool ToFCameraPMDMITKPlayerController::GetSourceData(char* sourceDataArray)
  {
    if(this->m_CurrentFrame < 0)
    {
      MITK_ERROR<<"Invalid source data request, update camera before data retrieval!";
      return false;
    }
    SourceDataStruct* sourceDataStruct = (SourceDataStruct*) this->m_MitkImage->GetSliceData(0, this->m_CurrentFrame, 0)->GetData();
    if (sourceDataStruct)
    {
      memcpy(&m_DataDescription, &(sourceDataStruct->dataDescription), sizeof(m_DataDescription));
      memcpy(&((SourceDataStruct*)sourceDataArray)->dataDescription, &m_DataDescription, sizeof(m_DataDescription));
      memcpy(&((SourceDataStruct*)sourceDataArray)->sourceData, &sourceDataStruct->sourceData, this->m_SourceDataSize);
      return true;
    }
    else
    {
      return false;
    }
  }

  int ToFCameraPMDMITKPlayerController::SetIntegrationTime(unsigned int integrationTime)
  {
    return integrationTime;
  }

  int ToFCameraPMDMITKPlayerController::GetIntegrationTime()
  {
    int integrationTime = m_DataDescription.img.integrationTime[0];
    return integrationTime;
  }

  int ToFCameraPMDMITKPlayerController::SetModulationFrequency(unsigned int modulationFrequency)
  {
    return modulationFrequency;
  }

  int ToFCameraPMDMITKPlayerController::GetModulationFrequency()
  {
    int modulationFrequency = m_DataDescription.img.modulationFrequency[0];
    return (modulationFrequency/1000000);
  }

}
