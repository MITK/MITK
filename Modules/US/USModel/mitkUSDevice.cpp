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

#include "mitkUSDevice.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

const std::string mitk::USDevice::US_INTERFACE_NAME = "org.mitk.services.UltrasoundDevice";
const std::string mitk::USDevice::US_PROPKEY_LABEL = US_INTERFACE_NAME + ".label";
const std::string mitk::USDevice::US_PROPKEY_ISCONNECTED = US_INTERFACE_NAME + ".isConnected";
const std::string mitk::USDevice::US_PROPKEY_ISACTIVE = US_INTERFACE_NAME + ".isActive";
const std::string mitk::USDevice::US_PROPKEY_CLASS = US_INTERFACE_NAME + ".class";

//mitk::USDevice* mitk::USDevice::exp_Device = 0;

mitk::USDevice::USImageCropArea mitk::USDevice::GetCropArea()
{
  MITK_INFO << "Return Crop Area L:" << m_CropArea.cropLeft << " R:" << m_CropArea.cropRight << " T:" << m_CropArea.cropTop << " B:" << m_CropArea.cropBottom;
  return m_CropArea;
}

mitk::USDevice::USDevice(std::string manufacturer, std::string model)
  : mitk::ImageSource(),
    m_MultiThreader(itk::MultiThreader::New()),
    m_ImageMutex(itk::FastMutexLock::New()),
    m_CameraActiveMutex(itk::FastMutexLock::New()),
    m_DeviceState(State_NoState),
    m_UnregisteringStarted(false)
{
  // Initialize Members
  m_Metadata = mitk::USImageMetadata::New();
  m_Metadata->SetDeviceManufacturer(manufacturer);
  m_Metadata->SetDeviceModel(model);

  USImageCropArea empty;
  empty.cropBottom = 0;
  empty.cropTop = 0;
  empty.cropLeft = 0;
  empty.cropRight = 0;
  this->m_CropArea = empty;

  //set number of outputs
  this->SetNumberOfOutputs(1);

  //create a new output
  mitk::USImage::Pointer newOutput = mitk::USImage::New();
  this->SetNthOutput(0,newOutput);
}

mitk::USDevice::USDevice(mitk::USImageMetadata::Pointer metadata)
  : mitk::ImageSource(),
    m_MultiThreader(itk::MultiThreader::New()),
    m_ImageMutex(itk::FastMutexLock::New()),
    m_CameraActiveMutex(itk::FastMutexLock::New()),
    m_DeviceState(State_NoState),
    m_UnregisteringStarted(false)
{
  m_Metadata = metadata;

  USImageCropArea empty;
  empty.cropBottom = 0;
  empty.cropTop = 0;
  empty.cropLeft = 0;
  empty.cropRight = 0;
  this->m_CropArea = empty;

  //set number of outputs
  this->SetNumberOfOutputs(1);

  //create a new output
  mitk::USImage::Pointer newOutput = mitk::USImage::New();
  this->SetNthOutput(0,newOutput);
}

mitk::USDevice::~USDevice()
{
  this->UnregisterOnService();
}

mitk::USAbstractControlInterface::Pointer mitk::USDevice::GetControlInterfaceCustom()
{
  MITK_INFO << "Custom control interface does not exist for this object.";
  return 0;
}

mitk::USControlInterfaceBMode::Pointer mitk::USDevice::GetControlInterfaceBMode()
{
  MITK_INFO << "Control interface BMode does not exist for this object.";
  return 0;
};

mitk::USControlInterfaceProbes::Pointer mitk::USDevice::GetControlInterfaceProbes()
{
  MITK_INFO << "Control interface BMode does not exist for this object.";
  return 0;
};

mitk::USControlInterfaceDoppler::Pointer mitk::USDevice::GetControlInterfaceDoppler()
{
  MITK_INFO << "Control interface BMode does not exist for this object.";
  return 0;
};

