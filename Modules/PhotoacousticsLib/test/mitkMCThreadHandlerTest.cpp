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

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkPAMonteCarloThreadHandler.h>

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResourceStream.h>

#include <random>
#include <chrono>

class mitkMCThreadHandlerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMCThreadHandlerTestSuite);
  MITK_TEST(testConstructorBehavior);
  MITK_TEST(testCorrectNumberOfPhotons);
  MITK_TEST(testCorrectNumberOfPhotonsWithUnevenPackageSize);
  MITK_TEST(testCorrectNumberOfPhotonsWithTooLargePackageSize);
  MITK_TEST(testCorrectTimeMeasure);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::pa::MonteCarloThreadHandler::Pointer m_MonteCarloThreadHandler;
  long m_NumberOrTime = 500;

public:

  void setUp()
  {
  }

  void testConstructorBehavior()
  {
    auto threadHandler1 = mitk::pa::MonteCarloThreadHandler::New(m_NumberOrTime, true, true);
    auto threadHandler2 = mitk::pa::MonteCarloThreadHandler::New(m_NumberOrTime, true);

    CPPUNIT_ASSERT(mitk::pa::Equal(threadHandler1, threadHandler2, 1e-6, true));
  }

  void testCorrectTimeMeasure()
  {
    for (int i = 0; i < 10; i++)
    {
      m_MonteCarloThreadHandler = mitk::pa::MonteCarloThreadHandler::New(m_NumberOrTime, true, false);
      auto timeBefore = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
      long nextWorkPackage = 0;
      while ((nextWorkPackage = m_MonteCarloThreadHandler->GetNextWorkPackage()) > 0)
      {//Do nothing
      }
      auto timeAfter = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

      //Assert that the time error is less than 10% in a 500ms sample size
      //This test might not be stable when on different machines.
      CPPUNIT_ASSERT(abs((timeAfter - timeBefore) - m_NumberOrTime) <= 50);
    }
  }

  void testCorrectNumberOfPhotons()
  {
    m_MonteCarloThreadHandler = mitk::pa::MonteCarloThreadHandler::New(m_NumberOrTime, false, false);
    m_MonteCarloThreadHandler->SetPackageSize(100);
    long numberOfPhotonsSimulated = 0;
    long nextWorkPackage = 0;
    while ((nextWorkPackage = m_MonteCarloThreadHandler->GetNextWorkPackage()) > 0)
    {
      numberOfPhotonsSimulated += nextWorkPackage;
    }
    CPPUNIT_ASSERT(numberOfPhotonsSimulated == m_NumberOrTime);
  }

  void testCorrectNumberOfPhotonsWithUnevenPackageSize()
  {
    m_MonteCarloThreadHandler = mitk::pa::MonteCarloThreadHandler::New(m_NumberOrTime, false, false);
    m_MonteCarloThreadHandler->SetPackageSize(77);
    long numberOfPhotonsSimulated = 0;
    long nextWorkPackage = 0;
    while ((nextWorkPackage = m_MonteCarloThreadHandler->GetNextWorkPackage()) > 0)
    {
      numberOfPhotonsSimulated += nextWorkPackage;
    }
    CPPUNIT_ASSERT(numberOfPhotonsSimulated == m_NumberOrTime);
  }

  void testCorrectNumberOfPhotonsWithTooLargePackageSize()
  {
    m_MonteCarloThreadHandler = mitk::pa::MonteCarloThreadHandler::New(m_NumberOrTime, false, false);
    m_MonteCarloThreadHandler->SetPackageSize(10000);
    long numberOfPhotonsSimulated = 0;
    long nextWorkPackage = 0;
    while ((nextWorkPackage = m_MonteCarloThreadHandler->GetNextWorkPackage()) > 0)
    {
      numberOfPhotonsSimulated += nextWorkPackage;
    }
    CPPUNIT_ASSERT(numberOfPhotonsSimulated == m_NumberOrTime);
  }

  void tearDown()
  {
    m_MonteCarloThreadHandler = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMCThreadHandler)
