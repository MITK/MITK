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
#include "mitkToFImageGrabber.h"
#include "itkCommand.h"

namespace mitk
{
ToFImageGrabber::ToFImageGrabber():
  m_ToFCameraDevice(NULL),
  m_CaptureWidth(-1),
  m_CaptureHeight(-1),
  m_PixelNumber(-1),
  m_RGBImageWidth(0),
  m_RGBImageHeight(0),
  m_RGBPixelNumber(0),
  m_ImageSequence(0),
  m_SourceDataSize(0),
  m_IntensityArray(NULL),
  m_DistanceArray(NULL),
  m_AmplitudeArray(NULL),
  m_SourceDataArray(NULL),
  m_RgbDataArray(NULL),
  m_DeviceObserverTag()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type TOutputImage
  OutputImageType::Pointer output0 = static_cast<OutputImageType*>(this->MakeOutput(0).GetPointer());
  OutputImageType::Pointer output1 = static_cast<OutputImageType*>(this->MakeOutput(1).GetPointer());
  OutputImageType::Pointer output2 = static_cast<OutputImageType*>(this->MakeOutput(2).GetPointer());
  OutputImageType::Pointer output3 = static_cast<OutputImageType*>(this->MakeOutput(3).GetPointer());
  mitk::ImageSource::SetNumberOfRequiredOutputs(3);
  mitk::ImageSource::SetNthOutput(0, output0.GetPointer());
  mitk::ImageSource::SetNthOutput(1, output1.GetPointer());
  mitk::ImageSource::SetNthOutput(2, output2.GetPointer());
  mitk::ImageSource::SetNthOutput(3, output3.GetPointer());
}

ToFImageGrabber::~ToFImageGrabber()
{
  if (m_IntensityArray||m_AmplitudeArray||m_DistanceArray||m_RgbDataArray)
  {
    if (m_ToFCameraDevice)
    {
      m_ToFCameraDevice->RemoveObserver(m_DeviceObserverTag);
    }
    this->DisconnectCamera();
    this->CleanUpImageArrays();
  }
}

void ToFImageGrabber::GenerateData()
{
  int requiredImageSequence = 0;
  // acquire new image data
  this->m_ToFCameraDevice->GetAllImages(this->m_DistanceArray, this->m_AmplitudeArray, this->m_IntensityArray, this->m_SourceDataArray,
                                        requiredImageSequence, this->m_ImageSequence, this->m_RgbDataArray );

  mitk::Image::Pointer distanceImage = this->GetOutput(0);
  if (m_DistanceArray)
  {
    distanceImage->SetSlice(this->m_DistanceArray, 0, 0, 0);
  }

  bool hasAmplitudeImage = false;
  m_ToFCameraDevice->GetBoolProperty("HasAmplitudeImage", hasAmplitudeImage);
  if((hasAmplitudeImage) && (m_AmplitudeArray))
  {
    mitk::Image::Pointer amplitudeImage = this->GetOutput(1);
    amplitudeImage->SetSlice(this->m_AmplitudeArray, 0, 0, 0);
  }

  bool hasIntensityImage = false;
  m_ToFCameraDevice->GetBoolProperty("HasIntensityImage", hasIntensityImage);
  if((hasIntensityImage) && (m_IntensityArray))
  {
    mitk::Image::Pointer intensityImage = this->GetOutput(2);
    intensityImage->SetSlice(this->m_IntensityArray, 0, 0, 0);
  }

  bool hasRGBImage = false;
  m_ToFCameraDevice->GetBoolProperty("HasRGBImage", hasRGBImage);
  if( hasRGBImage )
  {
    mitk::Image::Pointer rgbImage = this->GetOutput(3);
    if (m_RgbDataArray)
    {
      rgbImage->SetSlice(this->m_RgbDataArray, 0, 0, 0);
    }
  }
}

bool ToFImageGrabber::ConnectCamera()
{
  bool ok = m_ToFCameraDevice->ConnectCamera();
  if (ok)
  {
    this->m_CaptureWidth = this->m_ToFCameraDevice->GetCaptureWidth();
    this->m_CaptureHeight = this->m_ToFCameraDevice->GetCaptureHeight();
    this->m_PixelNumber = this->m_CaptureWidth * this->m_CaptureHeight;

    this->m_RGBImageWidth = this->m_ToFCameraDevice->GetRGBCaptureWidth();
    this->m_RGBImageHeight = this->m_ToFCameraDevice->GetRGBCaptureHeight();
    this->m_RGBPixelNumber = this->m_RGBImageWidth * this->m_RGBImageHeight;

    this->m_SourceDataSize = m_ToFCameraDevice->GetSourceDataSize();
    this->AllocateImageArrays();
    this->InitializeImages();
  }
  return ok;
}

