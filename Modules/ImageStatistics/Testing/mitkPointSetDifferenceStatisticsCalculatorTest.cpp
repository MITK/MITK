/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkPointSetDifferenceStatisticsCalculator.h"

/**
 * \brief Test class for mitkPointSetDifferenceStatisticsCalculator
 */

//Members used for testing purposes
mitk::PointSetDifferenceStatisticsCalculator::Pointer m_myPointSetDifferenceStatisticsCalculator;
mitk::PointSet::Pointer m_myTestPointSet1;
mitk::PointSet::Pointer m_myTestPointSet2;

//This method should be called before every new sub-test call in order to freshly intialize all relevant classes
void Setup()
{
  // let's create an object of our class
  m_myPointSetDifferenceStatisticsCalculator = mitk::PointSetDifferenceStatisticsCalculator::New();
  //and some empty test data
  m_myTestPointSet1 = mitk::PointSet::New();
  m_myTestPointSet2 = mitk::PointSet::New();
}

void PointSetDifferenceStatisticsCalculator_DefaultConstructor_ResultIsNotNull()
{
  Setup();
  // let's create an object of our class
  MITK_TEST_CONDITION_REQUIRED(m_myPointSetDifferenceStatisticsCalculator.IsNotNull(),"Testing instantiation with default constructor.");
}

void PointSetDifferenceStatisticsCalculator_NonDefaultConstructor_ResultIsNotNull()
{
  Setup();
  m_myPointSetDifferenceStatisticsCalculator = mitk::PointSetDifferenceStatisticsCalculator::New(m_myTestPointSet1,m_myTestPointSet2);
  MITK_TEST_CONDITION_REQUIRED(m_myPointSetDifferenceStatisticsCalculator.IsNotNull(),"Testing instantiation with non default constructor.");
}

void PointSetDifferenceStatisticsCalculator_TwoSimplePointSetsOfSizeTwo_ResultsInGroundTruthValues()
{

  MITK_TEST_OUTPUT(<< "Starting simple test case...");

  mitk::Point3D tmpPoint;

  //fill the point sets with simple test data
  mitk::FillVector3D(tmpPoint,0,0,0);
  m_myTestPointSet1->InsertPoint(0,tmpPoint);
  mitk::FillVector3D(tmpPoint,1,1,1);
  m_myTestPointSet1->InsertPoint(1,tmpPoint);

  mitk::FillVector3D(tmpPoint,0.5,0.5,0.5);
  m_myTestPointSet2->InsertPoint(0,tmpPoint);
  mitk::FillVector3D(tmpPoint,2,2,2);
  m_myTestPointSet2->InsertPoint(1,tmpPoint);

  //Ground truth values (No logic in tests! Do not change values! :))
  const double mean = 1.299038105676658E+00; // from (sqrt(0.75)+sqrt(3))/2;
  const double variance = 0.1875; // from ((sqrt(0.75)-mean)*(sqrt(0.75)-mean)+(sqrt(3)-mean)*(sqrt(3)-mean))/2;
  const double sd = 4.330127018922193E-01; //from sqrt(variance);
  const double rms = 1.369306393762915E+00; //from sqrt(3.75/2);
  const double min = 0.86602540378444; //from sqrt(0.75);
  const double max = 1.73205080756888; //from sqrt(3);
  const double median = 1.29903810567666; //from (min + max)/2;

  m_myPointSetDifferenceStatisticsCalculator->SetPointSets( m_myTestPointSet1, m_myTestPointSet2);

  MITK_TEST_CONDITION_REQUIRED((m_myPointSetDifferenceStatisticsCalculator->GetNumberOfPoints()==m_myTestPointSet1->GetSize()),".. Testing GetNumberOfPoints");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetMean(),mean, 1E-5),".. Testing GetMean");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetSD(),sd,1E-6),".. Testing GetSD");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetVariance(),variance,1E-4),".. Testing GetVariance");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetRMS(),rms,1E-5),".. Testing GetRMS");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetMin(),min,1E-6),".. Testing GetMin");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetMax(),max,1E-5),".. Testing GetMax");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetMedian(),median,1E-5),".. Testing GetMedian");
}

