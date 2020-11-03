/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNavigationDataTransformFilter.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"

#include <itkQuaternionRigidTransform.h>

#include <itkTransform.h>
#include <itkVector.h>
#include <iostream>


/**Documentation
*  test for the class "NavigationDataTransformFilter".
*/
typedef itk::VersorRigid3DTransform<mitk::ScalarType > TransformType;

typedef itk::Vector<double,3> VectorType;

int mitkNavigationDataTransformFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataTransformFilter")

    // let's create an object of our class
    mitk::NavigationDataTransformFilter::Pointer myFilter = mitk::NavigationDataTransformFilter::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myFilter.IsNotNull(),"Testing instantiation");

  /* create helper objects: navigation data with position as origin, zero quaternion, zero error and data valid */
  mitk::NavigationData::PositionType initialPos, resultPos;
  mitk::FillVector3D(initialPos, 1.1, 2.2, 3.3);
  mitk::FillVector3D(resultPos, 5.0, 5.0,5.0);


  mitk::NavigationData::OrientationType initialOri(0.0, 0.0, -0.7071, 0.7071);
  mitk::ScalarType initialError(0.0);
  bool initialValid(true);

  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->SetPosition(initialPos);
  nd1->SetOrientation(initialOri);
  nd1->SetPositionAccuracy(initialError);
  nd1->SetOrientationAccuracy(initialError);
  nd1->SetDataValid(initialValid);

  MITK_TEST_CONDITION(myFilter->GetPrecompose() == false, "Testing default Precompose mode");

  myFilter->SetPrecompose(true);
  MITK_TEST_CONDITION(myFilter->GetPrecompose() == true, "Testing Set-/GetPrecompose, part 1");

  myFilter->PrecomposeOff();
  MITK_TEST_CONDITION(myFilter->GetPrecompose() == false, "Testing PrecomposeOff");

  myFilter->PrecomposeOn();
  MITK_TEST_CONDITION(myFilter->GetPrecompose() == true, "Testing PrecomposeOn");

  myFilter->SetInput(nd1);
  MITK_TEST_CONDITION(myFilter->GetInput() == nd1, "Testing Set-/GetInput()");

  mitk::NavigationData* output = myFilter->GetOutput();

  /*test case no transform set*/
  MITK_TEST_FOR_EXCEPTION(std::exception, output->Update(););

  /* test transformation */
  mitk::NavigationDataTransformFilter::TransformType::Pointer transform =  mitk::NavigationDataTransformFilter::TransformType::New();
  VectorType translationVector;
  translationVector[0] = 3.9;
  translationVector[1] = 2.8;
  translationVector[2] = 1.7;


  //
  // Test translation-only transform with PrecomposeOff
  //
  transform->Translate(translationVector);

  myFilter->SetRigid3DTransform(transform);
  myFilter->SetPrecompose(false);

  output = myFilter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(output != nullptr, "Testing GetOutput()");

  output->Update(); // execute filter

  MITK_TEST_CONDITION(output->GetPosition() == resultPos, "Testing if translation was calculated correct");
  MITK_TEST_CONDITION( mitk::Equal( output->GetOrientation(),initialOri, 0.00001 ),"Testing if Orientation remains unchanged ");
  MITK_TEST_CONDITION(output->IsDataValid() == initialValid, "Testing if DataValid remains unchanged");

  // Repeat the same translation test with precompose on - results should
  // be the same for a translation-only transform
  myFilter->PrecomposeOn();
  MITK_TEST_CONDITION(output->GetPosition() == resultPos,
                      "Testing if precomposed translation was calculated correct");
  MITK_TEST_CONDITION(mitk::Equal(output->GetOrientation(),initialOri,0.00001),
                      "Testing if precomposed Orientation remains unchanged ");
  MITK_TEST_CONDITION(output->IsDataValid() == initialValid,
                      "Testing if precomposed DataValid remains unchanged");


  mitk::NavigationDataTransformFilter::Pointer myFilter2 = mitk::NavigationDataTransformFilter::New();
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myFilter2.IsNotNull(),"Testing instantiation");

  // Setup for subsequent PrecomposeOff/PrecomposeOn tests
  mitk::FillVector3D(initialPos, 1.1, 2.2, 3.3);
  initialOri[0] = 0;
  initialOri[1] = 0;
  initialOri[2] = 0;
  initialOri[3] = 1;

  nd1->SetPosition(initialPos);
  nd1->SetOrientation(initialOri);

  myFilter2->SetInput(nd1);

  //
  // Test rotation-only transform with PrecomposeOff
  //
  myFilter2->PrecomposeOff();

  // Rotate 90 degrees about Z axis
  mitk::NavigationDataTransformFilter::TransformType::MatrixType rotMatrix;
  rotMatrix[0][0] = 0;
  rotMatrix[0][1] = 1;
  rotMatrix[0][2] = 0;
  rotMatrix[1][0] = -1;
  rotMatrix[1][1] = 0;
  rotMatrix[1][2] = 0;
  rotMatrix[2][0] = 0;
  rotMatrix[2][1] = 0;
  rotMatrix[2][2] = 1;

  mitk::NavigationDataTransformFilter::TransformType::Pointer transform2 =  mitk::NavigationDataTransformFilter::TransformType::New();
  transform2->SetMatrix(rotMatrix);

  myFilter2->SetRigid3DTransform(transform2);

  mitk::NavigationData* output2 = myFilter2->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(output2 != nullptr, "Testing GetOutput()");

  output2->Update(); // execute filter

  mitk::FillVector3D(resultPos,  2.2, -1.1, 3.3);
  mitk::NavigationData::OrientationType resultOri(0.0, 0.0, -0.7071067690849304, 0.7071067690849304);
  MITK_TEST_CONDITION(mitk::Equal(output2->GetPosition(), resultPos, 0.00001), "Testing if position after rotation is correctly calculated");
  MITK_TEST_CONDITION(mitk::Equal(output2->GetOrientation(), resultOri, 0.00001),"Testing if orientation after rotation is correctly caclculated  ");
  MITK_TEST_CONDITION(output2->IsDataValid() == initialValid, "Testing if DataValid remains unchanged");

  //
  // Test rotation-only transform with PrecomposeOn
  //
  myFilter2->PrecomposeOn();
  output2->Update();

  // Output position should be the same as input position for rotation-only with PrecomposeOn
  MITK_TEST_CONDITION(output2->GetPosition() == initialPos,
                      "Testing if precomposed position after rotation is correctly calculated");
  MITK_TEST_CONDITION(mitk::Equal(output2->GetOrientation(), resultOri, 0.00001),
                      "Testing if precomposed orientation after rotation is correctly calculated");
  MITK_TEST_CONDITION(output2->IsDataValid() == initialValid,
                      "Testing if precomposed DataValid remains unchanged");

  //
  // test obscure rotation angle vs. ITK precision requirements
  //
  itk::QuaternionRigidTransform<mitk::NavigationDataTransformFilter::TransformType::ScalarType>::VnlQuaternionType obscureRotationQuat(37,29,71);
  obscureRotationQuat.normalize();                 // Just to demonstrate that even normalizing doesn't help

  itk::QuaternionRigidTransform<mitk::NavigationDataTransformFilter::TransformType::ScalarType>::Pointer rotation =
      itk::QuaternionRigidTransform<mitk::NavigationDataTransformFilter::TransformType::ScalarType>::New();
  rotation->SetRotation(obscureRotationQuat);

  mitk::NavigationDataTransformFilter::TransformType::Pointer transform3 = mitk::NavigationDataTransformFilter::TransformType::New();
  // For lack of an MITK Test macro that tests for *no* exception
  try
  {
    MITK_TEST_OUTPUT_NO_ENDL(<< "Testing whether NavigationDataTransformFilter's internal transform has sufficient precision for ITK ")
    transform3->SetMatrix(rotation->GetMatrix());
    MITK_TEST_OUTPUT(<< " [PASSED]")
    mitk::TestManager::GetInstance()->TestPassed();
  }
  catch(std::exception&)
  {
    MITK_TEST_OUTPUT(<< " [FAILED]")
    mitk::TestManager::GetInstance()->TestFailed();
  }

  //-----------------------------------------------------------------------------------------------------------
  /* now test the filter with multiple inputs */
  mitk::NavigationData::PositionType initialPos2, resultPos2;
  mitk::FillVector3D(initialPos, 1.1, 2.2, 3.3);
  mitk::FillVector3D(initialPos2, -1000.0, 567.89, 0.0);
  mitk::FillVector3D(resultPos, 2.2,-1.1, 3.3);
  mitk::FillVector3D(resultPos2, 567.89, 1000, 0.0);
  mitk::NavigationData::OrientationType initialOri2(0.5,0.5,0.5,0.5);
  mitk::NavigationData::OrientationType resultOri2(0.7071,0.0,0.0,0.7071);
  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();

  nd1->SetPosition(initialPos);
  nd1->SetOrientation(initialOri);
  nd1->SetPositionAccuracy(initialError);
  nd1->SetDataValid(initialValid);

  nd2->SetPosition(initialPos2);
  nd2->SetOrientation(initialOri2);
  nd2->SetPositionAccuracy(initialError);
  nd2->SetDataValid(initialValid);

  myFilter = nullptr;
  myFilter = mitk::NavigationDataTransformFilter::New();
  myFilter->SetRigid3DTransform(transform2);
  myFilter->SetInput(0, nd1);
  myFilter->SetInput(1, nd2);

  MITK_TEST_CONDITION(((myFilter->GetInput(0) == nd1)
    && (myFilter->GetInput(1) == nd2)), "Testing Set-/GetInput(index, data)");
  output = nullptr;
  output2 = nullptr;
  output = myFilter->GetOutput();
  output2 = myFilter->GetOutput(1);
  output2->Update(); // execute filter pipeline. this should update both outputs!
  MITK_TEST_CONDITION_REQUIRED(((output != nullptr) && (output2 != nullptr)), "Testing GetOutput(index)");

  MITK_TEST_CONDITION(mitk::Equal(output->GetPosition(), resultPos, 0.00001), "Testing if position rotation was calculated correct [output 0]");
  MITK_TEST_CONDITION(mitk::Equal(output->GetOrientation(), resultOri, 0.00001),"Testing if orientation rotation was calculated correct [output 0]");
  MITK_TEST_CONDITION(output->IsDataValid() == initialValid, "Testing if DataValid remains unchanged for output 0");

  MITK_TEST_CONDITION(mitk::Equal(output2->GetPosition(), resultPos2, 0.0001), "Testing if rotation was calculated correct [output 1]");
  MITK_TEST_CONDITION(mitk::Equal(output2->GetOrientation(), resultOri2, 0.00001),"Testing if orientation rotation was calculated correct [output 1]");
  MITK_TEST_CONDITION(output2->IsDataValid() == initialValid, "Testing if DataValid remains unchanged for output 1");
  //
  //  /* test if anything changes on second ->Update() */
  output->Update(); // nothing should change, since inputs remain unchanged
  MITK_TEST_CONDITION(mitk::Equal(output->GetPosition(), resultPos, 0.0001) &&
    mitk::Equal(output2->GetPosition(), resultPos2, 0.0001), "Testing translation calculation after second update()");

  /* change an input, see if output changes too */
  mitk::NavigationData::PositionType pos3, resultPos3;
  mitk::FillVector3D(pos3, 123.456, -234.567, 789.987);
  mitk::FillVector3D(resultPos3, -234.567, -123.456, 789.987);
  nd1->SetPosition(pos3);  // nd1 is modified, but nothing should change until pipe gets updated
  MITK_TEST_CONDITION(mitk::Equal(output->GetPosition(),resultPos, 0.00001)
    && mitk::Equal(output2->GetPosition(), resultPos2, 0.0001), "Testing transform calculation after input change, before update()");

  output->Update(); // update pipeline. should recalculate positions, because input has changed
  MITK_TEST_CONDITION(mitk::Equal(output->GetPosition(), resultPos3, 0.00001) &&
    mitk::Equal(output2->GetPosition(), resultPos2, 0.0001), "Testing transform calculation after input change, after update()");

  // always end with this!
  MITK_TEST_END();
}