bool ToFImageGrabber::DisconnectCamera()
{
  return m_ToFCameraDevice->DisconnectCamera();
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
bool ToFImageGrabber::IsCameraConnected()
{
  return m_ToFCameraDevice->IsCameraConnected();
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

int ToFImageGrabber::GetRGBImageWidth()
{
  return m_RGBImageWidth;
}

int ToFImageGrabber::GetRGBImageHeight()
{
  return m_RGBImageHeight;
}

int ToFImageGrabber::GetRGBPixelNumber()
{
  return m_RGBPixelNumber;
}

int ToFImageGrabber::SetModulationFrequency(int modulationFrequency)
{
  this->m_ToFCameraDevice->SetProperty("ModulationFrequency",mitk::IntProperty::New(modulationFrequency));
  this->Modified();
  modulationFrequency = this->GetModulationFrequency(); // return the new valid modulation frequency from the camera
  return modulationFrequency;
}

int ToFImageGrabber::SetIntegrationTime(int integrationTime)
{
  this->m_ToFCameraDevice->SetProperty("IntegrationTime",mitk::IntProperty::New(integrationTime));
  this->Modified();
  integrationTime = this->GetIntegrationTime(); // return the new valid integration time from the camera
  return integrationTime;
}

int ToFImageGrabber::GetIntegrationTime()
{
  int integrationTime = 0;
  this->m_ToFCameraDevice->GetIntProperty("IntegrationTime",integrationTime);
  return integrationTime;
}

int ToFImageGrabber::GetModulationFrequency()
{
  int modulationFrequency = 0;
  this->m_ToFCameraDevice->GetIntProperty("ModulationFrequency",modulationFrequency);
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

bool ToFImageGrabber::GetBoolProperty( const char* propertyKey)
{
  mitk::BoolProperty::Pointer boolProp = dynamic_cast<mitk::BoolProperty*>(GetProperty(propertyKey));
  if(!boolProp) return false;
  return boolProp->GetValue();
}

int ToFImageGrabber::GetIntProperty( const char* propertyKey)
{
  mitk::IntProperty::Pointer intProp = dynamic_cast<mitk::IntProperty*>(GetProperty(propertyKey));
  return intProp->GetValue();
}

float ToFImageGrabber::GetFloatProperty( const char* propertyKey)
{
  mitk::FloatProperty::Pointer floatProp = dynamic_cast<mitk::FloatProperty*>(GetProperty(propertyKey));
  return floatProp->GetValue();
}

const char* ToFImageGrabber::GetStringProperty( const char* propertyKey)
{
  mitk::StringProperty::Pointer stringProp = dynamic_cast<mitk::StringProperty*>(GetProperty(propertyKey));
  return stringProp->GetValue();
}

BaseProperty* ToFImageGrabber::GetProperty( const char *propertyKey)
{
  return this->m_ToFCameraDevice->GetProperty(propertyKey);
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
  if (m_RgbDataArray)
  {
    delete [] m_RgbDataArray;
    m_RgbDataArray = NULL;
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
  m_RgbDataArray = new unsigned char[m_RGBPixelNumber*3];
}

void ToFImageGrabber::InitializeImages()
{
  unsigned int dimensions[3];
  dimensions[0] = this->m_ToFCameraDevice->GetCaptureWidth();
  dimensions[1] = this->m_ToFCameraDevice->GetCaptureHeight();
  dimensions[2] = 1;
  mitk::PixelType FloatType = MakeScalarPixelType<float>();
  mitk::Image::Pointer distanceImage = this->GetOutput();
  distanceImage->ReleaseData();
  distanceImage->Initialize(FloatType, 3, dimensions, 1);

  bool hasAmplitudeImage = false;
  m_ToFCameraDevice->GetBoolProperty("HasAmplitudeImage", hasAmplitudeImage);
  if(hasAmplitudeImage)
  {
    mitk::Image::Pointer amplitudeImage = this->GetOutput(1);
    amplitudeImage->ReleaseData();
    amplitudeImage->Initialize(FloatType, 3, dimensions, 1);
  }

  bool hasIntensityImage = false;
  m_ToFCameraDevice->GetBoolProperty("HasIntensityImage", hasIntensityImage);
  if(hasIntensityImage)
  {
    mitk::Image::Pointer intensityImage = this->GetOutput(2);
    intensityImage->ReleaseData();
    intensityImage->Initialize(FloatType, 3, dimensions, 1);
  }

  bool hasRGBImage = false;
  m_ToFCameraDevice->GetBoolProperty("HasRGBImage", hasRGBImage);
  if(hasRGBImage)
  {
    unsigned int rgbDimension[3];
    rgbDimension[0] = this->m_ToFCameraDevice->GetRGBCaptureWidth();
    rgbDimension[1] = this->m_ToFCameraDevice->GetRGBCaptureHeight();
    rgbDimension[2] = 1 ;
    mitk::Image::Pointer rgbImage = this->GetOutput(3);
    rgbImage->ReleaseData();
    rgbImage->Initialize(mitk::PixelType(MakePixelType<unsigned char, itk::RGBPixel<unsigned char>, 3>()), 3, rgbDimension,1);
  }
}
}