us::ServiceProperties mitk::USDevice::ConstructServiceProperties()
{
  us::ServiceProperties props;
  std::string yes = "true";
  std::string no = "false";

  props[mitk::USDevice::US_PROPKEY_ISCONNECTED] = this->GetIsConnected() ? yes : no;

  if(this->GetIsActive())
    props[mitk::USDevice::US_PROPKEY_ISACTIVE] = yes;
  else
    props[mitk::USDevice::US_PROPKEY_ISACTIVE] = no;

  std::string isActive;
  if (GetIsActive()) isActive = " (Active)";
  else isActive = " (Inactive)";
  // e.g.: Zonare MyLab5 (Active)
  props[ mitk::USDevice::US_PROPKEY_LABEL] = m_Metadata->GetDeviceManufacturer() + " " + m_Metadata->GetDeviceModel() + isActive;

  if( m_Calibration.IsNotNull() )
    props[ mitk::USImageMetadata::PROP_DEV_ISCALIBRATED ] = yes;
  else
    props[ mitk::USImageMetadata::PROP_DEV_ISCALIBRATED ] = no;

  props[ mitk::USDevice::US_PROPKEY_CLASS ] = GetDeviceClass();
  props[ mitk::USImageMetadata::PROP_DEV_MANUFACTURER ] = m_Metadata->GetDeviceManufacturer();
  props[ mitk::USImageMetadata::PROP_DEV_MODEL ] = m_Metadata->GetDeviceModel();
  props[ mitk::USImageMetadata::PROP_DEV_COMMENT ] = m_Metadata->GetDeviceComment();
  props[ mitk::USImageMetadata::PROP_PROBE_NAME ] = m_Metadata->GetProbeName();
  props[ mitk::USImageMetadata::PROP_PROBE_FREQUENCY ] = m_Metadata->GetProbeFrequency();
  props[ mitk::USImageMetadata::PROP_ZOOM ] = m_Metadata->GetZoom();

  return props;
}

void mitk::USDevice::UnregisterOnService()
{
  // unregister on micro service
  if ( m_ServiceRegistration && ! m_UnregisteringStarted)
  {
    // make sure that unregister is not started a second
    // time due to a callback during unregister for example
    m_UnregisteringStarted = true;
    m_ServiceRegistration.Unregister();
    m_ServiceRegistration = 0;
  }
}

bool mitk::USDevice::Initialize()
{
  if (! this->OnInitialization() ) { return false; }

  // Get Context and Module
  us::ModuleContext* context = us::GetModuleContext();
  us::ServiceProperties props = this->ConstructServiceProperties();

  m_ServiceRegistration = context->RegisterService(this, props);

  m_DeviceState = State_Initialized;

  return true;
}

bool mitk::USDevice::Connect()
{
  if ( this->GetIsConnected() )
  {
    MITK_WARN("mitkUSDevice") << "Tried to connect an ultrasound device that was already connected. Ignoring call...";
    return false;
  }

  if ( ! this->GetIsInitialized() )
  {
    MITK_ERROR("mitkUSDevice") << "Cannot connect device if it is not in initialized state.";
    return false;
  }

  // Prepare connection, fail if this fails.
  if ( ! this->OnConnection() ) { return false; }

  // Update state
  m_DeviceState = State_Connected;

  us::ServiceProperties props = this->ConstructServiceProperties();
  m_ServiceRegistration.SetProperties(props);

  return true;
}

bool mitk::USDevice::Disconnect()
{
  if ( ! GetIsConnected())
  {
    MITK_WARN << "Tried to disconnect an ultrasound device that was not connected. Ignoring call...";
    return false;
  }
  // Prepare connection, fail if this fails.
   if (! this->OnDisconnection()) return false;

  // Update state
  m_DeviceState = State_Initialized;

  us::ServiceProperties props = this->ConstructServiceProperties();
  this->m_ServiceRegistration.SetProperties(props);

  return true;
}

bool mitk::USDevice::Activate()
{
  if (! this->GetIsConnected())
  {
    MITK_WARN("mitkUSDevice") << "Cannot activate device if it is not in connected state.";
    return false;
  }

  if ( OnActivation() )
  {
    m_DeviceState = State_Activated;

    // spawn thread for aquire images if us device is active
    this->m_ThreadID = this->m_MultiThreader->SpawnThread(this->Acquire, this);
  }

  us::ServiceProperties props = this->ConstructServiceProperties();
  m_ServiceRegistration.SetProperties(props);

  return m_DeviceState == State_Activated;
}


void mitk::USDevice::Deactivate()
{
  m_DeviceState = State_Connected;

  us::ServiceProperties props = ConstructServiceProperties();
  m_ServiceRegistration.SetProperties(props);
  OnDeactivation();
}

