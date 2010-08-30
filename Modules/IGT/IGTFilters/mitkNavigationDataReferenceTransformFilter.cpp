/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataReferenceTransformFilter.h"


mitk::NavigationDataReferenceTransformFilter::NavigationDataReferenceTransformFilter() : mitk::NavigationDataToNavigationDataFilter(),
m_SourcePoints (NULL),
m_TargetPoints (NULL),
m_ReferencePoints (NULL),
m_LandmarkTransformInitializer (NULL),
m_LandmarkTransform (NULL),
m_OneSourceRegistration (false),
m_ReferenceRegistration (false)
{

  m_LandmarkTransform = LandmarkTransformType::New();
  m_LandmarkTransformInitializer = TransformInitializerType::New();
  m_LandmarkTransformInitializer->SetTransform(m_LandmarkTransform);

  m_QuaternionTransform = QuaternionTransformType::New();
  m_QuaternionLandmarkTransform = QuaternionTransformType::New();
  m_QuaternionReferenceTransform = QuaternionTransformType::New();
}

mitk::NavigationDataReferenceTransformFilter::~NavigationDataReferenceTransformFilter()
{
  m_LandmarkTransform = NULL;
  m_LandmarkTransformInitializer = NULL;
  m_QuaternionTransform = NULL;
  m_QuaternionLandmarkTransform = NULL;
  m_QuaternionReferenceTransform =  NULL;
}


void mitk::NavigationDataReferenceTransformFilter::ResetFilter()
{
   m_OneSourceRegistration = false;
   m_ReferenceRegistration = false;

   m_SourcePoints.clear();
   m_TargetPoints.clear();
   m_ReferencePoints.clear();

   this->CreateOutputsForAllInputs();   

  
}


void mitk::NavigationDataReferenceTransformFilter::GenerateData()
{
  LandmarkPointContainer newSourcePoints;  // for the quaternion transformed reference landmarks

  if(m_OneSourceRegistration) // check if less than 3 reference inputs
  {
    NavigationData::ConstPointer nd = this->GetInput(m_ReferenceInputIndexes.at(0));

    if (nd->IsDataValid() == false)
    {
      for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
      {
        mitk::NavigationData::Pointer output = this->GetOutput(i);
        assert(output);
        output->SetDataValid(false);
      }
      return;
    }

    QuaternionTransformType::Pointer referenceTransform = QuaternionTransformType::New();
    QuaternionTransformType::VnlQuaternionType doubleOrientation(nd->GetOrientation().x(), nd->GetOrientation().y(), nd->GetOrientation().z(), nd->GetOrientation().r()); // convert to double quaternion as workaround for ITK 3.10 bug
    
    referenceTransform->SetRotation(doubleOrientation);
    referenceTransform->SetOffset(nd->GetPosition().GetVectorFromOrigin());
    referenceTransform->Modified();

    for (NavigationDataReferenceTransformFilter::LandmarkPointContainer::const_iterator it = m_ReferencePoints.begin(); it != m_ReferencePoints.end(); ++it)
    {
      TransformInitializerType::LandmarkPointType rLPoint;  // reference landmark point
      rLPoint = referenceTransform->TransformPoint(*it);   
      newSourcePoints.push_back(rLPoint);   
    }
      
      this->UpdateLandmarkTransform(newSourcePoints, m_TargetPoints);
      m_SourcePoints = newSourcePoints;
  }
 

  if(this->IsInitialized() && !m_OneSourceRegistration && m_ReferenceRegistration)
    this->GenerateSourceLandmarks(); // generates landmarks from the moving points


  this->CreateOutputsForAllInputs();

  TransformInitializerType::LandmarkPointType lPointIn, lPointOut;

  for(unsigned int i = 0; i < this->GetNumberOfOutputs(); i++)
  {
    mitk::NavigationData::Pointer output = this->GetOutput(i);
    assert(output);
    mitk::NavigationData::ConstPointer input = this->GetInput(i);
    assert(input);

    if(input->IsDataValid() == false)
    {
      output->SetDataValid(false);
      continue;
    }
    output->Graft(input); // First, copy all information from input to output


    if(this->IsInitialized() == false)
      continue;

    mitk::NavigationData::PositionType tempCoordinate;
    tempCoordinate = input->GetPosition();

    lPointIn[0] = tempCoordinate[0];
    lPointIn[1] = tempCoordinate[1];
    lPointIn[2] = tempCoordinate[2];

    /* transform position */
    lPointOut = m_LandmarkTransform->TransformPoint(lPointIn);

    tempCoordinate[0] = lPointOut[0];
    tempCoordinate[1] = lPointOut[1];
    tempCoordinate[2] = lPointOut[2];

    output->SetPosition(tempCoordinate);  // update output navigation data with new position

    /* transform orientation */
    NavigationData::OrientationType quatIn = input->GetOrientation();
    vnl_quaternion<double> const vnlQuatIn(quatIn.x(), quatIn.y(), quatIn.z(), quatIn.r());
    m_QuaternionTransform->SetRotation(vnlQuatIn);


    m_QuaternionLandmarkTransform->SetMatrix(m_LandmarkTransform->GetRotationMatrix());
    m_QuaternionLandmarkTransform->Compose(m_QuaternionTransform, true);


    vnl_quaternion<double> vnlQuatOut = m_QuaternionLandmarkTransform->GetRotation();
    NavigationData::OrientationType quatOut( vnlQuatOut[0], vnlQuatOut[1], vnlQuatOut[2], vnlQuatOut[3]);

    output->SetOrientation(quatOut);
    output->SetDataValid(true);

  }
}


