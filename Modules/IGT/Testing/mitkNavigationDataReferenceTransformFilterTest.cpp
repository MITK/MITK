/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-02-21 19:34:02 +0100 (So, 21 Feb 2010) $
Version:   $Revision: 16010 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataReferenceTransformFilter.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"

#include <itkQuaternionRigidTransform.h>

#include <iostream>


/**Documentation
 *  test for the class "NavigationDataReferenceTransformFilter".
 */
int mitkNavigationDataReferenceTransformFilterTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("NavigationDataReferenceTransformFilter")

  // let's create an object of our class  
  mitk::NavigationDataReferenceTransformFilter::Pointer myFilter = mitk::NavigationDataReferenceTransformFilter::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myFilter.IsNotNull(),"Testing instantiation");


  /*create helper objects: positions of the ND sources*/
  mitk::NavigationData::PositionType sourcePos1,sourcePos2, sourcePos3, targetPos1, targetPos2, targetPos3;
  mitk::FillVector3D(sourcePos1, 11.1, 11.1, 11.1);
  mitk::FillVector3D(sourcePos2, 22.2, 22.2, 22.2);
  mitk::FillVector3D(sourcePos3, 33.3, 33.3, 33.3);
  mitk::FillVector3D(targetPos1, -1.1, -2.2, -3.3);
  mitk::FillVector3D(targetPos2, -4.4, -5.5, -6.6);
  mitk::FillVector3D(targetPos3, -7.7, -8.8, -9.9);

  /*create helper objects: orientations of the ND sources*/
  mitk::NavigationData::OrientationType sourceOri1(0.1, 0.1, 0.1, 0.1);
  mitk::NavigationData::OrientationType sourceOri2(0.2, 0.2, 0.2, 0.2);
  mitk::NavigationData::OrientationType sourceOri3(0.3, 0.3, 0.3, 0.3);
  mitk::NavigationData::OrientationType targetOri1(0.4, 0.4, 0.4, 0.4);
  mitk::NavigationData::OrientationType targetOri2(0.5, 0.5, 0.5, 0.5);
  mitk::NavigationData::OrientationType targetOri3(0.6, 0.6, 0.6, 0.6);
  
  /*create helper objects: ND position accurancy and validity bool*/
  mitk::ScalarType initialError(0.0);
  bool initialValid(true);

  /*create helper objects: NDs for the source and target NDs*/
  mitk::NavigationData::Pointer snd1 = mitk::NavigationData::New();
  snd1->SetPosition(sourcePos1);
  snd1->SetOrientation(sourceOri1);
  snd1->SetPositionAccuracy(initialError);
  snd1->SetDataValid(initialValid);

  mitk::NavigationData::Pointer snd2 = mitk::NavigationData::New();
  snd2->SetPosition(sourcePos2);
  snd2->SetOrientation(sourceOri2);
  snd2->SetPositionAccuracy(initialError);
  snd2->SetDataValid(initialValid);

  mitk::NavigationData::Pointer snd3 = mitk::NavigationData::New();
  snd3->SetPosition(sourcePos3);
  snd3->SetOrientation(sourceOri3);
  snd3->SetPositionAccuracy(initialError);
  snd3->SetDataValid(initialValid);

  mitk::NavigationData::Pointer tnd1 = mitk::NavigationData::New();
  tnd1->SetPosition(targetPos1);
  tnd1->SetOrientation(targetOri1);
  tnd1->SetPositionAccuracy(initialError);
  tnd1->SetDataValid(initialValid);

  mitk::NavigationData::Pointer tnd2 = mitk::NavigationData::New();
  tnd2->SetPosition(targetPos2);
  tnd2->SetOrientation(targetOri2);
  tnd2->SetPositionAccuracy(initialError);
  tnd2->SetDataValid(initialValid);

  mitk::NavigationData::Pointer tnd3 = mitk::NavigationData::New();
  tnd3->SetPosition(targetPos3);
  tnd3->SetOrientation(targetOri3);
  tnd3->SetPositionAccuracy(initialError);
  tnd3->SetDataValid(initialValid);

  std::vector<mitk::NavigationData::Pointer> emptySourceNDs;

  std::vector<mitk::NavigationData::Pointer> oneSourceNDs;
  oneSourceNDs.push_back(snd1);

  std::vector<mitk::NavigationData::Pointer> twoSourceNDs;
  twoSourceNDs.push_back(snd1);
  twoSourceNDs.push_back(snd2);

  std::vector<mitk::NavigationData::Pointer> threeSourceNDs;
  threeSourceNDs.push_back(snd1);
  threeSourceNDs.push_back(snd2);
  threeSourceNDs.push_back(snd3);

  std::vector<mitk::NavigationData::Pointer> emptyTargetNDs;

  std::vector<mitk::NavigationData::Pointer> oneTargetNDs;
  oneTargetNDs.push_back(tnd1);

  std::vector<mitk::NavigationData::Pointer> twoTargetNDs;
  twoTargetNDs.push_back(tnd1);
  twoTargetNDs.push_back(tnd2);

  std::vector<mitk::NavigationData::Pointer> threeTargetNDs;
  threeTargetNDs.push_back(tnd1);
  threeTargetNDs.push_back(tnd2);
  threeTargetNDs.push_back(tnd3);

  // ------------------ setting no NDs ------------------

  myFilter->SetSourceNavigationDatas(emptySourceNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetSourceLandmarks()->IsEmpty() == true, "Testing behaviour if setting no source NDs");

  myFilter->SetSourceNavigationDatas(emptyTargetNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetTargetLandmarks()->IsEmpty() == true, "Testing behaviour if setting no target NDs");

  // ------------------ setting one ND ------------------
  myFilter->SetSourceNavigationDatas(oneSourceNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetSourceLandmarks()->GetSize() == 3, "Testing if 3 source points are generated from one source ND");

  myFilter->SetTargetNavigationDatas(oneTargetNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetTargetLandmarks()->GetSize() == 3, "Testing if 3 target points are generated from one target ND");

  // ------------------ setting two NDs ------------------
  myFilter->SetSourceNavigationDatas(twoSourceNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetSourceLandmarks()->GetSize() == 6, "Testing if 6 source points are generated from two source NDs");

  myFilter->SetTargetNavigationDatas(twoTargetNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetTargetLandmarks()->GetSize() == 6, "Testing if 6 target points are generated from two target NDs");

  // ------------------ setting three NDs ------------------
  myFilter->SetSourceNavigationDatas(threeSourceNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetSourceLandmarks()->GetSize() == 3, "Testing if 3 source NDs are passed to 3 source points");

  myFilter->SetTargetNavigationDatas(threeTargetNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetTargetLandmarks()->GetSize() == 3, "Testing if 3 target NDs are passed to 3 target points");



  // ------------------ setting different number of NDs for source and target ------------------
  bool firstInitialize = myFilter->InitializeTransform();
  myFilter->SetTargetNavigationDatas(twoTargetNDs);
  MITK_TEST_CONDITION_REQUIRED((firstInitialize == true && myFilter->InitializeTransform() == false), "Testing if initialization is denied, if different number of source and target NDs are set");

  // ------------------ reinit of this filter ------------------
  bool sourcePointsSet = myFilter->GetSourceLandmarks()->GetSize() > 0;
  bool targetPointsSet = myFilter->GetTargetLandmarks()->GetSize() > 0;
  MITK_TEST_CONDITION_REQUIRED(sourcePointsSet && targetPointsSet, "Testing if there are source and target landmarks set in the superclass");

  myFilter->ReinitFilter();
  bool sourcePointsCleared = myFilter->GetSourceLandmarks()->GetSize() == 0;
  bool targetPointsCleared = myFilter->GetTargetLandmarks()->GetSize() == 0;
  MITK_TEST_CONDITION_REQUIRED(sourcePointsCleared && targetPointsCleared, "Testing if reinit of filter was successful");

  // ------------------ testing the point generation ------------------
  
  myFilter->SetSourceNavigationDatas(oneSourceNDs); // set the ND with sourcePos1 and sourceOri1 for that the points will be generated

  itk::QuaternionRigidTransform<double>::Pointer quaternionTransform = itk::QuaternionRigidTransform<double>::New();
  vnl_quaternion<double> const vnlQuatIn(sourceOri1.x(), sourceOri1.y(), sourceOri1.z(), sourceOri1.r());
  quaternionTransform->SetRotation(vnlQuatIn);
  
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

  // quaternion transform the points
  pointA = quaternionTransform->GetRotationMatrix() * pointA;
  pointB = quaternionTransform->GetRotationMatrix() * pointB;
  pointC = quaternionTransform->GetRotationMatrix() * pointC;

  bool firstPoint0Same = sourcePos1[0] == myFilter->GetSourceLandmarks()->GetPoint(0)[0] - pointA[0];
  bool firstPoint1Same = sourcePos1[1] == myFilter->GetSourceLandmarks()->GetPoint(0)[1] - pointA[1];
  bool firstPoint2Same = sourcePos1[2] == myFilter->GetSourceLandmarks()->GetPoint(0)[2] - pointA[2];

  bool firstPointCorrect = firstPoint0Same && firstPoint1Same && firstPoint2Same;

  bool secondPoint0Same = sourcePos1[0] == myFilter->GetSourceLandmarks()->GetPoint(1)[0] - pointB[0];
  bool secondPoint1Same = sourcePos1[1] == myFilter->GetSourceLandmarks()->GetPoint(1)[1] - pointB[1];
  bool secondPoint2Same = sourcePos1[2] == myFilter->GetSourceLandmarks()->GetPoint(1)[2] - pointB[2];

  bool secondPointCorrect = secondPoint0Same && secondPoint1Same && secondPoint2Same;

  bool thirdPoint0Same = sourcePos1[0] == myFilter->GetSourceLandmarks()->GetPoint(2)[0] - pointC[0];
  bool thirdPoint1Same = sourcePos1[1] == myFilter->GetSourceLandmarks()->GetPoint(2)[1] - pointC[1];
  bool thirdPoint2Same = sourcePos1[2] == myFilter->GetSourceLandmarks()->GetPoint(2)[2] - pointC[2];

  bool thirdPointCorrect = thirdPoint0Same && thirdPoint1Same && thirdPoint2Same;

  //MITK_TEST_CONDITION_REQUIRED(firstPointCorrect && secondPointCorrect && thirdPointCorrect, "Testing if point generation is correct");


  // deleting helper objects
  myFilter = NULL;
  quaternionTransform = NULL;
  snd1 = NULL;
  snd2 = NULL;
  snd3 = NULL;
  tnd1 = NULL;
  tnd2 = NULL;
  tnd3 = NULL;

    // always end with this!
  MITK_TEST_END();
}


