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

  // MxN bridge surface
  MITK_TEST(HasFlagsReflectSetAndClearForAllMxNCallbacks);
  MITK_TEST(MxNWindowListInvokerReturnsProviderResult);
  MITK_TEST(MxNLayoutGetterRoundTripsString);
  MITK_TEST(MxNLayoutSetterEchoesProviderResult);
  MITK_TEST(MxNCameraGetterTransportsNoEditorException);
  MITK_TEST(MxNSelectedPositionInvokerForwardsArgsAndResult);
  MITK_TEST(MxNInvokersThrowWhenNoCallbackSet);
  MITK_TEST(PostResetInvocationThrowsCleanlyForEveryMxNInvoker);

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

    // Clear-and-verify each provider individually so a future
    // SetX(provider) that forgets to handle the empty-function case is
    // caught by this regression net, not just the bulk ResetCallbacks path.
    m_Bridge->SetEditorListProvider({});
    CPPUNIT_ASSERT(!m_Bridge->HasEditorListProvider());

    m_Bridge->SetStdMultiWindowListProvider({});
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiWindowListProvider());

    m_Bridge->SetStdMultiEditorScreenshotProvider({});
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiEditorScreenshotProvider());

    m_Bridge->SetStdMultiWindowScreenshotProvider({});
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiWindowScreenshotProvider());

    m_Bridge->SetStdMultiCameraGetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiCameraGetter());

    m_Bridge->SetStdMultiCameraSetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiCameraSetter());

    m_Bridge->SetStdMultiSelectedSliceGetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiSelectedSliceGetter());

    m_Bridge->SetStdMultiSelectedSliceStepSetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasStdMultiSelectedSliceStepSetter());
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

    // Also install every MxN callback so the bulk-clear is exercised end-to-end.
    m_Bridge->SetMxNWindowListProvider([]() { return std::vector<mitk::MxNWindowInfo>{}; });
    m_Bridge->SetMxNEditorScreenshotProvider(
      [](std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetMxNWindowScreenshotProvider(
      [](const std::string&, std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetMxNCameraGetter([](const std::string&) { return mitk::CameraState{}; });
    m_Bridge->SetMxNCameraSetter([](const std::string&, const mitk::CameraPatch&) {});
    m_Bridge->SetMxNSelectedSliceGetter([](const std::string&) { return mitk::SliceState{}; });
    m_Bridge->SetMxNSelectedSliceStepSetter([](const std::string&, unsigned int) {});
    m_Bridge->SetMxNSelectedPositionGetter([](const std::string&) { return mitk::SelectedPositionInfo{}; });
    m_Bridge->SetMxNSelectedPositionSetter([](const std::string&, const mitk::Point3D&) {});
    m_Bridge->SetMxNLayoutGetter([]() { return std::string("{}"); });
    m_Bridge->SetMxNLayoutSetter([](const std::string&) { return std::string("{}"); });

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
    CPPUNIT_ASSERT(!m_Bridge->HasMxNWindowListProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNEditorScreenshotProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNWindowScreenshotProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNCameraGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNCameraSetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedSliceGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedSliceStepSetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedPositionGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedPositionSetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNLayoutGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNLayoutSetter());
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
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), editors[0].windowIds.size());
    CPPUNIT_ASSERT(!editors[1].active);
  }

  void WindowListInvokerReturnsProviderResult()
  {
    m_Bridge->SetStdMultiWindowListProvider(
      []() {
        return std::vector<mitk::WindowInfo>{
          {"axial",    mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Axial},
          {"sagittal", mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Sagittal},
          {"coronal",  mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Coronal},
          {"3d",       mitk::WindowKind::ThreeD, std::nullopt}
        };
      });

    const auto windows = m_Bridge->ListStdMultiWindows();
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), windows.size());
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), windows[3].id);
    CPPUNIT_ASSERT(mitk::WindowKind::ThreeD == windows[3].kind);
    CPPUNIT_ASSERT(!windows[3].viewDirection.has_value());
    CPPUNIT_ASSERT(windows[0].viewDirection.has_value());
    CPPUNIT_ASSERT(mitk::AnatomicalPlane::Axial == *windows[0].viewDirection);
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

    // Every std-multi invoker must now throw a plain std::runtime_error -- not
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

  // ====================================================================
  // MxN bridge surface
  // ====================================================================

  void HasFlagsReflectSetAndClearForAllMxNCallbacks()
  {
    CPPUNIT_ASSERT(!m_Bridge->HasMxNWindowListProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNEditorScreenshotProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNWindowScreenshotProvider());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNCameraGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNCameraSetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedSliceGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedSliceStepSetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedPositionGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedPositionSetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNLayoutGetter());
    CPPUNIT_ASSERT(!m_Bridge->HasMxNLayoutSetter());

    m_Bridge->SetMxNWindowListProvider([]() { return std::vector<mitk::MxNWindowInfo>{}; });
    m_Bridge->SetMxNEditorScreenshotProvider(
      [](std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetMxNWindowScreenshotProvider(
      [](const std::string&, std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetMxNCameraGetter([](const std::string&) { return mitk::CameraState{}; });
    m_Bridge->SetMxNCameraSetter([](const std::string&, const mitk::CameraPatch&) {});
    m_Bridge->SetMxNSelectedSliceGetter([](const std::string&) { return mitk::SliceState{}; });
    m_Bridge->SetMxNSelectedSliceStepSetter([](const std::string&, unsigned int) {});
    m_Bridge->SetMxNSelectedPositionGetter([](const std::string&) { return mitk::SelectedPositionInfo{}; });
    m_Bridge->SetMxNSelectedPositionSetter([](const std::string&, const mitk::Point3D&) {});
    m_Bridge->SetMxNLayoutGetter([]() { return std::string("{}"); });
    m_Bridge->SetMxNLayoutSetter([](const std::string&) { return std::string("{}"); });

    CPPUNIT_ASSERT(m_Bridge->HasMxNWindowListProvider());
    CPPUNIT_ASSERT(m_Bridge->HasMxNEditorScreenshotProvider());
    CPPUNIT_ASSERT(m_Bridge->HasMxNWindowScreenshotProvider());
    CPPUNIT_ASSERT(m_Bridge->HasMxNCameraGetter());
    CPPUNIT_ASSERT(m_Bridge->HasMxNCameraSetter());
    CPPUNIT_ASSERT(m_Bridge->HasMxNSelectedSliceGetter());
    CPPUNIT_ASSERT(m_Bridge->HasMxNSelectedSliceStepSetter());
    CPPUNIT_ASSERT(m_Bridge->HasMxNSelectedPositionGetter());
    CPPUNIT_ASSERT(m_Bridge->HasMxNSelectedPositionSetter());
    CPPUNIT_ASSERT(m_Bridge->HasMxNLayoutGetter());
    CPPUNIT_ASSERT(m_Bridge->HasMxNLayoutSetter());

    // Clear-and-verify each individually so a future SetX(provider) regression
    // is caught here, not just by the bulk ResetCallbacks path.
    m_Bridge->SetMxNWindowListProvider({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNWindowListProvider());
    m_Bridge->SetMxNEditorScreenshotProvider({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNEditorScreenshotProvider());
    m_Bridge->SetMxNWindowScreenshotProvider({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNWindowScreenshotProvider());
    m_Bridge->SetMxNCameraGetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNCameraGetter());
    m_Bridge->SetMxNCameraSetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNCameraSetter());
    m_Bridge->SetMxNSelectedSliceGetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedSliceGetter());
    m_Bridge->SetMxNSelectedSliceStepSetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedSliceStepSetter());
    m_Bridge->SetMxNSelectedPositionGetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedPositionGetter());
    m_Bridge->SetMxNSelectedPositionSetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNSelectedPositionSetter());
    m_Bridge->SetMxNLayoutGetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNLayoutGetter());
    m_Bridge->SetMxNLayoutSetter({});
    CPPUNIT_ASSERT(!m_Bridge->HasMxNLayoutSetter());
  }

  void MxNWindowListInvokerReturnsProviderResult()
  {
    m_Bridge->SetMxNWindowListProvider(
      []() {
        // Field order: id, displayName, kind, viewDirection, selectionGroup.
        // Cell 0 carries a display name; the others omit it (nullopt).
        return std::vector<mitk::MxNWindowInfo>{
          {"mxn__widget0", std::string("Tumor axial"), mitk::WindowKind::TwoD, mitk::AnatomicalPlane::Axial,    "main"},
          {"mxn__widget1", std::nullopt,               mitk::WindowKind::TwoD, mitk::AnatomicalPlane::Sagittal, "main"},
          {"mxn__widget2", std::nullopt,               mitk::WindowKind::TwoD, mitk::AnatomicalPlane::Coronal,  "row2"}
        };
      });

    const auto windows = m_Bridge->ListMxNWindows();
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), windows.size());
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget0"), windows[0].id);
    CPPUNIT_ASSERT(windows[0].displayName.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("Tumor axial"), *windows[0].displayName);
    CPPUNIT_ASSERT(!windows[1].displayName.has_value());
    CPPUNIT_ASSERT(windows[0].viewDirection.has_value());
    CPPUNIT_ASSERT(mitk::AnatomicalPlane::Axial == *windows[0].viewDirection);
    CPPUNIT_ASSERT_EQUAL(std::string("main"), windows[0].selectionGroup);
    CPPUNIT_ASSERT_EQUAL(std::string("row2"), windows[2].selectionGroup);
    CPPUNIT_ASSERT(mitk::WindowKind::TwoD == windows[0].kind);
  }

  void MxNLayoutGetterRoundTripsString()
  {
    m_Bridge->SetMxNLayoutGetter(
      []() { return std::string(R"({"version":"2.0","root":{}})"); });

    const auto layout = m_Bridge->GetMxNLayout();
    CPPUNIT_ASSERT_EQUAL(std::string(R"({"version":"2.0","root":{}})"), layout);
  }

  void MxNLayoutSetterEchoesProviderResult()
  {
    // The setter contract: receive a layout JSON string, return the freshly
    // serialized state (so a PUT can echo without a follow-up GET).
    std::string captured;
    m_Bridge->SetMxNLayoutSetter(
      [&captured](const std::string& body) {
        captured = body;
        return std::string(R"({"version":"2.0","name":"echoed"})");
      });

    const auto echoed = m_Bridge->SetMxNLayout(R"({"version":"2.0","name":"sent"})");
    CPPUNIT_ASSERT_EQUAL(std::string(R"({"version":"2.0","name":"sent"})"), captured);
    CPPUNIT_ASSERT_EQUAL(std::string(R"({"version":"2.0","name":"echoed"})"), echoed);
  }

  void MxNCameraGetterTransportsNoEditorException()
  {
    m_Bridge->SetMxNCameraGetter(
      [](const std::string&) -> mitk::CameraState {
        throw mitk::RenderWindowBridgeNoEditorException("MxN editor not open");
      });

    bool caught = false;
    try
    {
      m_Bridge->GetMxNCamera("mxn__widget0");
    }
    catch (const mitk::RenderWindowBridgeNoEditorException& e)
    {
      caught = true;
      CPPUNIT_ASSERT_EQUAL(std::string("MxN editor not open"), std::string(e.what()));
    }
    CPPUNIT_ASSERT(caught);
  }

  void MxNSelectedPositionInvokerForwardsArgsAndResult()
  {
    // Verify GetMxNSelectedPosition forwards the windowId and returns the
    // provider's SelectedPositionInfo unchanged. Mirrors the global
    // PositionGetter shape, but window-name-keyed.
    std::string capturedName;
    m_Bridge->SetMxNSelectedPositionGetter(
      [&capturedName](const std::string& name) {
        capturedName = name;
        mitk::SelectedPositionInfo info;
        info.position[0] = 1.0; info.position[1] = 2.0; info.position[2] = 3.0;
        mitk::WorldBounds bounds;
        bounds.minPosition.Fill(-10.0); bounds.maxPosition.Fill(10.0);
        info.bounds = bounds;
        return info;
      });

    const auto info = m_Bridge->GetMxNSelectedPosition("mxn__widget7");
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget7"), capturedName);
    CPPUNIT_ASSERT_EQUAL(1.0, info.position[0]);
    CPPUNIT_ASSERT_EQUAL(2.0, info.position[1]);
    CPPUNIT_ASSERT_EQUAL(3.0, info.position[2]);
    CPPUNIT_ASSERT(info.bounds.has_value());

    // Setter: verify args are forwarded.
    std::string sName;
    mitk::Point3D sPos; sPos.Fill(0.0);
    m_Bridge->SetMxNSelectedPositionSetter(
      [&sName, &sPos](const std::string& name, const mitk::Point3D& p) {
        sName = name;
        sPos = p;
      });
    mitk::Point3D target;
    target[0] = 7.0; target[1] = 8.0; target[2] = 9.0;
    m_Bridge->SetMxNSelectedPosition("widgetX", target);
    CPPUNIT_ASSERT_EQUAL(std::string("widgetX"), sName);
    CPPUNIT_ASSERT_EQUAL(7.0, sPos[0]);
    CPPUNIT_ASSERT_EQUAL(9.0, sPos[2]);
  }

  void MxNInvokersThrowWhenNoCallbackSet()
  {
    CPPUNIT_ASSERT_THROW(m_Bridge->ListMxNWindows(), std::runtime_error);
    CPPUNIT_ASSERT_THROW(
      m_Bridge->TakeMxNEditorScreenshot(std::nullopt, mitk::ScreenshotFormat::Png),
      std::runtime_error);
    CPPUNIT_ASSERT_THROW(
      m_Bridge->TakeMxNWindowScreenshot("mxn__widget0", std::nullopt, mitk::ScreenshotFormat::Png),
      std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->GetMxNCamera("mxn__widget0"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->SetMxNCamera("mxn__widget0", mitk::CameraPatch{}), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->GetMxNSelectedSlice("mxn__widget0"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->SetMxNSelectedSliceStep("mxn__widget0", 0u), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->GetMxNSelectedPosition("mxn__widget0"), std::runtime_error);
    {
      mitk::Point3D p; p.Fill(0.0);
      CPPUNIT_ASSERT_THROW(m_Bridge->SetMxNSelectedPosition("mxn__widget0", p), std::runtime_error);
    }
    CPPUNIT_ASSERT_THROW(m_Bridge->GetMxNLayout(), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->SetMxNLayout("{}"), std::runtime_error);
  }

  /**
   * Shutdown-race regression for MxN callbacks: install all of them, reset,
   * then assert every invoker throws std::runtime_error rather than crashing.
   * Mirrors PostResetInvocationThrowsCleanlyForEveryStdMultiInvoker.
   */
  void PostResetInvocationThrowsCleanlyForEveryMxNInvoker()
  {
    m_Bridge->SetMxNWindowListProvider([]() { return std::vector<mitk::MxNWindowInfo>{}; });
    m_Bridge->SetMxNEditorScreenshotProvider(
      [](std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetMxNWindowScreenshotProvider(
      [](const std::string&, std::optional<std::pair<int,int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });
    m_Bridge->SetMxNCameraGetter([](const std::string&) { return mitk::CameraState{}; });
    m_Bridge->SetMxNCameraSetter([](const std::string&, const mitk::CameraPatch&) {});
    m_Bridge->SetMxNSelectedSliceGetter([](const std::string&) { return mitk::SliceState{}; });
    m_Bridge->SetMxNSelectedSliceStepSetter([](const std::string&, unsigned int) {});
    m_Bridge->SetMxNSelectedPositionGetter([](const std::string&) { return mitk::SelectedPositionInfo{}; });
    m_Bridge->SetMxNSelectedPositionSetter([](const std::string&, const mitk::Point3D&) {});
    m_Bridge->SetMxNLayoutGetter([]() { return std::string("{}"); });
    m_Bridge->SetMxNLayoutSetter([](const std::string&) { return std::string("{}"); });

    m_Bridge->ResetCallbacks();

    CPPUNIT_ASSERT_THROW(m_Bridge->ListMxNWindows(), std::runtime_error);
    CPPUNIT_ASSERT_THROW(
      m_Bridge->TakeMxNEditorScreenshot(std::nullopt, mitk::ScreenshotFormat::Png),
      std::runtime_error);
    CPPUNIT_ASSERT_THROW(
      m_Bridge->TakeMxNWindowScreenshot("mxn__widget0", std::nullopt, mitk::ScreenshotFormat::Png),
      std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->GetMxNCamera("mxn__widget0"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->SetMxNCamera("mxn__widget0", mitk::CameraPatch{}), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->GetMxNSelectedSlice("mxn__widget0"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->SetMxNSelectedSliceStep("mxn__widget0", 0u), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->GetMxNSelectedPosition("mxn__widget0"), std::runtime_error);
    {
      mitk::Point3D p; p.Fill(0.0);
      CPPUNIT_ASSERT_THROW(m_Bridge->SetMxNSelectedPosition("mxn__widget0", p), std::runtime_error);
    }
    CPPUNIT_ASSERT_THROW(m_Bridge->GetMxNLayout(), std::runtime_error);
    CPPUNIT_ASSERT_THROW(m_Bridge->SetMxNLayout("{}"), std::runtime_error);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRenderWindowBridge)
