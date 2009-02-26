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
: mitk::NavigationDataToNavigationDataFilter(), m_TargetPointsAreSet(false),m_SourcePointsAreSet(false)
{
  m_ITKLandmarkTransform = ITKVersorTransformType::New();

  m_LandmarkTransformInitializer = TransformInitializerType::New();
  m_LandmarkTransformInitializer->SetTransform(m_ITKLandmarkTransform);

  //transform to rotate orientation 
   m_QuatLandmarkTransform = itk::QuaternionRigidTransform<double>::New();
   m_QuatTransform = itk::QuaternionRigidTransform<double>::New();


}

mitk::NavigationDataLandmarkTransformFilter::~NavigationDataLandmarkTransformFilter()
{}

void mitk::NavigationDataLandmarkTransformFilter::InitializeLandmarkTransform()
{
  try
  {
    m_LandmarkTransformInitializer->SetMovingLandmarks(m_TargetPoints);
    m_LandmarkTransformInitializer->SetFixedLandmarks(m_SourcePoints);

    m_ITKLandmarkTransform->SetIdentity();

    m_LandmarkTransformInitializer->InitializeTransform();

    this->Modified();
  }
  catch (std::exception& e)
  {
    itkExceptionMacro("Initializing landmark-tranfrom failed\n. "<<e.what());
  }

}

void mitk::NavigationDataLandmarkTransformFilter::TransformNavigationDatas(ITKVersorTransformType::Pointer transform )
{
  //this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs
  // \warning This could erase outputs if inputs have been removed

  TransformInitializerType::LandmarkPointType lPointIn, lPointOut;
  
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
    tempCoordinateIn = input->GetPosition();

    lPointIn[0]=tempCoordinateIn[0];
    lPointIn[1]=tempCoordinateIn[1];
    lPointIn[2]=tempCoordinateIn[2];

    //do the landmark transform
    lPointOut = transform->TransformPoint(lPointIn);

    tempCoordinateOut[0]=lPointOut[0];
    tempCoordinateOut[1]=lPointOut[1];
    tempCoordinateOut[2]=lPointOut[2];

    output->Graft(input); // First, copy all information from input to output
    output->SetPosition(tempCoordinateOut);// Then change the member(s): add new position of navigation data after transformation
    output->SetDataValid(true); // operation was successful, therefore data of output is valid.



    //---transform orientation
    NavigationData::OrientationType  quatIn = input->GetOrientation();
    vnl_quaternion<double> const vnlQuatIn(quatIn.x(), quatIn.y(), quatIn.z(), quatIn.r());
       
    m_QuatLandmarkTransform->SetMatrix(transform->GetRotationMatrix());
    m_QuatTransform->SetRotation(vnlQuatIn);
    m_QuatLandmarkTransform->Compose(m_QuatTransform,true);

    vnl_quaternion<double> vnlQuatOut = m_QuatLandmarkTransform->GetRotation();
    NavigationData::OrientationType quatOut(vnlQuatOut[0], vnlQuatOut[1], vnlQuatOut[2], vnlQuatOut[3]);
    output->SetOrientation(quatOut);


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

  if (m_SourcePoints.size()>2)
    m_SourcePointsAreSet=true;
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

  if (m_TargetPoints.size()>2)
    m_TargetPointsAreSet=true;
  else
    itkExceptionMacro("TargetPointSet must contain at least 3 points");

  if(m_SourcePointsAreSet)
    this->InitializeLandmarkTransform();

}

void mitk::NavigationDataLandmarkTransformFilter::GenerateData()
{
     this->TransformNavigationDatas(m_ITKLandmarkTransform);
}
