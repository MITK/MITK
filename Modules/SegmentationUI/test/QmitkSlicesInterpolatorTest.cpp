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
#include <mitkSliceNavigationController.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkSurfaceInterpolationController.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkToolManager.h>

class QmitkSlicesInterpolatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkSlicesInterpolatorTestSuite);
  MITK_TEST(DestroyedWindow_NextWindowInitialized_DoesNotCrash);
  MITK_TEST(Uninitialized_ActiveLabelChangesWith2D_LeavesWindowAlone);
  MITK_TEST(Uninitialized_SurfaceControllerModifiedWith3D_DoesNotCrash);
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

  /**
   * The Segmentation view uninitializes the interpolator when the last render
   * window editor closes, which destroys the windows, while the label
   * inspector keeps setting the active label. The interpolator must not send
   * a slice through a window it no longer watches; with the window gone that
   * would be a use after free. The window stays alive here so that the slice
   * it would send can be counted.
   */
  void Uninitialized_ActiveLabelChangesWith2D_LeavesWindowAlone()
  {
    auto dataStorage = mitk::StandaloneDataStorage::New();
    auto toolManager = mitk::ToolManager::New(dataStorage);
    auto image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);

    QmitkSlicesInterpolator interpolator;
    interpolator.SetDataStorage(dataStorage);

    QmitkRenderWindow window(nullptr, "slicesInterpolatorTest_2D");
    mitk::RenderingManager::GetInstance()->InitializeView(window.GetVtkRenderWindow(), image->GetTimeGeometry());
    interpolator.Initialize(toolManager, { &window });
    interpolator.EnableInterpolation(true);
    interpolator.Uninitialize();

    auto* slicer = window.GetSliceNavigationController();
    int slicesSent = 0;
    const auto tag = slicer->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(nullptr, 0),
      [&slicesSent](const itk::EventObject&) { ++slicesSent; });

    interpolator.SetActiveLabelValue(2);

    slicer->RemoveObserver(tag);
    CPPUNIT_ASSERT_EQUAL(0, slicesSent);
  }

  /**
   * The surface interpolation controller is shared and also modified while no
   * render window editor is open, for example when a label of the
   * segmentation is removed.
   */
  void Uninitialized_SurfaceControllerModifiedWith3D_DoesNotCrash()
  {
    auto dataStorage = mitk::StandaloneDataStorage::New();
    auto toolManager = mitk::ToolManager::New(dataStorage);
    auto image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);

    QmitkSlicesInterpolator interpolator;
    interpolator.SetDataStorage(dataStorage);

    QmitkRenderWindow window(nullptr, "slicesInterpolatorTest_3D");
    mitk::RenderingManager::GetInstance()->InitializeView(window.GetVtkRenderWindow(), image->GetTimeGeometry());
    interpolator.Initialize(toolManager, { &window });
    interpolator.Enable3DInterpolation(true);
    interpolator.Uninitialize();

    mitk::SurfaceInterpolationController::GetInstance()->Modified();
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkSlicesInterpolator)
