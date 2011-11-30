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
#include "mitkToFImageGrabber.h"
#include "mitkToFCameraPMDCamCubeDevice.h"

#include "itkCommand.h"


namespace mitk
{
  ToFImageGrabber::ToFImageGrabber():m_CaptureWidth(204),m_CaptureHeight(204),m_PixelNumber(41616),m_ImageSequence(0),
    m_IntensityArray(NULL), m_DistanceArray(NULL), m_AmplitudeArray(NULL), m_SourceDataArray(NULL)
  {
    // Create the output. We use static_cast<> here because we know the default
    // output must be of type TOutputImage
    OutputImageType::Pointer output0 = static_cast<OutputImageType*>(this->MakeOutput(0).GetPointer()); 
    OutputImageType::Pointer output1 = static_cast<OutputImageType*>(this->MakeOutput(1).GetPointer()); 
    OutputImageType::Pointer output2 = static_cast<OutputImageType*>(this->MakeOutput(2).GetPointer()); 
    mitk::ImageSource::SetNumberOfRequiredOutputs(3);
    mitk::ImageSource::SetNthOutput(0, output0.GetPointer());
    mitk::ImageSource::SetNthOutput(1, output1.GetPointer());
    mitk::ImageSource::SetNthOutput(2, output2.GetPointer());
  }

  ToFImageGrabber::~ToFImageGrabber()
  {
    if (m_IntensityArray||m_AmplitudeArray||m_DistanceArray)
    {
      if (m_ToFCameraDevice)
      {
        m_ToFCameraDevice->RemoveObserver(m_DeviceObserverTag);
      }
      this->DisconnectCamera();
      this->CleanUpImageArrays();
    }
  }

  mitk::ImageSource::DataObjectPointer mitk::ImageSource::MakeOutput(unsigned int)
  {
    return static_cast<itk::DataObject*>(OutputImageType::New().GetPointer());
  }

  void ToFImageGrabber::GenerateData()
  {
    int requiredImageSequence = 0;
    int capturedImageSequence = 0;

    mitk::Image::Pointer distanceImage = this->GetOutput(0);
    mitk::Image::Pointer amplitudeImage = this->GetOutput(1);
    mitk::Image::Pointer intensityImage = this->GetOutput(2);

    if (!distanceImage->IsInitialized())
    {
      distanceImage->ReleaseData();
      amplitudeImage->ReleaseData();
      intensityImage->ReleaseData();

      unsigned int dimensions[3];
      dimensions[0] = this->m_ToFCameraDevice->GetCaptureWidth();
      dimensions[1] = this->m_ToFCameraDevice->GetCaptureHeight();
      dimensions[2] = 1;
      distanceImage->Initialize(mitk::PixelType(typeid(float)), 3, dimensions, 1);
      amplitudeImage->Initialize(mitk::PixelType(typeid(float)), 3, dimensions, 1);
      intensityImage->Initialize(mitk::PixelType(typeid(float)), 3, dimensions, 1);
    }

    if (m_DistanceArray&&m_AmplitudeArray&&m_IntensityArray)
    {
      this->m_ToFCameraDevice->GetAllImages(this->m_DistanceArray, this->m_AmplitudeArray, this->m_IntensityArray, this->m_SourceDataArray,
        requiredImageSequence, this->m_ImageSequence );

      capturedImageSequence = this->m_ImageSequence;
      distanceImage->SetSlice(this->m_DistanceArray, 0, 0, 0);
      amplitudeImage->SetSlice(this->m_AmplitudeArray, 0, 0, 0);
      intensityImage->SetSlice(this->m_IntensityArray, 0, 0, 0);
    }
  }

  bool ToFImageGrabber::ConnectCamera()
  {
    bool ok = m_ToFCameraDevice->ConnectCamera();
    if (ok)
    {
      m_CaptureWidth = m_ToFCameraDevice->GetCaptureWidth();
      m_CaptureHeight = m_ToFCameraDevice->GetCaptureHeight();
      m_PixelNumber = m_CaptureWidth * m_CaptureHeight;
      m_SourceDataSize = m_ToFCameraDevice->GetSourceDataSize();
      AllocateImageArrays();
    }
    return ok;
  }

  bool ToFImageGrabber::DisconnectCamera()
  {
    bool success = m_ToFCameraDevice->DisconnectCamera();

    return success;
  }

  void ToFImageGrabber::StartCamera()
  {
    m_ToFCameraDevice->StartCamera();
  }

