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

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::VirtualTrackingDevice::Pointer m_TestTracker;

public:

  void setUp()
  {
   m_TestTracker = mitk::VirtualTrackingDevice::New();
  }

  void tearDown()
  {

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

};

MITK_TEST_SUITE_REGISTRATION(mitkVirtualTrackingDevice)
