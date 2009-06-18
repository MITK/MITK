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
  m_SourcePointsAreSet(false), m_TargetPointsAreSet(false)
{
  m_ITKLandmarkTransform = ITKVersorTransformType::New();

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
    /* calculate transform from landmarks */
    m_LandmarkTransformInitializer->SetMovingLandmarks(m_TargetPoints);
    m_LandmarkTransformInitializer->SetFixedLandmarks(m_SourcePoints);
    m_ITKLandmarkTransform->SetIdentity();
    m_LandmarkTransformInitializer->InitializeTransform();

    /* Calculate error statistics for the transform */
    TransformInitializerType::LandmarkPointType curData;
    std::vector< mitk::ScalarType > errors;
    for (TransformInitializerType::LandmarkPointContainer::size_type index = 0; index < m_SourcePoints.size(); index++)
    {
      curData = m_ITKLandmarkTransform->TransformPoint(m_SourcePoints.at(index));
      errors.push_back(curData.EuclideanDistanceTo(m_TargetPoints.at(index)));
    }
    this->AccumulateStatistics(errors);
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
  mitk::PointSet::PointType mitkSourcePoint;
  TransformInitializerType::LandmarkPointType lPoint;
 
  int mitkPointSetSize = mitkSourcePointSet->GetPointSet()->GetNumberOfPoints();
  for (int i=0; i<mitkPointSetSize; i++)
  {
   mitkSourcePointSet->GetPointSet()->GetPoints()->GetElementIfIndexExists(i, &mitkSourcePoint);
   lPoint[0] = mitkSourcePoint[0];
   lPoint[1] = mitkSourcePoint[1];
   lPoint[2] = mitkSourcePoint[2];
   
   m_SourcePoints.push_back(lPoint);
  }

  if (m_SourcePoints.size() > 2)
    m_SourcePointsAreSet = true;
  else
  {
    itkExceptionMacro("SourcePointSet must contain at least 3 points");
  }
  
  if (m_TargetPointsAreSet)
    this->InitializeLandmarkTransform();
}


void mitk::NavigationDataLandmarkTransformFilter::SetTargetPoints(mitk::PointSet::Pointer mitkTargetPointSet)
{
  m_TargetPoints.clear();
  mitk::PointSet::PointType mitkTargetPoint;
  TransformInitializerType::LandmarkPointType lPoint;

  int mitkPointSetSize = mitkTargetPointSet->GetPointSet()->GetNumberOfPoints();
  for (int i = 0; i < mitkPointSetSize; i++)
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
  {
    itkExceptionMacro("TargetPointSet must contain at least 3 points");
  }
  if (m_SourcePointsAreSet)
    this->InitializeLandmarkTransform();
}


mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetFRE() const
{
  return m_ErrorMean;
}
mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetFREStdDev() const
{
  return m_ErrorStdDev;
}
mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetRMSError() const
{
  return m_ErrorRMS;
}
mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetMinError() const
{
  return m_ErrorMin;
}
mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetMaxError() const
{
  return m_ErrorMax;
}
mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetAbsMaxError() const
{
  return m_ErrorAbsMax;
}


void mitk::NavigationDataLandmarkTransformFilter::AccumulateStatistics(std::vector<mitk::ScalarType>& vector) 
{
  //mean, min, max
  m_ErrorMean = 0.0;
  m_ErrorMin = itk::NumericTraits<mitk::ScalarType>::max();
  m_ErrorMax = itk::NumericTraits<mitk::ScalarType>::min();
  m_ErrorAbsMax = 0.0;
  m_ErrorRMS = 0.0;
  for (std::vector<mitk::ScalarType>::size_type i = 0; i < vector.size(); i++)
  {
    m_ErrorMean += vector[i];       // mean
    m_ErrorRMS += pow(vector[i],2); // RMS
    if(vector[i] < m_ErrorMin)      // min
      m_ErrorMin = vector[i];
    if(vector[i] > m_ErrorMax)      // max
      m_ErrorMax = vector[i];
    if(fabs(vector[i]) > fabs(m_ErrorAbsMax)) // abs_max
      m_ErrorAbsMax = vector[i];
  }
  m_ErrorMean /= vector.size(); 
  m_ErrorRMS = sqrt(m_ErrorRMS/vector.size());

  //standard deviation
  m_ErrorStdDev = 0.0;
  for (std::vector<mitk::ScalarType>::size_type i = 0; i < vector.size(); i++)
    m_ErrorStdDev += pow(vector[i] - m_ErrorMean, 2);

  if(vector.size() > 1)
    m_ErrorStdDev = sqrt(m_ErrorStdDev / (vector.size() - 1.0));
  this->Modified();
}


void mitk::NavigationDataLandmarkTransformFilter::GenerateData()
{
  this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs

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
      output->SetDataValid(false);
      continue;
    }
    output->Graft(input); // First, copy all information from input to output
    
    if (this->IsInitialized() == false) // as long as there is no valid transformation matrix, only graft the outputs
      continue;

    mitk::NavigationData::PositionType tempCoordinate;
    tempCoordinate = input->GetPosition();
    lPointIn[0] = tempCoordinate[0]; // convert navigation data position to transform point
    lPointIn[1] = tempCoordinate[1];
    lPointIn[2] = tempCoordinate[2];

    /* transform position */
    lPointOut = m_ITKLandmarkTransform->TransformPoint(lPointIn); // transform position
    tempCoordinate[0] = lPointOut[0];  // convert back into navigation data position
    tempCoordinate[1] = lPointOut[1];
    tempCoordinate[2] = lPointOut[2];
    output->SetPosition(tempCoordinate); // update output navigation data with new position

    /* transform orientation */
    NavigationData::OrientationType  quatIn = input->GetOrientation();
    vnl_quaternion<double> const vnlQuatIn(quatIn.x(), quatIn.y(), quatIn.z(), quatIn.r());  // convert orientation into vnl quaternion
    m_QuatTransform->SetRotation(vnlQuatIn);  // convert orientation into transform

    m_QuatLandmarkTransform->SetMatrix(m_ITKLandmarkTransform->GetRotationMatrix());  // set rotation from landmark transform
    m_QuatLandmarkTransform->Compose(m_QuatTransform, true); // compose navigation data transform and landmark transform

    vnl_quaternion<double> vnlQuatOut = m_QuatLandmarkTransform->GetRotation();  // convert composed transform back into a quaternion
    NavigationData::OrientationType quatOut(vnlQuatOut[0], vnlQuatOut[1], vnlQuatOut[2], vnlQuatOut[3]); // convert back into navigation data orientation
    output->SetOrientation(quatOut); // update output navigation data with new orientation
    output->SetDataValid(true); // operation was successful, therefore data of output is valid.
  }
}


bool mitk::NavigationDataLandmarkTransformFilter::IsInitialized() const
{
  return m_SourcePointsAreSet && m_TargetPointsAreSet;
}
