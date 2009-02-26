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

#include <vtkTransform.h>
#include <iostream>


class mitkNavigationDataTransformFilterTestClass{ public:

/* false if difference abs(x1-y1) < delta, else true */
static bool compareTwoVectors(mitk::NavigationData::PositionType x, mitk::NavigationData::PositionType y)
{ 
  float delta = 1.0e-3;
  if((abs(x[0]-y[0]) < delta ) && (abs(x[1]-y[1]) < delta ) && (abs(x[2]-y[2]) < delta ))
    return true;
  else
    return false;
}

};


/**Documentation
 *  test for the class "NavigationDataTransformFilter".
 */
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

  mitk::NavigationData::OrientationType initialOri(0.3, 0.3, 0.3, 0.3);
  mitk::NavigationData::ErrorType initialError(0.0);
  bool initialValid(true);
  
  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->SetPosition(initialPos);
  nd1->SetOrientation(initialOri);
  nd1->SetError(initialError);
  nd1->SetDataValid(initialValid);

  myFilter->SetInput(nd1);
  MITK_TEST_CONDITION(myFilter->GetInput() == nd1, "Testing Set-/GetInput()");

  /* test identity transformation */
  vtkTransform* transform = vtkTransform::New();

  myFilter->SetTransform(transform);
  MITK_TEST_CONDITION(myFilter->GetTransform() == transform, "Testing Set-/GetTransform()");

  mitk::NavigationData* output = myFilter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(output != NULL, "Testing GetOutput()");

  output->Update(); // execute filter

  MITK_TEST_CONDITION(output->GetPosition() == initialPos, "Testing if Position remains unchanged");
  MITK_TEST_CONDITION(output->GetOrientation() == initialOri, "Testing if Orientation remains unchanged");
  MITK_TEST_CONDITION(output->GetError() == initialError, "Testing if Error remains unchanged");
  MITK_TEST_CONDITION(output->IsDataValid() == initialValid, "Testing if DataValid remains unchanged");


  /* test translation */
  mitk::NavigationDataTransformFilter::Pointer myFilter2 = mitk::NavigationDataTransformFilter::New();
  myFilter2->SetInput(nd1);
  vtkTransform* transform2 = vtkTransform::New();
  transform2->Translate(3.9, 2.8, 1.7);
  myFilter2->SetTransform(transform2);
  MITK_TEST_CONDITION(myFilter2->GetTransform() == transform2, "Testing Set-/GetTransform()");

  mitk::NavigationData* output2 = myFilter2->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(output2 != NULL, "Testing GetOutput()");

  output2->Update(); // execute filter

  MITK_TEST_CONDITION(output2->GetPosition() == resultPos, "Testing if translation was calculated correct");
  MITK_TEST_CONDITION(output2->GetOrientation() == initialOri, "Testing if Orientation remains unchanged");
  MITK_TEST_CONDITION(output2->GetError() == initialError, "Testing if Error remains unchanged");
  MITK_TEST_CONDITION(output2->IsDataValid() == initialValid, "Testing if DataValid remains unchanged");

  /* test scale */
  mitk::NavigationDataTransformFilter::Pointer myFilter3 = mitk::NavigationDataTransformFilter::New();
  myFilter3->SetInput(nd1);
  vtkTransform* transform3 = vtkTransform::New();
  transform3->Scale((5.0/1.1), (5.0/2.2), (5.0/3.3));
  myFilter3->SetTransform(transform3);
  MITK_TEST_CONDITION(myFilter3->GetTransform() == transform3, "Testing Set-/GetTransform()");

  mitk::NavigationData* output3 = myFilter3->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(output3 != NULL, "Testing GetOutput()");

  output3->Update(); // execute filter

  MITK_TEST_CONDITION(output3->GetPosition() == resultPos, "Testing if scale was calculated correct");
  MITK_TEST_CONDITION(output3->GetOrientation() == initialOri, "Testing if Orientation remains unchanged");
  MITK_TEST_CONDITION(output3->GetError() == initialError, "Testing if Error remains unchanged");
  MITK_TEST_CONDITION(output3->IsDataValid() == initialValid, "Testing if DataValid remains unchanged");


