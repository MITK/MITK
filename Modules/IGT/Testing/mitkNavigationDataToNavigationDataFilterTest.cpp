/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNavigationDataToNavigationDataFilter.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"

/**Documentation
* \brief test class to be able to instantiate the normally abstract (private constructor) mitk::NavigationDataToNavigationDataFilter
*/
class NavigationDataToNavigationDataFilterTestClass : public mitk::NavigationDataToNavigationDataFilter
{
public:
  mitkClassMacro(NavigationDataToNavigationDataFilterTestClass, NavigationDataToNavigationDataFilter);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
};

/**Documentation
 *  test for the class "NavigationDataToNavigationDataFilter".
 */
int mitkNavigationDataToNavigationDataFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataToNavigationDataFilter")

  // let's create an object of our class
  mitk::NavigationDataToNavigationDataFilter::Pointer myFilter = NavigationDataToNavigationDataFilterTestClass::New().GetPointer(); // create testing subclass, but treat it like the real NavigationDataToNavigationDataFilter

  MITK_TEST_CONDITION_REQUIRED(myFilter.IsNotNull(),"Testing instantiation");

  /* create helper objects: navigation data with position as origin, zero quaternion, zero error and data valid */
  mitk::NavigationData::PositionType initialPos;
  mitk::FillVector3D(initialPos, 1.0, 2.0, 3.0);
  mitk::NavigationData::OrientationType initialOri(0.1, 0.2, 0.3, 0.4);
  mitk::ScalarType initialError(22.22);
  bool initialValid(true);
  mitk::NavigationData::Pointer nd0 = mitk::NavigationData::New();
  nd0->SetPosition(initialPos);
  nd0->SetOrientation(initialOri);
  nd0->SetPositionAccuracy(initialError);
  nd0->SetDataValid(initialValid);
  nd0->SetName("testName");

  MITK_TEST_CONDITION(myFilter->GetOutput() != nullptr, "testing GetOutput()");

  MITK_TEST_CONDITION(myFilter->GetInput() == nullptr, "testing GetInput() without SetInput()");
  MITK_TEST_CONDITION(myFilter->GetInput(0) == nullptr, "testing GetInput(0) without SetInput()");

  myFilter->SetInput(nd0);
  MITK_TEST_CONDITION(myFilter->GetInput() == nd0, "testing Set-/GetInput()");
  MITK_TEST_CONDITION(myFilter->GetInput(0) == nd0, "testing Set-/GetInput(0)");
  MITK_TEST_CONDITION(myFilter->GetOutput() != nullptr, "testing GetOutput() after SetInput()");
  MITK_TEST_CONDITION(myFilter->GetOutput() != nullptr, "testing GetOutput() after SetInput()");
  MITK_TEST_CONDITION(myFilter->GetOutput() != nd0, "testing GetOutput() different object than input");

  // check getInput() string input
  MITK_TEST_CONDITION(myFilter->GetInput("invalidName") == nullptr, "testing GetInput(string) invalid string");
  MITK_TEST_CONDITION(myFilter->GetInput("testName") == nd0, "testing GetInput(string) valid string");

  // check getInputIndex() string input
  bool throwsException = false;
  try {
    myFilter->GetInputIndex("invalidName");
  }
  catch(std::invalid_argument&) {
    throwsException = true;
  }
  MITK_TEST_CONDITION_REQUIRED(throwsException, "testing GetInputIndex(string) invalid string");

  MITK_TEST_CONDITION(myFilter->GetInputIndex("testName") == 0, "testing GetInputIndex(string) valid string");


  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->Graft(nd0);
  nd1->SetDataValid(false);
  myFilter->SetInput(1, nd1);
  MITK_TEST_CONDITION(myFilter->GetInput(1) == nd1, "testing Set-/GetInput(1)");
  MITK_TEST_CONDITION(myFilter->GetInput(0) == nd0, "testing Set-/GetInput(0) again");
  MITK_TEST_CONDITION(myFilter->GetOutput(1) != nullptr, "testing GetOutput() after SetInput()");
  MITK_TEST_CONDITION(myFilter->GetOutput() != myFilter->GetOutput(1), "testing GetOutput() different object than GetOutput(1)");

  myFilter->SetInput(10, nd1);
  MITK_TEST_CONDITION(myFilter->GetNumberOfIndexedInputs() == 11, "testing SetInput(10) produces 11 outputs");
  MITK_TEST_CONDITION(myFilter->GetInput(10) == nd1, "testing Set-/GetInput(10)");

  myFilter->SetInput(10, nullptr);
  MITK_TEST_CONDITION(myFilter->GetNumberOfIndexedInputs() == 10, "testing SetInput(10, nullptr) removes output with index 10");

  myFilter->SetInput(1, nullptr);
  MITK_TEST_CONDITION(myFilter->GetNumberOfIndexedInputs() == 10, "testing SetInput(1, nullptr) does not change number of outputs");

  // always end with this!
  MITK_TEST_END();
}
