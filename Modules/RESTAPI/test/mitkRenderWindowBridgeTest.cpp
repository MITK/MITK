/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkRenderWindowBridge.h>

#include <stdexcept>
#include <string>

/**
 * @brief Unit tests for RenderWindowBridge.
 *
 * Covers Has...() set/clear round-trips for the std-multi callback set,
 * exception transport for the three typed bridge exceptions, ResetCallbacks()
 * clearing every registered callback, and inline invocation when no
 * dispatcher is set.
 */
class mitkRenderWindowBridgeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRenderWindowBridgeTestSuite);

  MITK_TEST(HasFlagsReflectSetAndClearForAllStdMultiCallbacks);
  MITK_TEST(ResetCallbacksClearsAllCallbacks);
  MITK_TEST(EditorListInvokerReturnsProviderResult);
  MITK_TEST(WindowListInvokerReturnsProviderResult);
  MITK_TEST(CameraGetterTransportsNoEditorException);
  MITK_TEST(CameraSetterTransportsUnknownWindowException);
  MITK_TEST(SelectedSliceGetterTransportsUnsupportedOperationException);
  MITK_TEST(InvokersThrowWhenNoCallbackSet);
  MITK_TEST(PostResetInvocationThrowsCleanlyForEveryStdMultiInvoker);

  CPPUNIT_TEST_SUITE_END();

private:
  std::unique_ptr<mitk::RenderWindowBridge> m_Bridge;