const mitk::NavigationDataReferenceTransformFilter::LandmarkPointContainer mitk::NavigationDataReferenceTransformFilter::GenerateReferenceLandmarks()
{
  LandmarkPointContainer lPoints;

  if(!m_ReferenceInputIndexes.empty())
  {
    TransformInitializerType::LandmarkPointType lPoint;

    if(m_ReferenceInputIndexes.size() < 3)
    {
      NavigationData::ConstPointer nD = this->GetInput(m_ReferenceInputIndexes.at(0));
      NavigationData::PositionType pos = nD->GetPosition();

      // fill position of reference source into sourcepoints container
      mitk::FillVector3D(lPoint, pos.GetElement(0), pos.GetElement(1), pos.GetElement(2));
      lPoints.push_back(lPoint);

      // generate additional virtual landmark point
      mitk::FillVector3D(lPoint, pos.GetElement(0), pos.GetElement(1)+100, pos.GetElement(2)); // reference source position + (0|100|0)
      lPoints.push_back(lPoint);

      // generate additional virtual landmark point
      mitk::FillVector3D(lPoint, pos.GetElement(0), pos.GetElement(1), pos.GetElement(2)+100); // reference source position + (0|0|100)
      lPoints.push_back(lPoint);
    }
   
    
    else if(m_ReferenceInputIndexes.size()>2)  // if there are at least 3 reference inputs
    {
      for(unsigned int i=0; i<m_ReferenceInputIndexes.size(); ++i)
      {
        NavigationData::ConstPointer nD = this->GetInput(m_ReferenceInputIndexes.at(i));
        NavigationData::PositionType pos = nD->GetPosition();
        mitk::FillVector3D(lPoint, pos.GetElement(0), pos.GetElement(1), pos.GetElement(2));
        lPoints.push_back(lPoint);
      }
    }

  }

  return lPoints;
}

void mitk::NavigationDataReferenceTransformFilter::GenerateSourceLandmarks()
{
  m_ReferenceRegistration = true;

  m_SourcePoints.clear();
  m_SourcePoints = this->GenerateReferenceLandmarks();  // fill source points with landmarks generated from reference inputs

  if(this->IsInitialized())
    this->UpdateLandmarkTransform(m_SourcePoints, m_TargetPoints);
}


void mitk::NavigationDataReferenceTransformFilter::GenerateTargetLandmarks()
{
  m_ReferenceRegistration = true;

  m_TargetPoints.clear();
  m_TargetPoints = this->GenerateReferenceLandmarks(); // fill target points with landmarks generated from reference inputs

  if(this->IsInitialized())
    this->UpdateLandmarkTransform(m_SourcePoints, m_TargetPoints);
}




bool mitk::NavigationDataReferenceTransformFilter::InitializeReferenceLandmarks()
{

  if (m_ReferenceInputIndexes.size() != 1 || (m_SourcePoints.size() < 3) || (m_ReferenceInputIndexes.size() <= 0) || m_ReferenceInputIndexes.size() > this->GetNumberOfInputs())
    return false;

  mitk::NavigationData::ConstPointer referenceND = this->GetInput(m_ReferenceInputIndexes.at(0));

  if(!referenceND->IsDataValid())
    return false;

  QuaternionTransformType::Pointer referenceTransform = QuaternionTransformType::New();
  QuaternionTransformType::VnlQuaternionType doubleOrientation(referenceND->GetOrientation().x(), referenceND->GetOrientation().y(),referenceND->GetOrientation().z(),referenceND->GetOrientation().r());

  referenceTransform->SetRotation(doubleOrientation);
  referenceTransform->SetOffset(referenceND->GetPosition().GetVectorFromOrigin());
  referenceTransform->Modified();

  try
  {
    bool okay = referenceTransform->GetInverse(m_QuaternionReferenceTransform);  // inverse transform to compensate rotation
    if(!okay)
      return false;
  }
  catch( ... )
  {
    return false;
  }

  m_ReferencePoints.clear();

  for(NavigationDataReferenceTransformFilter::LandmarkPointContainer::const_iterator it = m_SourcePoints.begin(); it != m_SourcePoints.end(); ++it)
  {
    QuaternionTransformType::InputPointType doublePoint;
    mitk::itk2vtk(*it,doublePoint); // workaround for itk bug
    TransformInitializerType::LandmarkPointType rLPoint;
    rLPoint = m_QuaternionReferenceTransform->TransformPoint(doublePoint);
    m_ReferencePoints.push_back(rLPoint);
  }
  return true;
}


