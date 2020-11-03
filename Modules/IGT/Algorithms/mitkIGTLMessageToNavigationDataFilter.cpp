/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTLMessageToNavigationDataFilter.h"
#include "igtlTrackingDataMessage.h"
#include "igtlQuaternionTrackingDataMessage.h"
#include "igtlTransformMessage.h"
#include "mitkQuaternion.h"
#include <vnl/vnl_det.h>

mitk::IGTLMessageToNavigationDataFilter::IGTLMessageToNavigationDataFilter()
  : mitk::NavigationDataSource()
{
  mitk::NavigationData::Pointer output = mitk::NavigationData::New();
  this->SetNumberOfRequiredOutputs(1);
  this->SetNthOutput(0, output.GetPointer());
}

mitk::IGTLMessageToNavigationDataFilter::~IGTLMessageToNavigationDataFilter()
{
}

void mitk::IGTLMessageToNavigationDataFilter::SetInput(const IGTLMessage* msg)
{
  this->SetInput(0, msg);
}

void mitk::IGTLMessageToNavigationDataFilter::SetInput(unsigned int idx, const IGTLMessage* msg)
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
  this->CreateOutputsForAllInputs();
}

const mitk::IGTLMessage*
mitk::IGTLMessageToNavigationDataFilter::GetInput(void) const
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;

  return static_cast<const IGTLMessage*>(this->ProcessObject::GetInput(0));
}

const mitk::IGTLMessage*
mitk::IGTLMessageToNavigationDataFilter::GetInput(unsigned int idx) const
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;

  return static_cast<const IGTLMessage*>(this->ProcessObject::GetInput(idx));
}

const mitk::IGTLMessage*
mitk::IGTLMessageToNavigationDataFilter::GetInput(std::string messageName) const
{
  const DataObjectPointerArray& inputs = const_cast<Self*>(this)->GetInputs();
  for (DataObjectPointerArray::const_iterator it = inputs.begin();
    it != inputs.end(); ++it)
  {
    if (std::string(messageName) ==
      (static_cast<IGTLMessage*>(it->GetPointer()))->GetName())
    {
      return static_cast<IGTLMessage*>(it->GetPointer());
    }
  }
  return nullptr;
}

itk::ProcessObject::DataObjectPointerArraySizeType
mitk::IGTLMessageToNavigationDataFilter::GetInputIndex(std::string messageName)
{
  DataObjectPointerArray outputs = this->GetInputs();
  for (DataObjectPointerArray::size_type i = 0; i < outputs.size(); ++i)
  {
    if (messageName ==
      (static_cast<IGTLMessage*>(outputs.at(i).GetPointer()))->GetName())
    {
      return i;
    }
  }
  throw std::invalid_argument("output name does not exist");
}

void mitk::IGTLMessageToNavigationDataFilter::ConnectTo(
  mitk::IGTLMessageSource* UpstreamFilter)
{
  for (DataObjectPointerArraySizeType i = 0;
    i < UpstreamFilter->GetNumberOfOutputs(); i++)
  {
    this->SetInput(i, UpstreamFilter->GetOutput(i));
  }
}

void mitk::IGTLMessageToNavigationDataFilter::SetNumberOfExpectedOutputs(
  unsigned int numOutputs)
{
  this->SetNumberOfIndexedOutputs(numOutputs);
  this->CreateOutputsForAllInputs();
}

void mitk::IGTLMessageToNavigationDataFilter::CreateOutputsForAllInputs()
{
  // create outputs for all inputs
  //  this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());
  bool isModified = false;
  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedOutputs(); ++idx)
  {
    if (this->GetOutput(idx) == nullptr)
    {
      mitk::NavigationData::Pointer newOutput = mitk::NavigationData::New();
      this->SetNthOutput(idx, newOutput);
      isModified = true;
    }
  }

  if (isModified)
    this->Modified();
}

