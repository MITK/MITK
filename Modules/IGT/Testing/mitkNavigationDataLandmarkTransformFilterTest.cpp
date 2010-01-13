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
  mitk::ScalarType initialError(0.0);
  bool initialValid(true);

  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->SetPosition(initialPos1);
  nd1->SetOrientation(initialOri);
  nd1->SetPositionAccuracy(initialError);
  nd1->SetDataValid(initialValid);

  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
  nd2->SetPosition(initialPos2);
  nd2->SetOrientation(initialOri);
  nd2->SetPositionAccuracy(initialError);
  nd2->SetDataValid(initialValid);


  myFilter->SetInput(0,nd1);
  MITK_TEST_CONDITION(myFilter->GetInput(0) == nd1, "Testing Set-/GetInput() ND1");

  mitk::NavigationData* output1 = myFilter->GetOutput(0);
  MITK_TEST_CONDITION_REQUIRED(output1 != NULL, "Testing GetOutput() ND1");

  MITK_TEST_CONDITION(myFilter->IsInitialized() == false, "Testing IsInitialized() before setting source points");

  myFilter->SetSourcePoints(sourcePoints);
  MITK_TEST_CONDITION(myFilter->IsInitialized() == false, "Testing IsInitialized() after setting source points and before setting target points");

  mitk::PointSet::Pointer zeroTargetPoints = mitk::PointSet::New();
  
  MITK_TEST_FOR_EXCEPTION(itk::ExceptionObject, myFilter->SetTargetPoints(zeroTargetPoints));
  MITK_TEST_CONDITION(myFilter->IsInitialized() == false, "Testing IsInitialized() after setting target pointset with insufficient points");

  myFilter->SetTargetPoints(targetPoints);
  MITK_TEST_CONDITION(myFilter->IsInitialized() == true, "Testing IsInitialized() after setting source& target points");
  
  //------------------------landmark transform should be initialized at this point------------------------
  output1->Update();
  MITK_TEST_CONDITION_REQUIRED(
     mitk::Equal(output1->GetPosition(),  resultPos1),
     "Testing ND1 position correctly transformed");


  //------------------------add another ND------------------------
  myFilter->SetInput(1,nd2);
  MITK_TEST_CONDITION(myFilter->GetInput(1) == nd2, "Testing Set-/GetInput() ND2");

  mitk::NavigationData* output2 = myFilter->GetOutput(1);
  MITK_TEST_CONDITION_REQUIRED(output2 != NULL, "Testing GetOutput() ND2");
  
  //------------------------update output1 but check result2------------------------
  output1->Update();
  MITK_TEST_CONDITION_REQUIRED(
    mitk::Equal(output2->GetPosition(),  resultPos2),
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
    mitk::Equal(output2->GetPosition(),  resultPos2),
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
    mitk::Equal(output1->GetPosition(),  resultPos1),
    "Testing ND1 position correctly transformed after targetPointSet changed");

  MITK_TEST_CONDITION(
    mitk::Equal(output2->GetPosition(),  resultPos2),
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
    mitk::Equal(output1->GetPosition(),  resultPos1),
    "Testing ND1 position correctly transformed after sourcePointSet changed");

  MITK_TEST_CONDITION(
    mitk::Equal(output2->GetPosition(),  resultPos2),
    "Testing ND2 position correctly transformed after sourcePointSet changed");

  //--------------------- Test ICP initialization --------------------------
  {
    mitk::PointSet::Pointer sourcePoints = mitk::PointSet::New();
    mitk::Point3D s1, s2, s3, s4, s5, s6;
    mitk::FillVector3D(s1, 1.1, 1.1, 1.1);
    mitk::FillVector3D(s2, 2.2, 2.2, 2.2);
    mitk::FillVector3D(s3, 3.3, 3.3, 3.3);
    mitk::FillVector3D(s4, 4.4, 4.4, 4.4);
    mitk::FillVector3D(s5, 5.5, 5.5, 5.5);
    mitk::FillVector3D(s6, 6.6, 6.6, 6.6);
    sourcePoints->SetPoint(1, s4);  // use random source point order
    sourcePoints->SetPoint(2, s6);
    sourcePoints->SetPoint(3, s3);
    sourcePoints->SetPoint(4, s1);
    sourcePoints->SetPoint(5, s2);
    sourcePoints->SetPoint(6, s5);
    
    mitk::PointSet::Pointer targetPoints = mitk::PointSet::New();
    mitk::Point3D t1, t2, t3, t4, t5, t6;
    mitk::FillVector3D(t1, 2.1, 2.1, 102.1);  // ==> targets have offset [1, 1, 101]
    mitk::FillVector3D(t2, 3.2, 3.2, 103.2);
    mitk::FillVector3D(t3, 4.3, 4.3, 104.3);
    mitk::FillVector3D(t4, 5.4, 5.4, 105.4);
    mitk::FillVector3D(t5, 6.5, 6.5, 106.5);
    mitk::FillVector3D(t6, 7.6, 7.6, 107.6);
    targetPoints->SetPoint(1, t1);
    targetPoints->SetPoint(2, t2);
    targetPoints->SetPoint(3, t3);
    targetPoints->SetPoint(4, t4);  
    targetPoints->SetPoint(5, t5);
    targetPoints->SetPoint(6, t6);

    mitk::NavigationDataLandmarkTransformFilter::Pointer myFilter = mitk::NavigationDataLandmarkTransformFilter::New();
    myFilter->UseICPInitializationOn();
    myFilter->SetSourcePoints(sourcePoints);
    myFilter->SetTargetPoints(targetPoints);  // errors would raise exceptions

    // prepare input
    mitk::NavigationData::PositionType initialPos1, resultPos1;
    mitk::FillVector3D(initialPos1, 1.1, 1.1, 1.1);
    mitk::FillVector3D(resultPos1, 2.1, 2.1, 102.1);
    mitk::NavigationData::OrientationType initialOri(0.1, 0.1, 0.1, 0.1);
    mitk::ScalarType initialError(0.0);
    bool initialValid(true);
    mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
    nd1->SetPosition(initialPos1);
    nd1->SetOrientation(initialOri);
    nd1->SetPositionAccuracy(initialError);
    nd1->SetDataValid(initialValid);

    myFilter->SetInput(0, nd1);
    mitk::NavigationData::Pointer output = myFilter->GetOutput(0);

    output->Update();

    //MITK_TEST_CONDITION(mitk::Equal(output->GetPosition(),  resultPos1), "Testing ND1 position correctly transformed after ICP initialization");
  }

  //------------------------catch exception --> source points < 3------------------------
  mitk::NavigationDataLandmarkTransformFilter::Pointer myFilter2 = mitk::NavigationDataLandmarkTransformFilter::New();
  MITK_TEST_CONDITION_REQUIRED(myFilter2.IsNotNull(),"Testing instantiation for second filter");

  mitk::PointSet::Pointer sourcePoints2 = mitk::PointSet::New();
  MITK_TEST_FOR_EXCEPTION(std::exception, myFilter2->SetSourcePoints(sourcePoints2););


  //------------------------catch exception --> target points < 3------------------------
  mitk::NavigationDataLandmarkTransformFilter::Pointer myFilter3 = mitk::NavigationDataLandmarkTransformFilter::New();
  MITK_TEST_CONDITION_REQUIRED(myFilter3.IsNotNull(),"Testing instantiation for second filter");

  mitk::PointSet::Pointer targetPoints2 = mitk::PointSet::New();
  MITK_TEST_FOR_EXCEPTION(std::exception, myFilter3->SetTargetPoints(targetPoints2););


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
    mitk::Equal(output1->GetPosition(), resultPos1),
    "Testing ND1 position correctly transformed ");

  MITK_TEST_CONDITION( 
    mitk::Equal(output1->GetOrientation(), resultQuat),
    "Testing ND1 orientation correctly transformed ");
  
  //------------------------test FRE calculation------------------------
  mitk::PointSet::Pointer refSet = mitk::PointSet::New();
  mitk::PointSet::Pointer movSet = mitk::PointSet::New();

  mitk::Point3D refPoint;
  mitk::Point3D movPoint;

  //Point 0
  refPoint.Fill(0); refSet->SetPoint(0, refPoint);
  movPoint.Fill(1); movSet->SetPoint(0, movPoint);

  //Point 1
  refPoint[0]=3; refPoint[1]=0; refPoint[2]=0; refSet->SetPoint(1, refPoint);
  movPoint[0]=2; movPoint[1]=1; movPoint[2]=1; movSet->SetPoint(1, movPoint);

  //Point 2
  refPoint[0]=0; refPoint[1]=0; refPoint[2]=3; refSet->SetPoint(2, refPoint);
  movPoint[0]=1; movPoint[1]=1; movPoint[2]=2; movSet->SetPoint(2, movPoint);

  //Point 3
  refPoint[0]=3; refPoint[1]=0; refPoint[2]=3; refSet->SetPoint(3, refPoint);
  movPoint[0]=2; movPoint[1]=1; movPoint[2]=2; movSet->SetPoint(3, movPoint);


  //Point 4
  refPoint[0]=0; refPoint[1]=3; refPoint[2]=0; refSet->SetPoint(4, refPoint);
  movPoint[0]=1; movPoint[1]=2; movPoint[2]=1; movSet->SetPoint(4, movPoint);

  //Point 5
  refPoint[0]=3; refPoint[1]=3; refPoint[2]=0; refSet->SetPoint(5, refPoint);
  movPoint[0]=2; movPoint[1]=2; movPoint[2]=1; movSet->SetPoint(5, movPoint);

  //Point 6
  refPoint[0]=0; refPoint[1]=3; refPoint[2]=3; refSet->SetPoint(6, refPoint);
  movPoint[0]=1; movPoint[1]=2; movPoint[2]=2; movSet->SetPoint(6, movPoint);

  //Point 7
  refPoint[0]=3; refPoint[1]=3; refPoint[2]=3; refSet->SetPoint(7, refPoint);
  movPoint[0]=2; movPoint[1]=2; movPoint[2]=2; movSet->SetPoint(7, movPoint);

  mitk::NavigationDataLandmarkTransformFilter::Pointer myFREFilter = mitk::NavigationDataLandmarkTransformFilter::New();
  myFREFilter->SetSourcePoints(refSet);
  myFREFilter->SetTargetPoints(movSet);
  
  MITK_TEST_CONDITION_REQUIRED(myFREFilter->GetFRE() == (float) sqrt(3.0),"Testing mean error calculation")
  MITK_TEST_CONDITION_REQUIRED(myFREFilter->GetMaxError() == (float) sqrt(3.0),"Testing mean error calculation")
  MITK_TEST_CONDITION_REQUIRED(myFREFilter->GetMinError() == (float) sqrt(3.0),"Testing mean error calculation")
  MITK_TEST_CONDITION_REQUIRED(myFREFilter->GetRMSError() == (float) sqrt(3.0),"Testing mean error calculation")
  MITK_TEST_CONDITION_REQUIRED(myFREFilter->GetFREStdDev() == (float) 0.0,"Testing mean error calculation")

    // always end with this!
  MITK_TEST_END();
}