void mitk::NavigationDataReferenceTransformFilter::PrintSelf( std::ostream& /*os*/, itk::Indent /*indent*/ ) const
{

}

void mitk::NavigationDataReferenceTransformFilter::SetSourceLandmarks(mitk::PointSet::Pointer sourcePointSet)
{
  m_OneSourceRegistration = false;
  m_ReferenceRegistration = false;

  m_SourcePoints.clear();
  mitk::PointSet::PointType mitkSourcePoint;
  TransformInitializerType::LandmarkPointType lPoint;

  for (mitk::PointSet::PointsContainer::ConstIterator it = sourcePointSet->GetPointSet()->GetPoints()->Begin(); 
    it != sourcePointSet->GetPointSet()->GetPoints()->End(); it++)
  {
    mitk::FillVector3D(lPoint, it->Value().GetElement(0), it->Value().GetElement(1), it->Value().GetElement(2));
    m_SourcePoints.push_back(lPoint);
  }

  if(m_SourcePoints.size() < 1)
  {
    itkExceptionMacro("SourcePointSet must contain at least 1 point");
  }

  if(this->IsInitialized())
    this->UpdateLandmarkTransform(m_SourcePoints, m_TargetPoints);

}


void mitk::NavigationDataReferenceTransformFilter::SetTargetLandmarks(mitk::PointSet::Pointer targetPointSet)
{
  m_OneSourceRegistration = false;
  m_ReferenceRegistration = false;


  m_TargetPoints.clear();
  TransformInitializerType::LandmarkPointType lPoint;


  for (mitk::PointSet::PointsContainer::ConstIterator it = targetPointSet->GetPointSet()->GetPoints()->Begin(); 
    it != targetPointSet->GetPointSet()->GetPoints()->End(); it++)
  {
    mitk::FillVector3D(lPoint, it->Value().GetElement(0), it->Value().GetElement(1), it->Value().GetElement(2));
    m_TargetPoints.push_back(lPoint);
  }

  if(m_TargetPoints.size() < 1)
  {
    itkExceptionMacro("TargetPointSet must contain at least 1 point");
  }

  if(this->IsInitialized())
    this->UpdateLandmarkTransform(m_SourcePoints, m_TargetPoints);

}


bool mitk::NavigationDataReferenceTransformFilter::IsInitialized() const
{
  return ( m_SourcePoints.size() >= 3 && (m_SourcePoints.size() == (m_TargetPoints.size())) ); // min. 3 landmarks and same size of source and targets
}


void mitk::NavigationDataReferenceTransformFilter::UpdateLandmarkTransform(const LandmarkPointContainer &sources,  const LandmarkPointContainer &targets)
{
  try 
  {
    m_LandmarkTransformInitializer->SetMovingLandmarks(targets);
    m_LandmarkTransformInitializer->SetFixedLandmarks(sources);
    m_LandmarkTransform->SetIdentity();
    m_LandmarkTransformInitializer->InitializeTransform();

    this->Modified();
  }
  catch (std::exception& e)
  {
    m_LandmarkTransform->SetIdentity();
    itkExceptionMacro("Initializing landmark-transform failed\n. " << e.what());
  }
}

void mitk::NavigationDataReferenceTransformFilter::SetReferenceInputIndexes(const std::vector<int>* indexes)
{
   if(indexes == NULL)
    return;

   m_ReferenceInputIndexes = *indexes;

  if(m_ReferenceInputIndexes.size()<3) 
  {
    m_OneSourceRegistration = true;
  }
  else
  {
    m_OneSourceRegistration = false;
  }

  // clearing landmark point containers so that switching between 1 and 3+ landmark registration modes is possible
  m_SourcePoints.clear();
  m_TargetPoints.clear();
  m_ReferencePoints.clear();
}


const std::vector<int> mitk::NavigationDataReferenceTransformFilter::GetReferenceInputIndexes()
{
  return m_ReferenceInputIndexes;
}



const mitk::NavigationDataReferenceTransformFilter::LandmarkPointContainer mitk::NavigationDataReferenceTransformFilter::GetSourcePoints()
{
  return m_SourcePoints;
}

const mitk::NavigationDataReferenceTransformFilter::LandmarkPointContainer mitk::NavigationDataReferenceTransformFilter::GetTargetPoints()
{
  return m_TargetPoints;
}

const mitk::NavigationDataReferenceTransformFilter::LandmarkPointContainer mitk::NavigationDataReferenceTransformFilter::GetReferencePoints()
{
  return m_ReferencePoints;
}




