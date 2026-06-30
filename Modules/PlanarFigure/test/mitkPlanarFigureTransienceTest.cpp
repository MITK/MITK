/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreServices.h>
#include <mitkIPropertyTransience.h>

#include <mitkPlanarCircle.h>
#include <mitkPointSet.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkPlanarFigureTransienceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureTransienceTestSuite);

  MITK_TEST(HoveringIsTransientForPlanarFigure);
  MITK_TEST(HoveringIsNotTransientForOtherType);
  MITK_TEST(SelectedIsTransientForPlanarFigure);

  CPPUNIT_TEST_SUITE_END();

public:
  void HoveringIsTransientForPlanarFigure()
  {
    mitk::CoreServicePointer<mitk::IPropertyTransience> transience(mitk::CoreServices::GetPropertyTransience());
    auto figure = mitk::PlanarCircle::New();

    CPPUNIT_ASSERT_MESSAGE("planarfigure.ishovering is transient for planar figures",
                           transience->IsTransient(figure, "planarfigure.ishovering"));
  }

  void HoveringIsNotTransientForOtherType()
  {
    mitk::CoreServicePointer<mitk::IPropertyTransience> transience(mitk::CoreServices::GetPropertyTransience());
    auto pointSet = mitk::PointSet::New();

    CPPUNIT_ASSERT_MESSAGE("planarfigure.ishovering does not apply to other data types",
                           !transience->IsTransient(pointSet, "planarfigure.ishovering"));
  }

  void SelectedIsTransientForPlanarFigure()
  {
    mitk::CoreServicePointer<mitk::IPropertyTransience> transience(mitk::CoreServices::GetPropertyTransience());
    auto figure = mitk::PlanarCircle::New();

    CPPUNIT_ASSERT_MESSAGE("selected is transient for planar figures (global Core rule)",
                           transience->IsTransient(figure, "selected"));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureTransience)
