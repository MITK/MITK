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
: mitk::NavigationDataToNavigationDataFilter(), m_FiducialMarkersAreSet(false), m_UseNavigationDataIDasReference(false)
{
  m_LandmarkTransform = TransformType::New();
  m_LandmarkTransform->SetModeToSimilarity();//set to rotation, translation and isotropic scaling
  m_FiducialMarkers = vtkPoints::New();
}

mitk::NavigationDataLandmarkTransformFilter::NavigationDataLandmarkTransformFilter(std::vector<int> IDs) 
: mitk::NavigationDataToNavigationDataFilter(), m_FiducialMarkersAreSet(false)
{
  m_LandmarkTransform = TransformType::New();
  m_LandmarkTransform->SetModeToSimilarity();//set to rotation, translation and isotropic scaling
  m_FiducialMarkers = vtkPoints::New();

  m_IDs = IDs;
  m_UseNavigationDataIDasReference = true;

}

mitk::NavigationDataLandmarkTransformFilter::~NavigationDataLandmarkTransformFilter()
{
   m_FiducialMarkers->Delete();
}

bool mitk::NavigationDataLandmarkTransformFilter::AllNavigationDatasValid()
{
  bool allValid = true;
  
  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
  {
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);

    allValid = (allValid && input->IsDataValid());
  }

  return allValid;
}

void mitk::NavigationDataLandmarkTransformFilter::FillPointsWithCurrentNDReferencePoints( vtkPoints* points)
{
  if(points == NULL)
    return;

  // use ND IDs to identify the reference NDs
  if(m_UseNavigationDataIDasReference)
  {
    int NUMBEROFREFERENCEPOINTS = m_IDs.size();
    points->SetNumberOfPoints(NUMBEROFREFERENCEPOINTS);

    for (int i=0; i<NUMBEROFREFERENCEPOINTS; i++)
    {
      const mitk::NavigationData* input = this->GetInput( m_IDs[i] );
      assert(input);
      
      if (input->IsDataValid() == false)
      {
        continue;
      }

      mitk::NavigationData::PositionType tempCoordinateIn;
      float tempPositionIn[3];
      //convert to vtk
      tempCoordinateIn = input->GetPosition();
      tempPositionIn[0] = tempCoordinateIn[0]; 
      tempPositionIn[1] = tempCoordinateIn[1]; 
      tempPositionIn[2] = tempCoordinateIn[2];

      points->SetPoint(i, tempPositionIn);
    }
    points->Modified();
  }
  //get reference ND using the "TYPE-field" of ND
  else
  {
   // mitk::TrackingToolData::TASKREFERENCE see QmitkFiducialRegistrationComponent line 116
  }

}

bool mitk::NavigationDataLandmarkTransformFilter::CalculateTransform()
{
  if (!m_FiducialMarkersAreSet)
    return false;
  
  //if the original markers are not yet set return with false. they need to be set when the landmark transform is computed
  if (m_FiducialMarkers->GetNumberOfPoints() < 1) 
    return false;
 
  //load the position of the Tools marked as reference into the vtkPoints
  static vtkPoints* currentMarkers = vtkPoints::New(); //do not instantiate and delete it all the time. use static!
  this->FillPointsWithCurrentNDReferencePoints(currentMarkers);

  //build up the vtkLandmarktransform
  //we need a transform from this coordinates of the fiducial markers to the coordinates of the initial position of the fiducial markers.
  //so currentMarkers is the Source and FiducialMarkers is the target. 
  m_LandmarkTransform->SetSourceLandmarks( currentMarkers );
  m_LandmarkTransform->SetTargetLandmarks( m_FiducialMarkers );//if the same, nothing will be done

  m_LandmarkTransform->Update();//done here, because we don't have to call an update for every transformation of a Tool. Later on only call InternalTransformPoint
  return true;

}


void mitk::NavigationDataLandmarkTransformFilter::TransformNavigationDatas( TransformType * transform )
{
  //this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs
  // \warning This could erase outputs if inputs have been removed

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
    float tempPositionIn[3], tempPositionOut[3];

    //convert to vtk
    tempCoordinateIn = input->GetPosition();
    tempPositionIn[0] = tempCoordinateIn[0]; 
    tempPositionIn[1] = tempCoordinateIn[1]; 
    tempPositionIn[2] = tempCoordinateIn[2]; 

    //do the landmark transform
    transform->InternalTransformPoint( tempPositionIn, tempPositionOut );  
    //InternalTransformPoint don't call the update of the Transform, done in CalculateTransform()

    //convert back to mitk
    tempCoordinateOut[0] = tempPositionOut[0]; 
    tempCoordinateOut[1] = tempPositionOut[1]; 
    tempCoordinateOut[2] = tempPositionOut[2]; 

    output->Graft(input); // First, copy all information from input to output
    output->SetPosition(tempCoordinateOut);// Then change the member(s): add new position of navigation data after transformation
    output->SetDataValid(true); // operation was successful, therefore data of output is valid.

  }
}
void mitk::NavigationDataLandmarkTransformFilter::GenerateData()
{
  if(this->AllNavigationDatasValid() && ! m_FiducialMarkersAreSet)
  {
    this->FillPointsWithCurrentNDReferencePoints(m_FiducialMarkers);
    m_FiducialMarkersAreSet = true;
  }
  else if (this->CalculateTransform())  //calculate transform if possible 
    this->TransformNavigationDatas(m_LandmarkTransform);
}