void PointSetDifferenceStatisticsCalculator_PointSetsOfSameSizeWithDifferentPointIDs_ResultsInGroundTruth()
{
  Setup();

  mitk::Point3D tmpPoint;

  //Fill the point sets with simple test data, but different point IDs
  mitk::FillVector3D(tmpPoint,1,1,1);
  m_myTestPointSet1->InsertPoint(2,tmpPoint);
  m_myTestPointSet2->InsertPoint(0,tmpPoint);
  mitk::FillVector3D(tmpPoint,3.5,4.9,2.1); //same point in both pointsets
  m_myTestPointSet1->InsertPoint(17,tmpPoint);
  m_myTestPointSet2->InsertPoint(522,tmpPoint);

  m_myPointSetDifferenceStatisticsCalculator->SetPointSets(m_myTestPointSet1, m_myTestPointSet2);

  //Compare results to ground truth which is 0.0, because the sets are identical
  MITK_TEST_CONDITION_REQUIRED((m_myPointSetDifferenceStatisticsCalculator->GetNumberOfPoints()==m_myTestPointSet1->GetSize()),".. Testing GetNumberOfPoints");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetMean(),0.0),".. Testing GetMean");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetSD(),0.0),".. Testing GetSD");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetVariance(),0.0),".. Testing GetVariance");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetRMS(),0.0),".. Testing GetRMS");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetMin(),0.0),".. Testing GetMin");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetMax(),0.0),".. Testing GetMax");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(m_myPointSetDifferenceStatisticsCalculator->GetMedian(),0.0),".. Testing GetMedian");
}

void PointSetDifferenceStatisticsCalculator_TwoPointSetsOfDifferentSize_ThrowsException()
{
  Setup();
  //One set with 2 points and one set with 1 point
  mitk::Point3D tmpPoint;
  tmpPoint.Fill(0);
  m_myTestPointSet1->InsertPoint(0, tmpPoint);
  m_myTestPointSet1->InsertPoint(1, tmpPoint);

  m_myTestPointSet2->InsertPoint(0, tmpPoint);

  m_myPointSetDifferenceStatisticsCalculator->SetPointSets(m_myTestPointSet1, m_myTestPointSet2);
  MITK_TEST_FOR_EXCEPTION(itk::ExceptionObject,m_myPointSetDifferenceStatisticsCalculator->GetMean());
}

void PointSetDifferenceStatisticsCalculator_PointSetWithSizeZero_ThrowsException()
{
  Setup();
  m_myPointSetDifferenceStatisticsCalculator->SetPointSets(m_myTestPointSet1, m_myTestPointSet2);
  MITK_TEST_FOR_EXCEPTION(itk::ExceptionObject,m_myPointSetDifferenceStatisticsCalculator->GetMean());
}

int mitkPointSetDifferenceStatisticsCalculatorTest(int, char* [])
{
  MITK_TEST_BEGIN("mitkPointSetDifferenceStatisticsCalculatorTest")
  PointSetDifferenceStatisticsCalculator_DefaultConstructor_ResultIsNotNull();
  PointSetDifferenceStatisticsCalculator_NonDefaultConstructor_ResultIsNotNull();
  PointSetDifferenceStatisticsCalculator_TwoSimplePointSetsOfSizeTwo_ResultsInGroundTruthValues();
  PointSetDifferenceStatisticsCalculator_PointSetWithSizeZero_ThrowsException();
  PointSetDifferenceStatisticsCalculator_TwoPointSetsOfDifferentSize_ThrowsException();
  PointSetDifferenceStatisticsCalculator_PointSetsOfSameSizeWithDifferentPointIDs_ResultsInGroundTruth();
  MITK_TEST_END()
}
