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
#include "mitkUSImageMetadata.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include "mitkModuleContext.h"


mitk::USDevice::USDevice(std::string manufacturer, std::string model, bool isVideoOnly) : mitk::ImageSource()
{
  // Initialize Members
  m_Metadata = mitk::USImageMetadata::New();
  m_Metadata->SetDeviceManufacturer(manufacturer);
  m_Metadata->SetDeviceModel(model);
  m_Metadata->SetDeviceIsVideoOnly(isVideoOnly);
  m_IsActive = false;
  
  //set number of outputs
  this->SetNumberOfOutputs(1);

  //create a new output
  mitk::USImage::Pointer newOutput = mitk::USImage::New();
  this->SetNthOutput(0,newOutput);
}

mitk::USDevice::USDevice(mitk::USImageMetadata::Pointer metadata, bool isVideoOnly) : mitk::ImageSource()
{
  m_Metadata = metadata;
  m_IsActive = false;

  //set number of outputs
  this->SetNumberOfOutputs(1);

  //create a new output
  mitk::USImage::Pointer newOutput = mitk::USImage::New();
  this->SetNthOutput(0,newOutput);
}


mitk::USDevice::~USDevice()
{

}

bool mitk::USDevice::Connect()
{
  //TODO Throw Exception is already activated before connection

  // Prepare connection, fail if this fails.
  if (! this->OnConnection()) return false;

  // Get Context and Module
  mitk::ModuleContext* context = GetModuleContext();

  // Define ServiceProps
  ServiceProperties props;
  props["DeviceClass"] = this->GetDeviceClass();
  props["IsActive"] = false;
  props["Class"] = this->GetClassName();
  m_ServiceRegistration = context->RegisterService<mitk::USDevice>(this, props);
  return true; 
}

bool mitk::USDevice::OnConnection()
{
  return true;
  // TODO: Make Abstract
}

bool mitk::USDevice::Disconnect()
{
  // Prepare connection, fail if this fails.
  if (! this->OnDisconnection()) return false;

  // Unregister
  m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
  return true;
}

bool mitk::USDevice::OnDisconnection()
{
  return true;
  // TODO Make Abstract
}


bool mitk::USDevice::Activate()
{
  if (! this->GetIsConnected()) return false;

  m_IsActive = OnActivation();

  return m_IsActive;
}

void mitk::USDevice::Deactivate()
{
  m_IsActive= false;
  OnDeactivation();
}


bool mitk::USDevice::OnActivation()
{
  return true;
  // TODO Make Abstract
}


void mitk::USDevice::OnDeactivation()
{
  // TODO Make Abstract
}


std::string mitk::USDevice::GetDeviceClass()
{
  return "org.mitk.Ultrasound.GenericDevice";
}



void mitk::USDevice::AddProbe(mitk::USProbe::Pointer probe)
{
  for(int i = 0; i < m_ConnectedProbes.size(); i++)
  {
    if (m_ConnectedProbes[i]->IsEqualToProbe(probe)) return;
  }
  this->m_ConnectedProbes.push_back(probe);
}

void mitk::USDevice::ActivateProbe(mitk::USProbe::Pointer probe){
  // currently, we may just add the probe. This behaviour must be changed, should more complicated SDK applications emerge 
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


void mitk::USDevice::GenerateData()
{

}


mitk::USImage* mitk::USDevice::GetOutput()
{
  if (this->GetNumberOfOutputs() < 1)
    return NULL;

  return static_cast<USImage*>(this->ProcessObject::GetOutput(0));
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


itk::ProcessObject::DataObjectPointer mitk::USDevice::MakeOutput( unsigned int /*idx */)
{
  mitk::USImage::Pointer p = mitk::USImage::New();
  return static_cast<itk::DataObject*>(p.GetPointer());
}

 //########### GETTER & SETTER ##################//

bool mitk::USDevice::GetIsActive()
{
  return m_IsActive;
}


bool mitk::USDevice::GetIsConnected()
{
  // a device is connected if it is registered with the service
  return (m_ServiceRegistration != 0);
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

bool mitk::USDevice::GetIsVideoOnly(){
  return this->m_Metadata->GetDeviceIsVideoOnly();
}

std::vector<mitk::USProbe::Pointer> mitk::USDevice::GetConnectedProbes()
{
  return m_ConnectedProbes;
}