void mitk::USDevice::AddProbe(mitk::USProbe::Pointer probe)
{
  for(int i = 0; i < m_ConnectedProbes.size(); i++)
  {
    if (m_ConnectedProbes[i]->IsEqualToProbe(probe)) return;
  }
  m_ConnectedProbes.push_back(probe);
}


void mitk::USDevice::ActivateProbe(mitk::USProbe::Pointer probe){
  // currently, we may just add the probe. This behaviour should be changed, should more complicated SDK applications emerge
  AddProbe(probe);
  int index = -1;
  for(int i = 0; i < m_ConnectedProbes.size(); i++)
  {
    if (m_ConnectedProbes[i]->IsEqualToProbe(probe)) index = i;
  }
  // index now contains the position of the original instance of this probe
  m_ActiveProbe = m_ConnectedProbes[index];
}


void mitk::USDevice::DeactivateProbe(){
  m_ActiveProbe = 0;
}


mitk::USImage* mitk::USDevice::GetOutput()
{
  if (this->GetNumberOfOutputs() < 1)
    return NULL;

  return static_cast<USImage*>(this->ProcessObject::GetPrimaryOutput());
}


mitk::USImage* mitk::USDevice::GetOutput(unsigned int idx)
{
  if (this->GetNumberOfOutputs() < 1)
    return NULL;
  return static_cast<USImage*>(this->ProcessObject::GetOutput(idx));
}


void mitk::USDevice::GraftOutput(itk::DataObject *graft)
{
  this->GraftNthOutput(0, graft);
}


void mitk::USDevice::GraftNthOutput(unsigned int idx, itk::DataObject *graft)
{
  if ( idx >= this->GetNumberOfOutputs() )
  {
    itkExceptionMacro(<<"Requested to graft output " << idx <<
                      " but this filter only has " << this->GetNumberOfOutputs() << " Outputs.");
  }

  if ( !graft )
  {
    itkExceptionMacro(<<"Requested to graft output with a NULL pointer object" );
  }

  itk::DataObject* output = this->GetOutput(idx);
  if ( !output )
  {
    itkExceptionMacro(<<"Requested to graft output that is a NULL pointer" );
  }
  // Call Graft on USImage to copy member data
  output->Graft( graft );
}

bool mitk::USDevice::ApplyCalibration(mitk::USImage::Pointer image){
  if ( m_Calibration.IsNull() ) return false;

  image->GetGeometry()->SetIndexToWorldTransform(m_Calibration);
  return true;
}

void mitk::USDevice::GrabImage()
{
  m_Image = this->GetUSImageSource()->GetNextImage();
}


//########### GETTER & SETTER ##################//

void mitk::USDevice::setCalibration (mitk::AffineTransform3D::Pointer calibration){
  if (calibration.IsNull())
  {
    MITK_ERROR << "Null pointer passed to SetCalibration of mitk::USDevice. Ignoring call.";
    return;
  }
  m_Calibration = calibration;
  m_Metadata->SetDeviceIsCalibrated(true);
  if (m_ServiceRegistration != 0)
  {
    us::ServiceProperties props = ConstructServiceProperties();
    this->m_ServiceRegistration.SetProperties(props);
  }
}

bool mitk::USDevice::GetIsInitialized()
{
  return m_DeviceState == State_Initialized;
}

bool mitk::USDevice::GetIsActive()
{
  return m_DeviceState == State_Activated;
}


bool mitk::USDevice::GetIsConnected()
{
  return m_DeviceState == State_Connected;
}


std::string mitk::USDevice::GetDeviceManufacturer(){
  return this->m_Metadata->GetDeviceManufacturer();
}

std::string mitk::USDevice::GetDeviceModel(){
  return this->m_Metadata->GetDeviceModel();
}

std::string mitk::USDevice::GetDeviceComment(){
  return this->m_Metadata->GetDeviceComment();
}

std::vector<mitk::USProbe::Pointer> mitk::USDevice::GetConnectedProbes()
{
  return m_ConnectedProbes;
}

ITK_THREAD_RETURN_TYPE mitk::USDevice::Acquire(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  mitk::USDevice * device = (mitk::USDevice *) pInfo->UserData;
  while (device->GetIsActive())
  {
    device->GrabImage();
  }
  return ITK_THREAD_RETURN_VALUE;
}
