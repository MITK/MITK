/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkSlicesInterpolator.h>

#include "QmitkTestQApplication.h"

#include <QmitkRenderWindow.h>

#include <mitkImageGenerator.h>
#include <mitkRenderingManager.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkToolManager.h>

class QmitkSlicesInterpolatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkSlicesInterpolatorTestSuite);
  MITK_TEST(DestroyedWindow_NextWindowInitialized_DoesNotCrash);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    EnsureQApplication();
  }

  /**
   * An editor may destroy the windows the interpolator watches before it
   * re-initializes the interpolator; the MxN display does so when it applies a
   * layout. The windows of the new layout are initialized while they are
   * built, which sends a time event, and the interpolator must have let go of
   * the destroyed windows by then.
   */
  void DestroyedWindow_NextWindowInitialized_DoesNotCrash()
  {
    auto dataStorage = mitk::StandaloneDataStorage::New();
    auto toolManager = mitk::ToolManager::New(dataStorage);
    auto image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);
    auto* renderingManager = mitk::RenderingManager::GetInstance();

    QmitkSlicesInterpolator interpolator;
    interpolator.SetDataStorage(dataStorage);

    auto* watched = new QmitkRenderWindow(nullptr, "slicesInterpolatorTest_watched");
    renderingManager->InitializeView(watched->GetVtkRenderWindow(), image->GetTimeGeometry());
    interpolator.Initialize(toolManager, { watched });
    delete watched;

    QmitkRenderWindow next(nullptr, "slicesInterpolatorTest_next");
    renderingManager->InitializeView(next.GetVtkRenderWindow(), image->GetTimeGeometry());

    interpolator.Uninitialize();
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkSlicesInterpolator)
