/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  : mitk::IGTLMessageSource(), m_IGTLDevice(nullptr)
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
    this->RemoveObservers();
    m_IGTLDevice = nullptr;
  }
}

void mitk::IGTLDeviceSource::GenerateData()
{
  if (m_IGTLDevice.IsNull())
    return;

  /* update output with message from the device */
  IGTLMessage* msgOut = this->GetOutput();
  assert(msgOut);
  igtl::MessageBase::Pointer msgIn = dynamic_cast<igtl::MessageBase*>(m_IGTLDevice->GetNextImage2dMessage().GetPointer());
  if (msgIn.IsNotNull())
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

void mitk::IGTLDeviceSource::RemoveObservers()
{
  if (this->m_IGTLDevice.IsNotNull())
  {
    this->m_IGTLDevice->RemoveObserver(m_IncomingMessageObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_IncomingCommandObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_LostConnectionObserverTag);
  }
}

void mitk::IGTLDeviceSource::SetIGTLDevice(mitk::IGTLDevice* igtlDevice)
{
  MITK_DEBUG << "Setting IGTLDevice to " << igtlDevice;
  if (this->m_IGTLDevice.GetPointer() != igtlDevice)
  {
    //check if we want to override the device
    if (this->m_IGTLDevice.IsNotNull())
    {
      //the device was set previously => we need to reset the observers
      this->RemoveObservers();
    }
    //set the device
    this->m_IGTLDevice = igtlDevice;
    this->CreateOutputs();
    std::stringstream name; // create a human readable name for the source
    name << "OIGTL Device Source ( " << igtlDevice->GetName() << " )";
    this->SetName(name.str());

    //setup a observer that listens to new messages and new commands
    typedef itk::SimpleMemberCommand<IGTLDeviceSource> DeviceSrcCommand;

    DeviceSrcCommand::Pointer msgReceivedCommand = DeviceSrcCommand::New();
    msgReceivedCommand->SetCallbackFunction(this, &IGTLDeviceSource::OnIncomingMessage);
    this->m_IncomingMessageObserverTag =
      this->m_IGTLDevice->AddObserver(mitk::MessageReceivedEvent(), msgReceivedCommand);

    DeviceSrcCommand::Pointer cmdReceivedCommand = DeviceSrcCommand::New();
    cmdReceivedCommand->SetCallbackFunction(this, &IGTLDeviceSource::OnIncomingCommand);
    this->m_IncomingCommandObserverTag =
      this->m_IGTLDevice->AddObserver(mitk::CommandReceivedEvent(), cmdReceivedCommand);

    DeviceSrcCommand::Pointer connectionLostCommand = DeviceSrcCommand::New();
    connectionLostCommand->SetCallbackFunction(this, &IGTLDeviceSource::OnLostConnection);
    this->m_LostConnectionObserverTag =
      this->m_IGTLDevice->AddObserver(mitk::LostConnectionEvent(), connectionLostCommand);
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
  if (this->GetOutput(0) == nullptr)
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

void mitk::IGTLDeviceSource::SetInput(unsigned int idx, const IGTLMessage* msg)
{
  if (msg == nullptr) // if an input is set to nullptr, remove it
  {
    this->RemoveInput(idx);
  }
  else
  {
    // ProcessObject is not const-correct so a const_cast is required here
    this->ProcessObject::SetNthInput(idx, const_cast<IGTLMessage*>(msg));
  }
  //  this->CreateOutputsForAllInputs();
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
    mitk::UIDGenerator("org.mitk.services.IGTLDeviceSource.id_");
  props[US_PROPKEY_ID] = uidGen.GetUID();
  props[US_PROPKEY_DEVICENAME] = this->GetName();
  props[US_PROPKEY_IGTLDEVICENAME] = m_Name;
  props[US_PROPKEY_DEVICETYPE] = m_Type;
  m_ServiceRegistration = context->RegisterService(this, props);

  MITK_INFO << "Registered new DeviceSource as microservice: " << uidGen.GetUID();
}

void mitk::IGTLDeviceSource::OnIncomingMessage()
{
}

void mitk::IGTLDeviceSource::OnIncomingCommand()
{
}

void mitk::IGTLDeviceSource::OnLostConnection()
{
}

const mitk::IGTLMessage* mitk::IGTLDeviceSource::GetInput(void) const
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;

  return static_cast<const IGTLMessage*>(this->ProcessObject::GetInput(0));
}

const mitk::IGTLMessage*
mitk::IGTLDeviceSource::GetInput(unsigned int idx) const
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;

  return static_cast<const IGTLMessage*>(this->ProcessObject::GetInput(idx));
}

const mitk::IGTLMessage*
mitk::IGTLDeviceSource::GetInput(std::string msgName) const
{
  const DataObjectPointerArray& inputs = const_cast<Self*>(this)->GetInputs();
  for (DataObjectPointerArray::const_iterator it = inputs.begin();
    it != inputs.end(); ++it)
    if (std::string(msgName) ==
      (static_cast<IGTLMessage*>(it->GetPointer()))->GetName())
      return static_cast<IGTLMessage*>(it->GetPointer());
  return nullptr;
}

itk::ProcessObject::DataObjectPointerArraySizeType
mitk::IGTLDeviceSource::GetInputIndex(std::string msgName)
{
  DataObjectPointerArray outputs = this->GetInputs();
  for (DataObjectPointerArray::size_type i = 0; i < outputs.size(); ++i)
    if (msgName ==
      (static_cast<IGTLMessage*>(outputs.at(i).GetPointer()))->GetName())
      return i;
  throw std::invalid_argument("output name does not exist");
}
