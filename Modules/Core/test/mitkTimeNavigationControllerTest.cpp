/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGeometry3D.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkTimeNavigationController.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkTimeNavigationControllerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkTimeNavigationControllerTestSuite);
  MITK_TEST(Update_FewerStepsThanSelected_StepperObserversReadValidTimePoint);
  CPPUNIT_TEST_SUITE_END();

  static mitk::ProportionalTimeGeometry::Pointer CreateTimeGeometry(mitk::TimeStepType timeSteps)
  {
    const double bounds[6] = { 0.0, 20.0, 0.0, 20.0, 0.0, 20.0 };
    auto geometry = mitk::Geometry3D::New();
    geometry->SetFloatBounds(bounds);

    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(geometry, timeSteps);
    timeGeometry->SetFirstTimePoint(0.0);
    timeGeometry->SetStepDuration(1.0);
    timeGeometry->Update();
    return timeGeometry;
  }

public:
  void Update_FewerStepsThanSelected_StepperObserversReadValidTimePoint()
  {
    auto controller = mitk::TimeNavigationController::New();
    controller->SetInputWorldTimeGeometry(CreateTimeGeometry(3));
    controller->Update();
    controller->GetStepper()->SetPos(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fixture: the last of three time steps", 2.0, controller->GetSelectedTimePoint());

    // Like the time slider of a navigation widget, read the selected time on every stepper change.
    const auto* observedController = controller.GetPointer();
    controller->GetStepper()->AddObserver(itk::ModifiedEvent(),
      [observedController](const itk::EventObject&) { observedController->GetSelectedTimePoint(); });

    controller->SetInputWorldTimeGeometry(CreateTimeGeometry(1));
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Updating to fewer time steps than the selected one", controller->Update());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The selected time point after the update", 0.0, controller->GetSelectedTimePoint());

    unsigned int timeEventCount = 0;
    controller->AddObserver(mitk::TimeNavigationController::TimeEvent(0),
      [&timeEventCount](const itk::EventObject&) { ++timeEventCount; });
    controller->Update();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Time events of the next update", 1u, timeEventCount);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkTimeNavigationController)
