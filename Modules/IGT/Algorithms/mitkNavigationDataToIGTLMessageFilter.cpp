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


const mitk::NavigationData* mitk::NavigationDataToIGTLMessageFilter::GetInput( void )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(0));
}


const mitk::NavigationData* mitk::NavigationDataToIGTLMessageFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(idx));
}


void mitk::NavigationDataToIGTLMessageFilter::CreateOutputsForAllInputs()
{
  switch (m_OperationMode)
  {
  case ModeSendQTDataMsg:
    // create one message output for all navigation data inputs
    this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());
    // set the type for this filter
    this->SetType("QTDATA");
    break;
  case ModeSendTDataMsg:
    // create one message output for all navigation data inputs
    this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());
    // set the type for this filter
    this->SetType("TDATA");
    break;
  case ModeSendQTransMsg:
    // create one message output for all navigation data input together
    this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());
    // set the type for this filter
    this->SetType("POSITION");
    break;
  case ModeSendTransMsg:
    // create one message output for all navigation data input together
    this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());
    // set the type for this filter
    this->SetType("TRANS");
    break;
  default:
    break;
  }

  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedOutputs(); ++idx)
  {
    if (this->GetOutput(idx) == NULL)
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
  for ( unsigned int r = 0; r < 3; r++ )
  {
    for ( unsigned int c = 0; c < 3; c++ )
    {
      igtlTransform[r][c] = matrix(r,c);
    }
    igtlTransform[r][3] = position[r];
  }
  for ( unsigned int c = 0; c < 3; c++ )
  {
    igtlTransform[3][c] = 0.0;
  }
  igtlTransform[3][3] = 1.0;
}

void mitk::NavigationDataToIGTLMessageFilter::GenerateDataModeSendQTransMsg()
{
  // for each output message
  for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs() ; ++i)
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
    posMsg->SetTimeStamp((unsigned int)input->GetIGTTimeStamp(), 0);
    posMsg->SetDeviceName(input->GetName());
    posMsg->Pack();

    //add the igtl message to the mitk::IGTLMessage
    output->SetMessage(posMsg.GetPointer());
  }
}

void mitk::NavigationDataToIGTLMessageFilter::GenerateDataModeSendTransMsg()
{
  // for each output message
  for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs() ; ++i)
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
    mitk::NavigationData::PositionType position = transform->GetOffset();

    //convert the transform into a igtl type
    igtl::Matrix4x4 igtlTransform;
    ConvertAffineTransformationIntoIGTLMatrix(transform, igtlTransform);

    //insert this information into the message
    igtl::TransformMessage::Pointer transMsg = igtl::TransformMessage::New();
    transMsg->SetMatrix(igtlTransform);
    transMsg->SetPosition(position[0], position[1], position[2]);
    transMsg->SetTimeStamp((unsigned int)input->GetIGTTimeStamp(), 0);
    transMsg->SetDeviceName(input->GetName());
    transMsg->Pack();

    //add the igtl message to the mitk::IGTLMessage
    output->SetMessage(transMsg.GetPointer());
  }
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

    //copy the time stamp
    //todo find a better way to do that
    qtdMsg->SetTimeStamp((unsigned int)nd->GetIGTTimeStamp(), 0);
  }
  qtdMsg->Pack();

  //add the igtl message to the mitk::IGTLMessage
  output->SetMessage(qtdMsg.GetPointer());
}

void mitk::NavigationDataToIGTLMessageFilter::GenerateDataModeSendTDataMsg()
{
  bool isValidData = true;
  mitk::IGTLMessage* output = this->GetOutput();
  assert(output);

  //create a output igtl message
  igtl::TrackingDataMessage::Pointer tdMsg = igtl::TrackingDataMessage::New();

  mitk::AffineTransform3D::Pointer transform;
  Vector3D position;
  igtl::Matrix4x4 igtlTransform;
  vnl_matrix_fixed<ScalarType, 3, 3> rotationMatrix;
  vnl_matrix_fixed<ScalarType, 3, 3> rotationMatrixTransposed;

  for (unsigned int index = 0; index < this->GetNumberOfIndexedInputs(); index++)
  {
    const mitk::NavigationData* nd = GetInput(index);
    assert(nd);

    //create a new tracking element
    igtl::TrackingDataElement::Pointer tde = igtl::TrackingDataElement::New();

    //get the navigation data components
    transform = nd->GetAffineTransform3D();
    position = transform->GetOffset();

    //check the rotation matrix
    rotationMatrix = transform->GetMatrix().GetVnlMatrix();
    rotationMatrixTransposed = rotationMatrix.transpose();
    // a quadratic matrix is a rotation matrix exactly when determinant is 1
    // and transposed is inverse
    if (!Equal(1.0, vnl_det(rotationMatrix), 0.1)
        || !((rotationMatrix*rotationMatrixTransposed).is_identity(0.1)))
    {
      //the rotation matrix is not valid! => invalidate the current element
      isValidData = false;
    }

    //convert the transform into a igtl type
    ConvertAffineTransformationIntoIGTLMatrix(transform, igtlTransform);

    //fill the tracking element with life
    tde->SetMatrix(igtlTransform);
    tde->SetPosition(position[0], position[1], position[2]);
    tde->SetName(nd->GetName());

    //insert this element into the tracking data message
    tdMsg->AddTrackingDataElement(tde);

    //copy the time stamp
    //todo find a better way to do that
    tdMsg->SetTimeStamp((unsigned int)nd->GetIGTTimeStamp(), 0);
  }
  tdMsg->Pack();
  //add the igtl message to the mitk::IGTLMessage
  output->SetMessage(tdMsg.GetPointer());
  output->SetDataValid(isValidData);
}

void mitk::NavigationDataToIGTLMessageFilter::SetOperationMode( OperationMode mode )
{
  m_OperationMode = mode;
  this->Modified();
  this->CreateOutputsForAllInputs();
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
