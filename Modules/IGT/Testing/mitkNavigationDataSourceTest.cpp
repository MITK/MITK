/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkNavigationDataSource.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"

  /**Documentation
  * \brief test class that only adds a public New() method to NavigationDataSource, so that it can be tested
  *
  */
class MyNavigationDataSourceTest : public mitk::NavigationDataSource
  {
  public:
    mitkClassMacro(MyNavigationDataSourceTest, mitk::NavigationDataSource);
    itkNewMacro(Self);
    void CreateOutput()
    {
      this->SetNumberOfOutputs(1);
      this->SetNthOutput(0, this->MakeOutput(0));
    };
  };

/** Class that holds static test methods to structure the test. */
class mitkNavigationDataSourceTestClass
  {
  public:

  static void TestInstantiation()
    {
    // let's create an object of our class
    MyNavigationDataSourceTest::Pointer myFilter = MyNavigationDataSourceTest::New();

    // first test: did this work?
    // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
    // it makes no sense to continue without an object.
    MITK_TEST_CONDITION_REQUIRED(myFilter.IsNotNull(), "Testing instantiation");

    // testing create outputs
    MITK_TEST_CONDITION(myFilter->GetNumberOfInputs() == 0, "testing initial number of inputs");
    MITK_TEST_CONDITION(myFilter->GetNumberOfOutputs() == 0, "testing initial number of outputs");
    myFilter->CreateOutput();
    MITK_TEST_CONDITION(myFilter->GetNumberOfOutputs() == 1, "testing SetNumberOfOutputs() and MakeOutput()");
    MITK_TEST_CONDITION(dynamic_cast<mitk::NavigationData*>(myFilter->GetOutput()) != NULL, "test GetOutput() returning valid output object");
    }

  static void TestMethodsNormalCases()
    {
    //create and initialize test objects
    MyNavigationDataSourceTest::Pointer myFilter = MyNavigationDataSourceTest::New();
    myFilter->CreateOutput();
    mitk::NavigationData::PositionType initialPos;
    mitk::FillVector3D(initialPos, 1.0, 2.0, 3.0);
    mitk::NavigationData::OrientationType initialOri(0.1, 0.2, 0.3, 0.4);
    mitk::ScalarType initialError(22.22);
    bool initialValid(true);
    mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
    nd1->SetPosition(initialPos);
    nd1->SetOrientation(initialOri);
    nd1->SetPositionAccuracy(initialError);
    nd1->SetDataValid(initialValid);

    //test method graft
    MITK_TEST_OUTPUT(<< "testing Graftoutput()");
    myFilter->GraftOutput(nd1);
    mitk::NavigationData::Pointer out = myFilter->GetOutput();
    MITK_TEST_CONDITION(out.GetPointer() != nd1.GetPointer(), "testing if output is same object as source of graft");
    MITK_TEST_CONDITION( mitk::Equal(out->GetPosition(), nd1->GetPosition()) &&
                         mitk::Equal(out->GetOrientation(), nd1->GetOrientation()) &&
                         (out->GetCovErrorMatrix() == nd1->GetCovErrorMatrix()) &&
                         (out->IsDataValid() == nd1->IsDataValid()) &&
                         mitk::Equal(out->GetTimeStamp(), nd1->GetTimeStamp()), "testing if content of output is equal to input of Graft");

    //test method GetParameters()
    mitk::PropertyList::ConstPointer list = myFilter->GetParameters();
    MITK_TEST_CONDITION(list.IsNotNull(), "testing GetParameters()");
    }

  static void TestMethodsInvalidCases()
    {
    //test invalid call of methods
    MyNavigationDataSourceTest::Pointer myFilter = MyNavigationDataSourceTest::New();

    mitk::NavigationData::Pointer testOutput = myFilter->GetOutput(0);
    MITK_TEST_CONDITION(testOutput.IsNull(), "testing GetOutput(int) before initialization");

    testOutput = myFilter->GetOutput("test");
    MITK_TEST_CONDITION(testOutput.IsNull(), "testing GetOutput(string) before initialization");

    //test GetOutputIndex() with invalid output name
    myFilter->CreateOutput();
    bool exceptionThrown=false;
    try
      {
      myFilter->GetOutputIndex("nonsense name");
      }
    catch(std::invalid_argument e)
      {
      exceptionThrown=true;
      }
    MITK_TEST_CONDITION(exceptionThrown,"Testing method GetOutputIndex with invalid navigation data name");

    //test method GraftNthOutput with invalid index
    exceptionThrown=false;
    try
      {
      mitk::NavigationData::Pointer graftObject;
      myFilter->GraftNthOutput(100,graftObject);
      }
    catch(itk::ExceptionObject e)
      {
      exceptionThrown=true;
      }
    MITK_TEST_CONDITION(exceptionThrown,"Testing method GraftNthOutput with invalid index");
    }

  static void TestMicroserviceRegister()
    {
    MyNavigationDataSourceTest::Pointer myFilter = MyNavigationDataSourceTest::New();
    myFilter->CreateOutput();
    mitk::NavigationData::PositionType initialPos;
    mitk::FillVector3D(initialPos, 1.0, 2.0, 3.0);
    mitk::NavigationData::OrientationType initialOri(0.1, 0.2, 0.3, 0.4);
    mitk::ScalarType initialError(22.22);
    bool initialValid(true);
    mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
    nd1->SetPosition(initialPos);
    nd1->SetOrientation(initialOri);
    nd1->SetPositionAccuracy(initialError);
    nd1->SetDataValid(initialValid);
    myFilter->RegisterAsMicroservice();
    MITK_TEST_CONDITION(myFilter->GetMicroserviceID()!="","Testing if microservice was registered successfully.");
    }
  static void TestMicroserviceAvailabilityAndUnregister()
    {
    //TODO: test if Microservice is available

    //TODO: remove Microservice

    //TODO: test if Microservice is not available any more

    }
  };

/**Documentation
 *  test for the class "NavigationDataSource".
 */
int mitkNavigationDataSourceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataSource");

  mitkNavigationDataSourceTestClass::TestInstantiation();
  mitkNavigationDataSourceTestClass::TestMethodsNormalCases();
  mitkNavigationDataSourceTestClass::TestMethodsInvalidCases();
  mitkNavigationDataSourceTestClass::TestMicroserviceRegister();
  mitkNavigationDataSourceTestClass::TestMicroserviceAvailabilityAndUnregister();

  // always end with this!
  MITK_TEST_END();
}
