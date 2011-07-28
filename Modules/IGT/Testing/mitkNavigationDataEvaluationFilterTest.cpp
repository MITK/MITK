/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataEvaluationFilter.h"
#include "mitkTestingMacros.h"

/**Documentation
* NavigationDataEvaluationFilter has a protected constructor and a protected itkNewMacro
* so that only it's friend class ClaronTrackingDevice is able to instantiate
* tool objects. Therefore, we derive from NavigationDataEvaluationFilter and add a
* public itkNewMacro, so that we can instantiate and test the class
*/
class NavigationDataEvaluationFilterTestClass : public mitk::NavigationDataEvaluationFilter
{
public:

static void TestInstantiation()
    {
    // let's create an object of our class
    mitk::NavigationDataEvaluationFilter::Pointer myNavigationDataEvaluationFilter = mitk::NavigationDataEvaluationFilter::New();
    MITK_TEST_CONDITION_REQUIRED(myNavigationDataEvaluationFilter.IsNotNull(),"Testing instantiation")
    }

static void TestSimpleCase()
    {
    MITK_TEST_OUTPUT(<< "Starting simple test case...");
    mitk::NavigationData::Pointer testData = mitk::NavigationData::New();
    mitk::NavigationDataEvaluationFilter::Pointer myNavigationDataEvaluationFilter = mitk::NavigationDataEvaluationFilter::New();
    myNavigationDataEvaluationFilter->SetInput(testData);
    myNavigationDataEvaluationFilter->Update();
    testData->SetDataValid(true);
    myNavigationDataEvaluationFilter->Update();
    MITK_TEST_CONDITION_REQUIRED(myNavigationDataEvaluationFilter->GetNumberOfAnalysedNavigationData(0)==1,".. Testing GetNumberOfAnalysedNavigationData");
    mitk::Point3D test;
    test[0] = 1;
    test[1] = 1;
    test[2] = 1;
    testData->SetPosition(test);
    myNavigationDataEvaluationFilter->Update();
    MITK_TEST_CONDITION_REQUIRED(myNavigationDataEvaluationFilter->GetNumberOfAnalysedNavigationData(0)==2,".. Testing GetNumberOfAnalysedNavigationData (2nd test)");
    MITK_TEST_CONDITION_REQUIRED(myNavigationDataEvaluationFilter->GetNumberOfInvalidSamples(0)==1,".. Testing GetNumberOfInvalidSamples");
    MITK_TEST_CONDITION_REQUIRED(((myNavigationDataEvaluationFilter->GetPercentageOfInvalidSamples(0)<33.34) && (myNavigationDataEvaluationFilter->GetPercentageOfInvalidSamples(0)>33.33)),".. Testing GetPercentageOfInvalidSamples");
    MITK_TEST_CONDITION_REQUIRED((myNavigationDataEvaluationFilter->GetPositionStandardDeviation(0)[0]==0.5),".. Testing GetPositionStandardDeviation");
    MITK_TEST_CONDITION_REQUIRED(((myNavigationDataEvaluationFilter->GetPositionSampleStandardDeviation(0)[0]<0.708)&&(myNavigationDataEvaluationFilter->GetPositionSampleStandardDeviation(0)[0]>0.707)),".. Testing GetPositionStandardDeviation");
    }

};

/**
 * This function tests the NavigationDataEvaluationFilter class.
 */
int mitkNavigationDataEvaluationFilterTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("NavigationDataEvaluationFilter")
  
  NavigationDataEvaluationFilterTestClass::TestInstantiation();
  NavigationDataEvaluationFilterTestClass::TestSimpleCase();

  // always end with this!
  MITK_TEST_END()
}
