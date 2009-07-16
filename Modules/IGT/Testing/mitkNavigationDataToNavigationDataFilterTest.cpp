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
  itkNewMacro(Self);
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

  
  MITK_TEST_CONDITION(myFilter->GetOutput() == NULL, "testing GetOutput()");

  myFilter->SetInput(nd0);
  MITK_TEST_CONDITION(myFilter->GetInput() == nd0, "testing Set-/GetInput()");
  MITK_TEST_CONDITION(myFilter->GetInput(0) == nd0, "testing Set-/GetInput(0)");
  MITK_TEST_CONDITION(myFilter->GetOutput() != NULL, "testing GetOutput() after SetInput()");
  MITK_TEST_CONDITION(myFilter->GetOutput(0) != NULL, "testing GetOutput() after SetInput()");
  MITK_TEST_CONDITION(myFilter->GetOutput(0) != nd0, "testing GetOutput() different object than input");

  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->Graft(nd0);
  nd1->SetDataValid(false);
  myFilter->SetInput(1, nd1);
  MITK_TEST_CONDITION(myFilter->GetInput(1) == nd1, "testing Set-/GetInput(1)");
  MITK_TEST_CONDITION(myFilter->GetInput(0) == nd0, "testing Set-/GetInput(0) again");
  MITK_TEST_CONDITION(myFilter->GetOutput(1) != NULL, "testing GetOutput() after SetInput()");
  MITK_TEST_CONDITION(myFilter->GetOutput(0) != myFilter->GetOutput(1), "testing GetOutput(0) different object than GetOutput(1)");

  myFilter->SetInput(10, nd1);
  MITK_TEST_CONDITION(myFilter->GetNumberOfInputs() == 11, "testing SetInput(10) produces 11 outputs");
  MITK_TEST_CONDITION(myFilter->GetInput(10) == nd1, "testing Set-/GetInput(10)");

  myFilter->SetInput(10, NULL);
  MITK_TEST_CONDITION(myFilter->GetNumberOfInputs() == 10, "testing SetInput(10, NULL) removes output with index 10");

  myFilter->SetInput(1, NULL);
  MITK_TEST_CONDITION(myFilter->GetNumberOfInputs() == 10, "testing SetInput(1, NULL) does not change number of outputs");

  // always end with this!
  MITK_TEST_END();
}
