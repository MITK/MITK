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
#include "mitkToFCameraDevice.h"


namespace mitk
{
  ToFCameraDevice::ToFCameraDevice():m_BufferSize(1),m_MaxBufferSize(100),m_CurrentPos(-1),m_FreePos(0),
    m_CaptureWidth(204),m_CaptureHeight(204),m_PixelNumber(41616),m_SourceDataSize(0),
    m_ThreadID(0),m_CameraActive(false),m_CameraConnected(false),m_ImageSequence(0)
  {
    this->m_AmplitudeArray = NULL;
    this->m_IntensityArray = NULL;
    this->m_DistanceArray = NULL;
    this->m_PropertyList = mitk::PropertyList::New();

    this->m_MultiThreader = itk::MultiThreader::New();
    this->m_ImageMutex = itk::FastMutexLock::New();
    this->m_CameraActiveMutex = itk::FastMutexLock::New();
  }

  ToFCameraDevice::~ToFCameraDevice()
  {
    CleanupPixelArrays();
  }

  void ToFCameraDevice::SetBoolProperty( const char* propertyKey, bool boolValue )
  {
    SetProperty(propertyKey, mitk::BoolProperty::New(boolValue));
  }

  void ToFCameraDevice::SetIntProperty( const char* propertyKey, int intValue )
  {
    SetProperty(propertyKey, mitk::IntProperty::New(intValue));
  }

  void ToFCameraDevice::SetFloatProperty( const char* propertyKey, float floatValue )
  {
    SetProperty(propertyKey, mitk::FloatProperty::New(floatValue));
  }

  void ToFCameraDevice::SetStringProperty( const char* propertyKey, const char* stringValue )
  {
    SetProperty(propertyKey, mitk::StringProperty::New(stringValue));
  }

  void ToFCameraDevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);
  }

  BaseProperty* ToFCameraDevice::GetProperty(const char *propertyKey)
  {
    return this->m_PropertyList->GetProperty(propertyKey);
  }

  bool ToFCameraDevice::GetBoolProperty(BaseProperty* propertyValue, bool& boolValue)
  {
    mitk::BoolProperty::Pointer boolprop = dynamic_cast<mitk::BoolProperty*>(propertyValue);
    if(boolprop.IsNull())
      return false;

    boolValue = boolprop->GetValue();
    return true;
  }

  bool ToFCameraDevice::GetStringProperty(BaseProperty* propertyValue, std::string& string)
  {
    mitk::StringProperty::Pointer stringProp = dynamic_cast<mitk::StringProperty*>(propertyValue);
    if(stringProp.IsNull())
    {
      return false;
    } 
    else 
    {
      string = stringProp->GetValue();
      return true;
    }
  }
  bool ToFCameraDevice::GetIntProperty(BaseProperty* propertyValue, int& integer)
  {
    mitk::IntProperty::Pointer intProp = dynamic_cast<mitk::IntProperty*>(propertyValue);
    if(intProp.IsNull())
    {
      return false;
    }
    else
    {
      integer = intProp->GetValue();
      return true;
    }
  }

  void ToFCameraDevice::CleanupPixelArrays()
  {
    if (m_IntensityArray)
    {
      delete [] m_IntensityArray;
    }
    if (m_DistanceArray)
    {
      delete [] m_DistanceArray;
    }
    if (m_AmplitudeArray)
    {
      delete [] m_AmplitudeArray;
    }
  }

  void ToFCameraDevice::AllocatePixelArrays()
  {
    // free memory if it was already allocated
    CleanupPixelArrays();
    // allocate buffer
    this->m_IntensityArray = new float[this->m_PixelNumber];
    for(int i=0; i<this->m_PixelNumber; i++) {this->m_IntensityArray[i]=0.0;}
    this->m_DistanceArray = new float[this->m_PixelNumber];
    for(int i=0; i<this->m_PixelNumber; i++) {this->m_DistanceArray[i]=0.0;}
    this->m_AmplitudeArray = new float[this->m_PixelNumber];
    for(int i=0; i<this->m_PixelNumber; i++) {this->m_AmplitudeArray[i]=0.0;}
  }
}
