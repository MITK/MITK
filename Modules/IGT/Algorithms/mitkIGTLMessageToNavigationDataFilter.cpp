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

#include "mitkIGTLMessageToNavigationDataFilter.h"
#include "igtlTrackingDataMessage.h"
#include "igtlQuaternionTrackingDataMessage.h"
#include "igtlTransformMessage.h"
#include "mitkQuaternion.h"


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


void mitk::IGTLMessageToNavigationDataFilter::SetInput( const IGTLMessage* msg )
{
  this->SetInput(0, msg);
}


void mitk::IGTLMessageToNavigationDataFilter::SetInput( unsigned int idx, const IGTLMessage* msg )
{
  if ( msg == NULL ) // if an input is set to NULL, remove it
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
mitk::IGTLMessageToNavigationDataFilter::GetInput( void ) const
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;

  return static_cast<const IGTLMessage*>(this->ProcessObject::GetInput(0));
}


const mitk::IGTLMessage*
mitk::IGTLMessageToNavigationDataFilter::GetInput( unsigned int idx ) const
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;

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
  return NULL;
}


itk::ProcessObject::DataObjectPointerArraySizeType
mitk::IGTLMessageToNavigationDataFilter::GetInputIndex( std::string messageName )
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


void mitk::IGTLMessageToNavigationDataFilter::CreateOutputsForAllInputs()
{
  // create outputs for all inputs
  this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());
  bool isModified = false;
  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedOutputs(); ++idx)
  {
    if (this->GetOutput(idx) == NULL)
    {
      mitk::NavigationData::Pointer newOutput = mitk::NavigationData::New();
      this->SetNthOutput(idx, newOutput);
      isModified = true;
    }
  }

  if(isModified)
    this->Modified();
}

