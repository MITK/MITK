/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

//Std includes
#include <iomanip>

//MITK includes
#include "mitkVirtualTrackingDevice.h"
#include "mitkVirtualTrackingDevice.h"
#include "mitkTrackingTool.h"

//ITK includes
#include "itksys/SystemTools.hxx"

class mitkVirtualTrackingDeviceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkVirtualTrackingDeviceTestSuite);

  MITK_TEST(NewVirtualTrackingDevice_IsCreated);
  MITK_TEST(StartTracking_NotReady_False);
  MITK_TEST(StopTracking);
  MITK_TEST(StartTracking_Ready_True);
  MITK_TEST(StartTrackingAfterConnectionClosed_False);
  MITK_TEST(GetToolCount_NoToolAdded);
  MITK_TEST(GetToolCount_SeveralToolsAdded);
  MITK_TEST(GetTool);
  MITK_TEST(SettingAndGettingCorrectBounds);
  MITK_TEST(SetToolSpeed_InvalidSpeed_Error);
  MITK_TEST(SetToolSpeed_InvalidToolNumber_Error);
  MITK_TEST(GetSplineCordLength_ValidToolIndex);
  MITK_TEST(GetSplineCordLength_InvaldiToolIndex_Error);
  MITK_TEST(StartTracking_NewPositionsProduced);
  MITK_TEST(SetParamsForGaussianNoise_GetCorrrectParams);


  CPPUNIT_TEST_SUITE_END();

private:

  mitk::VirtualTrackingDevice::Pointer m_TestTracker;

public:

  void setUp() override
  {
    m_TestTracker = mitk::VirtualTrackingDevice::New();
  }

  void tearDown() override
  {
    m_TestTracker->CloseConnection();
  }

  void NewVirtualTrackingDevice_IsCreated()
  {
    CPPUNIT_ASSERT_EQUAL(mitk::TrackingDevice::Setup, m_TestTracker->GetState());
  }

  void StartTracking_NotReady_False()
  {
    CPPUNIT_ASSERT_EQUAL(false, m_TestTracker->StartTracking());
  }

  void StopTracking()
  {
    CPPUNIT_ASSERT(m_TestTracker->StopTracking());
  }

  void StartTracking_Ready_True()
  {
    m_TestTracker->OpenConnection();
    CPPUNIT_ASSERT(m_TestTracker->StartTracking());
  }

  void StartTrackingAfterConnectionClosed_False()
  {
    m_TestTracker->OpenConnection();
    m_TestTracker->CloseConnection();
    CPPUNIT_ASSERT_EQUAL(false, m_TestTracker->StartTracking());
  }

  void GetToolCount_NoToolAdded()
  {
    unsigned int zero = 0;
    CPPUNIT_ASSERT_EQUAL(zero, m_TestTracker->GetToolCount());
  }

  void GetToolCount_SeveralToolsAdded()
  {
    unsigned int one = 1;
    unsigned int two = 2;
    unsigned int three = 3;
    m_TestTracker->AddTool("Tool1");
    CPPUNIT_ASSERT_EQUAL(one, m_TestTracker->GetToolCount());
    m_TestTracker->AddTool("Tool2");
    CPPUNIT_ASSERT_EQUAL(two, m_TestTracker->GetToolCount());
    m_TestTracker->AddTool("Tool3");
    CPPUNIT_ASSERT_EQUAL(three, m_TestTracker->GetToolCount());
  }

  void GetTool()
  {
    m_TestTracker->AddTool("Tool1");
    mitk::TrackingTool::Pointer tool = m_TestTracker->GetTool(0);
    CPPUNIT_ASSERT_EQUAL(m_TestTracker->GetToolByName("Tool1"), tool.GetPointer());
  }

  void SettingAndGettingCorrectBounds()
  {
    mitk::ScalarType bounds[6] = { 0.0, 10.0, 1.0, 20.0, 3.0, 30.0 };
    m_TestTracker->SetBounds(bounds);
    CPPUNIT_ASSERT_EQUAL(bounds[0], m_TestTracker->GetBounds()[0]);
    CPPUNIT_ASSERT_EQUAL(bounds[1], m_TestTracker->GetBounds()[1]);
    CPPUNIT_ASSERT_EQUAL(bounds[2], m_TestTracker->GetBounds()[2]);
    CPPUNIT_ASSERT_EQUAL(bounds[3], m_TestTracker->GetBounds()[3]);
    CPPUNIT_ASSERT_EQUAL(bounds[4], m_TestTracker->GetBounds()[4]);
    CPPUNIT_ASSERT_EQUAL(bounds[5], m_TestTracker->GetBounds()[5]);
  }

  void SetToolSpeed_InvalidSpeed_Error()
  {
    m_TestTracker->AddTool("Tool1");
    CPPUNIT_ASSERT_THROW(m_TestTracker->SetToolSpeed(0, -1), std::invalid_argument);
  }

  void SetToolSpeed_InvalidToolNumber_Error()
  {
    m_TestTracker->AddTool("Tool1");
    CPPUNIT_ASSERT_THROW(m_TestTracker->SetToolSpeed(1, 0.1), std::invalid_argument);
  }

  void GetSplineCordLength_ValidToolIndex()
  {
    m_TestTracker->AddTool("Tool1");
    mitk::ScalarType lengthBefore = m_TestTracker->GetSplineChordLength(0);
    m_TestTracker->OpenConnection();
    m_TestTracker->StartTracking();
    m_TestTracker->StopTracking();
    CPPUNIT_ASSERT_EQUAL(lengthBefore, m_TestTracker->GetSplineChordLength(0));
  }

  void GetSplineCordLength_InvaldiToolIndex_Error()
  {
    CPPUNIT_ASSERT_THROW(m_TestTracker->GetSplineChordLength(0), std::invalid_argument);
  }

  void StartTracking_NewPositionsProduced()
  {
    m_TestTracker->AddTool("Tool1");
    mitk::Point3D posBefore;
    mitk::Point3D posAfter;
    mitk::TrackingTool::Pointer tool = m_TestTracker->GetToolByName("Tool1");
    tool->GetPosition(posBefore);
    tool->GetPosition(posAfter);
    CPPUNIT_ASSERT_EQUAL(posBefore, posAfter);
    m_TestTracker->OpenConnection();
    m_TestTracker->StartTracking();
    itksys::SystemTools::Delay(500);  //wait for tracking thread to start generating positions
    tool->GetPosition(posAfter);
    CPPUNIT_ASSERT(posBefore != posAfter);
  }

  void SetParamsForGaussianNoise_GetCorrrectParams()
  {
    double meanDistribution = 2.5;
    double deviationDistribution = 1.2;
    m_TestTracker->SetParamsForGaussianNoise(meanDistribution, deviationDistribution);
    CPPUNIT_ASSERT_EQUAL(meanDistribution, m_TestTracker->GetMeanDistribution());
    CPPUNIT_ASSERT_EQUAL(deviationDistribution, m_TestTracker->GetDeviationDistribution());
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkVirtualTrackingDevice)