public:
  void setUp() override
  {
    m_Bridge = std::make_unique<mitk::RenderWindowBridge>();
  }

  void tearDown() override
  {
    m_Bridge.reset();
  }

  void HasFlagsReflectSetAndClearForAllStdMultiCallbacks()
  {
    CPPUNIT_ASSERT(!m_Bridge->HasEditorListProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiWindowListProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiEditorScreenshotProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiWindowScreenshotProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiCameraGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiCameraSetter());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiSelectedSliceGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiSelectedSliceStepSetter());

    m_Bridge->SetEditorListProvider([]() { return std::vector<mitk::EditorInfo>{}; });
    m_Bridge->SetStdMultiWindowListProvider([]() { return std::vector<mitk::WindowInfo>{}; });
    m_Bridge->SetStdMultiEditorScreenshotProvider(
      [](std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetStdMultiWindowScreenshotProvider(
      [](const std::string&, std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetStdMultiCameraGetter([](const std::string&) { return mitk::CameraState{}; });
    m_Bridge->SetStdMultiCameraSetter([](const std::string&, const mitk::CameraPatch&) {});
    m_Bridge->SetStdMultiSelectedSliceGetter([](const std::string&) { return mitk::SliceState{}; });
    m_Bridge->SetStdMultiSelectedSliceStepSetter([](const std::string&, unsigned int) {});

    CPPUNIT_ASSERT(m_Bridge->HasEditorListProvider());
    CPPUNIT_ASSERT(m_Bridge->HasStdMultiWindowListProvider());
    CPPUNIT_ASSERT(m_Bridge->HasStdMultiEditorScreenshotProvider());
    CPPUNIT_ASSERT(m_Bridge->HasStdMultiWindowScreenshotProvider());
    CPPUNIT_ASSERT(m_Bridge->HasStdMultiCameraGetter());
    CPPUNIT_ASSERT(m_Bridge->HasStdMultiCameraSetter());
    CPPUNIT_ASSERT(m_Bridge->HasStdMultiSelectedSliceGetter());
    CPPUNIT_ASSERT(m_Bridge->HasStdMultiSelectedSliceStepSetter());

    m_Bridge->SetEditorListProvider({});
    CPPUNIT_ASSERT(!m_Bridge->HasEditorListProvider());
  }

  void ResetCallbacksClearsAllCallbacks()
  {
    m_Bridge->SetScreenshotProvider(
      [](std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetPositionGetter([]() { return mitk::SelectedPositionInfo{}; });
    m_Bridge->SetPositionSetter([](const mitk::Point3D&) {});
    m_Bridge->SetEditorListProvider([]() { return std::vector<mitk::EditorInfo>{}; });
    m_Bridge->SetStdMultiWindowListProvider([]() { return std::vector<mitk::WindowInfo>{}; });
    m_Bridge->SetStdMultiEditorScreenshotProvider(
      [](std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetStdMultiWindowScreenshotProvider(
      [](const std::string&, std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetStdMultiCameraGetter([](const std::string&) { return mitk::CameraState{}; });
    m_Bridge->SetStdMultiCameraSetter([](const std::string&, const mitk::CameraPatch&) {});
    m_Bridge->SetStdMultiSelectedSliceGetter([](const std::string&) { return mitk::SliceState{}; });
    m_Bridge->SetStdMultiSelectedSliceStepSetter([](const std::string&, unsigned int) {});

    m_Bridge->ResetCallbacks();

    CPPUNIT_ASSERT(!m_Bridge->HasScreenshotProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasPositionGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasPositionSetter());
    CPPUNIT_ASSERT(!m_Bridge->HasEditorListProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiWindowListProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiEditorScreenshotProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiWindowScreenshotProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiCameraGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiCameraSetter());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiSelectedSliceGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiSelectedSliceStepSetter());
  }

  void EditorListInvokerReturnsProviderResult()
  {
    m_Bridge->SetEditorListProvider(
      []() {
        return std::vector<mitk::EditorInfo>{
          {"stdmulti", "org.mitk.editors.stdmultiwidget", true, {"axial","sagittal","coronal","3d"}},
          {"mxn", "org.mitk.editors.mxnmultiwidget", false, {}}
        };
      });

    const auto editors = m_Bridge->ListEditors();
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), editors.size());
    CPPUNIT_ASSERT_EQUAL(std::string("stdmulti"), editors[0].alias);
    CPPUNIT_ASSERT(editors[0].active);
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), editors[0].windowNames.size());
    CPPUNIT_ASSERT(!editors[1].active);
  }

  void WindowListInvokerReturnsProviderResult()
  {
    m_Bridge->SetStdMultiWindowListProvider(
      []() {
        return std::vector<mitk::WindowInfo>{
          {"axial", mitk::WindowKind::TwoD},
          {"sagittal", mitk::WindowKind::TwoD},
          {"coronal", mitk::WindowKind::TwoD},
          {"3d", mitk::WindowKind::ThreeD}
        };
      });

    const auto windows = m_Bridge->ListStdMultiWindows();
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), windows.size());
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), windows[3].name);
    CPPUNIT_ASSERT(mitk::WindowKind::ThreeD == windows[3].kind);
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), std::string(mitk::WindowKindToString(windows[3].kind)));
    CPPUNIT_ASSERT_EQUAL(std::string("2d"), std::string(mitk::WindowKindToString(windows[0].kind)));
  }

  void CameraGetterTransportsNoEditorException()
  {
    m_Bridge->SetStdMultiCameraGetter(
      [](const std::string&) -> mitk::CameraState {
        throw mitk::RenderWindowBridgeNoEditorException("editor not open");
      });

    bool caught = false;
    try
    {
      m_Bridge->GetStdMultiCamera("axial");
    }
    catch (const mitk::RenderWindowBridgeNoEditorException& e)
    {
      caught = true;
      CPPUNIT_ASSERT_EQUAL(std::string("editor not open"), std::string(e.what()));
    }
    CPPUNIT_ASSERT(caught);
  }

  void CameraSetterTransportsUnknownWindowException()
  {
    m_Bridge->SetStdMultiCameraSetter(
      [](const std::string& name, const mitk::CameraPatch&) {
        throw mitk::RenderWindowBridgeUnknownWindowException(name);
      });

    bool caught = false;
    try
    {
      m_Bridge->SetStdMultiCamera("bogus", mitk::CameraPatch{});
    }
    catch (const mitk::RenderWindowBridgeUnknownWindowException& e)
    {
      caught = true;
      CPPUNIT_ASSERT_EQUAL(std::string("bogus"), std::string(e.what()));
    }
    CPPUNIT_ASSERT(caught);
  }

  void SelectedSliceGetterTransportsUnsupportedOperationException()
  {
    m_Bridge->SetStdMultiSelectedSliceGetter(
      [](const std::string&) -> mitk::SliceState {
        throw mitk::RenderWindowBridgeUnsupportedOperationException("3d has no slice");
      });

    bool caught = false;
    try
    {
      m_Bridge->GetStdMultiSelectedSlice("3d");
    }
    catch (const mitk::RenderWindowBridgeUnsupportedOperationException& e)
    {
      caught = true;
      CPPUNIT_ASSERT_EQUAL(std::string("3d has no slice"), std::string(e.what()));
    }
    CPPUNIT_ASSERT(caught);
  }

  /**
   * Shutdown-race regression: after ResetCallbacks() clears every std-multi
   * callback, a concurrent invocation attempt on any std-multi invoker must
   * terminate with a std::runtime_error instead of crashing on a null
   * std::function. The throw path, not the call path, is the contract during
   * service UNREGISTERING.
   */
  void PostResetInvocationThrowsCleanlyForEveryStdMultiInvoker()
  {
    // Install every std-multi callback, then reset.
    m_Bridge->SetEditorListProvider([]() { return std::vector<mitk::EditorInfo>{}; });
    m_Bridge->SetStdMultiWindowListProvider([]() { return std::vector<mitk::WindowInfo>{}; });
    m_Bridge->SetStdMultiEditorScreenshotProvider(
      [](std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetStdMultiWindowScreenshotProvider(
      [](const std::string&, std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetStdMultiCameraGetter([](const std::string&) { mitk::CameraState s; return s; });
    m_Bridge->SetStdMultiCameraSetter([](const std::string&, const mitk::CameraPatch&) {});
    m_Bridge->SetStdMultiSelectedSliceGetter([](const std::string&) { mitk::SliceState s; return s; });
    m_Bridge->SetStdMultiSelectedSliceStepSetter([](const std::string&, unsigned int) {});

    m_Bridge->ResetCallbacks();

    // Every std-multi invoker must now throw a plain std::runtime_error — not
    // any of the typed bridge exceptions (which are callback-generated
    // signals), and not an access violation.
    CPPUNIT_ASSERT_THROW(m_Bridge->ListEditors(), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->ListStdMultiWindows(), std::runtime_error);
    CPPUNIT_ASSERT_THROW(
      m_Bridge->TakeStdMultiEditorScreenshot(std::nullopt, mitk::ScreenshotFormat::Png),
      std::runtime_error);
    CPPUNIT_ASSERT_THROW(
      m_Bridge->TakeStdMultiWindowScreenshot("axial", std::nullopt, mitk::ScreenshotFormat::Png),
      std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->GetStdMultiCamera("axial"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->SetStdMultiCamera("axial", mitk::CameraPatch{}), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->GetStdMultiSelectedSlice("axial"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->SetStdMultiSelectedSliceStep("axial", 0u), std::runtime_error);
  }

  void InvokersThrowWhenNoCallbackSet()
  {
    CPPUNIT_ASSERT_THROW(m_Bridge->ListEditors(), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->ListStdMultiWindows(), std::runtime_error);
    CPPUNIT_ASSERT_THROW(
      m_Bridge->TakeStdMultiEditorScreenshot(std::nullopt, mitk::ScreenshotFormat::Png),
      std::runtime_error);
    CPPUNIT_ASSERT_THROW(
      m_Bridge->TakeStdMultiWindowScreenshot("axial", std::nullopt, mitk::ScreenshotFormat::Png),
      std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->GetStdMultiCamera("axial"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->SetStdMultiCamera("axial", mitk::CameraPatch{}), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->GetStdMultiSelectedSlice("axial"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->SetStdMultiSelectedSliceStep("axial", 0u), std::runtime_error);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRenderWindowBridge)
