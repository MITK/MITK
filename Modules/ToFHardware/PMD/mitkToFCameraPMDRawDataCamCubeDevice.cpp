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
#include "mitkToFCameraPMDRawDataCamCubeDevice.h"
#include "mitkToFCameraPMDCamCubeController.h"

// vtk includes
#include "vtkSmartPointer.h"

namespace mitk
{
  ToFCameraPMDRawDataCamCubeDevice::ToFCameraPMDRawDataCamCubeDevice()
  {
    m_Controller = ToFCameraPMDCamCubeController::New();
  }

  ToFCameraPMDRawDataCamCubeDevice::~ToFCameraPMDRawDataCamCubeDevice()
  {
  }

  void ToFCameraPMDRawDataCamCubeDevice::GetChannelSourceData(short* sourceData, vtkShortArray* vtkChannelArray )
  {
    int i = 0;
    unsigned int channelSize = (this->GetCaptureHeight()*this->GetCaptureWidth()*2);
    this->SetChannelSize(channelSize);
    signed short* channel1;
    signed short* channel2;
    signed short* channel3;
    signed short* channel4;

    vtkSmartPointer<vtkShortArray> tempVTKChannelArray = vtkShortArray::New();
    tempVTKChannelArray->SetNumberOfComponents(channelSize);
    tempVTKChannelArray->SetNumberOfTuples(4);
    tempVTKChannelArray->Allocate(1);

    sourceData += 256;
    channel1 = sourceData;
    tempVTKChannelArray->InsertTupleValue(0,channel1);
    sourceData += channelSize;
    sourceData += 256;
    channel2 = sourceData;
    tempVTKChannelArray->InsertTupleValue(1,channel2);
    sourceData += channelSize;
    sourceData += 256;
    channel3 = sourceData;
    tempVTKChannelArray->InsertTupleValue(2,channel3);
    sourceData += channelSize;
    sourceData += 256;
    channel4 = sourceData;
    tempVTKChannelArray->InsertTupleValue(3,channel4);
    vtkChannelArray->DeepCopy(tempVTKChannelArray);
    tempVTKChannelArray->Delete();
  }

  void ToFCameraPMDRawDataCamCubeDevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    ToFCameraPMDRawDataDevice::SetProperty(propertyKey,propertyValue);
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);

    ToFCameraPMDCamCubeController::Pointer myController = dynamic_cast<mitk::ToFCameraPMDCamCubeController*>(this->m_Controller.GetPointer());

    bool boolValue = false;
    GetBoolProperty(propertyKey, boolValue);
    if (strcmp(propertyKey, "SetFPNCalibration") == 0)
    {
      myController->SetFPNCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetFPPNCalibration") == 0)
    {
      myController->SetFPPNCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetLinearityCalibration") == 0)
    {
      myController->SetLinearityCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetLensCalibration") == 0)
    {
      myController->SetLensCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetExposureMode") == 0)
    {
      if (boolValue)
      {
        myController->SetExposureMode(1);
      }
      else
      {
        myController->SetExposureMode(0);
      }
    }
  }

}
