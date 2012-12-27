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
#include "mitkNavigationDataHandEyeCalibrationFilter.h"

#include "mitkDataStorage.h"


mitk::NavigationDataHandEyeCalibrationFilter::NavigationDataHandEyeCalibrationFilter()
: NavigationDataToNavigationDataFilter(),
m_ObjectsToTransform(),m_HandEyeTransformations()
{
}


mitk::NavigationDataHandEyeCalibrationFilter::~NavigationDataHandEyeCalibrationFilter()
{
  m_ObjectsToTransform.clear();
  m_HandEyeTransformations.clear();
}

void mitk::NavigationDataHandEyeCalibrationFilter::SetObjectToTransform(unsigned int index, BaseData* data)
{
  m_ObjectsToTransform[index] = data;
}

void mitk::NavigationDataHandEyeCalibrationFilter::SetHandEyeTransformation(unsigned int index, mitk::Transform::Pointer handEyeTransformation)
{
  m_HandEyeTransformations[index] = handEyeTransformation;
}

void mitk::NavigationDataHandEyeCalibrationFilter::GenerateData()
{
  /*get each input, lookup the associated BaseData and transfer the data*/
  unsigned int numberOfInputs = this->GetNumberOfInputs();
  for (unsigned int index=0; index < numberOfInputs; index++)
  {
    //get the needed variables
    const mitk::NavigationData* nd = this->GetInput(index);
    assert(nd);

    mitk::NavigationData* output = this->GetOutput(index);
    assert(output);

    //check if the data is valid
    if (!nd->IsDataValid())
    {
      output->SetDataValid(false);
      continue;
    }
    output->Graft(nd); // copy all information from input to output
    // get the object to transform for current input index
    mitk::BaseData::Pointer objectToTransform = m_ObjectsToTransform[index];
    if (!objectToTransform)
    {
      continue;
    }
    // get the transformation of the hand-eye calibration for current input index
    QuaternionRigidTransformType::Pointer handEyeTransform = QuaternionRigidTransformType::New();;
    if (!m_HandEyeTransformations[index])
    {
      continue;
    }
    else
    {
      handEyeTransform->SetRotation(m_HandEyeTransformations[index]->GetOrientation());
      handEyeTransform->SetOffset(m_HandEyeTransformations[index]->GetPosition().GetVectorFromOrigin());
    }
    // get the transformation of the current navigation data
    QuaternionRigidTransformType::Pointer ndTransform = QuaternionRigidTransformType::New();
    ndTransform->SetRotation(nd->GetOrientation());
    ndTransform->SetOffset(nd->GetPosition().GetVectorFromOrigin());
    // compose hand-eye transformation and transformation of NavigationData to final transformation to be applied
    mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
    transform->SetIdentity();
    transform->Compose(handEyeTransform);
    transform->Compose(ndTransform);
    // apply final transformation to BaseData
    objectToTransform->GetGeometry()->SetIndexToWorldTransform(transform);
    objectToTransform->Modified();
    // operation was successful, therefore data of output is valid.
    output->SetDataValid(true);
  }
}
