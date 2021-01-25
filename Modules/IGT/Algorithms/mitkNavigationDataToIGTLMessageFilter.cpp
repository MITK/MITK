/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNavigationDataToIGTLMessageFilter.h"

#include "igtlQuaternionTrackingDataMessage.h"
#include "igtlTrackingDataMessage.h"
#include "igtlTransformMessage.h"
#include "igtlPositionMessage.h"

#include <mitkInteractionConst.h>
#include <itksys/SystemTools.hxx>

mitk::NavigationDataToIGTLMessageFilter::NavigationDataToIGTLMessageFilter()
{
  mitk::IGTLMessage::Pointer output = mitk::IGTLMessage::New();
  this->SetNumberOfRequiredOutputs(1);
  this->SetNthOutput(0, output.GetPointer());
  this->SetNumberOfRequiredInputs(1);

  //  m_OperationMode = Mode3D;
  m_CurrentTimeStep = 0;
  //  m_RingBufferSize = 50; //the default ring buffer size
  //  m_NumberForMean = 100;
}

mitk::NavigationDataToIGTLMessageFilter::~NavigationDataToIGTLMessageFilter()
{
}

void mitk::NavigationDataToIGTLMessageFilter::GenerateData()
{
  switch (m_OperationMode)
  {
  case ModeSendQTDataMsg:
    this->GenerateDataModeSendQTDataMsg();
    break;
  case ModeSendTDataMsg:
    this->GenerateDataModeSendTDataMsg();
    break;
  case ModeSendQTransMsg:
    this->GenerateDataModeSendQTransMsg();
    break;
  case ModeSendTransMsg:
    this->GenerateDataModeSendTransMsg();
    break;
  default:
    break;
  }
}

void mitk::NavigationDataToIGTLMessageFilter::SetInput(const NavigationData* nd)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast<NavigationData*>(nd));
  this->CreateOutputsForAllInputs();
}

void mitk::NavigationDataToIGTLMessageFilter::SetInput(unsigned int idx, const NavigationData* nd)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(idx, const_cast<NavigationData*>(nd));
  this->CreateOutputsForAllInputs();
}

const mitk::NavigationData* mitk::NavigationDataToIGTLMessageFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;
  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(0));
}

const mitk::NavigationData* mitk::NavigationDataToIGTLMessageFilter::GetInput(unsigned int idx)
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;
  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(idx));
}

void mitk::NavigationDataToIGTLMessageFilter::CreateOutputsForAllInputs()
{
  switch (m_OperationMode)
  {
  case ModeSendQTDataMsg:
    // create one message output for all navigation data inputs
    this->SetNumberOfIndexedOutputs(1);
    // set the type for this filter
    this->SetType("QTDATA");
    break;
  case ModeSendTDataMsg:
    // create one message output for all navigation data inputs
    this->SetNumberOfIndexedOutputs(1);
    // set the type for this filter
    this->SetType("TDATA");
    break;
  case ModeSendQTransMsg:
    // create one message output for all navigation data input together
    this->SetNumberOfIndexedOutputs(1);
    // set the type for this filter
    this->SetType("POSITION");
    break;
  case ModeSendTransMsg:
    // create one message output for each navigation data input
    this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());
    // set the type for this filter
    this->SetType("TRANS");
    break;
  default:
    break;
  }

  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedOutputs(); ++idx)
  {
    if (this->GetOutput(idx) == nullptr)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      this->SetNthOutput(idx, newOutput);
    }
    this->Modified();
  }
}

void ConvertAffineTransformationIntoIGTLMatrix(mitk::AffineTransform3D* trans,
  igtl::Matrix4x4 igtlTransform)
{
  const mitk::AffineTransform3D::MatrixType& matrix = trans->GetMatrix();
  mitk::Vector3D position = trans->GetOffset();
  //copy the data into a matrix type that igtl understands
  for (unsigned int r = 0; r < 3; r++)
  {
    for (unsigned int c = 0; c < 3; c++)
    {
      igtlTransform[r][c] = matrix(r, c);
    }
    igtlTransform[r][3] = position[r];
  }
  for (unsigned int c = 0; c < 3; c++)
  {
    igtlTransform[3][c] = 0.0;
  }
  igtlTransform[3][3] = 1.0;
}

void mitk::NavigationDataToIGTLMessageFilter::GenerateDataModeSendQTransMsg()
{
  // for each output message
  for (unsigned int i = 0; i < this->GetNumberOfIndexedInputs(); ++i)
  {
    mitk::IGTLMessage* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);
    // do not add navigation data to message if input is invalid
    if (input->IsDataValid() == false)
      continue;

    //get the navigation data components
    mitk::NavigationData::PositionType pos = input->GetPosition();
    mitk::NavigationData::OrientationType ori = input->GetOrientation();

    //insert this information into the message
    igtl::PositionMessage::Pointer posMsg = igtl::PositionMessage::New();
    posMsg->SetPosition(pos[0], pos[1], pos[2]);
    posMsg->SetQuaternion(ori[0], ori[1], ori[2], ori[3]);
    igtl::TimeStamp::Pointer timestamp = ConvertToIGTLTimeStamp(input->GetIGTTimeStamp());
    posMsg->SetTimeStamp(timestamp);
    posMsg->SetDeviceName(input->GetName());
    posMsg->Pack();

    //add the igtl message to the mitk::IGTLMessage
    output->SetMessage(posMsg.GetPointer());
  }
}