void mitk::IGTLMessageToNavigationDataFilter::GenerateTransformData()
{
  const mitk::IGTLMessage* input = this->GetInput(0);
  assert(input);

  //cast the input message into the proper type
  igtl::TransformMessage* tMsg =
    (igtl::TransformMessage*)(input->GetMessage().GetPointer());

  //check if cast was successful
  if (!tMsg)
  {
    mitkThrow() << "Cast from igtl::MessageBase to igtl::TransformMessage "
      << "failed! Please check the message.";
  }

  /* update outputs with tracking data from tools */
  for (unsigned int i = 0; i < this->GetNumberOfOutputs(); ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);

    if (input->IsDataValid() == false)
    {
      output->SetDataValid(false);
      continue;
    }

    //get the transformation matrix and convert it into an affinetransformation
    igtl::Matrix4x4 transformation_;
    tMsg->GetMatrix(transformation_);
    mitk::AffineTransform3D::Pointer affineTransformation =
      mitk::AffineTransform3D::New();
    mitk::Matrix3D transformation;
    mitk::Vector3D offset;
    for (unsigned int r = 0; r < 3; r++)
    {
      for (unsigned int c = 0; c < 3; c++)
      {
        transformation.GetVnlMatrix().set(r, c, transformation_[r][c]);
      }
      offset.SetElement(r, transformation_[r][3]);
    }
    //convert the igtl matrix here and set it in the affine transformation
    affineTransformation->SetMatrix(transformation);
    affineTransformation->SetOffset(offset);

    //create a new navigation data here, there is a neat constructor for
    //affine transformations that sets the orientation, position according to
    //the affine transformation. The other values are initialized with standard
    //values
    mitk::NavigationData::Pointer nd =
      mitk::NavigationData::New(affineTransformation, true);
    //set the time stamp
    nd->SetIGTTimeStamp(input->GetTimeStamp());
    //set the name
    nd->SetName(input->GetName());

    output->Graft(nd);
  }
}

void mitk::IGTLMessageToNavigationDataFilter::GenerateTrackingDataData()
{
  const mitk::IGTLMessage* input = this->GetInput(0);
  assert(input);

  //cast the input message into the proper type
  igtl::TrackingDataMessage* tdMsg =
    (igtl::TrackingDataMessage*)(input->GetMessage().GetPointer());

  //check if cast was successful
  if (!tdMsg)
  {
    mitkThrow() << "Cast from igtl::MessageBase to igtl::TrackingDataMessage "
      << "failed! Please check the message.";
  }

  //get the number of tracking data elements
  unsigned int numTrackingDataElements =
    tdMsg->GetNumberOfTrackingDataElements();

  if (!numTrackingDataElements)
  {
    MITK_ERROR("IGTLMsgToNavDataFilter") << "There are no tracking data "
      "elements in this message";
  }

  /* update outputs with tracking data from tools */
  for (unsigned int i = 0; i < this->GetNumberOfOutputs(); ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);

    //invalidate the output
    output->SetDataValid(false);

    //check if the current index, all outputs that have no corresponding input
    //tracking element stay invalidated, the others are validated according to
    //the tracking element
    if (input->IsDataValid() == false) { continue; }
    output->SetDataValid(true);

    //get the tracking data element which holds all the data
    igtl::TrackingDataElement::Pointer td;
    tdMsg->GetTrackingDataElement(i, td);

    //get the transformation matrix and convert it into an affinetransformation
    igtl::Matrix4x4 transformation_;
    td->GetMatrix(transformation_);
    mitk::AffineTransform3D::Pointer affineTransformation =
      mitk::AffineTransform3D::New();
    mitk::Matrix3D transformation;
    mitk::Vector3D offset;
    for (unsigned int r = 0; r < 3; r++)
    {
      for (unsigned int c = 0; c < 3; c++)
      {
        transformation.GetVnlMatrix().set(r, c, transformation_[r][c]);
      }
      offset.SetElement(r, transformation_[r][3]);
    }
    //convert the igtl matrix here and set it in the affine transformation
    affineTransformation->SetMatrix(transformation);
    affineTransformation->SetOffset(offset);

    mitk::NavigationData::Pointer nd;

    //check the rotation matrix
    vnl_matrix_fixed<ScalarType, 3, 3> rotationMatrix =
      affineTransformation->GetMatrix().GetVnlMatrix();
    vnl_matrix_fixed<ScalarType, 3, 3> rotationMatrixTransposed =
      rotationMatrix.transpose();
    // a quadratic matrix is a rotation matrix exactly when determinant is 1
    // and transposed is inverse
    if (!Equal(1.0, vnl_det(rotationMatrix), 0.1)
      || !((rotationMatrix*rotationMatrixTransposed).is_identity(0.1)))
    {
      MITK_ERROR("IGTLMsgToNavDataFilter") << "tried to initialize NavData "
        << "with non-rotation matrix :" << rotationMatrix << " (Does your "
        "AffineTransform3D object include spacing? This is not "
        "supported by NavigationData objects!)";
      nd = mitk::NavigationData::New();
    }
    else
    {
      //create a new navigation data here, there is a neat constructor for
      //affine transformations that sets the orientation, position according to
      //the affine transformation. The other values are initialized with standard
      //values
      nd = mitk::NavigationData::New(affineTransformation, true);
    }
    //set the time stamp
    nd->SetIGTTimeStamp(input->GetIGTTimeStamp());
    //set the name
    nd->SetName(td->GetName());
    output->Graft(nd);
  }
}

