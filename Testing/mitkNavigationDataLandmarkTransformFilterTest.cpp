/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 16010 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataLandmarkTransformFilter.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"

#include <iostream>


class mitkNavigationDataLandmarkTransformFilterTestClass{ public:

/* false if difference abs(x1-y1) < delta, else true */
static bool compareTwoVectors(mitk::NavigationData::PositionType x, mitk::NavigationData::PositionType y)
{ 
  float delta = 1.0e-3;
  if((abs(x[0]-y[0]) < delta ) && (abs(x[1]-y[1]) < delta ) && (abs(x[2]-y[2]) < delta ))
    return true;
  else
    return false;
}

/* false if difference abs(x1-y1) < delta, else true */
static bool compareTwoQuaternions(mitk::NavigationData::OrientationType x, mitk::NavigationData::OrientationType y)
{ 
  float delta = 1.0e-3;
  if((abs(x[0]-y[0]) < delta ) && (abs(x[1]-y[1]) < delta ) && (abs(x[2]-y[2]) < delta ))
    return true;
  else
    return false;
}

};


/**Documentation
 *  test for the class "NavigationDataLandmarkTransformFilter".
 */
int mitkNavigationDataLandmarkTransformFilterTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("NavigationDataLandmarkTransformFilter")

  // let's create an object of our class  
  mitk::NavigationDataLandmarkTransformFilter::Pointer myFilter = mitk::NavigationDataLandmarkTransformFilter::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myFilter.IsNotNull(),"Testing instantiation");

  /*create helper objects: source and target pointSets for landmark transform*/
  mitk::Point3D sPoint1, sPoint2, sPoint3, tPoint1, tPoint2, tPoint3;
  mitk::FillVector3D(sPoint1, 1.1, 1.1, 1.1);
  mitk::FillVector3D(sPoint2, 2.2, 2.2, 2.2);
  mitk::FillVector3D(sPoint3, 3.3, 3.3, 3.3);

  mitk::FillVector3D(tPoint1, 2.1, 2.1, 2.1);
  mitk::FillVector3D(tPoint2, 3.2, 3.2, 3.2);
  mitk::FillVector3D(tPoint3, 4.3, 4.3, 4.3);
  
  mitk::PointSet::Pointer sourcePoints = mitk::PointSet::New();
  mitk::PointSet::Pointer targetPoints = mitk::PointSet::New();
    
  sourcePoints->SetPoint(0,sPoint1);
  sourcePoints->SetPoint(1,sPoint2);
  sourcePoints->SetPoint(2,sPoint3);
 
  targetPoints->SetPoint(0,tPoint1);
  targetPoints->SetPoint(1,tPoint2);
  targetPoints->SetPoint(2,tPoint3);

  /* create helper objects: navigation data with position as origin, zero quaternion, zero error and data valid */
  mitk::NavigationData::PositionType initialPos1,initialPos2, resultPos1,resultPos2;
  mitk::FillVector3D(initialPos1, 1.1, 1.1, 1.1);
  mitk::FillVector3D(initialPos2, 22.2,22.2, 22.2);
  mitk::FillVector3D(resultPos1, 2.1, 2.1, 2.1);
  mitk::FillVector3D(resultPos2, 23.2, 23.2, 23.2);
  mitk::NavigationData::OrientationType initialOri(0.1, 0.1, 0.1, 0.1);
  mitk::NavigationData::ErrorType initialError(0.0);
  bool initialValid(true);

  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->SetPosition(initialPos1);
  nd1->SetOrientation(initialOri);
  nd1->SetError(initialError);
  nd1->SetDataValid(initialValid);

  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
  nd2->SetPosition(initialPos2);
  nd2->SetOrientation(initialOri);
  nd2->SetError(initialError);
  nd2->SetDataValid(initialValid);


  myFilter->SetInput(0,nd1);
  MITK_TEST_CONDITION(myFilter->GetInput(0) == nd1, "Testing Set-/GetInput() ND1");

  mitk::NavigationData* output1 = myFilter->GetOutput(0);
  MITK_TEST_CONDITION_REQUIRED(output1 != NULL, "Testing GetOutput() ND1");

  myFilter->SetSourcePoints(sourcePoints);
  myFilter->SetTargetPoints(targetPoints);
  
  //------------------------landmark transform should be initialized at this point------------------------
  output1->Update();
  MITK_TEST_CONDITION_REQUIRED(
     mitkNavigationDataLandmarkTransformFilterTestClass::compareTwoVectors(output1->GetPosition(),  resultPos1),
     "Testing ND1 position correctly transformed");


  //------------------------add another ND------------------------
  myFilter->SetInput(1,nd2);
  MITK_TEST_CONDITION(myFilter->GetInput(1) == nd2, "Testing Set-/GetInput() ND2");

  mitk::NavigationData* output2 = myFilter->GetOutput(1);
  MITK_TEST_CONDITION_REQUIRED(output2 != NULL, "Testing GetOutput() ND2");
  
  //------------------------update output1 but check result2------------------------
  output1->Update();
  MITK_TEST_CONDITION_REQUIRED(
    mitkNavigationDataLandmarkTransformFilterTestClass::compareTwoVectors(output2->GetPosition(),  resultPos2),
    "Testing ND2 position correctly transformed");

  //------------------------update ND on slot 1------------------------
  mitk::FillVector3D(initialPos2, 222.22, 222.22, 222.22);
  mitk::FillVector3D(resultPos2, 223.22, 223.22, 223.22);
  nd2->SetPosition(initialPos2);
  myFilter->SetInput(1,nd2);
  MITK_TEST_CONDITION(myFilter->GetInput(1) == nd2, "Testing Set-/GetInput() ND2 after updating value");

  output2 = myFilter->GetOutput(1);
  MITK_TEST_CONDITION_REQUIRED(output2 != NULL, "Testing GetOutput() ND2 after updating value");

  //------------------------update output2 and check result2------------------------
  output2->Update();
  MITK_TEST_CONDITION(
    mitkNavigationDataLandmarkTransformFilterTestClass::compareTwoVectors(output2->GetPosition(),  resultPos2),
    "Testing ND2 position correctly transformed after updating value");

  
  //------------------------change target PointSet------------------------
  mitk::FillVector3D(tPoint1, 3.1, 3.1, 3.1);
  mitk::FillVector3D(tPoint2, 4.2, 4.2, 4.2);
  mitk::FillVector3D(tPoint3, 5.3, 5.3, 5.3);
  mitk::FillVector3D(resultPos1, 3.1 ,3.1 ,3.1);
  mitk::FillVector3D(resultPos2, 224.22, 224.22, 224.22);


  targetPoints->SetPoint(0,tPoint1);
  targetPoints->SetPoint(1,tPoint2);
  targetPoints->SetPoint(2,tPoint3);
  
  myFilter->SetTargetPoints(targetPoints);

  output1->Update();
  
  MITK_TEST_CONDITION(
    mitkNavigationDataLandmarkTransformFilterTestClass::compareTwoVectors(output1->GetPosition(),  resultPos1),
    "Testing ND1 position correctly transformed after targetPointSet changed");

  MITK_TEST_CONDITION(
    mitkNavigationDataLandmarkTransformFilterTestClass::compareTwoVectors(output2->GetPosition(),  resultPos2),
    "Testing ND2 position correctly transformed after targetPointSet changed");


  //------------------------change source PointSet------------------------
  mitk::FillVector3D(sPoint1, 0.1, 0.1, 0.1);
  mitk::FillVector3D(sPoint2, 1.2, 1.2, 1.2);
  mitk::FillVector3D(sPoint3, 2.3, 2.3, 2.3);
  mitk::FillVector3D(resultPos1, 4.1 ,4.1 ,4.1);
  mitk::FillVector3D(resultPos2, 225.22, 225.22, 225.22);


  sourcePoints->SetPoint(0,sPoint1);
  sourcePoints->SetPoint(1,sPoint2);
  sourcePoints->SetPoint(2,sPoint3);

  myFilter->SetSourcePoints(sourcePoints);

  output1->Update();

  MITK_TEST_CONDITION(
    mitkNavigationDataLandmarkTransformFilterTestClass::compareTwoVectors(output1->GetPosition(),  resultPos1),
    "Testing ND1 position correctly transformed after sourcePointSet changed");

  MITK_TEST_CONDITION(
    mitkNavigationDataLandmarkTransformFilterTestClass::compareTwoVectors(output2->GetPosition(),  resultPos2),
    "Testing ND2 position correctly transformed after sourcePointSet changed");


  //------------------------catch exception --> source points < 3------------------------

  bool exceptionCatched=false;

  try
  {
    mitk::NavigationDataLandmarkTransformFilter::Pointer myFilter2 = mitk::NavigationDataLandmarkTransformFilter::New();
    MITK_TEST_CONDITION_REQUIRED(myFilter2.IsNotNull(),"Testing instantiation for second filter");

    mitk::PointSet::Pointer sourcePoints2 = mitk::PointSet::New();
    myFilter2->SetSourcePoints(sourcePoints2);
  }
  catch (std::exception& exp)
  {  exceptionCatched=true; /*std::cout<<exp.what()<<std::endl;*/}

  MITK_TEST_CONDITION(exceptionCatched,"Testing source points < 3");

  //------------------------catch exception --> target points < 3------------------------
  try
  { 
    exceptionCatched=false;
    mitk::NavigationDataLandmarkTransformFilter::Pointer myFilter3 = mitk::NavigationDataLandmarkTransformFilter::New();
    MITK_TEST_CONDITION_REQUIRED(myFilter3.IsNotNull(),"Testing instantiation for second filter");

    mitk::PointSet::Pointer targetPoints2 = mitk::PointSet::New();
    myFilter3->SetTargetPoints(targetPoints2);
    
  }
  catch (std::exception& exp)
  {  exceptionCatched=true;  /*std::cout<<exp.what()<<std::endl;*/ }
  MITK_TEST_CONDITION(exceptionCatched,"Testing target points < 3");


  //------------------------rotate orientation------------------------
  myFilter=NULL;
  myFilter = mitk::NavigationDataLandmarkTransformFilter::New();

  mitk::FillVector3D(sPoint1, 1.1, 1.1, 1.1);
  mitk::FillVector3D(sPoint2, 1.1, -1.1, 1.1);
  mitk::FillVector3D(sPoint3, -1.1, -1.1, 1.1);

  mitk::FillVector3D(tPoint1, -1.1, 1.1, 1.1);
  mitk::FillVector3D(tPoint2, 1.1, 1.1, 1.1);
  mitk::FillVector3D(tPoint3, 1.1, -1.1, 1.1);

  sourcePoints->SetPoint(0,sPoint1);
  sourcePoints->SetPoint(1,sPoint2);
  sourcePoints->SetPoint(2,sPoint3);

  targetPoints->SetPoint(0,tPoint1);
  targetPoints->SetPoint(1,tPoint2);
  targetPoints->SetPoint(2,tPoint3);

  myFilter->SetSourcePoints(sourcePoints);
  myFilter->SetTargetPoints(targetPoints);

  //set initial orientation (x y z r)
  mitk::NavigationData::OrientationType initialQuat(0.0, 0.0, 0.0, 1.0);
  mitk::NavigationData::OrientationType resultQuat(0.0, 0.0, -0.7071, -0.7071);
  
  //set position
  mitk::FillVector3D(initialPos1, 2.2, 2.2, 2.2);
  mitk::FillVector3D(resultPos1, -2.2, 2.2, 2.2);
  
  nd1->SetOrientation(initialQuat);
  nd1->SetPosition(initialPos1);

  myFilter->SetInput(0,nd1);

  output1 = myFilter->GetOutput(0);
  output1->Update();
  
  MITK_TEST_CONDITION( 
    mitkNavigationDataLandmarkTransformFilterTestClass::compareTwoVectors(output1->GetPosition(), resultPos1),
    "Testing ND1 position correctly transformed ");

  MITK_TEST_CONDITION( 
    mitkNavigationDataLandmarkTransformFilterTestClass::compareTwoQuaternions(output1->GetOrientation(), resultQuat),
    "Testing ND1 orientation correctly transformed ");
  
    // always end with this!
  MITK_TEST_END();

}

