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
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPercentageOfInvalidSamples(0),33.33333333),".. Testing GetPercentageOfInvalidSamples");
    MITK_TEST_CONDITION_REQUIRED((myNavigationDataEvaluationFilter->GetPositionStandardDeviation(0)[0]==0.5),".. Testing GetPositionStandardDeviation");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionSampleStandardDeviation(0)[0],0.70710672),".. Testing GetPositionStandardDeviation");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorMean(0),0.8660254),".. Testing GetPositionErrorMean");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorRMS(0),0.8660254),".. Testing GetPositionErrorRMS");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorMax(0),0.8660254),".. Testing GetPositionErrorMax");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorMedian(0),0.8660254),".. Testing GetPositionErrorMedian");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorMin(0),0.8660254),".. Testing GetPositionErrorMin");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorSampleStandardDeviation(0),0),".. Testing GetPositionErrorSampleStandardDeviation");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorStandardDeviation(0),0),".. Testing GetPositionErrorStandardDeviation");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetQuaternionMean(0)[0],0),".. Testing GetQuaternionMean");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetQuaternionStandardDeviation(0)[0],0),".. Testing GetQuaternionStandardDeviation");
    myNavigationDataEvaluationFilter->ResetStatistic();
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetNumberOfAnalysedNavigationData(0),0),".. Testing ResetStatistic");
    }
static void TestComplexCase()
    {
    MITK_TEST_OUTPUT(<< "Starting complex test case...");
    mitk::NavigationData::Pointer testData = mitk::NavigationData::New();
    mitk::NavigationDataEvaluationFilter::Pointer myNavigationDataEvaluationFilter = mitk::NavigationDataEvaluationFilter::New();
    myNavigationDataEvaluationFilter->SetInput(testData);
    testData->SetDataValid(true);
    mitk::Point3D testPoint;

    //1st point
    mitk::FillVector3D(testPoint,0,1,0);
    testData->SetPosition(testPoint);
    myNavigationDataEvaluationFilter->Update();

    //2nd point
    mitk::FillVector3D(testPoint,0,1,0.34);
    testData->SetPosition(testPoint);
    myNavigationDataEvaluationFilter->Update();

    //3rd point
    mitk::FillVector3D(testPoint,1,0.5,1);
    testData->SetPosition(testPoint);
    myNavigationDataEvaluationFilter->Update();

    //4th point
    mitk::FillVector3D(testPoint,15,3,2);
    testData->SetPosition(testPoint);
    myNavigationDataEvaluationFilter->Update();

    //5th point
    mitk::FillVector3D(testPoint,2,22.5,1.2655);
    testData->SetPosition(testPoint);
    myNavigationDataEvaluationFilter->Update();

    //6th point
    mitk::FillVector3D(testPoint,4,1.3,2);
    testData->SetPosition(testPoint);
    myNavigationDataEvaluationFilter->Update();

    //7th point
    mitk::FillVector3D(testPoint,0.001,0,1);
    testData->SetPosition(testPoint);
    myNavigationDataEvaluationFilter->Update();

    //8th point
    mitk::FillVector3D(testPoint,1.2525,2.22,3);
    testData->SetPosition(testPoint);
    myNavigationDataEvaluationFilter->Update();

    //9th point
    mitk::FillVector3D(testPoint,3.1,3,1);
    testData->SetPosition(testPoint);
    myNavigationDataEvaluationFilter->Update();

    MITK_TEST_CONDITION_REQUIRED(myNavigationDataEvaluationFilter->GetNumberOfAnalysedNavigationData(0)==9,".. Testing GetNumberOfAnalysedNavigationData");
    MITK_TEST_CONDITION_REQUIRED(myNavigationDataEvaluationFilter->GetNumberOfInvalidSamples(0)==0,".. Testing GetNumberOfInvalidSamples");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPercentageOfInvalidSamples(0),0),".. Testing GetPercentageOfInvalidSamples");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionStandardDeviation(0)[2],0.86614074),".. Testing GetPositionStandardDeviation");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionSampleStandardDeviation(0)[2],0.91868098),".. Testing GetPositionStandardDeviation");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorMean(0),6.06656587),".. Testing GetPositionErrorMean");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorRMS(0),8.0793161),".. Testing GetPositionErrorRMS");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorMax(0),18.6875241),".. Testing GetPositionErrorMax");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorMedian(0),4.18522229),".. Testing GetPositionErrorMedian");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorMin(0),0.90082741),".. Testing GetPositionErrorMin");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorSampleStandardDeviation(0),5.65960626),".. Testing GetPositionErrorSampleStandardDeviation");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetPositionErrorStandardDeviation(0),5.33592795),".. Testing GetPositionErrorStandardDeviation");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetQuaternionMean(0)[0],0),".. Testing GetQuaternionMean");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetQuaternionStandardDeviation(0)[0],0),".. Testing GetQuaternionStandardDeviation");
    myNavigationDataEvaluationFilter->ResetStatistic();
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myNavigationDataEvaluationFilter->GetNumberOfAnalysedNavigationData(0),0),".. Testing ResetStatistic");

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
  NavigationDataEvaluationFilterTestClass::TestComplexCase();

  // always end with this!
  MITK_TEST_END()
}
