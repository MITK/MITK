/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

// std includes
#include <limits>

// MITK includes
#include "mitkColorSequenceRainbow.h"

// VTK includes
#include <vtkDebugLeaks.h>

class mitkColorSequenceRainbowTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkColorSequenceRainbowTestSuite);

  MITK_TEST(GetNextColor_ReturnsADifferentColor);
  MITK_TEST(GoToBegin_NextCallReturnsSameColorAsFirstCall);
  MITK_TEST(GetNextColor_CanWrapAroundWithoutCrashing);
  CPPUNIT_TEST_SUITE_END();


public:
  /**
  * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used
  * members for a new test case. (If the members are not used in a test, the method does not need to be called).
  */
  void setUp() override
  {
  }

  void tearDown() override
  {
  }

  void GetNextColor_ReturnsADifferentColor()
  {
    mitk::ColorSequenceRainbow rainbowColorSequence;
    mitk::Color color1 = rainbowColorSequence.GetNextColor();
    mitk::Color color2 = rainbowColorSequence.GetNextColor();

    CPPUNIT_ASSERT_MESSAGE("Two consecutive colors are not equal.", color1 != color2);
  }

  void GoToBegin_NextCallReturnsSameColorAsFirstCall()
  {
    mitk::ColorSequenceRainbow rainbowColorSequence;
    mitk::Color color1 = rainbowColorSequence.GetNextColor();
    rainbowColorSequence.GoToBegin();
    mitk::Color color2 = rainbowColorSequence.GetNextColor();

    CPPUNIT_ASSERT_MESSAGE("GoToBegin is identical to beginning.", color1 == color2);
  }

  void GetNextColor_CanWrapAroundWithoutCrashing()
  {
    int counter = 0;
    try
    {
      mitk::ColorSequenceRainbow rainbowColorSequence;
      mitk::Color color1 = rainbowColorSequence.GetNextColor();
      mitk::Color color2 = rainbowColorSequence.GetNextColor();

      while (color1 != color2 && counter < std::numeric_limits<int>::max())
      {
        color2 = rainbowColorSequence.GetNextColor();
        ++counter;
      }
    }
    catch (...)
    {
      CPPUNIT_FAIL("Exception during rainbow color sequence color generation");
    }
    CPPUNIT_ASSERT_MESSAGE("Error free wraparound achieved.", counter < std::numeric_limits<int>::max());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkColorSequenceRainbow)
