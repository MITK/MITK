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

  // WP2 editor discovery tests
  MITK_TEST(GetEditorsWithoutProviderReturns503);
  MITK_TEST(GetEditorsReturns200WithAliases);
  MITK_TEST(GetStdmultiInfoWithEditorActiveReturns200);
  MITK_TEST(GetStdmultiInfoWhenEditorInactiveReturns503EditorNotActive);
  MITK_TEST(GetStdmultiWindowsWithoutProviderReturns503);
  MITK_TEST(GetStdmultiWindowsEditorNotOpenReturns503EditorNotActive);
  MITK_TEST(GetStdmultiWindowsReturns200);
  MITK_TEST(GetStdmultiWindowForUnknownNameReturns404);
  MITK_TEST(GetStdmultiWindowForAxialReturns200With2d);
  MITK_TEST(GetStdmultiWindowFor3dReturns200NoSelectedSlice);

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
    // No dispatcher: headless mode — tasks execute directly on the calling thread.
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
    CPPUNIT_ASSERT(json["bounds"].contains("min"));
    CPPUNIT_ASSERT(json["bounds"].contains("max"));
  }

  void GetSelectedPositionEditorNotOpenReturns503EditorNotActive()
  {
    m_RenderWindowBridge->SetPositionGetter(
      []() -> mitk::SelectedPositionInfo
      {
        throw mitk::RenderWindowBridgeNoEditorException(
          "StdMultiWidgetEditor is not open — cannot read crosshair position");
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
          "StdMultiWidgetEditor is not open — cannot set crosshair position");
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
    // No provider is set in setUp — bridge has no provider.
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

  // ===== WP2: editor discovery =====

  static std::vector<mitk::EditorInfo> FakeEditors(bool stdmultiActive)
  {
    mitk::EditorInfo stdmulti;
    stdmulti.alias = "stdmulti";
    stdmulti.pluginId = "org.mitk.editors.stdmultiwidget";
    stdmulti.active = stdmultiActive;
    if (stdmultiActive)
      stdmulti.windowNames = {"axial", "sagittal", "coronal", "3d"};

    mitk::EditorInfo mxn;
    mxn.alias = "mxn";
    mxn.pluginId = "org.mitk.editors.mxnmultiwidget";
    mxn.active = false;

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
          {"axial",    mitk::WindowKind::TwoD},
          {"sagittal", mitk::WindowKind::TwoD},
          {"coronal",  mitk::WindowKind::TwoD},
          {"3d",       mitk::WindowKind::ThreeD}
        };
      });

    const auto req = this->MakeRequest("/api/v1/rendering/editors/stdmulti/windows");
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindows(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json.is_array());
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), json.size());
    CPPUNIT_ASSERT_EQUAL(std::string("axial"), json[0]["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("2d"), json[0]["kind"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), json[3]["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), json[3]["kind"].get<std::string>());
    // RF3: no `plane` field on window list items.
    CPPUNIT_ASSERT(!json[0].contains("plane"));
  }

  void GetStdmultiWindowForUnknownNameReturns404()
  {
    // No providers set — unknown name must still be rejected controller-side.
    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/bogus", "",
      {{"name", "bogus"}});
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
          {"axial",    mitk::WindowKind::TwoD},
          {"sagittal", mitk::WindowKind::TwoD},
          {"coronal",  mitk::WindowKind::TwoD},
          {"3d",       mitk::WindowKind::ThreeD}
        };
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/axial", "",
      {{"name", "axial"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindow(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("axial"), json["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("2d"), json["kind"].get<std::string>());
    CPPUNIT_ASSERT(json["has_camera"].get<bool>());
    CPPUNIT_ASSERT(json["has_selected_slice"].get<bool>());
    CPPUNIT_ASSERT(!json.contains("plane"));
  }

  void GetStdmultiWindowFor3dReturns200NoSelectedSlice()
  {
    m_RenderWindowBridge->SetStdMultiWindowListProvider(
      []() {
        return std::vector<mitk::WindowInfo>{
          {"axial",    mitk::WindowKind::TwoD},
          {"sagittal", mitk::WindowKind::TwoD},
          {"coronal",  mitk::WindowKind::TwoD},
          {"3d",       mitk::WindowKind::ThreeD}
        };
      });

    const auto req = this->MakeRequest(
      "/api/v1/rendering/editors/stdmulti/windows/3d", "",
      {{"name", "3d"}});
    httplib::Response res;
    m_Controller->HandleGET_stdmultiWindow(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), json["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), json["kind"].get<std::string>());
    CPPUNIT_ASSERT(json["has_camera"].get<bool>());
    CPPUNIT_ASSERT(!json["has_selected_slice"].get<bool>());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRenderingController)