void mitk::IGTLMessageToNavigationDataFilter::GenerateData()
{
  this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs

  /* update outputs with tracking data from tools */
  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);
    const mitk::IGTLMessage* input = this->GetInput(i);
    assert(input);

    if (input->IsDataValid() == false)
    {
      output->SetDataValid(false);
      continue;
    }

    //check if the IGTL message has the proper type
    if( strcmp(input->GetIGTLMessageType(), "TRANSFORM") == 0 )
    {
      //get the tracking data message
//      igtl::TransformMessage::Pointer tMsg =
//        dynamic_cast<igtl::TransformMessage*>(test.GetPointer());

      igtl::TransformMessage* tMsg =
          (igtl::TransformMessage*)(input->GetMessage().GetPointer());

//      igtl::TransformMessage::Pointer tMsg =
//          igtl::TransformMessage::Pointer(input->GetMessage().GetPointer());
////      tMsg->Copy(test);

      //check if cast was successful
      if ( !tMsg )
      {
        mitkThrow() << "Cast from igtl::MessageBase to igtl::TransformMessage "
                    << "failed! Please check the message.";
        continue;
      }

      tMsg->Print(std::cout);

      //get the transformation matrix and convert it into an affinetransformation
      igtl::Matrix4x4 transformation_;
      tMsg->GetMatrix(transformation_);
      mitk::AffineTransform3D::Pointer affineTransformation =
          mitk::AffineTransform3D::New();
      mitk::Matrix3D transformation;
      mitk::Vector3D offset;
      for ( unsigned int r = 0; r < 3; r++ )
      {
        for ( unsigned int c = 0; c < 3; c++ )
        {
          transformation.GetVnlMatrix().set( r , c , transformation_[r][c] );
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
      output->Graft(nd);

      nd->Print(std::cout);
    }
    else if( strcmp(input->GetIGTLMessageType(), "TDATA") == 0 )
    {
      //get the tracking data message
      igtl::TrackingDataMessage::Pointer tdMsg =
          dynamic_cast<igtl::TrackingDataMessage*>(
            input->GetMessage().GetPointer());

      //get the number of tracking data elements
      unsigned int numTrackingDataElements =
          tdMsg->GetNumberOfTrackingDataElements();

      //one message can contain several tracking data elements, but is this
      //really used? If yes, what is a tracking element? If there is one tracking
      //element per connected tracker, then we definitely have check all of them.
      //Moreover, we have to read the message, check how many elements there are
      //and then adapt the number of outputs to the number of tracking element.
      //Every tracker should have a seperate output.
      //at the moment I just want the first one
      //check if there is at least one tracking data element
      if ( !numTrackingDataElements )
      {
        output->SetDataValid(false);
        MITK_ERROR("IGTLMsgToNavDataFilter") << "There are no tracking data "
                                                "elements in this message";
        continue;
      }

      //get the tracking data element which holds all the data
      igtl::TrackingDataElement::Pointer td;
      tdMsg->GetTrackingDataElement(0, td);

      //get the transformation matrix and convert it into an affinetransformation
      igtl::Matrix4x4 transformation_;
      td->GetMatrix(transformation_);
      mitk::AffineTransform3D::Pointer affineTransformation;
      mitk::Matrix3D transformation;
      mitk::Vector3D offset;
      for ( unsigned int r = 0; r < 3; r++ )
      {
        for ( unsigned int c = 0; c < 3; c++ )
        {
          transformation.GetVnlMatrix().set( r , c , transformation_[r][c] );
        }
        offset.SetElement(r, transformation_[r][4]);
      }
      //convert the igtl matrix here and set it in the affine transformation
      affineTransformation->SetMatrix(transformation);
      affineTransformation->SetOffset(offset);

//      //get the position and set it in the affine transformation
//      mitk::NavigationData::PositionType position;
//      td->GetPosition(position.pointer);
//      affineTransformation->SetOffset(position);

      //create a new navigation data here, there is a neat constructor for
      //affine transformations that sets the orientation, position according to
      //the affine transformation. The other values are initialized with standard
      //values
      mitk::NavigationData::Pointer nd =
          mitk::NavigationData::New(affineTransformation, true);
      output->Graft(nd);
    }
    else if( strcmp(input->GetIGTLMessageType(), "QTDATA") == 0 )
    {
      //get the tracking data message
      igtl::QuaternionTrackingDataMessage::Pointer tdMsg =
          dynamic_cast<igtl::QuaternionTrackingDataMessage*>(
            input->GetMessage().GetPointer());

      //get the tracking data element which holds all the data
      igtl::QuaternionTrackingDataElement::Pointer td;
      tdMsg->GetQuaternionTrackingDataElement(0, td);

      //get the quaternion and set it
      float quaternion_[4];                          //igtl quat type
      td->GetQuaternion(quaternion_);
      mitk::Quaternion quaternion;
      quaternion.put(0, quaternion_[0]);
      quaternion.put(1, quaternion_[1]);
      quaternion.put(2, quaternion_[2]);
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

      //the data is valid and there is no explicit covarience matrix
      output->SetDataValid(true);
      output->SetCovErrorMatrix(mitk::NavigationData::CovarianceMatrixType());
    }
    else
    {
      //the message has another type
      //set the output invalid and try the next input
      output->SetDataValid(false);
      continue;
    }

    //set the time stamp
    output->SetIGTTimeStamp(input->GetTimeStamp());
    //set the name
    output->SetName(input->GetName());

    output->Print(std::cout);
  }
}


void mitk::IGTLMessageToNavigationDataFilter::GenerateOutputInformation()
{
//  Superclass::GenerateOutputInformation();

//  mitk::NavigationData* output = this->GetOutput(0);
//  assert(output);
//  const mitk::IGTLMessage* input = this->GetInput(0);
//  assert(input);

  itkDebugMacro(<<"GenerateOutputInformation()");

//  output->Initialize(input->GetPixelType(), input->GetDimension(), input->GetDimensions());

//  // initialize geometry
//  output->SetPropertyList(input->GetPropertyList()->Clone());
//  mitk::TimeGeometry::Pointer clonGeometry = input->GetTimeGeometry()->Clone();
//  output->SetTimeGeometry(clonGeometry.GetPointer());
}
