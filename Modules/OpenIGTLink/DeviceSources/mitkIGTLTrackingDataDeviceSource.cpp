/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTLTrackingDataDeviceSource.h"

#include "mitkIGTLMessage.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

//itk
#include <itkCommand.h>

mitk::IGTLTrackingDataDeviceSource::IGTLTrackingDataDeviceSource()
  : mitk::IGTLDeviceSource() , m_trackingDataType(UNKNOWN)
{
  this->SetName("IGTLDeviceSource (tracking data)");
}

mitk::IGTLTrackingDataDeviceSource::~IGTLTrackingDataDeviceSource()
{
}

void mitk::IGTLTrackingDataDeviceSource::GenerateData()
{
  if (m_IGTLDevice.IsNull())
    return;

  /* update output with message from the device */
  IGTLMessage* msgOut = this->GetOutput();
  assert(msgOut);
  igtl::MessageBase::Pointer msgIn;
  switch (m_trackingDataType)
  {
  case UNKNOWN:
    {
    igtl::MessageBase::Pointer msgInTDATA = dynamic_cast<igtl::MessageBase*>(m_IGTLDevice->GetNextTrackingDataMessage().GetPointer());
    igtl::MessageBase::Pointer msgInTRANSFORM = dynamic_cast<igtl::MessageBase*>(m_IGTLDevice->GetNextTransformMessage().GetPointer());
    if (msgInTDATA.IsNull() && msgInTRANSFORM.IsNotNull()) { msgIn = msgInTRANSFORM; }
    else if (msgInTDATA.IsNotNull() && msgInTRANSFORM.IsNull()) { msgIn = msgInTDATA; }
    else if (msgInTDATA.IsNotNull() && msgInTRANSFORM.IsNotNull())
      {
      MITK_INFO << "Found both: TDATA and TRANSFORM messages. Using TRANSFORM as default.";
      }
    }
    break;
  case TDATA:
    msgIn = dynamic_cast<igtl::MessageBase*>(m_IGTLDevice->GetNextTrackingDataMessage().GetPointer());
    break;
  case QTDATA:
    MITK_WARN << "Receiving QTDATA is not implemented yet!";
    break;
  case TRANSFORM:
    msgIn = dynamic_cast<igtl::MessageBase*>(m_IGTLDevice->GetNextTransformMessage().GetPointer());
    break;
  }

  if (msgIn.IsNotNull())
  {
    assert(msgIn);
    msgOut->SetMessage(msgIn);
    msgOut->SetName(msgIn->GetDeviceName());
  }
}
