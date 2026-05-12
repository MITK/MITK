/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkRenderingController.h>
#include <mitkDataStorageBridge.h>
#include <mitkRenderWindowBridge.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkImage.h>
#include <mitkPixelType.h>

#include <nlohmann/json.hpp>

class mitkRenderingControllerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRenderingControllerTestSuite);

  // POST /rendering/update tests
  MITK_TEST(UpdateReturns204);
  MITK_TEST(UpdateWith2dTypeReturns204);
  MITK_TEST(UpdateWith3dTypeReturns204);
  MITK_TEST(UpdateWithInvalidTypeReturns400);
  MITK_TEST(UpdateWithInvalidJsonReturns400);

  // POST /rendering/reinit tests
  MITK_TEST(ReinitWithoutDataStorageReturns503);
  MITK_TEST(ReinitWithDataStorageReturns204);

  // POST /rendering/reinit with uids body (node-scoped reinit) tests
  MITK_TEST(NodeReinitWithoutDataStorageReturns503);
  MITK_TEST(NodeReinitUnknownUidReturns404);
  MITK_TEST(NodeReinitNodeWithNoDataReturns422);
  MITK_TEST(NodeReinitValidNodeReturns204);
  MITK_TEST(ReinitWithInvalidJsonBodyReturns400);
  MITK_TEST(MultiNodeReinitWithTwoUidsReturns204);
  MITK_TEST(MultiNodeReinitEmptyUidsArrayReturns400);
  MITK_TEST(MultiNodeReinitSecondUidUnknownReturns404);
  MITK_TEST(MultiNodeReinitSecondNodeNoDataReturns422);

  // GET /rendering/selected-position tests
  MITK_TEST(GetSelectedPositionWithoutBridgeReturns503);
  MITK_TEST(GetSelectedPositionWithGetterReturns200);
  MITK_TEST(GetSelectedPositionEditorNotOpenReturns503EditorNotActive);
  MITK_TEST(PutSelectedPositionWithoutBridgeReturns503);
  MITK_TEST(PutSelectedPositionWithSetterReturns204);
  MITK_TEST(PutSelectedPositionEditorNotOpenReturns503EditorNotActive);
  MITK_TEST(PutSelectedPositionMissingPositionFieldReturns400);
  MITK_TEST(PutSelectedPositionWrongArrayLengthReturns400);
  MITK_TEST(PutSelectedPositionInvalidJsonReturns400);

  // GET/PUT /rendering/selected-time tests
  MITK_TEST(GetSelectedTimeReturns200WithTimestepAndBoundsFields);
  MITK_TEST(PutSelectedTimeWithTimepointMsReturns204);
  MITK_TEST(PutSelectedTimeWithTimestepReturns204);
  MITK_TEST(PutSelectedTimeWithBothFieldsReturns400);
  MITK_TEST(PutSelectedTimeWithNeitherFieldReturns400);
  MITK_TEST(PutSelectedTimeWithInvalidTypeReturns400);

  // GET /rendering/screenshot tests
  MITK_TEST(GetScreenshotWithoutProviderReturns503);
  MITK_TEST(GetScreenshotWithInvalidFormatReturns400);
  MITK_TEST(GetScreenshotWithNonPositiveWidthReturns400);
  MITK_TEST(GetScreenshotWithExcessiveDimensionsReturns400);

  // Editor discovery tests
  MITK_TEST(GetEditorsWithoutProviderReturns503);
  MITK_TEST(GetEditorsReturns200WithAliases);
  MITK_TEST(GetEditorsReportsMxnActiveWhenEditorOpen);
  MITK_TEST(GetStdmultiInfoWithEditorActiveReturns200);
  MITK_TEST(GetStdmultiInfoWhenEditorInactiveReturns503EditorNotActive);
  MITK_TEST(GetStdmultiWindowsWithoutProviderReturns503);
  MITK_TEST(GetStdmultiWindowsEditorNotOpenReturns503EditorNotActive);
  MITK_TEST(GetStdmultiWindowsReturns200);
  MITK_TEST(GetStdmultiWindowForUnknownNameReturns404);
  MITK_TEST(GetStdmultiWindowForAxialReturns200With2d);
  MITK_TEST(GetStdmultiWindowFor3dReturns200NoSelectedSlice);

  // MxN editor discovery
  MITK_TEST(GetMxnInfoWithoutProviderReturns503);
  MITK_TEST(GetMxnInfoWhenEditorInactiveReturns503EditorNotActive);
  MITK_TEST(GetMxnInfoWithEditorActiveReturns200WithWindows);
  MITK_TEST(GetMxnWindowsWithoutProviderReturns503);
  MITK_TEST(GetMxnWindowsEditorNotOpenReturns503EditorNotActive);
  MITK_TEST(GetMxnWindowsReturns200WithViewDirectionAndLinks);
  MITK_TEST(GetMxnWindowForMalformedNameReturns400);
  MITK_TEST(GetMxnWindowForUnknownNameReturns404);
  MITK_TEST(GetMxnWindowForKnownCellReturns200WithFullSummary);

  // MxN layout
  MITK_TEST(GetMxnLayoutWithoutGetterReturns503);
  MITK_TEST(GetMxnLayoutEditorNotOpenReturns503EditorNotActive);
  MITK_TEST(GetMxnLayoutReturns200WithDocument);
  MITK_TEST(PutMxnLayoutEmptyBodyReturns400);
  MITK_TEST(PutMxnLayoutInvalidJsonReturns400);
  MITK_TEST(PutMxnLayoutSchemaViolationReturns400);
  MITK_TEST(PutMxnLayoutWithoutSetterReturns503);
  MITK_TEST(PutMxnLayoutEditorNotOpenReturns503EditorNotActive);
  MITK_TEST(PutMxnLayoutReturns200WithEchoedBody);

  // MxN camera
  MITK_TEST(GetMxnCameraMalformedNameReturns400);
  MITK_TEST(GetMxnCameraWithoutGetterReturns503);
  MITK_TEST(GetMxnCameraUnknownCellReturns404);
  MITK_TEST(GetMxnCameraReturns200With2dFields);
  MITK_TEST(PutMxnCameraEmptyBodyReturns400);
  MITK_TEST(PutMxnCameraRejects3dOnlyFieldUnderV2);
  MITK_TEST(PutMxnCameraAcceptsParallelScale);
  MITK_TEST(PutMxnCameraEditorNotOpenReturns503EditorNotActive);

  // MxN selected-slice
  MITK_TEST(GetMxnSelectedSliceWithoutGetterReturns503);
  MITK_TEST(GetMxnSelectedSliceReturns200WithStepAndBounds);
  MITK_TEST(PutMxnSelectedSliceWithStepReturns204);
  MITK_TEST(PutMxnSelectedSliceWithPositionReturns400WithHint);
  MITK_TEST(PutMxnSelectedSliceWithUnknownFieldReturns400);
  MITK_TEST(PutMxnSelectedSliceMissingStepReturns400);

  // MxN per-cell selected-position
  MITK_TEST(GetMxnSelectedPositionWithoutGetterReturns503);
  MITK_TEST(GetMxnSelectedPositionReturns200WithPositionAndBounds);
  MITK_TEST(GetMxnSelectedPositionNoGeometryReturnsNullBounds);
  MITK_TEST(PutMxnSelectedPositionForwardsToCell);
  MITK_TEST(PutMxnSelectedPositionMissingFieldReturns400);
  MITK_TEST(PutMxnSelectedPositionWrongShapeReturns400);
  MITK_TEST(PerCellPositionDoesNotInvolveGlobalGetter);

  // MxN screenshots
  MITK_TEST(GetMxnEditorScreenshotWithoutProviderReturns503);
  MITK_TEST(GetMxnEditorScreenshotEditorNotOpenReturns503EditorNotActive);
  MITK_TEST(GetMxnEditorScreenshotReturns200Png);
  MITK_TEST(GetMxnEditorScreenshotWithJpegAndSizeReturns200);
  MITK_TEST(GetMxnEditorScreenshotInvalidFormatReturns400);
  MITK_TEST(GetMxnEditorScreenshotExcessiveDimensionsReturns400);
  MITK_TEST(GetMxnWindowScreenshotMalformedNameReturns400);
  MITK_TEST(GetMxnWindowScreenshotUnknownNameReturns404);
  MITK_TEST(GetMxnWindowScreenshotReturns200Png);
  MITK_TEST(GetMxnWindowScreenshotForwardsName);

  // Camera tests
  MITK_TEST(GetCameraUnknownWindowReturns404);
  MITK_TEST(GetCameraWithoutGetterReturns503);
  MITK_TEST(GetCameraForAxialReturns200With2dFields);
  MITK_TEST(GetCameraFor3dReturns200With3dFields);
  MITK_TEST(GetCameraEditorNotOpenReturns503EditorNotActive);
  MITK_TEST(PutCameraUnknownWindowReturns404);
  MITK_TEST(PutCameraInvalidJsonReturns400);
  MITK_TEST(PutCameraEmptyBodyReturns400);
  MITK_TEST(PutCameraUnknownFieldReturns400);
  MITK_TEST(PutCameraParallelScaleOn3dReturns400);
  MITK_TEST(PutCameraPerspectiveAngleOn2dReturns400);
  MITK_TEST(PutCameraNonPositiveParallelScaleReturns400);
  MITK_TEST(PutCameraPerspectiveAngleOutOfRangeReturns400);
  MITK_TEST(PutCameraUnknownStandardViewReturns400);
  MITK_TEST(PutCameraWrongArrayLengthReturns400);
  MITK_TEST(PutCameraWithoutSetterReturns503);
  MITK_TEST(PutCameraOnAxialReturns204);
  MITK_TEST(PutCameraOn3dReturns204);
  MITK_TEST(PutCameraStandardViewAppliedFirst);
  MITK_TEST(PutCameraStandardViewWithPositionReturns400);
  MITK_TEST(PutCameraStandardViewWithFocalPointReturns400);
  MITK_TEST(PutCameraStandardViewWithViewUpReturns400);
  MITK_TEST(PutCameraSetterThrowsMitkExceptionReturns422);

  // Selected-slice tests
  MITK_TEST(GetSliceUnknownWindowReturns404);
  MITK_TEST(GetSliceOn3dReturns404UnsupportedOperation);
  MITK_TEST(GetSliceWithoutGetterReturns503);
  MITK_TEST(GetSliceForAxialReturns200);
  MITK_TEST(GetSliceNoGeometryBoundsNull);
  MITK_TEST(PutSliceUnknownWindowReturns404);
  MITK_TEST(PutSliceOn3dReturns404UnsupportedOperation);
  MITK_TEST(PutSliceInvalidJsonReturns400);
  MITK_TEST(PutSliceEmptyBodyReturns400);
  MITK_TEST(PutSliceMissingStepReturns400);
  MITK_TEST(PutSliceNegativeStepReturns400);
  MITK_TEST(PutSliceNonIntegerStepReturns400);
  MITK_TEST(PutSlicePositionFieldReturns400WithHint);
  MITK_TEST(PutSliceUnknownFieldReturns400);
  MITK_TEST(PutSliceWithoutSetterReturns503);
  MITK_TEST(PutSliceOnAxialReturns204);
  MITK_TEST(PutSliceSetterThrowsMitkExceptionReturns422);

  // Window/editor screenshot tests
  MITK_TEST(GetEditorScreenshotWithoutProviderReturns503);
  MITK_TEST(GetEditorScreenshotEditorNotOpenReturns503EditorNotActive);
  MITK_TEST(GetEditorScreenshotReturns200Png);
  MITK_TEST(GetEditorScreenshotWithJpegAndSizeReturns200);
  MITK_TEST(GetEditorScreenshotInvalidFormatReturns400);
  MITK_TEST(GetEditorScreenshotExcessiveDimensionsReturns400);
  MITK_TEST(GetWindowScreenshotUnknownWindowReturns404);
  MITK_TEST(GetWindowScreenshotWithoutProviderReturns503);
  MITK_TEST(GetWindowScreenshotReturns200Png);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  std::unique_ptr<mitk::DataStorageBridge> m_Bridge;
  std::unique_ptr<mitk::RenderWindowBridge> m_RenderWindowBridge;
  std::unique_ptr<mitk::RenderingController> m_Controller;

  httplib::Request MakeRequest(const std::string& path = "",
                               const std::string& body = "",
                               const std::unordered_map<std::string, std::string>& pathParams = {})
  {
    httplib::Request req;
    req.path = path;
    req.body = body;
    req.path_params = pathParams;
    return req;
  }

  // Create a 10x10x10 unsigned-char image with a valid geometry.
  mitk::Image::Pointer MakeImage()
  {
    auto image = mitk::Image::New();
    unsigned int dims[3] = {10, 10, 10};
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dims);
    return image;
  }

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Bridge = std::make_unique<mitk::DataStorageBridge>();
    m_Bridge->SetDataStorage(m_DataStorage);
    m_RenderWindowBridge = std::make_unique<mitk::RenderWindowBridge>();
    m_Controller = std::make_unique<mitk::RenderingController>(*m_Bridge);
    m_Controller->SetRenderWindowBridge(m_RenderWindowBridge.get());
    // No dispatcher: headless mode -- tasks execute directly on the calling thread.
  }

  void tearDown() override
  {
    m_Controller.reset();
    m_RenderWindowBridge.reset();
    m_Bridge->SetDataStorage(nullptr);
    m_Bridge.reset();
    m_DataStorage = nullptr;
  }

  // ===== POST /rendering/update =====

  void UpdateReturns204()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/update");
    httplib::Response res;

    m_Controller->HandlePOST_update(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
  }

  void UpdateWith2dTypeReturns204()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/update", R"({"type":"2d"})");
    httplib::Response res;

    m_Controller->HandlePOST_update(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
  }

  void UpdateWith3dTypeReturns204()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/update", R"({"type":"3d"})");
    httplib::Response res;

    m_Controller->HandlePOST_update(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
  }

  void UpdateWithInvalidTypeReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/update", R"({"type":"invalid"})");
    httplib::Response res;

    m_Controller->HandlePOST_update(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void UpdateWithInvalidJsonReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/update", "not-json");
    httplib::Response res;

    m_Controller->HandlePOST_update(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  // ===== POST /rendering/reinit =====

  void ReinitWithoutDataStorageReturns503()
  {
    m_Bridge->SetDataStorage(nullptr);

    const auto req = this->MakeRequest("/api/v1/rendering/reinit");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("DATASTORAGE_NOT_AVAILABLE"), json["error"]["code"].get<std::string>());
  }

  void ReinitWithDataStorageReturns204()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/reinit");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
  }

  // ===== POST /rendering/reinit with uids body (node-scoped reinit) =====

  void NodeReinitWithoutDataStorageReturns503()
  {
    m_Bridge->SetDataStorage(nullptr);

    const auto req = this->MakeRequest("/api/v1/rendering/reinit", R"({"uids":["some-uid"]})");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("DATASTORAGE_NOT_AVAILABLE"), json["error"]["code"].get<std::string>());
  }

  void NodeReinitUnknownUidReturns404()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/reinit", R"({"uids":["unknown-uid"]})");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), json["error"]["code"].get<std::string>());
  }

  void NodeReinitNodeWithNoDataReturns422()
  {
    // Add a node without data
    auto node = mitk::DataNode::New();
    node->SetName("EmptyNode");
    m_DataStorage->Add(node);

    const auto uid = m_Bridge->GetNodeUid(node.GetPointer());

    const auto req = this->MakeRequest("/api/v1/rendering/reinit", R"({"uids":[")" + uid + R"("]})");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(422, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NO_DATA"), json["error"]["code"].get<std::string>());
  }

  void NodeReinitValidNodeReturns204()
  {
    // Add a node with an initialized image (valid geometry)
    auto node = mitk::DataNode::New();
    node->SetName("ImageNode");
    node->SetData(this->MakeImage());
    m_DataStorage->Add(node);

    const auto uid = m_Bridge->GetNodeUid(node.GetPointer());

    const auto req = this->MakeRequest("/api/v1/rendering/reinit", R"({"uids":[")" + uid + R"("]})");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
  }

  void ReinitWithInvalidJsonBodyReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/reinit", "not-json");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void MultiNodeReinitWithTwoUidsReturns204()
  {
    auto node1 = mitk::DataNode::New();
    node1->SetName("Node1");
    node1->SetData(this->MakeImage());
    m_DataStorage->Add(node1);

    auto node2 = mitk::DataNode::New();
    node2->SetName("Node2");
    node2->SetData(this->MakeImage());
    m_DataStorage->Add(node2);

    const auto uid1 = m_Bridge->GetNodeUid(node1.GetPointer());
    const auto uid2 = m_Bridge->GetNodeUid(node2.GetPointer());

    const auto req = this->MakeRequest("/api/v1/rendering/reinit",
      R"({"uids":[")" + uid1 + R"(",")" + uid2 + R"("]})");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
  }

  void MultiNodeReinitEmptyUidsArrayReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/reinit", R"({"uids":[]})");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void MultiNodeReinitSecondUidUnknownReturns404()
  {
    auto node = mitk::DataNode::New();
    node->SetName("ValidNode");
    node->SetData(this->MakeImage());
    m_DataStorage->Add(node);

    const auto uid = m_Bridge->GetNodeUid(node.GetPointer());

    const auto req = this->MakeRequest("/api/v1/rendering/reinit",
      R"({"uids":[")" + uid + R"(","unknown-uid"]})");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), json["error"]["code"].get<std::string>());
  }

  void MultiNodeReinitSecondNodeNoDataReturns422()
  {
    auto node1 = mitk::DataNode::New();
    node1->SetName("ValidNode");
    node1->SetData(this->MakeImage());
    m_DataStorage->Add(node1);

    auto node2 = mitk::DataNode::New();
    node2->SetName("EmptyNode");
    m_DataStorage->Add(node2);

    const auto uid1 = m_Bridge->GetNodeUid(node1.GetPointer());
    const auto uid2 = m_Bridge->GetNodeUid(node2.GetPointer());

    const auto req = this->MakeRequest("/api/v1/rendering/reinit",
      R"({"uids":[")" + uid1 + R"(",")" + uid2 + R"("]})");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(422, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NO_DATA"), json["error"]["code"].get<std::string>());
  }
  // ===== GET /rendering/selected-position =====

  void GetSelectedPositionWithoutBridgeReturns503()
  {
    m_Controller->SetRenderWindowBridge(nullptr);

    const auto req = this->MakeRequest("/api/v1/rendering/selected-position");
    httplib::Response res;

    m_Controller->HandleGET_selectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetSelectedPositionWithGetterReturns200()
  {
    mitk::Point3D expectedPos;
    expectedPos[0] = 1.0; expectedPos[1] = 2.0; expectedPos[2] = 3.0;
    m_RenderWindowBridge->SetPositionGetter(
      [expectedPos]() -> mitk::SelectedPositionInfo
      {
        mitk::SelectedPositionInfo info;
        info.position = expectedPos;
        // bounds defaults to std::nullopt; no geometry available in headless test.
        return info;
      });

    const auto req = this->MakeRequest("/api/v1/rendering/selected-position");
    httplib::Response res;

    m_Controller->HandleGET_selectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json.contains("position"));
    CPPUNIT_ASSERT(json["position"].is_array());
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), json["position"].size());
    CPPUNIT_ASSERT(json.contains("bounds"));
    CPPUNIT_ASSERT(json["bounds"].contains("min_position"));
    CPPUNIT_ASSERT(json["bounds"].contains("max_position"));
  }

  void GetSelectedPositionEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetPositionGetter(
      []() -> mitk::SelectedPositionInfo
      {
        throw mitk::RenderWindowBridgeNoEditorException(
          "StdMultiWidgetEditor is not open -- cannot read crosshair position");
      });

    const auto req = this->MakeRequest("/api/v1/rendering/selected-position");
    httplib::Response res;

    m_Controller->HandleGET_selectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void PutSelectedPositionWithoutBridgeReturns503()
  {
    m_Controller->SetRenderWindowBridge(nullptr);

    const auto req = this->MakeRequest("/api/v1/rendering/selected-position",
                                       R"({"position": [1.0, 2.0, 3.0]})");
    httplib::Response res;

    m_Controller->HandlePUT_selectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void PutSelectedPositionWithSetterReturns204()
  {
    mitk::Point3D capturedPos;
    capturedPos.Fill(0.0);
    m_RenderWindowBridge->SetPositionSetter(
      [&capturedPos](const mitk::Point3D& pos) { capturedPos = pos; });

    const auto req = this->MakeRequest("/api/v1/rendering/selected-position",
                                       R"({"position": [1.0, 2.0, 3.0]})");
    httplib::Response res;

    m_Controller->HandlePUT_selectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, capturedPos[0], 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, capturedPos[1], 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, capturedPos[2], 1e-6);
  }

  void PutSelectedPositionEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetPositionSetter(
      [](const mitk::Point3D&)
      {
        throw mitk::RenderWindowBridgeNoEditorException(
          "StdMultiWidgetEditor is not open -- cannot set crosshair position");
      });

    const auto req = this->MakeRequest("/api/v1/rendering/selected-position",
                                       R"({"position": [1.0, 2.0, 3.0]})");
    httplib::Response res;

    m_Controller->HandlePUT_selectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void PutSelectedPositionMissingPositionFieldReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/selected-position", "{}");
    httplib::Response res;

    m_Controller->HandlePUT_selectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void PutSelectedPositionWrongArrayLengthReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/selected-position",
                                       R"({"position": [1.0, 2.0]})");
    httplib::Response res;

    m_Controller->HandlePUT_selectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void PutSelectedPositionInvalidJsonReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/selected-position", "not-json");
    httplib::Response res;

    m_Controller->HandlePUT_selectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  // ===== GET/PUT /rendering/selected-time =====

  void GetSelectedTimeReturns200WithTimestepAndBoundsFields()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/selected-time");
    httplib::Response res;

    m_Controller->HandleGET_selectedTime(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json.contains("timepoint_ms"));
    CPPUNIT_ASSERT(json.contains("timestep"));
    CPPUNIT_ASSERT(json.contains("bounds"));
    CPPUNIT_ASSERT(json["bounds"].contains("min_timepoint_ms"));
    CPPUNIT_ASSERT(json["bounds"].contains("max_timepoint_ms"));
    CPPUNIT_ASSERT(json["bounds"].contains("steps"));
  }

  void PutSelectedTimeWithTimepointMsReturns204()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/selected-time",
                                       R"({"timepoint_ms": 0.0})");
    httplib::Response res;

    m_Controller->HandlePUT_selectedTime(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
  }

  void PutSelectedTimeWithTimestepReturns204()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/selected-time",
                                       R"({"timestep": 0})");
    httplib::Response res;

    m_Controller->HandlePUT_selectedTime(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
  }

  void PutSelectedTimeWithBothFieldsReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/selected-time",
                                       R"({"timepoint_ms": 0.0, "timestep": 0})");
    httplib::Response res;

    m_Controller->HandlePUT_selectedTime(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void PutSelectedTimeWithNeitherFieldReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/selected-time", "{}");
    httplib::Response res;

    m_Controller->HandlePUT_selectedTime(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void PutSelectedTimeWithInvalidTypeReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/selected-time",
                                       R"({"timestep": "zero"})");
    httplib::Response res;

    m_Controller->HandlePUT_selectedTime(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  // ===== GET /rendering/screenshot =====

  void GetScreenshotWithoutProviderReturns503()
  {
    // No provider is set in setUp -- bridge has no provider.
    const auto req = this->MakeRequest("/api/v1/rendering/screenshot");
    httplib::Response res;

    m_Controller->HandleGET_screenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetScreenshotWithInvalidFormatReturns400()
  {
    m_RenderWindowBridge->SetScreenshotProvider(
      [](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });

    httplib::Request req;
    req.path = "/api/v1/rendering/screenshot";
    req.params.emplace("format", "bmp");
    httplib::Response res;

    m_Controller->HandleGET_screenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void GetScreenshotWithNonPositiveWidthReturns400()
  {
    m_RenderWindowBridge->SetScreenshotProvider(
      [](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });

    httplib::Request req;
    req.path = "/api/v1/rendering/screenshot";
    req.params.emplace("width", "0");
    req.params.emplace("height", "100");
    httplib::Response res;

    m_Controller->HandleGET_screenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void GetScreenshotWithExcessiveDimensionsReturns400()
  {
    m_RenderWindowBridge->SetScreenshotProvider(
      [](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) {
        return std::vector<unsigned char>{};
      });

    httplib::Request req;
    req.path = "/api/v1/rendering/screenshot";
    req.params.emplace("width", "10000");
    req.params.emplace("height", "10000");
    httplib::Response res;

    m_Controller->HandleGET_screenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  // ===== Editor discovery =====

  static std::vector<mitk::EditorInfo> FakeEditors(bool stdmultiActive,
                                                   bool mxnActive = false)
  {
    mitk::EditorInfo stdmulti;
    stdmulti.alias = "stdmulti";
    stdmulti.pluginId = "org.mitk.editors.stdmultiwidget";
    stdmulti.active = stdmultiActive;
    if (stdmultiActive)
      stdmulti.windowIds = {"axial", "sagittal", "coronal", "3d"};

    mitk::EditorInfo mxn;
    mxn.alias = "mxn";
    mxn.pluginId = "org.mitk.editors.mxnmultiwidget";
    mxn.active = mxnActive;
    if (mxnActive)
      mxn.windowIds = {"mxn__widget0", "mxn__widget1"};

    return {stdmulti, mxn};
  }

  void GetEditorsWithoutProviderReturns503()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/editors");
    httplib::Response res;
    m_Controller->HandleGET_editors(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetEditorsReturns200WithAliases()
  {
    m_RenderWindowBridge->SetEditorListProvider(
      []() { return FakeEditors(/*stdmultiActive=*/true); });

    const auto req = this->MakeRequest("/api/v1/rendering/editors");
    httplib::Response res;
    m_Controller->HandleGET_editors(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json.is_array());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), json.size());
    CPPUNIT_ASSERT_EQUAL(std::string("stdmulti"), json[0]["alias"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("mxn"), json[1]["alias"].get<std::string>());
    CPPUNIT_ASSERT(json[0]["active"].get<bool>());
    CPPUNIT_ASSERT(!json[1]["active"].get<bool>());
    // E1 never reports the windows list (that is an E2-only field).
    CPPUNIT_ASSERT(!json[0].contains("windows"));
  }

  void GetEditorsReportsMxnActiveWhenEditorOpen()
  {
    // The MxN entry reports `active: true` when the editor is open.
    m_RenderWindowBridge->SetEditorListProvider(
      []() { return FakeEditors(/*stdmultiActive=*/false, /*mxnActive=*/true); });

    const auto req = this->MakeRequest("/api/v1/rendering/editors");
    httplib::Response res;
    m_Controller->HandleGET_editors(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), json.size());
    CPPUNIT_ASSERT_EQUAL(std::string("mxn"), json[1]["alias"].get<std::string>());
    CPPUNIT_ASSERT(json[1]["active"].get<bool>());
    CPPUNIT_ASSERT(!json[0]["active"].get<bool>());
  }

  void GetStdmultiInfoWithEditorActiveReturns200()
  {
    m_RenderWindowBridge->SetEditorListProvider(
      []() { return FakeEditors(/*stdmultiActive=*/true); });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/stdmulti");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiInfo(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("stdmulti"), json["alias"].get<std::string>());
    CPPUNIT_ASSERT(json["active"].get<bool>());
    CPPUNIT_ASSERT(json["windows"].is_array());
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), json["windows"].size());
  }

  void GetStdmultiInfoWhenEditorInactiveReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetEditorListProvider(
      []() { return FakeEditors(/*stdmultiActive=*/false); });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/stdmulti");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiInfo(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetStdmultiWindowsWithoutProviderReturns503()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/editors/stdmulti/windows");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindows(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetStdmultiWindowsEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetStdMultiWindowListProvider(
      []() -> std::vector<mitk::WindowInfo>
      {
        throw mitk::RenderWindowBridgeNoEditorException(
          "StdMultiWidgetEditor is not open");
      });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/stdmulti/windows");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindows(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetStdmultiWindowsReturns200()
  {
    m_RenderWindowBridge->SetStdMultiWindowListProvider(
      []() {
        return std::vector<mitk::WindowInfo>{
          {"axial",    mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Axial},
          {"sagittal", mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Sagittal},
          {"coronal",  mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Coronal},
          {"3d",       mitk::WindowKind::ThreeD, std::nullopt}
        };
      });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/stdmulti/windows");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindows(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json.is_array());
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), json.size());
    CPPUNIT_ASSERT_EQUAL(std::string("axial"), json[0]["id"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("2d"), json[0]["kind"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("axial"), json[0]["view_direction"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), json[3]["id"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), json[3]["kind"].get<std::string>());
    // 3D window has no anatomical plane.
    CPPUNIT_ASSERT(!json[3].contains("view_direction"));
    // No `plane` field on window list items -- orientation is read from the camera.
    CPPUNIT_ASSERT(!json[0].contains("plane"));
  }

  void GetStdmultiWindowForUnknownNameReturns404()
  {
    // No providers set -- unknown name must still be rejected controller-side.
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/bogus", "",
      {{"id", "bogus"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindow(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_FOUND"),
                         json["error"]["code"].get<std::string>());
  }

  void GetStdmultiWindowForAxialReturns200With2d()
  {
    m_RenderWindowBridge->SetStdMultiWindowListProvider(
      []() {
        return std::vector<mitk::WindowInfo>{
          {"axial",    mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Axial},
          {"sagittal", mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Sagittal},
          {"coronal",  mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Coronal},
          {"3d",       mitk::WindowKind::ThreeD, std::nullopt}
        };
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial", "",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindow(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("axial"), json["id"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("2d"), json["kind"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("axial"), json["view_direction"].get<std::string>());
    CPPUNIT_ASSERT(json["has_camera"].get<bool>());
    CPPUNIT_ASSERT(json["has_selected_slice"].get<bool>());
    CPPUNIT_ASSERT(!json.contains("plane"));
  }

  void GetStdmultiWindowFor3dReturns200NoSelectedSlice()
  {
    m_RenderWindowBridge->SetStdMultiWindowListProvider(
      []() {
        return std::vector<mitk::WindowInfo>{
          {"axial",    mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Axial},
          {"sagittal", mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Sagittal},
          {"coronal",  mitk::WindowKind::TwoD,   mitk::AnatomicalPlane::Coronal},
          {"3d",       mitk::WindowKind::ThreeD, std::nullopt}
        };
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/3d", "",
      {{"id", "3d"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindow(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), json["id"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), json["kind"].get<std::string>());
    CPPUNIT_ASSERT(!json.contains("view_direction"));
    CPPUNIT_ASSERT(json["has_camera"].get<bool>());
    CPPUNIT_ASSERT(!json["has_selected_slice"].get<bool>());
  }
  // ===== MxN editor discovery =====

  /** Two-cell MxN window list for tests: mxn__widget0 axial/main (with display
   *  name), mxn__widget1 sagittal/row2 (no display name).
   *  Field order: id, displayName, kind, viewDirection, selectionGroup. */
  static std::vector<mitk::MxNWindowInfo> FakeMxNWindows()
  {
    return {
      {"mxn__widget0", std::string("Tumor axial"), mitk::WindowKind::TwoD, mitk::AnatomicalPlane::Axial,    "main"},
      {"mxn__widget1", std::nullopt,               mitk::WindowKind::TwoD, mitk::AnatomicalPlane::Sagittal, "row2"}
    };
  }

  void GetMxnInfoWithoutProviderReturns503()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn");
    httplib::Response res;
    m_Controller->HandleGET_mxnInfo(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnInfoWhenEditorInactiveReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetEditorListProvider(
      []() { return FakeEditors(/*stdmultiActive=*/true, /*mxnActive=*/false); });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn");
    httplib::Response res;
    m_Controller->HandleGET_mxnInfo(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnInfoWithEditorActiveReturns200WithWindows()
  {
    m_RenderWindowBridge->SetEditorListProvider(
      []() { return FakeEditors(/*stdmultiActive=*/false, /*mxnActive=*/true); });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn");
    httplib::Response res;
    m_Controller->HandleGET_mxnInfo(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("mxn"), json["alias"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("org.mitk.editors.mxnmultiwidget"),
                         json["plugin_id"].get<std::string>());
    CPPUNIT_ASSERT(json["active"].get<bool>());
    CPPUNIT_ASSERT(json["windows"].is_array());
    // FakeEditors mxnActive populates two canonical qualified ids.
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), json["windows"].size());
  }

  void GetMxnWindowsWithoutProviderReturns503()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/windows");
    httplib::Response res;
    m_Controller->HandleGET_mxnWindows(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnWindowsEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetMxNWindowListProvider(
      []() -> std::vector<mitk::MxNWindowInfo> {
        throw mitk::RenderWindowBridgeNoEditorException("mxn editor not open");
      });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/windows");
    httplib::Response res;
    m_Controller->HandleGET_mxnWindows(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnWindowsReturns200WithViewDirectionAndLinks()
  {
    m_RenderWindowBridge->SetMxNWindowListProvider([]() { return FakeMxNWindows(); });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/windows");
    httplib::Response res;
    m_Controller->HandleGET_mxnWindows(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json.is_array());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), json.size());

    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget0"), json[0]["id"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("Tumor axial"),
                         json[0]["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("2d"),      json[0]["kind"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("axial"),   json[0]["view_direction"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("main"),
                         json[0]["links"]["selection"].get<std::string>());

    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget1"),  json[1]["id"].get<std::string>());
    CPPUNIT_ASSERT_MESSAGE("Cells without a display label must NOT emit a 'name' key",
                           !json[1].contains("name"));
    CPPUNIT_ASSERT_EQUAL(std::string("sagittal"), json[1]["view_direction"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("row2"),
                         json[1]["links"]["selection"].get<std::string>());
  }

  void GetMxnWindowForMalformedNameReturns400()
  {
    // Names that don't match the OAS MxNWindowName pattern are rejected at
    // the controller boundary with 400 INVALID_REQUEST, before any bridge
    // round-trip.
    m_RenderWindowBridge->SetMxNWindowListProvider([]() { return FakeMxNWindows(); });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/foo%20bar", "",
      {{"id", "foo bar"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnWindow(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnWindowForUnknownNameReturns404()
  {
    // Well-formed id (matches the OAS pattern) but not present in the layout
    // -- must fall through to the bridge and surface as 404.
    m_RenderWindowBridge->SetMxNWindowListProvider([]() { return FakeMxNWindows(); });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__bogus", "",
      {{"id", "mxn__bogus"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnWindow(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_FOUND"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnWindowForKnownCellReturns200WithFullSummary()
  {
    m_RenderWindowBridge->SetMxNWindowListProvider([]() { return FakeMxNWindows(); });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0", "",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnWindow(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget0"), json["id"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("Tumor axial"),
                         json["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("2d"),      json["kind"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("axial"),   json["view_direction"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("main"),
                         json["links"]["selection"].get<std::string>());
    CPPUNIT_ASSERT(json["has_camera"].get<bool>());
    CPPUNIT_ASSERT(json["has_selected_slice"].get<bool>());     // 2D cell -> has slice
    CPPUNIT_ASSERT(json["has_selected_position"].get<bool>());
  }

  // ===== MxN layout =====

  static constexpr const char* kFakeMxNLayout = R"({
    "version": "2.0",
    "name": "Fake",
    "groups": { "main": { "select_all": true } },
    "root": {
      "type": "split", "orientation": "horizontal",
      "children": [
        { "type": "window", "id": "mxn__widget0", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
        { "type": "window", "id": "mxn__widget1", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
      ]
    }
  })";

  void GetMxnLayoutWithoutGetterReturns503()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/layout");
    httplib::Response res;
    m_Controller->HandleGET_mxnLayout(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnLayoutEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetMxNLayoutGetter(
      []() -> std::string {
        throw mitk::RenderWindowBridgeNoEditorException("mxn editor not open");
      });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/layout");
    httplib::Response res;
    m_Controller->HandleGET_mxnLayout(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnLayoutReturns200WithDocument()
  {
    m_RenderWindowBridge->SetMxNLayoutGetter(
      []() { return std::string(kFakeMxNLayout); });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/layout");
    httplib::Response res;
    m_Controller->HandleGET_mxnLayout(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    // The body is the bridge string verbatim -- application/json content type.
    CPPUNIT_ASSERT(res.body.find("\"version\"") != std::string::npos);
    const auto parsed = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("2.0"), parsed["version"].get<std::string>());
  }

  void PutMxnLayoutEmptyBodyReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/layout", "");
    httplib::Response res;
    m_Controller->HandlePUT_mxnLayout(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  void PutMxnLayoutInvalidJsonReturns400()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/layout", "not-json");
    httplib::Response res;
    m_Controller->HandlePUT_mxnLayout(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  void PutMxnLayoutSchemaViolationReturns400()
  {
    // The bridge setter throws mitk::Exception (engine's native shape) for
    // every schema/structural failure (version, missing field, duplicate
    // names, unknown view direction, ...). The controller maps it to 400
    // INVALID_REQUEST locally.
    m_RenderWindowBridge->SetMxNLayoutSetter(
      [](const std::string&) -> std::string {
        mitkThrow() << "duplicate window name 'mxn__widget0'";
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/layout", kFakeMxNLayout);
    httplib::Response res;
    m_Controller->HandlePUT_mxnLayout(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT(json["error"]["message"].get<std::string>().find("duplicate window name") != std::string::npos);
  }

  void PutMxnLayoutWithoutSetterReturns503()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/layout", kFakeMxNLayout);
    httplib::Response res;
    m_Controller->HandlePUT_mxnLayout(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void PutMxnLayoutEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetMxNLayoutSetter(
      [](const std::string&) -> std::string {
        throw mitk::RenderWindowBridgeNoEditorException("mxn editor not open");
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/layout", kFakeMxNLayout);
    httplib::Response res;
    m_Controller->HandlePUT_mxnLayout(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void PutMxnLayoutReturns200WithEchoedBody()
  {
    // The setter contract: receive a layout JSON, return the freshly serialized
    // result so callers can refresh cached cell names without an extra GET.
    std::string captured;
    m_RenderWindowBridge->SetMxNLayoutSetter(
      [&captured](const std::string& body) {
        captured = body;
        // Echo a minimal document with a marker so the test asserts it came
        // from the setter, not the request body.
        return std::string(R"({"version":"2.0","name":"echoed-by-setter"})");
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/layout", kFakeMxNLayout);
    httplib::Response res;
    m_Controller->HandlePUT_mxnLayout(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    CPPUNIT_ASSERT(captured.find("mxn__widget0") != std::string::npos);
    const auto parsed = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("echoed-by-setter"), parsed["name"].get<std::string>());
  }

  // ===== MxN camera =====

  void GetMxnCameraMalformedNameReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/foo%20bar/camera", "",
      {{"id", "foo bar"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnCameraWithoutGetterReturns503()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/camera", "",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnCameraUnknownCellReturns404()
  {
    m_RenderWindowBridge->SetMxNCameraGetter(
      [](const std::string& n) -> mitk::CameraState {
        throw mitk::RenderWindowBridgeUnknownWindowException(n);
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__bogus/camera", "",
      {{"id", "mxn__bogus"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_FOUND"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnCameraReturns200With2dFields()
  {
    m_RenderWindowBridge->SetMxNCameraGetter(
      [](const std::string&) { return MakeFakeCameraState(/*is3d=*/false); });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/camera", "",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json.contains("position"));
    CPPUNIT_ASSERT(json.contains("focal_point"));
    CPPUNIT_ASSERT(json.contains("view_up"));
    // 2D MxN cells under v2 always carry parallel_scale, never perspective_angle.
    CPPUNIT_ASSERT(json.contains("parallel_scale"));
    CPPUNIT_ASSERT(!json.contains("perspective_angle"));
  }

  void PutMxnCameraEmptyBodyReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/camera", "",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  void PutMxnCameraRejects3dOnlyFieldUnderV2()
  {
    // is3d=false hard-coded under v2; perspective_angle is rejected.
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/camera",
      R"({"perspective_angle": 30.0})",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  void PutMxnCameraAcceptsParallelScale()
  {
    std::string capturedName;
    std::optional<double> capturedScale;
    m_RenderWindowBridge->SetMxNCameraSetter(
      [&capturedName, &capturedScale](const std::string& n, const mitk::CameraPatch& p) {
        capturedName = n;
        capturedScale = p.parallelScale;
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/camera",
      R"({"parallel_scale": 120.5})",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget0"), capturedName);
    CPPUNIT_ASSERT(capturedScale.has_value());
    CPPUNIT_ASSERT_EQUAL(120.5, *capturedScale);
  }

  void PutMxnCameraEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetMxNCameraSetter(
      [](const std::string&, const mitk::CameraPatch&) {
        throw mitk::RenderWindowBridgeNoEditorException("mxn editor not open");
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/camera",
      R"({"parallel_scale": 100.0})",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  // ===== MxN selected-slice =====

  static mitk::SliceState MakeFakeSliceState()
  {
    mitk::SliceState s;
    s.step = 7;
    s.position[0] = 1.0; s.position[1] = 2.0; s.position[2] = 3.0;
    s.bounds.steps = 100;
    s.bounds.minPosition[0] = -10.0; s.bounds.minPosition[1] = -10.0; s.bounds.minPosition[2] = -10.0;
    s.bounds.maxPosition[0] =  10.0; s.bounds.maxPosition[1] =  10.0; s.bounds.maxPosition[2] =  10.0;
    s.bounds.hasPositions = true;
    return s;
  }

  void GetMxnSelectedSliceWithoutGetterReturns503()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-slice", "",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnSelectedSliceReturns200WithStepAndBounds()
  {
    m_RenderWindowBridge->SetMxNSelectedSliceGetter(
      [](const std::string&) { return MakeFakeSliceState(); });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-slice", "",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(7, json["step"].get<int>());
    CPPUNIT_ASSERT(json["position"].is_array());
    CPPUNIT_ASSERT_EQUAL(100, json["bounds"]["steps"].get<int>());
  }

  void PutMxnSelectedSliceWithStepReturns204()
  {
    std::string capturedName;
    unsigned int capturedStep = 0;
    m_RenderWindowBridge->SetMxNSelectedSliceStepSetter(
      [&capturedName, &capturedStep](const std::string& n, unsigned int s) {
        capturedName = n;
        capturedStep = s;
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-slice",
      R"({"step": 42})",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget0"), capturedName);
    CPPUNIT_ASSERT_EQUAL(42u, capturedStep);
  }

  void PutMxnSelectedSliceWithPositionReturns400WithHint()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-slice",
      R"({"position": [1.0, 2.0, 3.0]})",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
    // Hint must point at both the per-cell and global selected-position
    // resources so clients are routed to the right primitive without confusion.
    const auto msg = json["error"]["message"].get<std::string>();
    CPPUNIT_ASSERT(msg.find("selected-position") != std::string::npos);
  }

  void PutMxnSelectedSliceWithUnknownFieldReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-slice",
      R"({"step": 1, "bogus": 2})",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  void PutMxnSelectedSliceMissingStepReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-slice",
      R"({})",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  // ===== MxN per-cell selected-position =====

  void GetMxnSelectedPositionWithoutGetterReturns503()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-position", "",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnSelectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnSelectedPositionReturns200WithPositionAndBounds()
  {
    m_RenderWindowBridge->SetMxNSelectedPositionGetter(
      [](const std::string&) {
        mitk::SelectedPositionInfo info;
        info.position[0] = 1.5; info.position[1] = 2.5; info.position[2] = 3.5;
        mitk::WorldBounds b;
        b.minPosition[0] = -10.0; b.minPosition[1] = -20.0; b.minPosition[2] = -30.0;
        b.maxPosition[0] =  10.0; b.maxPosition[1] =  20.0; b.maxPosition[2] =  30.0;
        info.bounds = b;
        return info;
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-position", "",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnSelectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(1.5, json["position"][0].get<double>());
    CPPUNIT_ASSERT_EQUAL(3.5, json["position"][2].get<double>());
    CPPUNIT_ASSERT_EQUAL(-10.0, json["bounds"]["min_position"][0].get<double>());
    CPPUNIT_ASSERT_EQUAL( 30.0, json["bounds"]["max_position"][2].get<double>());
  }

  void GetMxnSelectedPositionNoGeometryReturnsNullBounds()
  {
    m_RenderWindowBridge->SetMxNSelectedPositionGetter(
      [](const std::string&) {
        mitk::SelectedPositionInfo info;
        info.position.Fill(0.0);
        // bounds intentionally not set
        return info;
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-position", "",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnSelectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["bounds"]["min_position"].is_null());
    CPPUNIT_ASSERT(json["bounds"]["max_position"].is_null());
  }

  void PutMxnSelectedPositionForwardsToCell()
  {
    std::string capturedName;
    mitk::Point3D capturedPos;
    capturedPos.Fill(0.0);
    m_RenderWindowBridge->SetMxNSelectedPositionSetter(
      [&capturedName, &capturedPos](const std::string& n, const mitk::Point3D& p) {
        capturedName = n;
        capturedPos = p;
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget7/selected-position",
      R"({"position": [4.0, 5.0, 6.0]})",
      {{"id", "mxn__widget7"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnSelectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget7"), capturedName);
    CPPUNIT_ASSERT_EQUAL(4.0, capturedPos[0]);
    CPPUNIT_ASSERT_EQUAL(5.0, capturedPos[1]);
    CPPUNIT_ASSERT_EQUAL(6.0, capturedPos[2]);
  }

  void PutMxnSelectedPositionMissingFieldReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-position",
      R"({})",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnSelectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void PutMxnSelectedPositionWrongShapeReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/selected-position",
      R"({"position": [1.0, 2.0]})",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnSelectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  /**
   * Cell-independence smoke test: a per-cell PUT must not invoke the global
   * crosshair setter. Whether the per-cell change visibly propagates to
   * other cells / the global anchor is workbench-UI state (observed but
   * not contracted by REST). This test pins the contract bit: REST does
   * not bake the propagation in itself.
   */
  void PerCellPositionDoesNotInvolveGlobalGetter()
  {
    bool globalSetterCalled = false;
    bool perCellSetterCalled = false;

    m_RenderWindowBridge->SetPositionSetter(
      [&globalSetterCalled](const mitk::Point3D&) { globalSetterCalled = true; });
    m_RenderWindowBridge->SetMxNSelectedPositionSetter(
      [&perCellSetterCalled](const std::string&, const mitk::Point3D&) {
        perCellSetterCalled = true;
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widgetA/selected-position",
      R"({"position": [1.0, 2.0, 3.0]})",
      {{"id", "mxn__widgetA"}});
    httplib::Response res;
    m_Controller->HandlePUT_mxnSelectedPosition(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
    CPPUNIT_ASSERT(perCellSetterCalled);
    CPPUNIT_ASSERT(!globalSetterCalled);
  }

  // ===== MxN screenshots =====

  void GetMxnEditorScreenshotWithoutProviderReturns503()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/screenshot");
    httplib::Response res;
    m_Controller->HandleGET_mxnScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnEditorScreenshotEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetMxNEditorScreenshotProvider(
      [](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) -> std::vector<unsigned char>
      {
        throw mitk::RenderWindowBridgeNoEditorException("editor not open");
      });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/screenshot");
    httplib::Response res;
    m_Controller->HandleGET_mxnScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnEditorScreenshotReturns200Png()
  {
    const std::vector<unsigned char> fakeBytes{0x89, 0x50, 0x4E, 0x47};
    m_RenderWindowBridge->SetMxNEditorScreenshotProvider(
      [fakeBytes](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) { return fakeBytes; });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/mxn/screenshot");
    httplib::Response res;
    m_Controller->HandleGET_mxnScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    CPPUNIT_ASSERT_EQUAL(fakeBytes.size(), res.body.size());
  }

  void GetMxnEditorScreenshotWithJpegAndSizeReturns200()
  {
    mitk::ScreenshotFormat capturedFormat = mitk::ScreenshotFormat::Png;
    std::optional<std::pair<int, int>> capturedSize;
    m_RenderWindowBridge->SetMxNEditorScreenshotProvider(
      [&](std::optional<std::pair<int, int>> s, mitk::ScreenshotFormat f) {
        capturedSize = s; capturedFormat = f;
        return std::vector<unsigned char>{0xFF, 0xD8};
      });

    httplib::Request req;
    req.path = "/api/v1/rendering/editors/mxn/screenshot";
    req.params.emplace("format", "jpeg");
    req.params.emplace("width", "640");
    req.params.emplace("height", "480");
    httplib::Response res;
    m_Controller->HandleGET_mxnScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    CPPUNIT_ASSERT(capturedFormat == mitk::ScreenshotFormat::Jpeg);
    CPPUNIT_ASSERT(capturedSize.has_value());
    CPPUNIT_ASSERT_EQUAL(640, capturedSize->first);
    CPPUNIT_ASSERT_EQUAL(480, capturedSize->second);
  }

  void GetMxnEditorScreenshotInvalidFormatReturns400()
  {
    m_RenderWindowBridge->SetMxNEditorScreenshotProvider(
      [](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) { return std::vector<unsigned char>{}; });

    httplib::Request req;
    req.path = "/api/v1/rendering/editors/mxn/screenshot";
    req.params.emplace("format", "bmp");
    httplib::Response res;
    m_Controller->HandleGET_mxnScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void GetMxnEditorScreenshotExcessiveDimensionsReturns400()
  {
    m_RenderWindowBridge->SetMxNEditorScreenshotProvider(
      [](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) { return std::vector<unsigned char>{}; });

    httplib::Request req;
    req.path = "/api/v1/rendering/editors/mxn/screenshot";
    req.params.emplace("width", "9000");
    req.params.emplace("height", "9000");
    httplib::Response res;
    m_Controller->HandleGET_mxnScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void GetMxnWindowScreenshotMalformedNameReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/foo%20bar/screenshot", "",
      {{"id", "foo bar"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnWindowScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  void GetMxnWindowScreenshotUnknownNameReturns404()
  {
    m_RenderWindowBridge->SetMxNWindowScreenshotProvider(
      [](const std::string& n, std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) -> std::vector<unsigned char>
      {
        throw mitk::RenderWindowBridgeUnknownWindowException(n);
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__bogus/screenshot", "",
      {{"id", "mxn__bogus"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnWindowScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
  }

  void GetMxnWindowScreenshotReturns200Png()
  {
    const std::vector<unsigned char> fakeBytes{0x89, 0x50, 0x4E, 0x47};
    m_RenderWindowBridge->SetMxNWindowScreenshotProvider(
      [fakeBytes](const std::string&, std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) {
        return fakeBytes;
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/mxn/windows/mxn__widget0/screenshot", "",
      {{"id", "mxn__widget0"}});
    httplib::Response res;
    m_Controller->HandleGET_mxnWindowScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    CPPUNIT_ASSERT_EQUAL(fakeBytes.size(), res.body.size());
  }

  void GetMxnWindowScreenshotForwardsName()
  {
    std::string capturedName;
    mitk::ScreenshotFormat capturedFormat = mitk::ScreenshotFormat::Png;
    m_RenderWindowBridge->SetMxNWindowScreenshotProvider(
      [&capturedName, &capturedFormat](
        const std::string& n, std::optional<std::pair<int, int>>, mitk::ScreenshotFormat f) {
        capturedName = n; capturedFormat = f;
        return std::vector<unsigned char>{0xFF, 0xD8};
      });

    httplib::Request req;
    req.path = "/api/v1/rendering/editors/mxn/windows/mxn__widget7/screenshot";
    req.path_params["id"] = "mxn__widget7";
    req.params.emplace("format", "jpeg");
    httplib::Response res;
    m_Controller->HandleGET_mxnWindowScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget7"), capturedName);
    CPPUNIT_ASSERT(capturedFormat == mitk::ScreenshotFormat::Jpeg);
  }

  // ===== Camera =====

  static mitk::CameraState MakeFakeCameraState(bool is3d)
  {
    mitk::CameraState s;
    s.position[0] = 1.0;  s.position[1] = 2.0;  s.position[2] = 3.0;
    s.focalPoint[0] = 4.0; s.focalPoint[1] = 5.0; s.focalPoint[2] = 6.0;
    s.viewUp[0] = 0.0;    s.viewUp[1] = 1.0;    s.viewUp[2] = 0.0;
    if (is3d) s.perspectiveAngle = 30.0;
    else      s.parallelScale    = 120.0;
    return s;
  }

  void GetCameraUnknownWindowReturns404()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/bogus/camera", "",
      {{"id", "bogus"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_FOUND"),
                         json["error"]["code"].get<std::string>());
  }

  void GetCameraWithoutGetterReturns503()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/camera", "",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetCameraForAxialReturns200With2dFields()
  {
    m_RenderWindowBridge->SetStdMultiCameraGetter(
      [](const std::string&) { return MakeFakeCameraState(/*is3d=*/false); });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/camera", "",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json.contains("position"));
    CPPUNIT_ASSERT(json.contains("focal_point"));
    CPPUNIT_ASSERT(json.contains("view_up"));
    CPPUNIT_ASSERT(json.contains("parallel_scale"));
    CPPUNIT_ASSERT(!json.contains("perspective_angle"));
  }

  void GetCameraFor3dReturns200With3dFields()
  {
    m_RenderWindowBridge->SetStdMultiCameraGetter(
      [](const std::string&) { return MakeFakeCameraState(/*is3d=*/true); });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/3d/camera", "",
      {{"id", "3d"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json.contains("perspective_angle"));
    CPPUNIT_ASSERT(!json.contains("parallel_scale"));
  }

  void GetCameraEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetStdMultiCameraGetter(
      [](const std::string&) -> mitk::CameraState
      {
        throw mitk::RenderWindowBridgeNoEditorException("editor not open");
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/camera", "",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void PutCameraUnknownWindowReturns404()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/bogus/camera",
      R"({"parallel_scale": 120.0})",
      {{"id", "bogus"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
  }

  void PutCameraInvalidJsonReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/camera",
      "not-json", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void PutCameraEmptyBodyReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/camera",
      "{}", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  void PutCameraUnknownFieldReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/camera",
      R"({"mystery": 42})", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  // Helper: assert that a PUT camera body produces 400 INVALID_REQUEST and
  // that the error message contains the expected substring.
  void AssertPutCameraReturns400(const std::string& windowId,
                                 const std::string& body,
                                 const std::string& expectedMessageSubstr)
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/" + windowId + "/camera",
      body, {{"id", windowId}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
    const auto msg = json["error"]["message"].get<std::string>();
    CPPUNIT_ASSERT(msg.find(expectedMessageSubstr) != std::string::npos);
  }

  void PutCameraParallelScaleOn3dReturns400()
  {
    this->AssertPutCameraReturns400(
      "3d", R"({"parallel_scale": 100.0})", "parallel_scale");
  }

  void PutCameraPerspectiveAngleOn2dReturns400()
  {
    this->AssertPutCameraReturns400(
      "axial", R"({"perspective_angle": 30.0})", "perspective_angle");
  }

  void PutCameraNonPositiveParallelScaleReturns400()
  {
    this->AssertPutCameraReturns400(
      "axial", R"({"parallel_scale": 0.0})", "parallel_scale");
  }

  void PutCameraPerspectiveAngleOutOfRangeReturns400()
  {
    this->AssertPutCameraReturns400(
      "3d", R"({"perspective_angle": 180.0})", "perspective_angle");
  }

  void PutCameraUnknownStandardViewReturns400()
  {
    this->AssertPutCameraReturns400(
      "axial", R"({"standard_view": "oblique"})", "standard_view");
  }

  void PutCameraWrongArrayLengthReturns400()
  {
    this->AssertPutCameraReturns400(
      "axial", R"({"position": [1.0, 2.0]})", "position");
  }

  void PutCameraStandardViewWithPositionReturns400()
  {
    // Issue #1: combining standard_view with explicit pose fields would leave
    // the CameraController's internal "standard view" memo inconsistent with
    // the actual camera pose -- controller rejects the combination upfront.
    this->AssertPutCameraReturns400(
      "axial",
      R"({"standard_view": "anterior", "position": [1.0, 2.0, 3.0]})",
      "standard_view");
  }

  void PutCameraStandardViewWithFocalPointReturns400()
  {
    this->AssertPutCameraReturns400(
      "axial",
      R"({"standard_view": "anterior", "focal_point": [0.0, 0.0, 0.0]})",
      "standard_view");
  }

  void PutCameraStandardViewWithViewUpReturns400()
  {
    this->AssertPutCameraReturns400(
      "axial",
      R"({"standard_view": "anterior", "view_up": [0.0, 1.0, 0.0]})",
      "standard_view");
  }

  void PutCameraSetterThrowsMitkExceptionReturns422()
  {
    m_RenderWindowBridge->SetStdMultiCameraSetter(
      [](const std::string&, const mitk::CameraPatch&)
      {
        mitkThrow() << "synthetic camera failure";
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/camera",
      R"({"parallel_scale": 120.0})", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(422, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDERING_ERROR"),
                         json["error"]["code"].get<std::string>());
  }

  void PutSliceSetterThrowsMitkExceptionReturns422()
  {
    m_RenderWindowBridge->SetStdMultiSelectedSliceStepSetter(
      [](const std::string&, unsigned int)
      {
        mitkThrow() << "synthetic slice failure";
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice",
      R"({"step": 0})", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(422, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDERING_ERROR"),
                         json["error"]["code"].get<std::string>());
  }

  void PutCameraWithoutSetterReturns503()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/camera",
      R"({"parallel_scale": 120.0})", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void PutCameraOnAxialReturns204()
  {
    mitk::CameraPatch captured;
    std::string capturedName;
    m_RenderWindowBridge->SetStdMultiCameraSetter(
      [&](const std::string& n, const mitk::CameraPatch& p)
      { capturedName = n; captured = p; });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/camera",
      R"({"position": [1.0, 2.0, 3.0], "parallel_scale": 150.0})",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
    CPPUNIT_ASSERT_EQUAL(std::string("axial"), capturedName);
    CPPUNIT_ASSERT(captured.position.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, (*captured.position)[0], 1e-6);
    CPPUNIT_ASSERT(captured.parallelScale.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(150.0, *captured.parallelScale, 1e-6);
    CPPUNIT_ASSERT(!captured.perspectiveAngle.has_value());
  }

  void PutCameraOn3dReturns204()
  {
    mitk::CameraPatch captured;
    m_RenderWindowBridge->SetStdMultiCameraSetter(
      [&](const std::string&, const mitk::CameraPatch& p) { captured = p; });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/3d/camera",
      R"({"perspective_angle": 45.0})", {{"id", "3d"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
    CPPUNIT_ASSERT(captured.perspectiveAngle.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(45.0, *captured.perspectiveAngle, 1e-6);
  }

  void PutCameraStandardViewAppliedFirst()
  {
    mitk::CameraPatch captured;
    m_RenderWindowBridge->SetStdMultiCameraSetter(
      [&](const std::string&, const mitk::CameraPatch& p) { captured = p; });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/camera",
      R"({"standard_view": "anterior", "parallel_scale": 120.0})",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiCamera(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
    // The controller forwards both fields; ordering is a plugin-side concern
    // (standard_view applied first before individual fields). We assert the
    // wire contract: both fields were sent through intact.
    CPPUNIT_ASSERT(captured.standardView.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("anterior"), *captured.standardView);
    CPPUNIT_ASSERT(captured.parallelScale.has_value());
  }
  // ===== Selected-slice =====

  static mitk::SliceState MakeFakeSliceState(bool withBounds)
  {
    mitk::SliceState s;
    s.step = 5;
    s.position[0] = 10.0; s.position[1] = 20.0; s.position[2] = 30.0;
    s.bounds.steps = 90;
    if (withBounds)
    {
      s.bounds.minPosition[0] = 0.0;  s.bounds.minPosition[1] = 0.0;  s.bounds.minPosition[2] = 0.0;
      s.bounds.maxPosition[0] = 100.0; s.bounds.maxPosition[1] = 100.0; s.bounds.maxPosition[2] = 100.0;
      s.bounds.hasPositions = true;
    }
    return s;
  }

  void GetSliceUnknownWindowReturns404()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/bogus/selected-slice", "",
      {{"id", "bogus"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_FOUND"),
                         json["error"]["code"].get<std::string>());
  }

  void GetSliceOn3dReturns404UnsupportedOperation()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/3d/selected-slice", "",
      {{"id", "3d"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("UNSUPPORTED_OPERATION"),
                         json["error"]["code"].get<std::string>());
  }

  void GetSliceWithoutGetterReturns503()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice", "",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetSliceForAxialReturns200()
  {
    m_RenderWindowBridge->SetStdMultiSelectedSliceGetter(
      [](const std::string&) { return MakeFakeSliceState(/*withBounds=*/true); });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice", "",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(5, json["step"].get<int>());
    CPPUNIT_ASSERT(json["position"].is_array());
    CPPUNIT_ASSERT_EQUAL(90, json["bounds"]["steps"].get<int>());
    CPPUNIT_ASSERT(json["bounds"]["min_position"].is_array());
    CPPUNIT_ASSERT(json["bounds"]["max_position"].is_array());
    // No `plane` field on slice response -- orientation is read from the camera.
    CPPUNIT_ASSERT(!json.contains("plane"));
  }

  void GetSliceNoGeometryBoundsNull()
  {
    m_RenderWindowBridge->SetStdMultiSelectedSliceGetter(
      [](const std::string&) { return MakeFakeSliceState(/*withBounds=*/false); });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice", "",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["bounds"]["min_position"].is_null());
    CPPUNIT_ASSERT(json["bounds"]["max_position"].is_null());
  }

  void PutSliceUnknownWindowReturns404()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/bogus/selected-slice",
      R"({"step": 0})", {{"id", "bogus"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
  }

  void PutSliceOn3dReturns404UnsupportedOperation()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/3d/selected-slice",
      R"({"step": 0})", {{"id", "3d"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("UNSUPPORTED_OPERATION"),
                         json["error"]["code"].get<std::string>());
  }

  void PutSliceInvalidJsonReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice",
      "not-json", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void PutSliceEmptyBodyReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice",
      "", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void PutSliceMissingStepReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice",
      "{}", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
                         json["error"]["code"].get<std::string>());
  }

  void PutSliceNegativeStepReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice",
      R"({"step": -1})", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void PutSliceNonIntegerStepReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice",
      R"({"step": 1.5})", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void PutSlicePositionFieldReturns400WithHint()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice",
      R"({"position": [1.0, 2.0, 3.0]})", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    const auto msg = json["error"]["message"].get<std::string>();
    CPPUNIT_ASSERT(msg.find("selected-position") != std::string::npos);
  }

  void PutSliceUnknownFieldReturns400()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice",
      R"({"mystery": 0})", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void PutSliceWithoutSetterReturns503()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice",
      R"({"step": 5})", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
  }

  void PutSliceOnAxialReturns204()
  {
    std::string capturedName;
    unsigned int capturedStep = 0;
    m_RenderWindowBridge->SetStdMultiSelectedSliceStepSetter(
      [&](const std::string& n, unsigned int s) { capturedName = n; capturedStep = s; });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/selected-slice",
      R"({"step": 42})", {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandlePUT_stdmultiSelectedSlice(req, res);

    CPPUNIT_ASSERT_EQUAL(204, res.status);
    CPPUNIT_ASSERT_EQUAL(std::string("axial"), capturedName);
    CPPUNIT_ASSERT_EQUAL(42u, capturedStep);
  }
  // ===== Window/editor screenshots =====

  void GetEditorScreenshotWithoutProviderReturns503()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/editors/stdmulti/screenshot");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_AVAILABLE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetEditorScreenshotEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetStdMultiEditorScreenshotProvider(
      [](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) -> std::vector<unsigned char>
      {
        throw mitk::RenderWindowBridgeNoEditorException("editor not open");
      });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/stdmulti/screenshot");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("EDITOR_NOT_ACTIVE"),
                         json["error"]["code"].get<std::string>());
  }

  void GetEditorScreenshotReturns200Png()
  {
    const std::vector<unsigned char> fakeBytes{0x89, 0x50, 0x4E, 0x47};
    m_RenderWindowBridge->SetStdMultiEditorScreenshotProvider(
      [fakeBytes](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) { return fakeBytes; });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/stdmulti/screenshot");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    // Content-Type is set via set_content; body bytes match provider output.
    CPPUNIT_ASSERT_EQUAL(fakeBytes.size(), res.body.size());
  }

  void GetEditorScreenshotWithJpegAndSizeReturns200()
  {
    mitk::ScreenshotFormat capturedFormat = mitk::ScreenshotFormat::Png;
    std::optional<std::pair<int, int>> capturedSize;
    m_RenderWindowBridge->SetStdMultiEditorScreenshotProvider(
      [&](std::optional<std::pair<int, int>> s, mitk::ScreenshotFormat f) {
        capturedSize = s; capturedFormat = f;
        return std::vector<unsigned char>{0xFF, 0xD8};
      });

    httplib::Request req;
    req.path = "/api/v1/rendering/editors/stdmulti/screenshot";
    req.params.emplace("format", "jpeg");
    req.params.emplace("width", "640");
    req.params.emplace("height", "480");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    CPPUNIT_ASSERT(capturedFormat == mitk::ScreenshotFormat::Jpeg);
    CPPUNIT_ASSERT(capturedSize.has_value());
    CPPUNIT_ASSERT_EQUAL(640, capturedSize->first);
    CPPUNIT_ASSERT_EQUAL(480, capturedSize->second);
  }

  void GetEditorScreenshotInvalidFormatReturns400()
  {
    m_RenderWindowBridge->SetStdMultiEditorScreenshotProvider(
      [](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) { return std::vector<unsigned char>{}; });

    httplib::Request req;
    req.path = "/api/v1/rendering/editors/stdmulti/screenshot";
    req.params.emplace("format", "bmp");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void GetEditorScreenshotExcessiveDimensionsReturns400()
  {
    m_RenderWindowBridge->SetStdMultiEditorScreenshotProvider(
      [](std::optional<std::pair<int, int>>, mitk::ScreenshotFormat) { return std::vector<unsigned char>{}; });

    httplib::Request req;
    req.path = "/api/v1/rendering/editors/stdmulti/screenshot";
    req.params.emplace("width", "9000");
    req.params.emplace("height", "9000");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
  }

  void GetWindowScreenshotUnknownWindowReturns404()
  {
    // Window-name validation runs before bridge-availability for all stdmulti
    // window handlers, so an unknown name yields 404 regardless of whether a
    // provider is bound.
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/bogus/screenshot", "",
      {{"id", "bogus"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindowScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("RENDER_WINDOW_NOT_FOUND"),
                         json["error"]["code"].get<std::string>());
  }

  void GetWindowScreenshotWithoutProviderReturns503()
  {
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/screenshot", "",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindowScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
  }

  void GetWindowScreenshotReturns200Png()
  {
    const std::vector<unsigned char> fakeBytes{0x89, 0x50, 0x4E, 0x47};
    std::string capturedName;
    m_RenderWindowBridge->SetStdMultiWindowScreenshotProvider(
      [fakeBytes, &capturedName](
        const std::string& name, std::optional<std::pair<int, int>>, mitk::ScreenshotFormat)
      {
        capturedName = name;
        return fakeBytes;
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial/screenshot", "",
      {{"id", "axial"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindowScreenshot(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    CPPUNIT_ASSERT_EQUAL(std::string("axial"), capturedName);
    CPPUNIT_ASSERT_EQUAL(fakeBytes.size(), res.body.size());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRenderingController)
