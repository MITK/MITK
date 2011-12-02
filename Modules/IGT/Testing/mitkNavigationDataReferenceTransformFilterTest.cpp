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

  // setting no NDs

  myFilter->SetSourceNavigationDatas(emptySourceNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetSourceLandmarks()->IsEmpty() == true, "Testing behaviour if setting no source NDs");

  myFilter->SetSourceNavigationDatas(emptyTargetNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetTargetLandmarks()->IsEmpty() == true, "Testing behaviour if setting no target NDs");

  // setting one ND
  myFilter->SetSourceNavigationDatas(oneSourceNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetSourceLandmarks()->GetSize() == 3, "Testing if 3 source points are generated from one source ND");

  myFilter->SetTargetNavigationDatas(oneTargetNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetTargetLandmarks()->GetSize() == 3, "Testing if 3 target points are generated from one target ND");

  // setting two NDs
  myFilter->SetSourceNavigationDatas(twoSourceNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetSourceLandmarks()->GetSize() == 6, "Testing if 6 source points are generated from two source NDs");

  myFilter->SetTargetNavigationDatas(twoTargetNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetTargetLandmarks()->GetSize() == 6, "Testing if 6 target points are generated from two target NDs");

  // setting three NDs
  myFilter->SetSourceNavigationDatas(threeSourceNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetSourceLandmarks()->GetSize() == 3, "Testing if 3 source NDs are passed to 3 source points");

  myFilter->SetTargetNavigationDatas(threeTargetNDs);
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetTargetLandmarks()->GetSize() == 3, "Testing if 3 target NDs are passed to 3 target points");




  //mitk::NavigationData::PositionType spos1 = myFilter->GetSourcePoints().at(0);
  //mitk::NavigationData::PositionType spos2 = myFilter->GetSourcePoints().at(1);
  //mitk::NavigationData::PositionType spos3 = myFilter->GetSourcePoints().at(2);

  //mitk::NavigationData::PositionType::RealType distance1 = spos1.EuclideanDistanceTo(spos2);
  //mitk::NavigationData::PositionType::RealType distance2 = spos1.EuclideanDistanceTo(spos3);

  //MITK_TEST_CONDITION(mitk::Equal(distance1, distance2), "Testing distance between landmark Point and virtual landmark Points");

  //MITK_TEST_CONDITION_REQUIRED(myFilter->GetReferencePoints().size() == 0, "Testing ReferencePoints size before initialization");
  //
  //MITK_TEST_CONDITION_REQUIRED(myFilter->InitializeReferenceLandmarks() == true, "Testing performing InitializeReferenceLandmarks()");
  //MITK_TEST_CONDITION_REQUIRED(myFilter->GetReferencePoints().size() == myFilter->GetSourcePoints().size(), "Testing ReferencePoints size after initialization");

  //
  //mitk::NavigationDataReferenceTransformFilter::QuaternionTransformType::Pointer quatTransform = mitk::NavigationDataReferenceTransformFilter::QuaternionTransformType::New();
  //mitk::NavigationDataReferenceTransformFilter::QuaternionTransformType::Pointer inverseQuatTransform = mitk::NavigationDataReferenceTransformFilter::QuaternionTransformType::New();
  //mitk::NavigationDataReferenceTransformFilter::QuaternionTransformType::VnlQuaternionType doubleOrientation(initialOri1.x(), initialOri1.y(),initialOri1.z(),initialOri1.r());

  //quatTransform->SetRotation(doubleOrientation);
  //quatTransform->SetOffset(initialPos1.GetVectorFromOrigin());
  //quatTransform->Modified();
  //quatTransform->GetInverse(inverseQuatTransform);


  //for(unsigned int i = 0; i < myFilter->GetReferencePoints().size(); ++i)
  //{
  // MITK_TEST_CONDITION_REQUIRED(
  //  mitk::Equal(myFilter->GetReferencePoints().at(i), inverseQuatTransform->TransformPoint(myFilter->GetSourcePoints().at(i))), "Testing inverse quaternion transform of SourcePoints");
  //}


  ///* create helper object: moved reference input ND */
  //mitk::NavigationData::PositionType newPos1;
  //mitk::FillVector3D(newPos1, 1.1, -16.1, 21.1);
  //mitk::NavigationData::OrientationType newOrientation(0.1, 0.2, 0.3, 0.4);
  //mitk::NavigationData::Pointer newnd1 = mitk::NavigationData::New();
  //newnd1->SetPosition(newPos1);
  //newnd1->SetOrientation(newOrientation);
  //newnd1->SetPositionAccuracy(initialError);
  //newnd1->SetDataValid(initialValid);


  //doubleOrientation[0] = newOrientation.x();
  //doubleOrientation[1] = newOrientation.y();
  //doubleOrientation[2] = newOrientation.z();
  //doubleOrientation[3] = newOrientation.r();

  //quatTransform->SetRotation(doubleOrientation);
  //quatTransform->SetOffset(newPos1.GetVectorFromOrigin());
  //quatTransform->Modified();

  //myFilter->SetInput(0,newnd1);
  //output1->Update();


  //for(unsigned int i = 0; i < myFilter->GetReferencePoints().size();  ++i)
  //{
  //  MITK_TEST_CONDITION_REQUIRED(
  //  mitk::Equal(quatTransform->TransformPoint(myFilter->GetReferencePoints().at(i)), myFilter->GetSourcePoints().at(i)),  "Testing quaternion transform of ReferencePoints ") ;
  //}
 


  //// testing landmark point generation with 3 reference inputs */

  ///* filling reference input IDs vector */
  //refIDs->clear();
  //refIDs->push_back(0);
  //refIDs->push_back(1);
  //refIDs->push_back(2);

  //myFilter->SetReferenceInputIndexes(refIDs);

  //bool oneSourceReg = myFilter->GetOneSourceRegistration();
  //bool refReg = myFilter->GetReferenceRegistration();
  //
  //MITK_TEST_CONDITION_REQUIRED(oneSourceReg == false && refReg == true,"Testing reference registration bool flags after switching from one reference input to three reference inputs" );
  //MITK_TEST_CONDITION_REQUIRED(myFilter->GetReferenceInputIndexes().size()==3, "Testing size of ReferenceInputIndexes");
  //
  //myFilter->GenerateSourceLandmarks();
  //myFilter->GenerateTargetLandmarks();
  //MITK_TEST_CONDITION_REQUIRED(myFilter->IsInitialized() == true, "Testing IsInitialized() 3 after reference input source and target points are set");



  //mitk::Point3D sPoint1, sPoint2, sPoint3, tPoint1, tPoint2, tPoint3;
  //mitk::FillVector3D(sPoint1, 1.1, 1.1, 1.1);
  //mitk::FillVector3D(sPoint2, 2.2, 2.2, 2.2);
  //mitk::FillVector3D(sPoint3, 3.3, 3.3, 3.3);

  //mitk::FillVector3D(tPoint1, 2.1, 2.1, 2.1);
  //mitk::FillVector3D(tPoint2, 3.2, 3.2, 3.2);
  //mitk::FillVector3D(tPoint3, 4.3, 4.3, 4.3);
  //
  //mitk::PointSet::Pointer sourcePoints = mitk::PointSet::New();
  //mitk::PointSet::Pointer targetPoints = mitk::PointSet::New();

  //sourcePoints->SetPoint(0,sPoint1);
  //sourcePoints->SetPoint(1,sPoint2);
  //sourcePoints->SetPoint(2,sPoint3);
 
  //targetPoints->SetPoint(0,tPoint1);
  //targetPoints->SetPoint(1,tPoint2);
  //targetPoints->SetPoint(2,tPoint3);


  ///* create helper objects: navigation data with position as origin, zero quaternion, zero error and data valid */
  //mitk::NavigationData::PositionType resultPos1, resultPos2;

  //mitk::FillVector3D(initialPos1, 1.1, 1.1, 1.1);
  //mitk::FillVector3D(initialPos2, 22.2,22.2, 22.2);
  //mitk::FillVector3D(resultPos1, 2.1, 2.1, 2.1);
  //mitk::FillVector3D(resultPos2, 23.2, 23.2, 23.2);
  //mitk::NavigationData::OrientationType initialOri(0.1, 0.1, 0.1, 0.1);

  //nd1->SetPosition(initialPos1);
  //nd1->SetOrientation(initialOri);
  //nd1->SetPositionAccuracy(initialError);
  //nd1->SetDataValid(initialValid);

  //nd2->SetPosition(initialPos2);
  //nd2->SetOrientation(initialOri);
  //nd2->SetPositionAccuracy(initialError);
  //nd2->SetDataValid(initialValid);

  //myFilter->SetInput(0,nd1);

  //myFilter->SetSourceLandmarks(sourcePoints);
  //myFilter->SetTargetLandmarks(targetPoints);
  //
  //MITK_TEST_CONDITION_REQUIRED(myFilter->GetOneSourceRegistration() == false,"Testing switch from OneSourceRegistration true to false" );



  //  

  ////------------------------landmark transform should be initialized at this point------------------------
  //output1->Update();
  //MITK_TEST_CONDITION_REQUIRED(
  //   mitk::Equal(output1->GetPosition(),  resultPos1),
  //   "Testing ND1 position correctly transformed");

  ////------------------------add another ND------------------------
  //myFilter->SetInput(1,nd2);
  //MITK_TEST_CONDITION(myFilter->GetInput(1) == nd2, "Testing Set-/GetInput() ND2");

  //output2 = myFilter->GetOutput(1);
  //MITK_TEST_CONDITION_REQUIRED(output2 != NULL, "Testing GetOutput() ND2");

  ////------------------------update output1 but check result2------------------------
  //output1->Update();
  //MITK_TEST_CONDITION_REQUIRED(
  //  mitk::Equal(output2->GetPosition(),  resultPos2),
  //  "Testing ND2 position correctly transformed");

  ////------------------------update ND on slot 1------------------------
  //mitk::FillVector3D(initialPos2, 222.22, 222.22, 222.22);
  //mitk::FillVector3D(resultPos2, 223.22, 223.22, 223.22);
  //nd2->SetPosition(initialPos2);
  //myFilter->SetInput(1,nd2);
  //MITK_TEST_CONDITION(myFilter->GetInput(1) == nd2, "Testing Set-/GetInput() ND2 after updating value");

  //output2 = myFilter->GetOutput(1);
  //MITK_TEST_CONDITION_REQUIRED(output2 != NULL, "Testing GetOutput() ND2 after updating value");

  //
  ////------------------------update output2 and check result2------------------------
  //output2->Update();
  //MITK_TEST_CONDITION(
  //  mitk::Equal(output2->GetPosition(),  resultPos2),
  //  "Testing ND2 position correctly transformed after updating value");

  //
  ////------------------------change target PointSet------------------------
  //mitk::FillVector3D(tPoint1, 3.1, 3.1, 3.1);
  //mitk::FillVector3D(tPoint2, 4.2, 4.2, 4.2);
  //mitk::FillVector3D(tPoint3, 5.3, 5.3, 5.3);
  //mitk::FillVector3D(resultPos1, 3.1 ,3.1 ,3.1);
  //mitk::FillVector3D(resultPos2, 224.22, 224.22, 224.22);


  //targetPoints->SetPoint(0,tPoint1);
  //targetPoints->SetPoint(1,tPoint2);
  //targetPoints->SetPoint(2,tPoint3);
  //
  //myFilter->SetTargetLandmarks(targetPoints);

  //output1->Update();
  //
  //MITK_TEST_CONDITION(
  //  mitk::Equal(output1->GetPosition(),  resultPos1),
  //  "Testing ND1 position correctly transformed after targetPointSet changed");

  //MITK_TEST_CONDITION(
  //  mitk::Equal(output2->GetPosition(),  resultPos2),
  //  "Testing ND2 position correctly transformed after targetPointSet changed");

  // //------------------------change source PointSet------------------------
  //mitk::FillVector3D(sPoint1, 0.1, 0.1, 0.1);
  //mitk::FillVector3D(sPoint2, 1.2, 1.2, 1.2);
  //mitk::FillVector3D(sPoint3, 2.3, 2.3, 2.3);
  //mitk::FillVector3D(resultPos1, 4.1 ,4.1 ,4.1);
  //mitk::FillVector3D(resultPos2, 225.22, 225.22, 225.22);


  //sourcePoints->SetPoint(0,sPoint1);
  //sourcePoints->SetPoint(1,sPoint2);
  //sourcePoints->SetPoint(2,sPoint3);

  //myFilter->SetSourceLandmarks(sourcePoints);

  //output1->Update();

  //MITK_TEST_CONDITION(
  //  mitk::Equal(output1->GetPosition(),  resultPos1),
  //  "Testing ND1 position correctly transformed after sourcePointSet changed");

  //MITK_TEST_CONDITION(
  //  mitk::Equal(output2->GetPosition(),  resultPos2),
  //  "Testing ND2 position correctly transformed after sourcePointSet changed");



  ////------------------------rotate orientation------------------------
  //myFilter=NULL;
  //myFilter = mitk::NavigationDataReferenceTransformFilter::New();

  //mitk::FillVector3D(sPoint1, 1.1, 1.1, 1.1);
  //mitk::FillVector3D(sPoint2, 1.1, -1.1, 1.1);
  //mitk::FillVector3D(sPoint3, -1.1, -1.1, 1.1);

  //mitk::FillVector3D(tPoint1, -1.1, 1.1, 1.1);
  //mitk::FillVector3D(tPoint2, 1.1, 1.1, 1.1);
  //mitk::FillVector3D(tPoint3, 1.1, -1.1, 1.1);

  //sourcePoints->SetPoint(0,sPoint1);
  //sourcePoints->SetPoint(1,sPoint2);
  //sourcePoints->SetPoint(2,sPoint3);

  //targetPoints->SetPoint(0,tPoint1);
  //targetPoints->SetPoint(1,tPoint2);
  //targetPoints->SetPoint(2,tPoint3);

  //myFilter->SetSourceLandmarks(sourcePoints);
  //myFilter->SetTargetLandmarks(targetPoints);

  ////set initial orientation (x y z r)
  //mitk::NavigationData::OrientationType initialQuat(0.0, 0.0, 0.0, 1.0);
  //mitk::NavigationData::OrientationType resultQuat(0.0, 0.0, -0.7071, -0.7071);
  //
  ////set position
  //mitk::FillVector3D(initialPos1, 2.2, 2.2, 2.2);
  //mitk::FillVector3D(resultPos1, -2.2, 2.2, 2.2);
  //
  //nd1->SetOrientation(initialQuat);
  //nd1->SetPosition(initialPos1);

  //myFilter->SetInput(0,nd1);

  //output1 = myFilter->GetOutput(0);
  //output1->Update();
  //
  //MITK_TEST_CONDITION( 
  //  mitk::Equal(output1->GetPosition(), resultPos1),
  //  "Testing ND1 position correctly transformed ");

  //MITK_TEST_CONDITION( 
  //  mitk::Equal(output1->GetOrientation(), resultQuat),
  //  "Testing ND1 orientation correctly transformed ");



  //


  //delete refIDs;

    // always end with this!
  MITK_TEST_END();
}


