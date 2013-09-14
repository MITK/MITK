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

#include "mitkNavigationDataTransformFilter.h"


mitk::NavigationDataTransformFilter::NavigationDataTransformFilter()
: mitk::NavigationDataToNavigationDataFilter()
{
  m_Transform = NULL;

  //transform to rotate orientation
  m_QuatOrgRigidTransform = itk::QuaternionRigidTransform<double>::New();
  m_QuatTmpTransform = itk::QuaternionRigidTransform<double>::New();
}


mitk::NavigationDataTransformFilter::~NavigationDataTransformFilter()
{
  m_Transform = NULL;
}

void mitk::NavigationDataTransformFilter::SetRigid3DTransform( TransformType::Pointer transform )
{
  m_Transform = transform;
  this->Modified();
}

void mitk::NavigationDataTransformFilter::GenerateData()
{

  // only update data if m_Transform was set
  if(m_Transform.IsNull())
  {
    itkExceptionMacro("Invalid parameter: Transform was not set!  Use SetRigid3DTransform() before updating the filter.");
    return;
  }
  else
  {
    this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs

    /* update outputs with tracking data from tools */
    for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs() ; ++i)
    {
      mitk::NavigationData* output = this->GetOutput(i);
      assert(output);
      const mitk::NavigationData* input = this->GetInput(i);
      assert(input);

      if (input->IsDataValid() == false)
      {
        output->SetDataValid(false);
        continue;
      }

      mitk::NavigationData::PositionType tempCoordinateIn, tempCoordinateOut;
      tempCoordinateIn = input->GetPosition();

      itk::Point<float,3> itkPointIn, itkPointOut;
      itkPointIn[0] = tempCoordinateIn[0];
      itkPointIn[1] = tempCoordinateIn[1];
      itkPointIn[2] = tempCoordinateIn[2];

      //do the transform
      itkPointOut = m_Transform->TransformPoint( itkPointIn );

      tempCoordinateOut[0] = itkPointOut[0];
      tempCoordinateOut[1] = itkPointOut[1];
      tempCoordinateOut[2] = itkPointOut[2];

      output->Graft(input); // First, copy all information from input to output
      output->SetPosition(tempCoordinateOut);// Then change the member(s): add new position of navigation data after tranformation
      output->SetDataValid(true); // operation was successful, therefore data of output is valid.

      //---transform orientation
      NavigationData::OrientationType  quatIn = input->GetOrientation();
      vnl_quaternion<double> const vnlQuatIn(quatIn.x(), quatIn.y(), quatIn.z(), quatIn.r());

      TransformType::MatrixType rotMatrixD = m_Transform->GetMatrix();

      m_QuatOrgRigidTransform->SetMatrix(rotMatrixD);//SetRotation(vnlQ);
      m_QuatTmpTransform->SetRotation(vnlQuatIn);
      m_QuatTmpTransform->Compose(m_QuatOrgRigidTransform,false);
      vnl_quaternion<double> vnlQuatOut = m_QuatTmpTransform->GetRotation();
      NavigationData::OrientationType quatOut(vnlQuatOut[0], vnlQuatOut[1], vnlQuatOut[2], vnlQuatOut[3]);

      output->SetOrientation(quatOut);


    }
  }
}

