/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include "mitkRenderingController.h"
#include "mitkDataStorageBridge.h"

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

  // POST /rendering/reinit with uid body (node-scoped reinit) tests
  MITK_TEST(NodeReinitWithoutDataStorageReturns503);
  MITK_TEST(NodeReinitUnknownUidReturns404);
  MITK_TEST(NodeReinitNodeWithNoDataReturns422);
  MITK_TEST(NodeReinitValidNodeReturns204);
  MITK_TEST(ReinitWithInvalidJsonBodyReturns400);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  std::unique_ptr<mitk::DataStorageBridge> m_Bridge;
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
    m_Controller = std::make_unique<mitk::RenderingController>(*m_Bridge);
    // No dispatcher: headless mode — tasks execute directly on the calling thread.
  }

  void tearDown() override
  {
    m_Controller.reset();
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

  // ===== POST /rendering/reinit with uid body (node-scoped reinit) =====

  void NodeReinitWithoutDataStorageReturns503()
  {
    m_Bridge->SetDataStorage(nullptr);

    const auto req = this->MakeRequest("/api/v1/rendering/reinit", R"({"uid":"some-uid"})");
    httplib::Response res;

    m_Controller->HandlePOST_reinit(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("DATASTORAGE_NOT_AVAILABLE"), json["error"]["code"].get<std::string>());
  }

  void NodeReinitUnknownUidReturns404()
  {
    const auto req = this->MakeRequest("/api/v1/rendering/reinit", R"({"uid":"unknown-uid"})");
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

    const auto req = this->MakeRequest("/api/v1/rendering/reinit", R"({"uid":")" + uid + R"("})");
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

    const auto req = this->MakeRequest("/api/v1/rendering/reinit", R"({"uid":")" + uid + R"("})");
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
};

MITK_TEST_SUITE_REGISTRATION(mitkRenderingController)
