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

#include "mitkArbitraryTimeGeometry.h"
#include "mitkGeometry3D.h"

#include "mitkTestingMacros.h"
#include <limits>

static const mitk::ScalarType test_eps = 1E-6; /* some reference values in the test seem
                                               to have been calculated with float precision. Thus, set this to float precision epsilon.*/
static const mitk::ScalarType test_eps_square = 1E-3;

class mitkArbitraryTimeGeometryTestClass
{
public:
  mitk::Geometry3D::Pointer m_Geometry1;
  mitk::Geometry3D::Pointer m_Geometry2;
  mitk::Geometry3D::Pointer m_Geometry3;
  mitk::Geometry3D::Pointer m_Geometry3_5;
  mitk::Geometry3D::Pointer m_Geometry4;
  mitk::Geometry3D::Pointer m_Geometry5;
  mitk::Geometry3D::Pointer m_InvalidGeometry;
  mitk::Geometry3D::Pointer m_NewGeometry;

  mitk::TimePointType m_Geometry1MinTP;
  mitk::TimePointType m_Geometry2MinTP;
  mitk::TimePointType m_Geometry3MinTP;
  mitk::TimePointType m_Geometry3_5MinTP;
  mitk::TimePointType m_Geometry4MinTP;
  mitk::TimePointType m_Geometry5MinTP;
  mitk::TimePointType m_NewGeometryMinTP;

  mitk::TimePointType m_Geometry1MaxTP;
  mitk::TimePointType m_Geometry2MaxTP;
  mitk::TimePointType m_Geometry3MaxTP;
  mitk::TimePointType m_Geometry3_5MaxTP;
  mitk::TimePointType m_Geometry4MaxTP;
  mitk::TimePointType m_Geometry5MaxTP;
  mitk::TimePointType m_NewGeometryMaxTP;

  mitk::ArbitraryTimeGeometry::Pointer m_emptyTimeGeometry;
  mitk::ArbitraryTimeGeometry::Pointer m_initTimeGeometry;
  mitk::ArbitraryTimeGeometry::Pointer m_12345TimeGeometry;
  mitk::ArbitraryTimeGeometry::Pointer m_123TimeGeometry;

  void SetupTestInventar()
  {
    mitk::TimeBounds bounds;
    m_Geometry1 = mitk::Geometry3D::New();
    m_Geometry2 = mitk::Geometry3D::New();
    m_Geometry3 = mitk::Geometry3D::New();
    m_Geometry3_5 = mitk::Geometry3D::New();
    m_Geometry4 = mitk::Geometry3D::New();
    m_Geometry5 = mitk::Geometry3D::New();

    m_Geometry1MinTP = 1;
    m_Geometry2MinTP = 2;
    m_Geometry3MinTP = 3;
    m_Geometry3_5MinTP = 3.5;
    m_Geometry4MinTP = 4;
    m_Geometry5MinTP = 5;

    m_Geometry1MaxTP = 1.9;
    m_Geometry2MaxTP = 2.9;
    m_Geometry3MaxTP = 3.9;
    m_Geometry3_5MaxTP = 3.9;
    m_Geometry4MaxTP = 4.9;
    m_Geometry5MaxTP = 5.9;

    m_NewGeometry = mitk::Geometry3D::New();
    m_NewGeometryMinTP = 20;
    m_NewGeometryMaxTP = 21.9;

    mitk::Point3D origin(42);
    m_NewGeometry->SetOrigin(origin);

    m_emptyTimeGeometry = mitk::ArbitraryTimeGeometry::New();
    m_emptyTimeGeometry->ClearAllGeometries();

    m_initTimeGeometry = mitk::ArbitraryTimeGeometry::New();
    m_initTimeGeometry->Initialize();

    m_12345TimeGeometry = mitk::ArbitraryTimeGeometry::New();
    m_12345TimeGeometry->ClearAllGeometries();
    m_12345TimeGeometry->AppendTimeStep(m_Geometry1,m_Geometry1MaxTP,m_Geometry1MinTP);
    m_12345TimeGeometry->AppendTimeStep(m_Geometry2,m_Geometry2MaxTP,m_Geometry2MinTP);
    m_12345TimeGeometry->AppendTimeStep(m_Geometry3,m_Geometry3MaxTP,m_Geometry3MinTP);
    m_12345TimeGeometry->AppendTimeStep(m_Geometry4,m_Geometry4MaxTP,m_Geometry4MinTP);
    m_12345TimeGeometry->AppendTimeStep(m_Geometry5,m_Geometry5MaxTP,m_Geometry5MinTP);

    m_123TimeGeometry = mitk::ArbitraryTimeGeometry::New();
    m_123TimeGeometry->ClearAllGeometries();
    m_123TimeGeometry->AppendTimeStep(m_Geometry1,m_Geometry1MaxTP,m_Geometry1MinTP);
    m_123TimeGeometry->AppendTimeStep(m_Geometry2,m_Geometry2MaxTP,m_Geometry2MinTP);
    m_123TimeGeometry->AppendTimeStep(m_Geometry3,m_Geometry3MaxTP,m_Geometry3MinTP);
  }

};


int mitkArbitraryTimeGeometryTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkArbitraryTimeGeometryTestClass);

  mitkArbitraryTimeGeometryTestClass testClass;
  testClass.SetupTestInventar();

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->CountTimeSteps() == 0, "Testing CountTimeSteps with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->CountTimeSteps() == 1, "Testing CountTimeSteps with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->CountTimeSteps() == 5, "Testing CountTimeSteps with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetMinimumTimePoint() == 0.0, "Testing GetMinimumTimePoint with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetMinimumTimePoint() == 0.0, "Testing GetMinimumTimePoint with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetMinimumTimePoint() == 1.0, "Testing GetMinimumTimePoint with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetMinimumTimePoint(2) == 0.0, "Testing GetMinimumTimePoint(2) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetMinimumTimePoint(2) == 0.0, "Testing GetMinimumTimePoint(2) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetMinimumTimePoint(2) == 2.9, "Testing GetMinimumTimePoint(2) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetMaximumTimePoint() == 0.0, "Testing GetMaximumTimePoint with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetMaximumTimePoint() == 1.0, "Testing GetMaximumTimePoint with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetMaximumTimePoint() == 5.9, "Testing GetMaximumTimePoint with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetMaximumTimePoint(2) == 0.0, "Testing GetMaximumTimePoint(2) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetMaximumTimePoint(2) == 0.0, "Testing GetMaximumTimePoint(2) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetMaximumTimePoint(2) == 3.9, "Testing GetMaximumTimePoint(2) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetTimeBounds()[0] == 0.0, "Testing GetTimeBounds lower part with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetTimeBounds()[0] == 0.0, "Testing GetTimeBounds lower part with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetTimeBounds()[0] == 1.0, "Testing GetTimeBounds lower part with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetTimeBounds()[1] == 0.0, "Testing GetTimeBounds with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetTimeBounds()[1] == 1.0, "Testing GetTimeBounds with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetTimeBounds()[1] == 5.9, "Testing GetTimeBounds with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetTimeBounds(3)[0] == 0.0, "Testing GetTimeBounds(3) lower part with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetTimeBounds(3)[0] == 0.0, "Testing GetTimeBounds(3) lower part with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetTimeBounds(3)[0] == 3.9, "Testing GetTimeBounds(3) lower part with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetTimeBounds(3)[1] == 0.0, "Testing GetTimeBounds(3) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetTimeBounds(3)[1] == 0.0, "Testing GetTimeBounds(3) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetTimeBounds(3)[1] == 4.9, "Testing GetTimeBounds(3) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->IsValidTimePoint(-1) == false, "Testing IsValidTimePoint(-1) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->IsValidTimePoint(-1) == false, "Testing IsValidTimePoint(-1) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->IsValidTimePoint(-1) == false, "Testing IsValidTimePoint(-1) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->IsValidTimePoint(0) == false, "Testing IsValidTimePoint(0) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->IsValidTimePoint(0) == true, "Testing IsValidTimePoint(0) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->IsValidTimePoint(0) == false, "Testing IsValidTimePoint(0) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->IsValidTimePoint(1) == false, "Testing IsValidTimePoint(1) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->IsValidTimePoint(1) == false, "Testing IsValidTimePoint(1) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->IsValidTimePoint(1) == true, "Testing IsValidTimePoint(1) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->IsValidTimePoint(2.5) == false, "Testing IsValidTimePoint(2.5) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->IsValidTimePoint(2.5) == false, "Testing IsValidTimePoint(2.5) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->IsValidTimePoint(2.5) == true, "Testing IsValidTimePoint(2.5) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->IsValidTimePoint(5.89) == false, "Testing IsValidTimePoint(5.89) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->IsValidTimePoint(5.89) == false, "Testing IsValidTimePoint(5.89) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->IsValidTimePoint(5.89) == true, "Testing IsValidTimePoint(5.89) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->IsValidTimePoint(10) == false, "Testing IsValidTimePoint(10) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->IsValidTimePoint(10) == false, "Testing IsValidTimePoint(10) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->IsValidTimePoint(10) == false, "Testing IsValidTimePoint(10) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->IsValidTimeStep(0) == false, "Testing IsValidTimeStep(0) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->IsValidTimeStep(0) == true, "Testing IsValidTimeStep(0) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->IsValidTimeStep(0) == true, "Testing IsValidTimeStep(0) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->IsValidTimeStep(1) == false, "Testing IsValidTimeStep(1) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->IsValidTimeStep(1) == false, "Testing IsValidTimeStep(1) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->IsValidTimeStep(1) == true, "Testing IsValidTimeStep(1) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->IsValidTimeStep(6) == false, "Testing IsValidTimeStep(6) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->IsValidTimeStep(6) == false, "Testing IsValidTimeStep(6) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->IsValidTimeStep(6) == false, "Testing IsValidTimeStep(6) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->TimeStepToTimePoint(0) == 0.0, "Testing TimeStepToTimePoint(0) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->TimeStepToTimePoint(0) == 0.0, "Testing TimeStepToTimePoint(0) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->TimeStepToTimePoint(0) == 1.0, "Testing TimeStepToTimePoint(0) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->TimeStepToTimePoint(1) == 0.0, "Testing TimeStepToTimePoint(1) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->TimeStepToTimePoint(1) == 0.0, "Testing TimeStepToTimePoint(1) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->TimeStepToTimePoint(1) == 1.9, "Testing TimeStepToTimePoint(1) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->TimeStepToTimePoint(6) == 0.0, "Testing TimeStepToTimePoint(6) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->TimeStepToTimePoint(6) == 0.0, "Testing TimeStepToTimePoint(6) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->TimeStepToTimePoint(6) == 0.0, "Testing TimeStepToTimePoint(6) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->TimePointToTimeStep(0.0) == 0, "Testing TimePointToTimeStep(0.0) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->TimePointToTimeStep(0.0) == 0, "Testing TimePointToTimeStep(0.0) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->TimePointToTimeStep(0.0) == 0, "Testing TimePointToTimeStep(0.0) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->TimePointToTimeStep(0.5) == 0, "Testing TimePointToTimeStep(0.5) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->TimePointToTimeStep(0.5) == 0, "Testing TimePointToTimeStep(0.5) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->TimePointToTimeStep(0.5) == 0, "Testing TimePointToTimeStep(0.5) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->TimePointToTimeStep(3.5) == 0, "Testing TimePointToTimeStep(3.5) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->TimePointToTimeStep(3.5) == 0, "Testing TimePointToTimeStep(3.5) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->TimePointToTimeStep(3.5) == 2, "Testing TimePointToTimeStep(3.5) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->TimePointToTimeStep(5.8) == 0, "Testing TimePointToTimeStep(5.8) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->TimePointToTimeStep(5.8) == 0, "Testing TimePointToTimeStep(5.8) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->TimePointToTimeStep(5.8) == 4, "Testing TimePointToTimeStep(5.8) with m_12345TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->TimePointToTimeStep(5.9) == 0, "Testing TimePointToTimeStep(5.9) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetGeometryCloneForTimeStep(0).IsNull(), "Testing GetGeometryCloneForTimeStep(0) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetGeometryCloneForTimeStep(0).IsNotNull(), "Testing GetGeometryCloneForTimeStep(0) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryCloneForTimeStep(0).IsNotNull(), "Testing GetGeometryCloneForTimeStep(0) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetGeometryForTimeStep(0).IsNull(), "Testing GetGeometryForTimePoint(0) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetGeometryForTimeStep(0).IsNotNull(), "Testing GetGeometryForTimePoint(0) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetGeometryForTimeStep(1).IsNull(), "Testing GetGeometryForTimePoint(1) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryForTimeStep(0).GetPointer() == testClass.m_Geometry1.GetPointer(), "Testing GetGeometryForTimePoint(0) with m_12345TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryForTimeStep(3).GetPointer() == testClass.m_Geometry4.GetPointer(), "Testing GetGeometryForTimePoint(3) with m_12345TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryForTimeStep(4).GetPointer() == testClass.m_Geometry5.GetPointer(), "Testing GetGeometryForTimePoint(4) with m_12345TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryForTimeStep(5).IsNull(), "Testing GetGeometryForTimePoint(5) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetGeometryForTimePoint(0).IsNull(), "Testing GetGeometryForTimeStep(0) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetGeometryForTimePoint(0).IsNotNull(), "Testing GetGeometryForTimeStep(0) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryForTimePoint(0).IsNull(), "Testing GetGeometryForTimeStep(0) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetGeometryForTimePoint(1.5).IsNull(), "Testing GetGeometryForTimeStep(1.5) with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->GetGeometryForTimePoint(1.5).IsNull(), "Testing GetGeometryForTimeStep(1.5) with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryForTimePoint(1.5).GetPointer() == testClass.m_Geometry1.GetPointer(), "Testing GetGeometryForTimeStep(1.5) with m_12345TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryForTimePoint(3.5).GetPointer() == testClass.m_Geometry3.GetPointer(), "Testing GetGeometryForTimeStep(3.5) with m_12345TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryForTimePoint(5.9).IsNull(), "Testing GetGeometryForTimeStep(5.9) with m_12345TimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->IsValid() == false, "Testing IsValid() with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_initTimeGeometry->IsValid() == true, "Testing IsValid() with m_initTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->IsValid() == true, "Testing IsValid() with m_12345TimeGeometry");

  //initialize is implicetly tested via m_initTimeGeometry

  testClass.m_12345TimeGeometry->Expand(3);
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->CountTimeSteps() == 5, "Testing Expand(3) doesn't change m_12345TimeGeometry");

  testClass.m_12345TimeGeometry->Expand(7);
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->CountTimeSteps() == 7, "Testing Expand(7) with m_12345TimeGeometry");

  //Test replace time step geometries
  testClass.m_12345TimeGeometry->ReplaceTimeStepGeometries(testClass.m_NewGeometry);
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->CountTimeSteps() == 7, "Testing ReplaceTimeStepGeometries() with m_12345TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryForTimeStep(0)->GetOrigin() == testClass.m_NewGeometry->GetOrigin(), "Testing ReplaceTimeStepGeometries(): check if first geometry of m_12345TimeGeometry was replaced m_12345TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetGeometryForTimeStep(6)->GetOrigin() == testClass.m_NewGeometry->GetOrigin(), "Testing ReplaceTimeStepGeometries(): check if last geometry of m_12345TimeGeometry was replaced m_12345TimeGeometry");

  //Test clear all geometries
  testClass.m_12345TimeGeometry->ClearAllGeometries();
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->CountTimeSteps() == 0, "Testing ClearAllGeometries() with m_12345TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetMinimumTimePoint() == 0, "Testing ClearAllGeometries() with m_12345TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_12345TimeGeometry->GetMaximumTimePoint() == 0, "Testing ClearAllGeometries() with m_12345TimeGeometry");

  //Test append
  MITK_TEST_FOR_EXCEPTION(mitk::Exception, testClass.m_123TimeGeometry->AppendTimeStep(NULL,0,1));
  MITK_TEST_FOR_EXCEPTION(mitk::Exception, testClass.m_123TimeGeometry->AppendTimeStep(testClass.m_Geometry2,testClass.m_Geometry2MaxTP));

  testClass.m_emptyTimeGeometry->AppendTimeStep(testClass.m_Geometry4,testClass.m_Geometry4MaxTP,testClass.m_Geometry4MinTP);
  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->CountTimeSteps() == 1, "Testing AppendTimeStep() with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetMinimumTimePoint() == 4, "Testing ClearAllGeometries() with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_emptyTimeGeometry->GetMaximumTimePoint() == 4.9, "Testing ClearAllGeometries() with m_emptyTimeGeometry");

  MITK_TEST_CONDITION_REQUIRED(testClass.m_123TimeGeometry->CountTimeSteps() == 3, "Testing AppendTimeStep() with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_123TimeGeometry->GetMinimumTimePoint() == 1, "Testing ClearAllGeometries() with m_emptyTimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_123TimeGeometry->GetMaximumTimePoint() == 3.9, "Testing ClearAllGeometries() with m_emptyTimeGeometry");
  testClass.m_123TimeGeometry->AppendTimeStep(testClass.m_Geometry4,testClass.m_Geometry4MaxTP,testClass.m_Geometry4MinTP);
  MITK_TEST_CONDITION_REQUIRED(testClass.m_123TimeGeometry->CountTimeSteps() == 4, "Testing AppendTimeStep() with m_123TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_123TimeGeometry->GetMinimumTimePoint() == 1, "Testing AppendTimeStep() with m_123TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_123TimeGeometry->GetMaximumTimePoint() == 4.9, "Testing AppendTimeStep() with m_123TimeGeometry");
  MITK_TEST_CONDITION_REQUIRED(testClass.m_123TimeGeometry->GetMinimumTimePoint(3) == 3.9, "Testing AppendTimeStep() with m_123TimeGeometry");

  MITK_TEST_END();

  return EXIT_SUCCESS;
}