void
mitk::IGTLMessageToNavigationDataFilter::GenerateQuaternionTrackingDataData()
{
  const mitk::IGTLMessage* input = this->GetInput(0);
  assert(input);

  //cast the input message into the proper type
  igtl::QuaternionTrackingDataMessage* tdMsg =
    (igtl::QuaternionTrackingDataMessage*)(input->GetMessage().GetPointer());

  //check if cast was successful
  if (!tdMsg)
  {
    mitkThrow() << "Cast from igtl::MessageBase to igtl::TrackingDataMessage "
      << "failed! Please check the message.";
  }

  //get the number of tracking data elements
  unsigned int numTrackingDataElements =
    tdMsg->GetNumberOfQuaternionTrackingDataElements();

  if (!numTrackingDataElements)
  {
    MITK_ERROR("IGTLMsgToNavDataFilter") << "There are no tracking data "
      "elements in this message";
  }

  /* update outputs with tracking data from tools */
  for (unsigned int i = 0; i < this->GetNumberOfOutputs(); ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);

    //invalidate the output
    output->SetDataValid(false);

    //check if the current index, all outputs that have no corresponding input
    //tracking element stay invalidated, the others are validated according to
    //the tracking element
    if (input->IsDataValid() == false || i >= numTrackingDataElements)
    {
      continue;
    }
    output->SetDataValid(true);

    //get the tracking data element which holds all the data
    igtl::QuaternionTrackingDataElement::Pointer td;
    tdMsg->GetQuaternionTrackingDataElement(i, td);

    //get the quaternion and set it
    float quaternion_[4];                          //igtl quat type
    td->GetQuaternion(quaternion_);
    mitk::Quaternion quaternion;
    quaternion.put(0, quaternion_[0]);
    quaternion.put(1, quaternion_[1]);
    quaternion.put(2, quaternion_[2]);
    quaternion.put(3, quaternion_[3]);
    output->SetOrientation(quaternion);
    output->SetHasOrientation(true);

    //get the position and set it
    float position_[3];                          //igtl position type
    td->GetPosition(position_);
    mitk::NavigationData::PositionType position; //mitk position type
    position.SetElement(0, position_[0]);
    position.SetElement(1, position_[1]);
    position.SetElement(2, position_[2]);
    output->SetPosition(position);
    output->SetHasPosition(true);
    //set the time stamp
    output->SetIGTTimeStamp(input->GetTimeStamp());
    //set the name
    output->SetName(td->GetName());

    //there is no explicit covarience matrix
    output->SetCovErrorMatrix(mitk::NavigationData::CovarianceMatrixType());
  }
}

void mitk::IGTLMessageToNavigationDataFilter::GenerateData()
{
  //get the IGTLMessage from the previous filter
  const mitk::IGTLMessage* input = this->GetInput(0);
  assert(input);

  //check if the message is valid, if it is not valid we do not generate new
  //outputs
  if (!input->IsDataValid())
  {
    MITK_DEBUG("IGTLMessageToNavigationDataFilter") << "Input data is invalid.";
    return;
  }

  //get the message type
  const char* msgType = input->GetIGTLMessageType();

  //check if the IGTL message has the proper type
  if (strcmp(msgType, "TRANSFORM") == 0)
  {
    this->GenerateTransformData();
  }
  else if (strcmp(msgType, "TDATA") == 0)
  {
    this->GenerateTrackingDataData();
  }
  else if (strcmp(msgType, "QTDATA") == 0)
  {
    this->GenerateQuaternionTrackingDataData();
  }
  else
  {
    //the message has another type
    //ignore
    MITK_INFO("IGTLMessageToNavigationDataFilter") << "The input has a unknown "
      << "message type: "
      << msgType;
  }
}

void mitk::IGTLMessageToNavigationDataFilter::GenerateOutputInformation()
{
  //  Superclass::GenerateOutputInformation();

  //  mitk::NavigationData* output = this->GetOutput(0);
  //  assert(output);
  //  const mitk::IGTLMessage* input = this->GetInput(0);
  //  assert(input);

  itkDebugMacro(<< "GenerateOutputInformation()");

  //  output->Initialize(input->GetPixelType(), input->GetDimension(), input->GetDimensions());

  //  // initialize geometry
  //  output->SetPropertyList(input->GetPropertyList()->Clone());
  //  mitk::TimeGeometry::Pointer clonGeometry = input->GetTimeGeometry()->Clone();
  //  output->SetTimeGeometry(clonGeometry.GetPointer());
}
