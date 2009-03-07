/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataTransformFilter.h"


mitk::NavigationDataTransformFilter::NavigationDataTransformFilter() 
: mitk::NavigationDataToNavigationDataFilter()
{
  //transform to rotate orientation 
  m_QuatOrgRigidTransform = itk::QuaternionRigidTransform<double>::New();
  m_QuatTmpTransform = itk::QuaternionRigidTransform<double>::New();
}


mitk::NavigationDataTransformFilter::~NavigationDataTransformFilter()
{}

void mitk::NavigationDataTransformFilter::SetRigid3DTransform( TransformType::Pointer transform )
{
  m_Transform = transform; 
  this->Modified();

}

void mitk::NavigationDataTransformFilter::GenerateData()
{

  //this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs
  // \warning This could erase outputs if inputs have been removed

  // only update data if m_Transform was set
  if(m_Transform)
  {
    /* update outputs with tracking data from tools */
    for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
    {
      mitk::NavigationData* output = this->GetOutput(i);
      assert(output);
      const mitk::NavigationData* input = this->GetInput(i);
      assert(input);

      if (input->IsDataValid() == false)
      {
        continue;
      }

      mitk::NavigationData::PositionType tempCoordinateIn, tempCoordinateOut;
      tempCoordinateIn= input->GetPosition();

      itk::Point<float,3> itkPointIn, itkPointOut;
      itkPointIn[0]=tempCoordinateIn[0];
      itkPointIn[1]=tempCoordinateIn[1];
      itkPointIn[2]=tempCoordinateIn[2];
     
      //do the transform
      itkPointOut = m_Transform->TransformPoint( itkPointIn );  

      tempCoordinateOut[0]=itkPointOut[0];
      tempCoordinateOut[1]=itkPointOut[1];
      tempCoordinateOut[2]=itkPointOut[2];

      output->Graft(input); // First, copy all information from input to output
      output->SetPosition(tempCoordinateOut);// Then change the member(s): add new position of navigation data after tranformation
      output->SetDataValid(true); // operation was successful, therefore data of output is valid.

      //---transform orientation
      NavigationData::OrientationType  quatIn = input->GetOrientation();
      vnl_quaternion<double> const vnlQuatIn(quatIn.x(), quatIn.y(), quatIn.z(), quatIn.r());


      itk::Matrix<float,3,3> rotMatrix = m_Transform->GetMatrix();
      
      itk::Matrix<double,3,3> rotMatrixD;
     
      for (unsigned int i = 0; i < 3; i++)
         for( int j=0; j < 3; ++j )
        rotMatrixD[i][j] = rotMatrix[i][j];

      m_QuatOrgRigidTransform->SetRotationMatrix(rotMatrixD);
      m_QuatTmpTransform->SetRotation(vnlQuatIn);
      m_QuatTmpTransform->Compose(m_QuatOrgRigidTransform,false);

      vnl_quaternion<double> vnlQuatOut = m_QuatTmpTransform->GetRotation();
      NavigationData::OrientationType quatOut(vnlQuatOut[0], vnlQuatOut[1], vnlQuatOut[2], vnlQuatOut[3]);
      output->SetOrientation(quatOut);


    }
  }
}

