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
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::pa::MonteCarloThreadHandler::Pointer m_MonteCarloThreadHandler;
  long m_NumberOrTime = 500;

public:

  void setUp() override
  {
  }

  void testConstructorBehavior()
  {
    auto threadHandler1 = mitk::pa::MonteCarloThreadHandler::New(m_NumberOrTime, true, true);
    auto threadHandler2 = mitk::pa::MonteCarloThreadHandler::New(m_NumberOrTime, true);

    CPPUNIT_ASSERT(mitk::pa::Equal(threadHandler1, threadHandler2, 1e-6, true));
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

  void tearDown() override
  {
    m_MonteCarloThreadHandler = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMCThreadHandler)