//-----------------------------------------------------------------------------------------------------------
  /* now test the filter with multiple inputs */
  mitk::NavigationData::PositionType initialPos2, resultPos2;
  mitk::FillVector3D(initialPos2, -1000.0, 567.89, 0.0);
  mitk::FillVector3D(resultPos2, -996.1, 570.69, 1.7);
  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
  nd2->SetPosition(initialPos2);
  nd2->SetOrientation(initialOri);
  nd2->SetError(initialError);
  nd2->SetDataValid(initialValid);

  myFilter = NULL;
  myFilter = mitk::NavigationDataTransformFilter::New();
  myFilter->SetTransform(transform2);
  myFilter->SetInput(0, nd1);
  myFilter->SetInput(1, nd2);

  MITK_TEST_CONDITION(((myFilter->GetInput(0) == nd1) 
                    && (myFilter->GetInput(1) == nd2)), "Testing Set-/GetInput(index, data)");
  output=NULL;
  output2=NULL;
  output = myFilter->GetOutput(0);
  output2 = myFilter->GetOutput(1);
  output2->Update(); // execute filter pipeline. this should update both outputs!
  MITK_TEST_CONDITION(((output != NULL) 
                    && (output2 != NULL)), "Testing GetOutput(index)");

  MITK_TEST_CONDITION(output->GetPosition() == resultPos, "Testing if translation was calculated correct [output 0]");
  MITK_TEST_CONDITION(output2->GetPosition() == resultPos2, "Testing if translation was calculated correct [output 1]");
  
  MITK_TEST_CONDITION(output2->GetOrientation() == initialOri, "Testing if Orientation remains unchanged for output 1");
  MITK_TEST_CONDITION(output2->GetError() == initialError, "Testing if Error remains unchanged for output 1");
  MITK_TEST_CONDITION(output2->IsDataValid() == initialValid, "Testing if DataValid remains unchanged for output 1");

  /* test if anything changes on second ->Update() */
  output->Update(); // nothing should change, since inputs remain unchanged
  MITK_TEST_CONDITION((output->GetPosition() == (resultPos))
                    &&(output2->GetPosition() == (resultPos2)), "Testing translation calculation after second update()");

  /* change an input, see if output changes too */
  mitk::NavigationData::PositionType pos3, resultPos3;
  mitk::FillVector3D(pos3, 123.456, -234.567, 789.987);
  mitk::FillVector3D(resultPos3, 127.356, -231.767, 791.687);
  nd1->SetPosition(pos3);  // nd1 is modified, but nothing should change until pipe gets updated
  MITK_TEST_CONDITION((output->GetPosition() == (resultPos))
    &&(output2->GetPosition() == (resultPos2)), "Testing translation calculation after input change, before update()");
  output->Update(); // update pipeline. should recalculate positions, because input has changed
  MITK_TEST_CONDITION((output->GetPosition() == (resultPos3))
    &&(output2->GetPosition() == (resultPos2)), "Testing translation calculation after input change, after update()");

  /* now, change filter parameter and check if pipeline updates correctly */
  mitk::NavigationData::PositionType resultPos4, resultPos5;
  mitk::FillVector3D(resultPos4, 600, 283.945, 0.0);
  mitk::FillVector3D(resultPos5, -74.0736, -117.284, -315.995);
  vtkTransform* transform4 = vtkTransform::New();
  transform4->Scale(-0.6, 0.5, -0.4);
  myFilter->SetTransform(transform4);
  MITK_TEST_CONDITION(
    (output->GetPosition() == (resultPos3)) &&
    (output2->GetPosition() == (resultPos2)),
    "Testing transformation after transform parameter change,before update()");

  output->Update(); // update pipeline. should recalculate positions, because input has changed
  
  MITK_TEST_CONDITION(
    mitkNavigationDataTransformFilterTestClass::compareTwoVectors(output2->GetPosition(), resultPos4) && 
    mitkNavigationDataTransformFilterTestClass::compareTwoVectors(output->GetPosition(), resultPos5 ),
    "Testing transformation after transform parameter change,after update()");

//  mitk::NavigationData::PositionType x= output->GetPosition();
//  std::cout<<"\n\n[output->GetPosition()] x0="<< x[0] << "\tx2="<< x[1] << "\tx3=" << x[2] <<"\n";
//  std::cout<<"[resultPos5] \t\tx0="<< resultPos5[0] << "\tx2="<< resultPos5[1] << "\tx3=" << resultPos5[2] <<"\n\n";

  // always end with this!
  MITK_TEST_END();

}


