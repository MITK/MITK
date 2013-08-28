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

#include "mitkStandardFileLocations.h"
#include "mitkTestingMacros.h"
#include "mitkPointSetStatisticsCalculator.h"

#include <iomanip>

//#include <QtCore>

/**
 * \brief Test class for mitkPointSetStatisticsCalculator
 */
class mitkPointSetStatisticsCalculatorTestClass
{
public:

  static void TestInstantiation()
  {
    // let's create an object of our class
    mitk::PointSetStatisticsCalculator::Pointer myPointSetStatisticsCalculator = mitk::PointSetStatisticsCalculator::New();
    MITK_TEST_CONDITION_REQUIRED(myPointSetStatisticsCalculator.IsNotNull(),"Testing instantiation with constructor 1.");

    mitk::PointSet::Pointer myTestPointSet = mitk::PointSet::New();
    mitk::PointSetStatisticsCalculator::Pointer myPointSetStatisticsCalculator2 = mitk::PointSetStatisticsCalculator::New(myTestPointSet);
    MITK_TEST_CONDITION_REQUIRED(myPointSetStatisticsCalculator2.IsNotNull(),"Testing instantiation with constructor 2.");
  }

  static void TestSimpleCase()
  {

    MITK_TEST_OUTPUT(<< "Starting simple test case...");

    mitk::Point3D test;
    mitk::PointSet::Pointer testPointSet = mitk::PointSet::New();

    mitk::FillVector3D(test,0,0,0);
    testPointSet->InsertPoint(0,test);

    mitk::FillVector3D(test,1,1,1);
    testPointSet->InsertPoint(1,test);

    mitk::PointSetStatisticsCalculator::Pointer myPointSetStatisticsCalculator = mitk::PointSetStatisticsCalculator::New(testPointSet);

    MITK_TEST_CONDITION((myPointSetStatisticsCalculator->GetPositionMean()[0]==0.5),".. Testing GetPositionMean");
    MITK_TEST_CONDITION((myPointSetStatisticsCalculator->GetPositionStandardDeviation()[0]==0.5),".. Testing GetPositionStandardDeviation");

    // expected result is 0.5 * sqrt(2), using the vnl constant for that
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionSampleStandardDeviation()[0], vnl_math::sqrt1_2),".. Testing GetPositionSampleStandardDeviation");

    // expected result is half the length of the cube diagonal
    // no pre-defined constants for that, we need to compute them ourselves
    const double sqrt_3_half = 0.5 * std::sqrt(3.0);
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorMean(), sqrt_3_half ),".. Testing GetPositionErrorMean");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorRMS(), sqrt_3_half),".. Testing GetPositionErrorRMS");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorMax(), sqrt_3_half),".. Testing GetPositionErrorMax");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorMedian(), sqrt_3_half),".. Testing GetPositionErrorMedian");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorMin(), sqrt_3_half),".. Testing GetPositionErrorMin");

    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorSampleStandardDeviation(),0),".. Testing GetPositionErrorSampleStandardDeviation");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorStandardDeviation(),0),".. Testing GetPositionErrorStandardDeviation");
  }

  static void TestComplexCase()
  {

    MITK_TEST_OUTPUT(<< "Starting complex test case...");
    mitk::Point3D testPoint;
    mitk::PointSet::Pointer testPointSet = mitk::PointSet::New();

    //1st point
    mitk::FillVector3D(testPoint,0,1,0);
    testPointSet->InsertPoint(0,testPoint);

    //2nd point
    mitk::FillVector3D(testPoint,0,1,0.34);
    testPointSet->InsertPoint(1,testPoint);

    //3rd point
    mitk::FillVector3D(testPoint,1,0.5,1);
    testPointSet->InsertPoint(2,testPoint);

    //4th point
    mitk::FillVector3D(testPoint,15,3,2);
    testPointSet->InsertPoint(3,testPoint);

    //5th point
    mitk::FillVector3D(testPoint,2,22.5,1.2655);
    testPointSet->InsertPoint(4,testPoint);

    //6th point
    mitk::FillVector3D(testPoint,4,1.4,2);
    testPointSet->InsertPoint(5,testPoint);

    //7th point
    mitk::FillVector3D(testPoint,0.001,0,1);
    testPointSet->InsertPoint(6,testPoint);

    //8th point
    mitk::FillVector3D(testPoint,1.25,2.25,3);
    testPointSet->InsertPoint(7,testPoint);

    //9th point
    mitk::FillVector3D(testPoint,3.2,3,1);
    testPointSet->InsertPoint(8,testPoint);

    mitk::PointSetStatisticsCalculator::Pointer myPointSetStatisticsCalculator = mitk::PointSetStatisticsCalculator::New();
    myPointSetStatisticsCalculator->SetPointSet(testPointSet);

    // reference values calculated manually in gnumeric
    const double double_ref[3] = {2.939, 3.85, 1.2895};
    const double stdev_ref[3] = {4.472918659369219E+00,6.667666591677914E+00,866.140737858333121E-03};
    const double samplestdev_ref[3] = {4.744246673603723E+00,7.072128392499672E+00,918.680983802320772E-03};
    const double position_error_mean_ref = 6.0665418290843;
    const double position_error_max_ref = 18.673639093652848686;
    const double position_error_median_ref = 4.20259101626604;
    const double position_error_min_ref = 0.93511028761318;
    const double position_error_rms_ref = 8.07557916739653;
    const double position_error_stdev_ref = 5.33029540690377;
    const double position_error_sstdev_ref = 5.65363204192375;

    // fill the corresponding types with the reference values
    mitk::Point3D mean_reference(&double_ref[0]);
    mitk::Vector3D sample_stdev_reference(&samplestdev_ref[0]);
    mitk::Vector3D stdev_reference(&stdev_ref[0]);

    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionMean(), mean_reference),".. Testing GetPositionMean");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionStandardDeviation(),stdev_reference),".. Testing GetPositionStandardDeviation");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionSampleStandardDeviation(), sample_stdev_reference),".. Testing GetPositionSampleStandardDeviation");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorMean(),position_error_mean_ref),".. Testing GetPositionErrorMean");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorRMS(), position_error_rms_ref),".. Testing GetPositionErrorRMS");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorMax(), position_error_max_ref),".. Testing GetPositionErrorMax");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorMedian(), position_error_median_ref),".. Testing GetPositionErrorMedian");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorMin(), position_error_min_ref),".. Testing GetPositionErrorMin");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorSampleStandardDeviation(),position_error_sstdev_ref),".. Testing GetPositionErrorSampleStandardDeviation");
    MITK_TEST_CONDITION(mitk::Equal(myPointSetStatisticsCalculator->GetPositionErrorStandardDeviation(),position_error_stdev_ref),".. Testing GetPositionErrorStandardDeviation");

  }


};

int mitkPointSetStatisticsCalculatorTest(int, char* [])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkPointSetStatisticsCalculatorTest")

      mitkPointSetStatisticsCalculatorTestClass::TestInstantiation();
  mitkPointSetStatisticsCalculatorTestClass::TestSimpleCase();
  mitkPointSetStatisticsCalculatorTestClass::TestComplexCase();

  MITK_TEST_END()
}
