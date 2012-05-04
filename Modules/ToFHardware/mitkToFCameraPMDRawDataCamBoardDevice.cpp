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
#include "mitkToFCameraPMDRawDataCamBoardDevice.h"
#include "mitkToFCameraPMDCamBoardController.h"

// vtk includes
#include "vtkSmartPointer.h"

namespace mitk
{
  ToFCameraPMDRawDataCamBoardDevice::ToFCameraPMDRawDataCamBoardDevice()
  {
    m_Controller = ToFCameraPMDCamBoardController::New();
  }

  ToFCameraPMDRawDataCamBoardDevice::~ToFCameraPMDRawDataCamBoardDevice()
  {
  }

  void ToFCameraPMDRawDataCamBoardDevice::GetChannelSourceData(short* sourceData, vtkShortArray* vtkChannelArray )
  {
    int i = 0;
    //unsigned int channelDataPosition = 0;
    unsigned int channelSize = (this->GetCaptureHeight()*this->GetCaptureWidth()*2);
    this->SetChannelSize(channelSize);
    signed short* channel1;
    signed short* channel2;
    signed short* channel3;
    signed short* channel4;

    vtkSmartPointer<vtkShortArray> cvtkChannelArray = vtkShortArray::New();
    cvtkChannelArray->SetNumberOfComponents(channelSize);
    cvtkChannelArray->SetNumberOfTuples(4);
    cvtkChannelArray->Allocate(1);

    channel1 = sourceData;
    cvtkChannelArray->InsertTupleValue(0,channel1);
    sourceData += channelSize;
    channel2 = sourceData;
    cvtkChannelArray->InsertTupleValue(1,channel2);
    sourceData += channelSize;
    channel3 = sourceData;
    cvtkChannelArray->InsertTupleValue(2,channel3);
    sourceData += channelSize;
    channel4 = sourceData;
    cvtkChannelArray->InsertTupleValue(3,channel4);
    vtkChannelArray->DeepCopy(cvtkChannelArray);
    cvtkChannelArray->Delete();
  }

  void ToFCameraPMDRawDataCamBoardDevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    ToFCameraPMDRawDataDevice::SetProperty(propertyKey,propertyValue);
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);

    ToFCameraPMDCamBoardController::Pointer myController = dynamic_cast<mitk::ToFCameraPMDCamBoardController*>(this->m_Controller.GetPointer());

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
