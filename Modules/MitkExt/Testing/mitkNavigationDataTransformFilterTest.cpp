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

#include "mitkNavigationDataTransformFilter.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"

#include <itkTransform.h>
#include <itkVector.h>
#include <iostream>


/**Documentation
*  test for the class "NavigationDataTransformFilter".
*/
typedef itk::Rigid3DTransform<mitk::ScalarType > TransformType;

typedef itk::Vector<mitk::ScalarType,3> VectorType;

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
  nd1->SetDataValid(initialValid);

  myFilter->SetInput(nd1);
  MITK_TEST_CONDITION(myFilter->GetInput() == nd1, "Testing Set-/GetInput()");

  mitk::NavigationData* output = myFilter->GetOutput(0);

  /*test case no transform set*/
  MITK_TEST_FOR_EXCEPTION(std::exception, output->Update(););

  /* test transformation */
  mitk::NavigationDataTransformFilter::TransformType::Pointer transform =  mitk::NavigationDataTransformFilter::TransformType::New();
  VectorType translationVector;
  translationVector[0] = 3.9;
  translationVector[1] = 2.8;
  translationVector[2] = 1.7;


  /* test translation */
  transform->Translate(translationVector);

  myFilter->SetRigid3DTransform(transform);

  output = myFilter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(output != NULL, "Testing GetOutput()");

  output->Update(); // execute filter

  MITK_TEST_CONDITION(output->GetPosition() == resultPos, "Testing if translation was calculated correct");
  MITK_TEST_CONDITION( mitk::Equal(output->GetOrientation(),initialOri),"Testing if Orientation remains unchanged ");
  MITK_TEST_CONDITION(output->IsDataValid() == initialValid, "Testing if DataValid remains unchanged");


  // test rotation
  mitk::NavigationDataTransformFilter::Pointer myFilter2 = mitk::NavigationDataTransformFilter::New();
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myFilter2.IsNotNull(),"Testing instantiation");

  mitk::FillVector3D(initialPos, 1.1, 1.1, 1.1);
  mitk::FillVector3D(resultPos,  1.1, -1.1, 1.1);

  mitk::NavigationData::OrientationType resultOri(0.0, 0.0, -0.7071, 0.7071);
  initialOri[0] = 0;
  initialOri[1] = 0;
  initialOri[2] = 0;
  initialOri[3] = 1;

  nd1->SetPosition(initialPos);
  nd1->SetOrientation(initialOri);

  myFilter2->SetInput(nd1);

  itk::Matrix<mitk::ScalarType,3,3> rotMatrix;
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
  transform2->SetRotationMatrix(rotMatrix);

  myFilter2->SetRigid3DTransform(transform2);

  mitk::NavigationData* output2 = myFilter2->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(output2 != NULL, "Testing GetOutput()");

  output2->Update(); // execute filter

  MITK_TEST_CONDITION(output2->GetPosition() == resultPos, "Testing if position after rotation is correctly calculated");
  MITK_TEST_CONDITION( mitk::Equal(output2->GetOrientation(), resultOri),"Testing if orientation after rotation is correctly caclculated  ");
  MITK_TEST_CONDITION(output2->IsDataValid() == initialValid, "Testing if DataValid remains unchanged");


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

  myFilter = NULL;
  myFilter = mitk::NavigationDataTransformFilter::New();
  myFilter->SetRigid3DTransform(transform2);
  myFilter->SetInput(0, nd1);
  myFilter->SetInput(1, nd2);

  MITK_TEST_CONDITION(((myFilter->GetInput(0) == nd1) 
    && (myFilter->GetInput(1) == nd2)), "Testing Set-/GetInput(index, data)");
  output = NULL;
  output2 = NULL;
  output = myFilter->GetOutput(0);
  output2 = myFilter->GetOutput(1);
  output2->Update(); // execute filter pipeline. this should update both outputs!
  MITK_TEST_CONDITION_REQUIRED(((output != NULL) && (output2 != NULL)), "Testing GetOutput(index)");

  MITK_TEST_CONDITION(output->GetPosition() == resultPos, "Testing if position rotation was calculated correct [output 0]");
  MITK_TEST_CONDITION(mitk::Equal(output->GetOrientation(), resultOri),"Testing if orientation rotation was calculated correct [output 0]");
  MITK_TEST_CONDITION(output->IsDataValid() == initialValid, "Testing if DataValid remains unchanged for output 0");

  MITK_TEST_CONDITION(output2->GetPosition() == resultPos2, "Testing if rotation was calculated correct [output 1]");
  MITK_TEST_CONDITION(mitk::Equal(output2->GetOrientation(), resultOri2),"Testing if orientation rotation was calculated correct [output 1]");  
  MITK_TEST_CONDITION(output2->IsDataValid() == initialValid, "Testing if DataValid remains unchanged for output 1");
  //
  //  /* test if anything changes on second ->Update() */
  output->Update(); // nothing should change, since inputs remain unchanged
  MITK_TEST_CONDITION((output->GetPosition() == (resultPos)) && (output2->GetPosition() == (resultPos2)), "Testing translation calculation after second update()");

  /* change an input, see if output changes too */
  mitk::NavigationData::PositionType pos3, resultPos3;
  mitk::FillVector3D(pos3, 123.456, -234.567, 789.987);
  mitk::FillVector3D(resultPos3, -234.567, -123.456, 789.987);
  nd1->SetPosition(pos3);  // nd1 is modified, but nothing should change until pipe gets updated
  MITK_TEST_CONDITION((output->GetPosition() == (resultPos))
    &&(output2->GetPosition() == (resultPos2)), "Testing transfomr calculation after input change, before update()");

  output->Update(); // update pipeline. should recalculate positions, because input has changed
  MITK_TEST_CONDITION((output->GetPosition() == (resultPos3)) && (output2->GetPosition() == (resultPos2)), "Testing transform calculation after input change, after update()");

  // always end with this!
  MITK_TEST_END();
}

