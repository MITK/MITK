/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkNavigationDataReferenceTransformFilter.h"



mitk::NavigationDataReferenceTransformFilter::NavigationDataReferenceTransformFilter() : mitk::NavigationDataLandmarkTransformFilter()
,m_QuaternionTransform(NULL)
,m_SourceLandmarksFromNavigationDatas(NULL)
,m_TargetLandmarksFromNavigationDatas(NULL)
{
  // initialize transform and point containers
  m_QuaternionTransform = QuaternionTransformType::New();
  m_SourceLandmarksFromNavigationDatas = mitk::PointSet::New();
  m_TargetLandmarksFromNavigationDatas = mitk::PointSet::New();
}


mitk::NavigationDataReferenceTransformFilter::~NavigationDataReferenceTransformFilter()
{
  m_QuaternionTransform = NULL;
}


void mitk::NavigationDataReferenceTransformFilter::SetSourceNavigationDatas(const std::vector<mitk::NavigationData::Pointer>& sourceNavigationDatas)
{
  if(m_SourceLandmarksFromNavigationDatas.IsNotNull()) // check if source landmark container available and clear it
    m_SourceLandmarksFromNavigationDatas->Clear();
  else
    return;

  if(sourceNavigationDatas.empty()) // if no ND's passed set filter back
  {
    this->ReinitFilter();
    return;
  }

  if(sourceNavigationDatas.size() < 3) // if less than 3 ND's passed, more source points have to be generated
    this->CreateLandmarkPointsForSingleNavigationData(m_SourceLandmarksFromNavigationDatas, sourceNavigationDatas);
  else{
    for(int i=0; i < sourceNavigationDatas.size(); ++i){
      mitk::Point3D point = sourceNavigationDatas.at(i)->GetPosition();
      m_SourceLandmarksFromNavigationDatas->InsertPoint(i,point); // pass the position of every ND as point to the source points container
    }
  }

  this->InitializeTransform();
}


void mitk::NavigationDataReferenceTransformFilter::SetTargetNavigationDatas(const std::vector<mitk::NavigationData::Pointer>& targetNavigationDatas)
{
  if(m_TargetLandmarksFromNavigationDatas.IsNotNull()) // check if target landmark container available and clear it
    m_TargetLandmarksFromNavigationDatas->Clear(); 
  else
    return;

  if(targetNavigationDatas.empty()) // if no ND's passed set filter back
  {
    this->ReinitFilter();
    return;
  }

  if(targetNavigationDatas.size() < 3) // if less than 3 ND's passed, more target points have to be generated
    this->CreateLandmarkPointsForSingleNavigationData(m_TargetLandmarksFromNavigationDatas, targetNavigationDatas);
  else {
    for(int i=0; i < targetNavigationDatas.size(); ++i){
      mitk::Point3D point = targetNavigationDatas.at(i)->GetPosition();
      m_TargetLandmarksFromNavigationDatas->InsertPoint(i,point);// pass the position of every ND as point to the target points container
    }
  }

  this->InitializeTransform();
}

bool mitk::NavigationDataReferenceTransformFilter::InitializeTransform()
{
  bool sameSize = m_SourceLandmarksFromNavigationDatas->GetSize() == m_TargetLandmarksFromNavigationDatas->GetSize();
  if(!sameSize)
    return false;

  if(m_SourceLandmarksFromNavigationDatas->GetSize() >= 3 && m_TargetLandmarksFromNavigationDatas->GetSize() >= 3)
  {
    m_SourcePoints.clear();
    m_TargetPoints.clear();

    this->SetSourceLandmarks(this->GetSourceLandmarks());
    this->SetTargetLandmarks(this->GetTargetLandmarks());

    return true;
  }

  return false;
}

void mitk::NavigationDataReferenceTransformFilter::ReinitFilter()
{

  // clear this class source and target points
  m_SourceLandmarksFromNavigationDatas->Clear();
  m_TargetLandmarksFromNavigationDatas->Clear();

  //clear superclass source and target points
  m_TargetPoints.clear();
  m_SourcePoints.clear();

  this->Modified();
}


mitk::PointSet::Pointer mitk::NavigationDataReferenceTransformFilter::CreateLandmarkPointsForSingleNavigationData(mitk::PointSet::Pointer landmarkContainer, const std::vector<mitk::NavigationData::Pointer>& navigationDatas)
{
  if(m_QuaternionTransform.IsNull()) // if quaternion transform not available return
    return landmarkContainer;

  for(int i=0; i < navigationDatas.size(); ++i)
  {
    mitk::Point3D pointA;
    mitk::Point3D pointB;
    mitk::Point3D pointC;

     //initializing three points with position(0|0|0)
    pointA.Fill(0);
    pointB.Fill(0);
    pointC.Fill(0);

    // changing position off all points in order to make them orthogonal
    pointA[0] = 1;
    pointB[1] = 1;
    pointC[2] = 1;

    // current NavigationData
    mitk::NavigationData::Pointer nd = navigationDatas.at(i);

    // orientation of NavigationData from parameter
    mitk::NavigationData::OrientationType quatIn = nd->GetOrientation();

    // set orientation to quaternion transform
    vnl_quaternion<double> const vnlQuatIn(quatIn.x(), quatIn.y(), quatIn.z(), quatIn.r());

    m_QuaternionTransform->SetRotation(vnlQuatIn);

    // transform each point
    pointA = m_QuaternionTransform->TransformPoint(pointA);
    pointB = m_QuaternionTransform->TransformPoint(pointB);
    pointC = m_QuaternionTransform->TransformPoint(pointC);

    // add position data from NavigationData parameter to each point
    pointA[0] += nd->GetPosition()[0];
    pointA[1] += nd->GetPosition()[1];
    pointA[2] += nd->GetPosition()[2];

    pointB[0] += nd->GetPosition()[0];
    pointB[1] += nd->GetPosition()[1];
    pointB[2] += nd->GetPosition()[2];

    pointC[0] += nd->GetPosition()[0];
    pointC[1] += nd->GetPosition()[1];
    pointC[2] += nd->GetPosition()[2];


    int currSize = landmarkContainer->GetSize();
    // insert transformed points in landmark container
    landmarkContainer->InsertPoint(currSize++,pointA);
    landmarkContainer->InsertPoint(currSize++,pointB);
    landmarkContainer->InsertPoint(currSize++,pointC);
  }

  return landmarkContainer;
}

const mitk::PointSet::Pointer mitk::NavigationDataReferenceTransformFilter::GetSourceLandmarks()
{
  return m_SourceLandmarksFromNavigationDatas;
}

const mitk::PointSet::Pointer mitk::NavigationDataReferenceTransformFilter::GetTargetLandmarks()
{
  return m_TargetLandmarksFromNavigationDatas;
}