void mitk::NavigationDataToIGTLMessageFilter::GenerateDataModeSendTransMsg()
{
  // for each output message
  for (unsigned int i = 0; i < this->GetNumberOfIndexedInputs(); ++i)
  {
    mitk::IGTLMessage* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);
    // do not add navigation data to message if input is invalid
    if (input->IsDataValid() == false)
      continue;

    //get the navigation data components
    mitk::AffineTransform3D::Pointer transform = input->GetAffineTransform3D();
    mitk::NavigationData::PositionType position = Point3D(transform->GetOffset());

    //convert the transform into a igtl type
    igtl::Matrix4x4 igtlTransform;
    ConvertAffineTransformationIntoIGTLMatrix(transform, igtlTransform);

    //insert this information into the message
    igtl::TransformMessage::Pointer transMsg = igtl::TransformMessage::New();
    transMsg->SetMatrix(igtlTransform);
    transMsg->SetPosition(position[0], position[1], position[2]);
    igtl::TimeStamp::Pointer timestamp = ConvertToIGTLTimeStamp(input->GetIGTTimeStamp());
    transMsg->SetTimeStamp(timestamp);
    transMsg->SetDeviceName(input->GetName());
    transMsg->Pack();

    //add the igtl message to the mitk::IGTLMessage
    output->SetMessage(transMsg.GetPointer());
  }
}
igtl::TimeStamp::Pointer mitk::NavigationDataToIGTLMessageFilter::ConvertToIGTLTimeStamp(double IGTTimeStamp)
{
  igtl::TimeStamp::Pointer timestamp = igtl::TimeStamp::New();
  timestamp->SetTime(IGTTimeStamp / 1000, (int)(IGTTimeStamp) % 1000);
  return timestamp;
}

void mitk::NavigationDataToIGTLMessageFilter::GenerateDataModeSendQTDataMsg()
{
  mitk::IGTLMessage* output = this->GetOutput();
  assert(output);

  //create a output igtl message
  igtl::QuaternionTrackingDataMessage::Pointer qtdMsg =
    igtl::QuaternionTrackingDataMessage::New();

  mitk::NavigationData::PositionType pos;
  mitk::NavigationData::OrientationType ori;

  for (unsigned int index = 0; index < this->GetNumberOfIndexedInputs(); index++)
  {
    const mitk::NavigationData* nd = GetInput(index);
    assert(nd);

    //get the navigation data components
    pos = nd->GetPosition();
    ori = nd->GetOrientation();

    //insert the information into the tracking element
    igtl::QuaternionTrackingDataElement::Pointer tde =
      igtl::QuaternionTrackingDataElement::New();
    tde->SetPosition(pos[0], pos[1], pos[2]);
    tde->SetQuaternion(ori[0], ori[1], ori[2], ori[3]);
    tde->SetName(nd->GetName());

    //insert this element into the tracking data message
    qtdMsg->AddQuaternionTrackingDataElement(tde);

    MITK_INFO << ConvertToIGTLTimeStamp(nd->GetIGTTimeStamp());
  }
  qtdMsg->Pack();

  //add the igtl message to the mitk::IGTLMessage
  output->SetMessage(qtdMsg.GetPointer());
}

void mitk::NavigationDataToIGTLMessageFilter::GenerateDataModeSendTDataMsg()
{
  igtl::TrackingDataMessage::Pointer tdMsg = igtl::TrackingDataMessage::New();
  mitk::IGTLMessage* output = this->GetOutput(0);
  assert(output);

  // for each output message
  for (unsigned int i = 0; i < this->GetNumberOfIndexedInputs(); ++i)
  {
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);
    // do not add navigation data to message if input is invalid
    if (input->IsDataValid() == false)
      continue;

    //get the navigation data components
    mitk::AffineTransform3D::Pointer transform = input->GetAffineTransform3D();
    mitk::NavigationData::PositionType position = Point3D(transform->GetOffset());

    //convert the transform into a igtl type
    igtl::Matrix4x4 igtlTransform;
    ConvertAffineTransformationIntoIGTLMatrix(transform, igtlTransform);

    //insert this information into the message
    igtl::TrackingDataElement::Pointer tde = igtl::TrackingDataElement::New();
    tde->SetMatrix(igtlTransform);
    tde->SetPosition(position[0], position[1], position[2]);
    tde->SetName(input->GetName());
    tde->SetType(igtl::TrackingDataElement::TYPE_6D);
    tdMsg->AddTrackingDataElement(tde);
  }

  //use time stamp from first input
  igtl::TimeStamp::Pointer timestamp = ConvertToIGTLTimeStamp(this->GetInput(0)->GetIGTTimeStamp());
  tdMsg->SetTimeStamp(timestamp);
  //tdMsg->SetDeviceName("MITK OpenIGTLink Connection");
  tdMsg->Pack();
  tdMsg->SetDeviceName("MITK OpenIGTLink Source");
  output->SetMessage(tdMsg.GetPointer());
}

void mitk::NavigationDataToIGTLMessageFilter::SetOperationMode(OperationMode mode)
{
  m_OperationMode = mode;
  this->Modified();
}

void mitk::NavigationDataToIGTLMessageFilter::ConnectTo(
  mitk::NavigationDataSource* UpstreamFilter)
{
  for (DataObjectPointerArraySizeType i = 0;
    i < UpstreamFilter->GetNumberOfOutputs(); i++)
  {
    this->SetInput(i, UpstreamFilter->GetOutput(i));
  }
}
