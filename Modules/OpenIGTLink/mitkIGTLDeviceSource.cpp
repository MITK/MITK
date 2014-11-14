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

#include "mitkIGTLDeviceSource.h"

#include "mitkIGTLDevice.h"
#include "mitkIGTLMessage.h"

//#include "mitkIGTTimeStamp.h"
//#include "mitkIGTException.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

//itk
#include <itkCommand.h>


const std::string mitk::IGTLDeviceSource::US_PROPKEY_IGTLDEVICENAME =
    mitk::IGTLMessageSource::US_INTERFACE_NAME + ".igtldevicename";

mitk::IGTLDeviceSource::IGTLDeviceSource()
  : mitk::IGTLMessageSource(), m_IGTLDevice(NULL)
{
  this->SetName("IGTLDeviceSource (no defined type)");
}

mitk::IGTLDeviceSource::~IGTLDeviceSource()
{
  if (m_IGTLDevice.IsNotNull())
  {
    if (m_IGTLDevice->GetState() == mitk::IGTLDevice::Running)
    {
      this->StopCommunication();
    }
    if (m_IGTLDevice->GetState() == mitk::IGTLDevice::Ready)
    {
      this->Disconnect();
    }
    m_IGTLDevice = NULL;
  }
}

void mitk::IGTLDeviceSource::GenerateData()
{
  if (m_IGTLDevice.IsNull())
    return;

  /* update output with message from the device */
  IGTLMessage* msgOut = this->GetOutput();
  assert(msgOut);
  igtl::MessageBase::Pointer msgIn = m_IGTLDevice->GetLatestMessage();
  if ( msgIn.IsNotNull() )
  {
    assert(msgIn);

    msgOut->SetMessage(msgIn);
    msgOut->SetName(msgIn->GetDeviceName());
  }
//  else
//  {
//    MITK_ERROR("IGTLDeviceSource") << "Could not get the latest message.";
//  }
}

void mitk::IGTLDeviceSource::SetIGTLDevice( mitk::IGTLDevice* igtlDevice )
{
  MITK_DEBUG << "Setting IGTLDevice to " << igtlDevice;
  if (this->m_IGTLDevice.GetPointer() != igtlDevice)
  {
    this->m_IGTLDevice = igtlDevice;
    this->CreateOutputs();
    std::stringstream name; // create a human readable name for the source
    name << "OIGTL Device Source ( " << igtlDevice->GetName() << " )";
    this->SetName(name.str());

    //setup a observer that listens to new messages
    typedef itk::SimpleMemberCommand<mitk::IGTLDeviceSource> CurCommandType;
    CurCommandType::Pointer messageReceivedCommand = CurCommandType::New();
    messageReceivedCommand->SetCallbackFunction(
      this, &IGTLDeviceSource::OnIncomingMessage );
    this->m_IGTLDevice->AddObserver(mitk::MessageReceivedEvent(),
                                    messageReceivedCommand);

  }
}

void mitk::IGTLDeviceSource::CreateOutputs()
{
  //if outputs are set then delete them
  if (this->GetNumberOfOutputs() > 0)
  {
    for (int numOP = this->GetNumberOfOutputs() - 1; numOP >= 0; numOP--)
      this->RemoveOutput(numOP);
    this->Modified();
  }

  //fill the outputs if a valid OpenIGTLink device is set
  if (m_IGTLDevice.IsNull())
    return;

  this->SetNumberOfIndexedOutputs(1);
  if (this->GetOutput(0) == NULL)
  {
    DataObjectPointer newOutput = this->MakeOutput(0);
    this->SetNthOutput(0, newOutput);
    this->Modified();
  }
}

void mitk::IGTLDeviceSource::Connect()
{
  if (m_IGTLDevice.IsNull())
  {
    throw std::invalid_argument("mitk::IGTLDeviceSource: "
                                "No OpenIGTLink device set");
  }
  if (this->IsConnected())
  {
    return;
  }
  try
  {
    m_IGTLDevice->OpenConnection();
  }
  catch (mitk::Exception &e)
  {
    throw std::runtime_error(std::string("mitk::IGTLDeviceSource: Could not open"
            "connection to OpenIGTLink device. Error: ") + e.GetDescription());
  }
}

void mitk::IGTLDeviceSource::StartCommunication()
{
  if (m_IGTLDevice.IsNull())
    throw std::invalid_argument("mitk::IGTLDeviceSource: "
                                "No OpenIGTLink device set");
  if (m_IGTLDevice->GetState() == mitk::IGTLDevice::Running)
    return;
  if (m_IGTLDevice->StartCommunication() == false)
    throw std::runtime_error("mitk::IGTLDeviceSource: "
                             "Could not start communication");
}

void mitk::IGTLDeviceSource::Disconnect()
{
  if (m_IGTLDevice.IsNull())
    throw std::invalid_argument("mitk::IGTLDeviceSource: "
                                "No OpenIGTLink device set");
  if (m_IGTLDevice->CloseConnection() == false)
    throw std::runtime_error("mitk::IGTLDeviceSource: Could not close connection"
                             " to OpenIGTLink device");
}

void mitk::IGTLDeviceSource::StopCommunication()
{
  if (m_IGTLDevice.IsNull())
    throw std::invalid_argument("mitk::IGTLDeviceSource: "
                                "No OpenIGTLink device set");
  if (m_IGTLDevice->StopCommunication() == false)
    throw std::runtime_error("mitk::IGTLDeviceSource: "
                             "Could not stop communicating");
}

void mitk::IGTLDeviceSource::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}

bool mitk::IGTLDeviceSource::IsConnected()
{
  if (m_IGTLDevice.IsNull())
    return false;

  return (m_IGTLDevice->GetState() == mitk::IGTLDevice::Ready) ||
         (m_IGTLDevice->GetState() == mitk::IGTLDevice::Running);
}

bool mitk::IGTLDeviceSource::IsCommunicating()
{
  if (m_IGTLDevice.IsNull())
    return false;

  return m_IGTLDevice->GetState() == mitk::IGTLDevice::Running;
}

void mitk::IGTLDeviceSource::RegisterAsMicroservice()
{
  // Get Context
  us::ModuleContext* context = us::GetModuleContext();

  // Define ServiceProps
  us::ServiceProperties props;
  mitk::UIDGenerator uidGen =
      mitk::UIDGenerator ("org.mitk.services.IGTLDeviceSource.id_", 16);
  props[ US_PROPKEY_ID ] = uidGen.GetUID();
  props[ US_PROPKEY_DEVICENAME ] = this->GetName();
  props[ US_PROPKEY_IGTLDEVICENAME ] = m_Name;
  m_ServiceRegistration = context->RegisterService(this, props);
}


void mitk::IGTLDeviceSource::OnIncomingMessage()
{

}