  void ToFImageGrabber::StopCamera()
  {
    m_ToFCameraDevice->StopCamera();
  }

  bool ToFImageGrabber::IsCameraActive()
  {
    return m_ToFCameraDevice->IsCameraActive();
  }

  void ToFImageGrabber::SetCameraDevice(ToFCameraDevice* aToFCameraDevice)
  {
    m_ToFCameraDevice = aToFCameraDevice;
    itk::SimpleMemberCommand<ToFImageGrabber>::Pointer modifiedCommand = itk::SimpleMemberCommand<ToFImageGrabber>::New();
    modifiedCommand->SetCallbackFunction(this, &ToFImageGrabber::OnToFCameraDeviceModified);
    m_DeviceObserverTag = m_ToFCameraDevice->AddObserver(itk::ModifiedEvent(), modifiedCommand);
    this->Modified();
  }

  ToFCameraDevice* ToFImageGrabber::GetCameraDevice()
  {
    return m_ToFCameraDevice;
  }

  int ToFImageGrabber::GetCaptureWidth()
  {
    return m_CaptureWidth;
  }

  int ToFImageGrabber::GetCaptureHeight()
  {
    return m_CaptureHeight;
  }

  int ToFImageGrabber::GetPixelNumber()
  {
    return m_PixelNumber;
  }

  int ToFImageGrabber::SetModulationFrequency(int modulationFrequency)
  {
    this->m_ToFCameraDevice->SetProperty("ModulationFrequency",mitk::IntProperty::New(modulationFrequency));
    this->Modified();
    return modulationFrequency;
  }

  int ToFImageGrabber::SetIntegrationTime(int integrationTime)
  {
    this->m_ToFCameraDevice->SetProperty("IntegrationTime",mitk::IntProperty::New(integrationTime));
    this->Modified();
    return integrationTime;
  }

  int ToFImageGrabber::GetIntegrationTime()
  {
    int integrationTime = 0;
    BaseProperty* property = this->m_ToFCameraDevice->GetProperty("IntegrationTime");
    this->m_ToFCameraDevice->GetIntProperty(property,integrationTime);
    return integrationTime;
  }

  int ToFImageGrabber::GetModulationFrequency()
  {
    int modulationFrequency = 0;
    BaseProperty* property = this->m_ToFCameraDevice->GetProperty("ModulationFrequency");
    this->m_ToFCameraDevice->GetIntProperty(property,modulationFrequency);
    return modulationFrequency;
  }
  void ToFImageGrabber::SetBoolProperty( const char* propertyKey, bool boolValue )
  {
    SetProperty(propertyKey, mitk::BoolProperty::New(boolValue));
  }

  void ToFImageGrabber::SetIntProperty( const char* propertyKey, int intValue )
  {
    SetProperty(propertyKey, mitk::IntProperty::New(intValue));
  }

  void ToFImageGrabber::SetFloatProperty( const char* propertyKey, float floatValue )
  {
    SetProperty(propertyKey, mitk::FloatProperty::New(floatValue));
  }

  void ToFImageGrabber::SetStringProperty( const char* propertyKey, const char* stringValue )
  {
    SetProperty(propertyKey, mitk::StringProperty::New(stringValue));
  }

  void ToFImageGrabber::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    this->m_ToFCameraDevice->SetProperty(propertyKey, propertyValue);
  }

  void ToFImageGrabber::OnToFCameraDeviceModified()
  {
    this->Modified();
  }

  void ToFImageGrabber::CleanUpImageArrays()
  {
    // free buffer
    if (m_IntensityArray)
    {
      delete [] m_IntensityArray;
      m_IntensityArray = NULL;
    }
    if (m_DistanceArray)
    {
      delete [] m_DistanceArray;
      m_DistanceArray = NULL;
    }
    if (m_AmplitudeArray)
    {
      delete [] m_AmplitudeArray;
      m_AmplitudeArray = NULL;
    }
    if (m_SourceDataArray)
    {
      delete [] m_SourceDataArray;
      m_SourceDataArray = NULL;
    }
  }

  void ToFImageGrabber::AllocateImageArrays()
  {
    // cleanup memory if necessary
    this->CleanUpImageArrays();
    // allocate buffer
    m_IntensityArray = new float[m_PixelNumber];
    m_DistanceArray = new float[m_PixelNumber];
    m_AmplitudeArray = new float[m_PixelNumber];
    m_SourceDataArray = new char[m_SourceDataSize];
  }
}
