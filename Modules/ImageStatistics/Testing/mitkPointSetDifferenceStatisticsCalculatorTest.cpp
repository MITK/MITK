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

#include "mitkStandardFileLocations.h"
#include "mitkTestingMacros.h"
#include "mitkPointSetDifferenceStatisticsCalculator.h"

//#include <QtCore>

/**
 * \brief Test class for mitkPointSetDifferenceStatisticsCalculator
 */
class mitkPointSetDifferenceStatisticsCalculatorTestClass
{

public:

  static void TestInstantiation()
    {
    // let's create an object of our class
    mitk::PointSetDifferenceStatisticsCalculator::Pointer myPointSetDifferenceStatisticsCalculator = mitk::PointSetDifferenceStatisticsCalculator::New();
    MITK_TEST_CONDITION_REQUIRED(myPointSetDifferenceStatisticsCalculator.IsNotNull(),"Testing instantiation with constructor 1.");

    mitk::PointSet::Pointer myTestPointSet1 = mitk::PointSet::New();
    mitk::PointSet::Pointer myTestPointSet2 = mitk::PointSet::New();
    mitk::PointSetDifferenceStatisticsCalculator::Pointer myPointSetDifferenceStatisticsCalculator2 = mitk::PointSetDifferenceStatisticsCalculator::New(myTestPointSet1,myTestPointSet2);
    MITK_TEST_CONDITION_REQUIRED(myPointSetDifferenceStatisticsCalculator2.IsNotNull(),"Testing instantiation with constructor 2.");
    }

static void TestSimpleCase()
    {
    
    MITK_TEST_OUTPUT(<< "Starting simple test case...");

    mitk::Point3D test;
    mitk::PointSet::Pointer testPointSet1 = mitk::PointSet::New();

    mitk::FillVector3D(test,0,0,0);
    testPointSet1->InsertPoint(0,test);

    mitk::FillVector3D(test,1,1,1);
    testPointSet1->InsertPoint(1,test);

    mitk::PointSet::Pointer testPointSet2 = mitk::PointSet::New();

    mitk::FillVector3D(test,0.5,0.5,0.5);
    testPointSet2->InsertPoint(0,test);

    mitk::FillVector3D(test,2,2,2);
    testPointSet2->InsertPoint(1,test);
    double squaredDistance1 = 0.75;
    double squaredDistance2 = 3;
    double mean = (sqrt(squaredDistance1)+sqrt(squaredDistance2))/2;
    double variance = ((sqrt(squaredDistance1)-mean)*(sqrt(squaredDistance1)-mean)+(sqrt(squaredDistance2)-mean)*(sqrt(squaredDistance2)-mean))/2;
    double sd = sqrt(variance);
    double ms = 3.75/2;
    double rms = sqrt(ms);
    double min = sqrt(squaredDistance1);
    double max = sqrt(squaredDistance2);
    double median = (min + max)/2;

    mitk::PointSetDifferenceStatisticsCalculator::Pointer myPointSetDifferenceStatisticsCalculator = mitk::PointSetDifferenceStatisticsCalculator::New(testPointSet1,testPointSet2);

    MITK_TEST_CONDITION_REQUIRED((myPointSetDifferenceStatisticsCalculator->GetNumberOfPoints()==testPointSet1->GetSize()),".. Testing GetNumberOfPoints");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myPointSetDifferenceStatisticsCalculator->GetMean(),mean),".. Testing GetMean");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myPointSetDifferenceStatisticsCalculator->GetSD(),sd),".. Testing GetSD");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myPointSetDifferenceStatisticsCalculator->GetVariance(),variance),".. Testing GetVariance");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myPointSetDifferenceStatisticsCalculator->GetRMS(),rms),".. Testing GetRMS");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myPointSetDifferenceStatisticsCalculator->GetMin(),min),".. Testing GetMin");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myPointSetDifferenceStatisticsCalculator->GetMax(),max),".. Testing GetMax");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(myPointSetDifferenceStatisticsCalculator->GetMedian(),median),".. Testing GetMedian");

    testPointSet2->InsertPoint(2,test);
    myPointSetDifferenceStatisticsCalculator->SetPointSets(testPointSet1,testPointSet2);
    MITK_TEST_OUTPUT(<<"Test for exception when using differently sized point sets");
    MITK_TEST_FOR_EXCEPTION(itk::ExceptionObject,myPointSetDifferenceStatisticsCalculator->GetMean());
    mitk::PointSetDifferenceStatisticsCalculator::Pointer myPointSetDifferenceStatisticsCalculator2 = mitk::PointSetDifferenceStatisticsCalculator::New();
    MITK_TEST_OUTPUT(<<"Test for exception when using point sets with size 0");
    MITK_TEST_FOR_EXCEPTION(itk::ExceptionObject,myPointSetDifferenceStatisticsCalculator2->GetMean());
}
};

int mitkPointSetDifferenceStatisticsCalculatorTest(int, char* [])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkPointSetDifferenceStatisticsCalculatorTest")

  // let's create an object of our class
  mitk::PointSetDifferenceStatisticsCalculator::Pointer myPointSetDifferenceStatisticsCalculator = mitk::PointSetDifferenceStatisticsCalculator::New();
  MITK_TEST_CONDITION_REQUIRED(myPointSetDifferenceStatisticsCalculator.IsNotNull(),"Testing instantiation with constructor 1.");

  mitk::PointSet::Pointer myTestPointSet1 = mitk::PointSet::New();
  mitk::PointSet::Pointer myTestPointSet2 = mitk::PointSet::New();
  mitk::PointSetDifferenceStatisticsCalculator::Pointer myPointSetDifferenceStatisticsCalculator2 = mitk::PointSetDifferenceStatisticsCalculator::New(myTestPointSet1,myTestPointSet2);
  MITK_TEST_CONDITION_REQUIRED(myPointSetDifferenceStatisticsCalculator2.IsNotNull(),"Testing instantiation with constructor 2.");

  mitkPointSetDifferenceStatisticsCalculatorTestClass::TestInstantiation();
  mitkPointSetDifferenceStatisticsCalculatorTestClass::TestSimpleCase();
  MITK_TEST_END()
}
