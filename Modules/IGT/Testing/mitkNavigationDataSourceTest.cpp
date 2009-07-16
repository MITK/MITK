/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 14:52:01 +0200 (Mi, 13. Mai 2009) $
Version:   $Revision: 17230 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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

/**Documentation
 *  test for the class "NavigationDataSource".
 */
int mitkNavigationDataSourceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataSource");

  // let's create an object of our class  
  MyNavigationDataSourceTest::Pointer myFilter = MyNavigationDataSourceTest::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myFilter.IsNotNull(), "Testing instantiation");

  MITK_TEST_CONDITION(myFilter->GetInputs().size() == 0, "testing initial number of inputs"); 
  MITK_TEST_CONDITION(myFilter->GetOutputs().size() == 0, "testing initial number of outputs");
  myFilter->CreateOutput();
  MITK_TEST_CONDITION(myFilter->GetOutputs().size() == 1, "testing SetNumberOfOutputs() and MakeOutput()");
  MITK_TEST_CONDITION(dynamic_cast<mitk::NavigationData*>(myFilter->GetOutput()) != NULL, "test GetOutput() returning valid output object");


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
  
  MITK_TEST_OUTPUT(<< "testing Graftoutput()");
  myFilter->GraftOutput(nd1);
  mitk::NavigationData::Pointer out = myFilter->GetOutput();
  MITK_TEST_CONDITION(out.GetPointer() != nd1.GetPointer(), "testing if output is same object as source of graft");
  MITK_TEST_CONDITION( mitk::Equal(out->GetPosition(), nd1->GetPosition()) &&
                       mitk::Equal(out->GetOrientation(), nd1->GetOrientation()) &&
                       (out->GetCovErrorMatrix() == nd1->GetCovErrorMatrix()) &&
                       (out->IsDataValid() == nd1->IsDataValid()) &&
                       mitk::Equal(out->GetTimeStamp(), nd1->GetTimeStamp()), "testing if content of output is equal to input of Graft");

  mitk::PropertyList::ConstPointer list = myFilter->GetParameters();
  MITK_TEST_CONDITION(list.IsNotNull(), "testing GetParameters()");
  // always end with this!
  MITK_TEST_END();
}
