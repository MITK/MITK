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

#include "mitkNavigationDataLandmarkTransformFilter.h"


mitk::NavigationDataLandmarkTransformFilter::NavigationDataLandmarkTransformFilter() 
: mitk::NavigationDataToNavigationDataFilter(), 
  m_SourcePointsAreSet(false),
  m_TargetPointsAreSet(false)
  
{
  m_ITKLandmarkTransform = ITKVersorTransformType::New();
  m_ITKLandmarkTransform->SetIdentity();
  m_LandmarkTransformInitializer = TransformInitializerType::New();
  m_LandmarkTransformInitializer->SetTransform(m_ITKLandmarkTransform);

  //transform to rotate orientation 
   m_QuatLandmarkTransform = itk::QuaternionRigidTransform<double>::New();
   m_QuatTransform = itk::QuaternionRigidTransform<double>::New();
}


mitk::NavigationDataLandmarkTransformFilter::~NavigationDataLandmarkTransformFilter()
{
  m_ITKLandmarkTransform = NULL;
  m_LandmarkTransformInitializer = NULL;
  m_QuatLandmarkTransform = NULL;
  m_QuatTransform = NULL;
}


void mitk::NavigationDataLandmarkTransformFilter::InitializeLandmarkTransform()
{
  try
  {
    m_LandmarkTransformInitializer->SetMovingLandmarks(m_TargetPoints);
    m_LandmarkTransformInitializer->SetFixedLandmarks(m_SourcePoints);

    m_ITKLandmarkTransform->SetIdentity();

    m_LandmarkTransformInitializer->InitializeTransform();
    m_QuatLandmarkTransform->SetMatrix(m_ITKLandmarkTransform->GetRotationMatrix());
    this->Modified();
  }
  catch (std::exception& e)
  {
    itkExceptionMacro("Initializing landmark-tranfrom failed\n. "<<e.what());
  }
}


void mitk::NavigationDataLandmarkTransformFilter::SetSourcePoints(mitk::PointSet::Pointer mitkSourcePointSet)
{
  m_SourcePoints.clear();

  mitk::Point3D mitkSourcePoint;
  TransformInitializerType::LandmarkPointType lPoint;
 
  int mitkPointSetSize = mitkSourcePointSet->GetPointSet()->GetNumberOfPoints();
  
  for (int i=0; i<mitkPointSetSize; i++)
  {
   mitkSourcePointSet->GetPointSet()->GetPoints()->GetElementIfIndexExists(i, &mitkSourcePoint);
   lPoint[0]=mitkSourcePoint[0];
   lPoint[1]=mitkSourcePoint[1];
   lPoint[2]=mitkSourcePoint[2];
   
   m_SourcePoints.push_back(lPoint);
  }

  if (m_SourcePoints.size() > 2)
    m_SourcePointsAreSet = true;
  else
    itkExceptionMacro("SourcePointSet must contain at least 3 points");

  
  if(m_TargetPointsAreSet)
    this->InitializeLandmarkTransform();

}

void mitk::NavigationDataLandmarkTransformFilter::SetTargetPoints(mitk::PointSet::Pointer mitkTargetPointSet)
{
  m_TargetPoints.clear();

  mitk::Point3D mitkTargetPoint;
  TransformInitializerType::LandmarkPointType lPoint;

  int mitkPointSetSize = mitkTargetPointSet->GetPointSet()->GetNumberOfPoints();

  for (int i=0; i<mitkPointSetSize; i++)
  {
    mitkTargetPointSet->GetPointSet()->GetPoints()->GetElementIfIndexExists(i, &mitkTargetPoint);
    lPoint[0]=mitkTargetPoint[0];
    lPoint[1]=mitkTargetPoint[1];
    lPoint[2]=mitkTargetPoint[2];
    
    m_TargetPoints.push_back(lPoint);
  }

  if (m_TargetPoints.size() > 2)
    m_TargetPointsAreSet = true;
  else
    itkExceptionMacro("TargetPointSet must contain at least 3 points");

  if (m_SourcePointsAreSet)
    this->InitializeLandmarkTransform();

}

void mitk::NavigationDataLandmarkTransformFilter::GenerateData()
{
  this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs

  /* update outputs with tracking data from tools */
  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
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
    output->Graft(input); // First, copy all information from input to output
    if ((m_SourcePointsAreSet == false) || (m_TargetPointsAreSet == false)) // as long as there is no valid transformation matrix, only graft the outputs
      continue;

    TransformInitializerType::LandmarkPointType lPointIn;
    TransformInitializerType::LandmarkPointType lPointOut;
    mitk::NavigationData::PositionType tempCoordinate;
    tempCoordinate = input->GetPosition();

    lPointIn[0] = tempCoordinate[0];
    lPointIn[1] = tempCoordinate[1];
    lPointIn[2] = tempCoordinate[2];

    /* transform position */
    lPointOut = m_ITKLandmarkTransform->TransformPoint(lPointIn);
    tempCoordinate[0] = lPointOut[0];
    tempCoordinate[1] = lPointOut[1];
    tempCoordinate[2] = lPointOut[2];
    output->SetPosition(tempCoordinate);  // update position of output to transformed position

    /* transform orientation */
    NavigationData::OrientationType  quatIn = input->GetOrientation();
    vnl_quaternion<double> const vnlQuatIn(quatIn.x(), quatIn.y(), quatIn.z(), quatIn.r());

    m_QuatTransform->SetIdentity();
    m_QuatTransform->SetRotation(vnlQuatIn);
    m_QuatLandmarkTransform->SetIdentity();
    m_QuatLandmarkTransform->Compose(m_QuatTransform, true);

    vnl_quaternion<double> vnlQuatOut = m_QuatLandmarkTransform->GetRotation();
    NavigationData::OrientationType quatOut(vnlQuatOut[0], vnlQuatOut[1], vnlQuatOut[2], vnlQuatOut[3]);
    output->SetOrientation(quatOut);      // update orientation of output to transformed orientation
  }
}
