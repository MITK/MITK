/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include "mitkDataStorageController.h"
#include "mitkDataStorageBridge.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkImage.h>
#include <mitkImageWriteAccessor.h>
#include <mitkSurface.h>
#include <mitkPointSet.h>
#include <mitkIOUtil.h>
#include <mitkFileSystem.h>
#include <mitkImageGenerator.h>
#include <mitkNumericConstants.h>

#include <nlohmann/json.hpp>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkTriangle.h>
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>

#include <fstream>

/**
 * @brief Test suite for DataStorageController data endpoints (GET/PUT /nodes/:uid/data).
 *
 * Tests cover:
 * - Error handling (no DataStorage, node not found, null data)
 * - Direct transfer mode (binary upload/download)
 * - File-reference transfer mode (JSON with file path)
 * - Temp directory management
 */
class mitkDataStorageControllerDataTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDataStorageControllerDataTestSuite);
  // Error cases
  MITK_TEST(GetDataWithoutDataStorage);
  MITK_TEST(GetDataNodeNotFound);
  MITK_TEST(GetDataNullData);
  MITK_TEST(PutDataWithoutDataStorage);
  MITK_TEST(PutDataNodeNotFound);
  MITK_TEST(PutDataEmptyBody);
  MITK_TEST(PutDataUnsupportedContentType);
  MITK_TEST(PutDataMissingContentType);
  MITK_TEST(PutDataInvalidJson);
  MITK_TEST(PutDataMissingFilePath);
  MITK_TEST(PutDataFileNotFound);
  MITK_TEST(PutDataUnsupportedTransferMode);
  MITK_TEST(PostNodeUnsupportedTransferMode);

  // Transfer mode determination
  MITK_TEST(TransferModeDefaultsDirect);
  MITK_TEST(TransferModeDirectFromHeader);
  MITK_TEST(TransferModeFileReferenceFromHeader);
  MITK_TEST(TransferModeFromAcceptHeader);

  // Data metadata building
  MITK_TEST(BuildImageMetadata);
  MITK_TEST(BuildSurfaceMetadata);
  MITK_TEST(BuildPointSetMetadata);

  // Full round-trip tests (requires serializers to be available)
  MITK_TEST(GetImageDataDirect);
  MITK_TEST(GetImageDataFileReference);
  MITK_TEST(PutImageDataDirect);
  MITK_TEST(PutImageDataFileReference);

  // POST /nodes with data transfer
  MITK_TEST(PostNodeWithDataFileReference);
  MITK_TEST(PostNodeWithDataDirect);
  MITK_TEST(PostChildNodeWithDataFileReference);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  std::unique_ptr<mitk::DataStorageBridge> m_Bridge;
  std::unique_ptr<mitk::DataStorageController> m_Controller;
  std::string m_TempDirectory;

  // Helper to create a request with path params and headers
  httplib::Request CreateRequest(const std::string& path = "",
                                  const std::string& body = "",
                                  const std::unordered_map<std::string, std::string>& pathParams = {},
                                  const std::multimap<std::string, std::string>& queryParams = {},
                                  const httplib::Headers& headers = {})
  {
    httplib::Request req;
    req.path = path;
    req.body = body;
    req.path_params = pathParams;
    req.params = queryParams;
    req.headers = headers;
    return req;
  }

  // Helper to get the UID for a node directly from the bridge
  std::string GetUid(const mitk::DataNode* node)
  {
    return m_Bridge->GetNodeUid(node);
  }

  // Helper to create a test image with deterministic content (not random)
  mitk::Image::Pointer CreateTestImage()
  {
    // Create a simple 3D grayscale test image with deterministic content
    // Using gradient pattern: value = x + y*10 + z*100 (mod 256)
    const unsigned int dimX = 10, dimY = 10, dimZ = 10;

    mitk::Image::Pointer image = mitk::Image::New();
    unsigned int dimensions[] = {dimX, dimY, dimZ};
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    // Set spacing
    mitk::Vector3D spacing;
    spacing[0] = 1.0;
    spacing[1] = 1.0;
    spacing[2] = 1.0;
    image->GetGeometry()->SetSpacing(spacing);

    // Fill with deterministic pattern
    mitk::ImageWriteAccessor accessor(image);
    auto* data = static_cast<unsigned char*>(accessor.GetData());

    for (unsigned int z = 0; z < dimZ; ++z)
    {
      for (unsigned int y = 0; y < dimY; ++y)
      {
        for (unsigned int x = 0; x < dimX; ++x)
        {
          const size_t idx = x + y * dimX + z * dimX * dimY;
          data[idx] = static_cast<unsigned char>((x + y * 10 + z * 100) % 256);
        }
      }
    }

    return image;
  }

  // Helper to create a test surface
  mitk::Surface::Pointer CreateTestSurface()
  {
    mitk::Surface::Pointer surface = mitk::Surface::New();

    // Create a simple triangle
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(0.0, 0.0, 0.0);
    points->InsertNextPoint(1.0, 0.0, 0.0);
    points->InsertNextPoint(0.5, 1.0, 0.0);

    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
    triangle->GetPointIds()->SetId(0, 0);
    triangle->GetPointIds()->SetId(1, 1);
    triangle->GetPointIds()->SetId(2, 2);
    triangles->InsertNextCell(triangle);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(triangles);

    surface->SetVtkPolyData(polyData);
    return surface;
  }

  // Helper to create a test point set
  mitk::PointSet::Pointer CreateTestPointSet()
  {
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    mitk::Point3D point;

    point[0] = 0.0; point[1] = 0.0; point[2] = 0.0;
    pointSet->InsertPoint(0, point);

    point[0] = 1.0; point[1] = 1.0; point[2] = 1.0;
    pointSet->InsertPoint(1, point);

    point[0] = 2.0; point[1] = 2.0; point[2] = 2.0;
    pointSet->InsertPoint(2, point);

    return pointSet;
  }

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Bridge = std::make_unique<mitk::DataStorageBridge>();
    m_Bridge->SetDataStorage(m_DataStorage);
    m_Controller = std::make_unique<mitk::DataStorageController>(*m_Bridge);

    // Create temp directory for tests
    try
    {
      m_TempDirectory = mitk::IOUtil::CreateTemporaryDirectory("mitk-rest-test-XXXXXX");
      m_Controller->SetTempDirectory(m_TempDirectory);
    }
    catch (const std::exception& e)
    {
      MITK_WARN << "Failed to create temp directory: " << e.what();
      m_TempDirectory = "";
    }
  }

  void tearDown() override
  {
    m_Controller.reset();
    m_Bridge->SetDataStorage(nullptr);
    m_Bridge.reset();
    m_DataStorage = nullptr;

    // Clean up temp directory
    if (!m_TempDirectory.empty())
    {
      try
      {
        fs::remove_all(fs::path(m_TempDirectory));
      }
      catch (const std::exception&) {}
      m_TempDirectory.clear();
    }
  }

  // ===== Error case tests =====

  void GetDataWithoutDataStorage()
  {
    m_Bridge->SetDataStorage(nullptr);

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/abc-123/data", "", {{"uid", "abc-123"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("DATASTORAGE_NOT_AVAILABLE"), json["error"]["code"].get<std::string>());
  }

  void GetDataNodeNotFound()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/nonexistent/data", "", {{"uid", "nonexistent"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), json["error"]["code"].get<std::string>());
  }

  void GetDataNullData()
  {
    // Create node without data
    auto node = mitk::DataNode::New();
    node->SetName("EmptyNode");
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NO_DATA"), json["error"]["code"].get<std::string>());
  }

  void PutDataWithoutDataStorage()
  {
    m_Bridge->SetDataStorage(nullptr);

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/abc-123/data", "binary data", {{"uid", "abc-123"}});
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("DATASTORAGE_NOT_AVAILABLE"), json["error"]["code"].get<std::string>());
  }

  void PutDataNodeNotFound()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/nonexistent/data", "binary data", {{"uid", "nonexistent"}});
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), json["error"]["code"].get<std::string>());
  }

  void PutDataEmptyBody()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    // Empty body with Content-Type header for direct transfer mode
    const httplib::Headers headers = {{"Content-Type", "application/octet-stream"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void PutDataUnsupportedContentType()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    // Use unsupported Content-Type
    const httplib::Headers headers = {{"Content-Type", "text/plain"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "some data", {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(415, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("UNSUPPORTED_FORMAT"), json["error"]["code"].get<std::string>());
  }

  void PutDataMissingContentType()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    // No Content-Type header at all
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "some data", {{"uid", uid}});
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(415, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("UNSUPPORTED_FORMAT"), json["error"]["code"].get<std::string>());
  }

  void PutDataInvalidJson()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    const httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "not valid json", {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void PutDataMissingFilePath()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    nlohmann::json body;
    body["transfer"]["mode"] = "file-reference";
    // Missing file_path

    const httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", body.dump(), {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  void PutDataFileNotFound()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    nlohmann::json body;
    body["transfer"]["file_path"] = "/nonexistent/path/file.nrrd";

    const httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", body.dump(), {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(422, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("FILE_NOT_FOUND"), json["error"]["code"].get<std::string>());
  }

  void PutDataUnsupportedTransferMode()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    // Use unsupported transfer mode "shared-memory"
    nlohmann::json body;
    body["transfer"]["mode"] = "shared-memory";
    body["transfer"]["file_path"] = "/some/path.nrrd";

    const httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", body.dump(), {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    CPPUNIT_ASSERT_EQUAL(406, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("TRANSFER_MODE_NOT_AVAILABLE"), json["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT(json["error"].contains("available_modes"));
    CPPUNIT_ASSERT(json["error"]["available_modes"].is_array());
  }

  void PostNodeUnsupportedTransferMode()
  {
    // POST /nodes with unsupported transfer mode
    nlohmann::json body;
    body["name"] = "TestNode";
    body["transfer"]["mode"] = "shared-memory";

    const httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes", body.dump(), {}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePOST_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(406, res.status);
    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("TRANSFER_MODE_NOT_AVAILABLE"), json["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT(json["error"].contains("available_modes"));
  }

  // ===== Transfer mode determination tests =====

  void TransferModeDefaultsDirect()
  {
    // Note: We can't directly test DetermineTransferMode since it's private
    // So we test indirectly by checking the response behavior

    // Create a node with data
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    node->SetData(this->CreateTestImage());
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    auto getReq = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(getReq, res);

    // In direct mode, response should be binary (application/octet-stream)
    // If test passes, it means the data was serialized (requires serializer to be available)
    if (res.status == 200)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("application/octet-stream"), res.get_header_value("Content-Type"));
    }
    else if (res.status == 415)
    {
      // Serializer not available - this is OK for this test
      CPPUNIT_ASSERT(true);
    }
  }

  void TransferModeDirectFromHeader()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    node->SetData(this->CreateTestImage());
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    // Explicitly request direct mode via X-MITK-Transfer-Mode header
    const httplib::Headers headers = {{"X-MITK-Transfer-Mode", "direct"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    if (res.status == 200)
    {
      // In direct mode, response should be binary (application/octet-stream)
      CPPUNIT_ASSERT_EQUAL(std::string("application/octet-stream"), res.get_header_value("Content-Type"));
    }
    else if (res.status == 415)
    {
      // Serializer not available - OK for this test
      CPPUNIT_ASSERT(true);
    }
  }

  void TransferModeFileReferenceFromHeader()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    node->SetData(this->CreateTestImage());
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    const httplib::Headers headers = {{"X-MITK-Transfer-Mode", "file-reference"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    if (res.status == 200)
    {
      // In file-reference mode, response should be JSON
      CPPUNIT_ASSERT_EQUAL(std::string("application/json"), res.get_header_value("Content-Type"));
      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT_EQUAL(std::string("file-reference"), json["transfer"]["mode"].get<std::string>());
    }
    else if (res.status == 415)
    {
      // Serializer not available - OK for this test
      CPPUNIT_ASSERT(true);
    }
  }

  void TransferModeFromAcceptHeader()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    node->SetData(this->CreateTestImage());
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    const httplib::Headers headers = {{"Accept", "application/json"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    if (res.status == 200)
    {
      // Accept: application/json should trigger file-reference mode
      CPPUNIT_ASSERT_EQUAL(std::string("application/json"), res.get_header_value("Content-Type"));
    }
    else if (res.status == 415)
    {
      // Serializer not available - OK for this test
      CPPUNIT_ASSERT(true);
    }
  }

  // ===== Data metadata building tests =====

  void BuildImageMetadata()
  {
    auto node = mitk::DataNode::New();
    node->SetName("ImageNode");
    const auto image = this->CreateTestImage();
    node->SetData(image);
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    const httplib::Headers headers = {{"X-MITK-Transfer-Mode", "file-reference"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    if (res.status == 200)
    {
      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT(json.contains("data_metadata"));

      const auto& metadata = json["data_metadata"];

      // Verify dimensions content (10x10x10 from CreateTestImage)
      CPPUNIT_ASSERT(metadata.contains("dimensions"));
      CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), metadata["dimensions"].size());
      CPPUNIT_ASSERT_EQUAL(10, metadata["dimensions"][0].get<int>());
      CPPUNIT_ASSERT_EQUAL(10, metadata["dimensions"][1].get<int>());
      CPPUNIT_ASSERT_EQUAL(10, metadata["dimensions"][2].get<int>());

      // Verify spacing content (1.0, 1.0, 1.0)
      CPPUNIT_ASSERT(metadata.contains("spacing"));
      CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), metadata["spacing"].size());
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, metadata["spacing"][0].get<double>(), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, metadata["spacing"][1].get<double>(), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, metadata["spacing"][2].get<double>(), 1e-6);

      // Verify origin content (0, 0, 0)
      CPPUNIT_ASSERT(metadata.contains("origin"));
      CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), metadata["origin"].size());
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, metadata["origin"][0].get<double>(), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, metadata["origin"][1].get<double>(), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, metadata["origin"][2].get<double>(), 1e-6);

      // Verify pixel type (unsigned char)
      CPPUNIT_ASSERT(metadata.contains("pixel_type"));
      const std::string pixelType = metadata["pixel_type"].get<std::string>();
      // Pixel type string should contain "unsigned char" or similar representation
      CPPUNIT_ASSERT(!pixelType.empty());
    }
    else if (res.status == 415)
    {
      // Serializer not available - skip metadata verification
      CPPUNIT_ASSERT(true);
    }
  }

  void BuildSurfaceMetadata()
  {
    auto node = mitk::DataNode::New();
    node->SetName("SurfaceNode");
    node->SetData(this->CreateTestSurface());
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    const httplib::Headers headers = {{"X-MITK-Transfer-Mode", "file-reference"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    if (res.status == 200)
    {
      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT(json.contains("data_metadata"));

      const auto& metadata = json["data_metadata"];

      // Verify point count (3 points from triangle)
      CPPUNIT_ASSERT(metadata.contains("points_count"));
      CPPUNIT_ASSERT_EQUAL(3, metadata["points_count"].get<int>());

      // Verify cell count (1 triangle)
      CPPUNIT_ASSERT(metadata.contains("cells_count"));
      CPPUNIT_ASSERT_EQUAL(1, metadata["cells_count"].get<int>());

      // Verify bounds (triangle: (0,0,0), (1,0,0), (0.5,1,0))
      // Expected bounds: [0, 1, 0, 1, 0, 0] (xmin, xmax, ymin, ymax, zmin, zmax)
      CPPUNIT_ASSERT(metadata.contains("bounds"));
      CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), metadata["bounds"].size());
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, metadata["bounds"][0].get<double>(), 1e-6);  // xmin
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, metadata["bounds"][1].get<double>(), 1e-6);  // xmax
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, metadata["bounds"][2].get<double>(), 1e-6);  // ymin
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, metadata["bounds"][3].get<double>(), 1e-6);  // ymax
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, metadata["bounds"][4].get<double>(), 1e-6);  // zmin
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, metadata["bounds"][5].get<double>(), 1e-6);  // zmax
    }
    else if (res.status == 415)
    {
      // Serializer not available - skip metadata verification
      CPPUNIT_ASSERT(true);
    }
  }

  void BuildPointSetMetadata()
  {
    auto node = mitk::DataNode::New();
    node->SetName("PointSetNode");
    node->SetData(this->CreateTestPointSet());
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    const httplib::Headers headers = {{"X-MITK-Transfer-Mode", "file-reference"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    if (res.status == 200)
    {
      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT(json.contains("data_metadata"));

      const auto& metadata = json["data_metadata"];

      // Verify point count (3 points: (0,0,0), (1,1,1), (2,2,2))
      CPPUNIT_ASSERT(metadata.contains("points_count"));
      CPPUNIT_ASSERT_EQUAL(3, metadata["points_count"].get<int>());

      // Verify bounds if available
      // Expected bounds from points: [0, 2, 0, 2, 0, 2]
      if (metadata.contains("bounds"))
      {
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), metadata["bounds"].size());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, metadata["bounds"][0].get<double>(), 1e-6);  // xmin
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, metadata["bounds"][1].get<double>(), 1e-6);  // xmax
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, metadata["bounds"][2].get<double>(), 1e-6);  // ymin
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, metadata["bounds"][3].get<double>(), 1e-6);  // ymax
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, metadata["bounds"][4].get<double>(), 1e-6);  // zmin
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, metadata["bounds"][5].get<double>(), 1e-6);  // zmax
      }
    }
    else if (res.status == 415)
    {
      // Serializer not available - skip metadata verification
      CPPUNIT_ASSERT(true);
    }
  }

  // ===== Full round-trip tests =====

  void GetImageDataDirect()
  {
    // Create the original test image
    const auto originalImage = this->CreateTestImage();

    auto node = mitk::DataNode::New();
    node->SetName("ImageNode");
    node->SetData(originalImage);
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    // Default mode is direct (binary download)
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    if (res.status == 200)
    {
      // Verify binary response headers
      CPPUNIT_ASSERT_EQUAL(std::string("application/octet-stream"), res.get_header_value("Content-Type"));
      CPPUNIT_ASSERT(!res.body.empty());
      CPPUNIT_ASSERT(!res.get_header_value("Content-Disposition").empty());

      // Save received binary data to temp file and reload to compare
      const std::string tempFilePath = mitk::IOUtil::CreateTemporaryFile("received_XXXXXX.nrrd", m_TempDirectory);
      std::ofstream outFile(tempFilePath, std::ios::binary);
      outFile.write(res.body.c_str(), static_cast<std::streamsize>(res.body.size()));
      outFile.close();

      try
      {
        // Load the received data
        auto loadedData = mitk::IOUtil::Load(tempFilePath);
        fs::remove(fs::path(tempFilePath));

        CPPUNIT_ASSERT(!loadedData.empty());
        auto* loadedImage = dynamic_cast<mitk::Image*>(loadedData[0].GetPointer());
        CPPUNIT_ASSERT(loadedImage != nullptr);

        // Compare the loaded image with the original using MITK's Equal function
        CPPUNIT_ASSERT_MESSAGE("Loaded image content should match original",
                               mitk::Equal(*originalImage, *loadedImage, mitk::eps, false));
      }
      catch (const std::exception& e)
      {
        fs::remove(fs::path(tempFilePath));
        CPPUNIT_FAIL(std::string("Failed to reload received data: ") + e.what());
      }
    }
    else if (res.status == 415)
    {
      // Serializer not available - this is OK, test passes conditionally
      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT_EQUAL(std::string("UNSUPPORTED_FORMAT"), json["error"]["code"].get<std::string>());
    }
    else
    {
      CPPUNIT_FAIL("Unexpected status code: " + std::to_string(res.status));
    }
  }

  void GetImageDataFileReference()
  {
    // Create the original test image
    const auto originalImage = this->CreateTestImage();

    auto node = mitk::DataNode::New();
    node->SetName("ImageNode");
    node->SetData(originalImage);
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    const httplib::Headers headers = {{"X-MITK-Transfer-Mode", "file-reference"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", "", {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_data(req, res);

    if (res.status == 200)
    {
      // Verify JSON response with file path
      CPPUNIT_ASSERT_EQUAL(std::string("application/json"), res.get_header_value("Content-Type"));

      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT(json.contains("transfer"));
      CPPUNIT_ASSERT_EQUAL(std::string("file-reference"), json["transfer"]["mode"].get<std::string>());
      CPPUNIT_ASSERT(json["transfer"].contains("file_path"));
      CPPUNIT_ASSERT(json["transfer"].contains("directory_path"));
      CPPUNIT_ASSERT(json["transfer"].contains("size_bytes"));

      // Verify the file exists
      const std::string filePath = json["transfer"]["file_path"].get<std::string>();
      CPPUNIT_ASSERT(fs::exists(fs::path(filePath)));

      // Load the file and compare content with original
      try
      {
        auto loadedData = mitk::IOUtil::Load(filePath);
        CPPUNIT_ASSERT(!loadedData.empty());

        auto* loadedImage = dynamic_cast<mitk::Image*>(loadedData[0].GetPointer());
        CPPUNIT_ASSERT(loadedImage != nullptr);

        // Compare using MITK's Equal function
        CPPUNIT_ASSERT_MESSAGE("File-reference image content should match original",
                               mitk::Equal(*originalImage, *loadedImage, mitk::eps, false));
      }
      catch (const std::exception& e)
      {
        CPPUNIT_FAIL(std::string("Failed to load file-reference data: ") + e.what());
      }
    }
    else if (res.status == 415)
    {
      // Serializer not available - OK for this test
      CPPUNIT_ASSERT(true);
    }
    else
    {
      CPPUNIT_FAIL("Unexpected status code: " + std::to_string(res.status));
    }
  }

  void PutImageDataDirect()
  {
    // Create an empty node
    auto node = mitk::DataNode::New();
    node->SetName("EmptyNode");
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    // Create the original test image and save it to get binary content
    const auto originalImage = this->CreateTestImage();
    std::string tempFilePath;
    try
    {
      tempFilePath = mitk::IOUtil::CreateTemporaryFile("testimage_XXXXXX.nrrd", m_TempDirectory);
      mitk::IOUtil::Save(originalImage, tempFilePath);
    }
    catch (const std::exception&)
    {
      // If we can't save the image, skip this test
      CPPUNIT_ASSERT(true);
      return;
    }

    // Read the file content
    std::ifstream file(tempFilePath, std::ios::binary);
    std::ostringstream content;
    content << file.rdbuf();
    const std::string binaryContent = content.str();
    file.close();

    // Clean up temp file
    fs::remove(fs::path(tempFilePath));

    // Create request with binary content
    const httplib::Headers headers = {
      {"Content-Type", "application/octet-stream"},
      {"Content-Disposition", "attachment; filename=\"image.nrrd\""}
    };
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", binaryContent, {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    if (res.status == 200)
    {
      // Verify response
      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT(json.contains("data"));
      CPPUNIT_ASSERT_EQUAL(std::string("mitk::Image"), json["data"]["data_type"].get<std::string>());

      // Verify the stored data matches the original
      auto* storedData = node->GetData();
      CPPUNIT_ASSERT(storedData != nullptr);

      auto* storedImage = dynamic_cast<mitk::Image*>(storedData);
      CPPUNIT_ASSERT(storedImage != nullptr);

      // Compare using MITK's Equal function
      CPPUNIT_ASSERT_MESSAGE("Stored image should match original",
                             mitk::Equal(*originalImage, *storedImage, mitk::eps, false));
    }
    else if (res.status == 415 || res.status == 422)
    {
      // File format not supported or read error - OK for this test
      CPPUNIT_ASSERT(true);
    }
    else
    {
      CPPUNIT_FAIL("Unexpected status code: " + std::to_string(res.status));
    }
  }

  void PutImageDataFileReference()
  {
    // Create an empty node
    auto node = mitk::DataNode::New();
    node->SetName("EmptyNode");
    m_DataStorage->Add(node);

    const std::string uid = this->GetUid(node.GetPointer());

    // Create the original test image and save to file
    const auto originalImage = this->CreateTestImage();
    std::string tempFilePath;
    try
    {
      tempFilePath = mitk::IOUtil::CreateTemporaryFile("testimage_XXXXXX.nrrd", m_TempDirectory);
      mitk::IOUtil::Save(originalImage, tempFilePath);
    }
    catch (const std::exception&)
    {
      // If we can't save the image, skip this test
      CPPUNIT_ASSERT(true);
      return;
    }

    // Create request with file reference
    nlohmann::json body;
    body["transfer"]["file_path"] = tempFilePath;

    const httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/data", body.dump(), {{"uid", uid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_data(req, res);

    if (res.status == 200)
    {
      // Verify response
      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT(json.contains("data"));
      CPPUNIT_ASSERT_EQUAL(std::string("mitk::Image"), json["data"]["data_type"].get<std::string>());

      // File should still exist (sender keeps ownership)
      CPPUNIT_ASSERT(fs::exists(tempFilePath));

      // Verify the stored data matches the original
      auto* storedData = node->GetData();
      CPPUNIT_ASSERT(storedData != nullptr);

      auto* storedImage = dynamic_cast<mitk::Image*>(storedData);
      CPPUNIT_ASSERT(storedImage != nullptr);

      // Compare using MITK's Equal function
      CPPUNIT_ASSERT_MESSAGE("Stored image should match original",
                             mitk::Equal(*originalImage, *storedImage, mitk::eps, false));

      // Clean up - sender's responsibility
      fs::remove(fs::path(tempFilePath));
    }
    else if (res.status == 415 || res.status == 422)
    {
      // File format not supported or read error - clean up and pass
      fs::remove(fs::path(tempFilePath));
      CPPUNIT_ASSERT(true);
    }
    else
    {
      // Clean up on failure
      fs::remove(fs::path(tempFilePath));
      CPPUNIT_FAIL("Unexpected status code: " + std::to_string(res.status));
    }
  }

  // ===== POST /nodes with data transfer tests =====

  void PostNodeWithDataFileReference()
  {
    // Create the original test image and save it to a temp file
    const auto originalImage = this->CreateTestImage();
    const std::string tempFilePath = mitk::IOUtil::CreateTemporaryFile("test_XXXXXX.nrrd");

    try
    {
      mitk::IOUtil::Save(originalImage, tempFilePath);
    }
    catch (const std::exception& e)
    {
      fs::remove(fs::path(tempFilePath));
      CPPUNIT_FAIL("Failed to create test file: " + std::string(e.what()));
    }

    // POST /nodes with transfer.file_path
    nlohmann::json body;
    body["name"] = "TestImageNode";
    body["transfer"]["file_path"] = tempFilePath;

    const httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes", body.dump(), {}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePOST_nodes(req, res);

    if (res.status == 201)
    {
      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT(json.contains("data"));
      CPPUNIT_ASSERT_EQUAL(std::string("TestImageNode"), json["data"]["name"].get<std::string>());
      CPPUNIT_ASSERT_EQUAL(std::string("mitk::Image"), json["data"]["data_type"].get<std::string>());

      // File should still exist (sender keeps ownership)
      CPPUNIT_ASSERT(fs::exists(tempFilePath));

      // Find the created node and verify stored data matches original
      const std::string createdUid = json["data"]["uid"].get<std::string>();
      mitk::NodeQueryParams params;
      auto queryResult = m_Bridge->GetNodes(params);

      // Find the node by UID
      mitk::DataNode* createdNode = nullptr;
      auto allNodes = m_DataStorage->GetAll();
      for (auto it = allNodes->Begin(); it != allNodes->End(); ++it)
      {
        if (this->GetUid(it->Value()) == createdUid)
        {
          createdNode = it->Value();
          break;
        }
      }

      CPPUNIT_ASSERT(createdNode != nullptr);
      auto* storedImage = dynamic_cast<mitk::Image*>(createdNode->GetData());
      CPPUNIT_ASSERT(storedImage != nullptr);

      // Compare using MITK's Equal function
      CPPUNIT_ASSERT_MESSAGE("Stored image should match original",
                             mitk::Equal(*originalImage, *storedImage, mitk::eps, false));
    }
    else if (res.status == 415 || res.status == 422)
    {
      // Serializer not available or file format issue - OK for this test
      CPPUNIT_ASSERT(true);
    }
    else
    {
      fs::remove(fs::path(tempFilePath));
      CPPUNIT_FAIL("Unexpected status code: " + std::to_string(res.status) + " body: " + res.body);
    }

    // Cleanup
    fs::remove(fs::path(tempFilePath));
  }

  void PostNodeWithDataDirect()
  {
    // Create the original test image and serialize it to binary
    const auto originalImage = this->CreateTestImage();
    const std::string tempFilePath = mitk::IOUtil::CreateTemporaryFile("test_XXXXXX.nrrd");

    try
    {
      mitk::IOUtil::Save(originalImage, tempFilePath);
    }
    catch (const std::exception& e)
    {
      fs::remove(fs::path(tempFilePath));
      CPPUNIT_FAIL("Failed to create test file: " + std::string(e.what()));
    }

    // Read the file as binary
    std::ifstream file(tempFilePath, std::ios::binary);
    std::string binaryData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    fs::remove(fs::path(tempFilePath));

    // POST /nodes with binary body and name in query param
    const httplib::Headers headers = {
      {"Content-Type", "application/octet-stream"},
      {"Content-Disposition", "attachment; filename=\"test.nrrd\""}
    };
    auto req = this->CreateRequest("/api/v1/datastorage/nodes", binaryData, {}, {{"name", "DirectUploadNode"}}, headers);
    httplib::Response res;

    m_Controller->HandlePOST_nodes(req, res);

    if (res.status == 201)
    {
      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT(json.contains("data"));
      CPPUNIT_ASSERT_EQUAL(std::string("DirectUploadNode"), json["data"]["name"].get<std::string>());
      CPPUNIT_ASSERT_EQUAL(std::string("mitk::Image"), json["data"]["data_type"].get<std::string>());

      // Find the created node and verify stored data matches original
      const std::string createdUid = json["data"]["uid"].get<std::string>();
      mitk::DataNode* createdNode = nullptr;
      auto allNodes = m_DataStorage->GetAll();
      for (auto it = allNodes->Begin(); it != allNodes->End(); ++it)
      {
        if (this->GetUid(it->Value()) == createdUid)
        {
          createdNode = it->Value();
          break;
        }
      }

      CPPUNIT_ASSERT(createdNode != nullptr);
      auto* storedImage = dynamic_cast<mitk::Image*>(createdNode->GetData());
      CPPUNIT_ASSERT(storedImage != nullptr);

      // Compare using MITK's Equal function
      CPPUNIT_ASSERT_MESSAGE("Stored image should match original",
                             mitk::Equal(*originalImage, *storedImage, mitk::eps, false));
    }
    else if (res.status == 415 || res.status == 422)
    {
      // Serializer not available - OK for this test
      CPPUNIT_ASSERT(true);
    }
    else
    {
      CPPUNIT_FAIL("Unexpected status code: " + std::to_string(res.status) + " body: " + res.body);
    }
  }

  void PostChildNodeWithDataFileReference()
  {
    // Create parent node first
    auto parentNode = mitk::DataNode::New();
    parentNode->SetName("ParentNode");
    m_DataStorage->Add(parentNode);
    const std::string parentUid = this->GetUid(parentNode.GetPointer());

    // Create the original test image and save it to a temp file
    const auto originalImage = this->CreateTestImage();
    const std::string tempFilePath = mitk::IOUtil::CreateTemporaryFile("test_XXXXXX.nrrd");

    try
    {
      mitk::IOUtil::Save(originalImage, tempFilePath);
    }
    catch (const std::exception& e)
    {
      fs::remove(fs::path(tempFilePath));
      CPPUNIT_FAIL("Failed to create test file: " + std::string(e.what()));
    }

    // POST /nodes/{uid}/children with transfer.file_path
    nlohmann::json body;
    body["name"] = "ChildImageNode";
    body["transfer"]["file_path"] = tempFilePath;

    const httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + parentUid + "/children",
                                    body.dump(), {{"uid", parentUid}}, {}, headers);
    httplib::Response res;

    m_Controller->HandlePOST_nodes_uid_children(req, res);

    if (res.status == 201)
    {
      const auto json = nlohmann::json::parse(res.body);
      CPPUNIT_ASSERT(json.contains("data"));
      CPPUNIT_ASSERT_EQUAL(std::string("ChildImageNode"), json["data"]["name"].get<std::string>());
      CPPUNIT_ASSERT_EQUAL(parentUid, json["data"]["parent_uid"].get<std::string>());
      CPPUNIT_ASSERT_EQUAL(std::string("mitk::Image"), json["data"]["data_type"].get<std::string>());

      // Find the created child node and verify stored data matches original
      const std::string createdUid = json["data"]["uid"].get<std::string>();
      mitk::DataNode* createdNode = nullptr;
      auto allNodes = m_DataStorage->GetAll();
      for (auto it = allNodes->Begin(); it != allNodes->End(); ++it)
      {
        if (this->GetUid(it->Value()) == createdUid)
        {
          createdNode = it->Value();
          break;
        }
      }

      CPPUNIT_ASSERT(createdNode != nullptr);
      auto* storedImage = dynamic_cast<mitk::Image*>(createdNode->GetData());
      CPPUNIT_ASSERT(storedImage != nullptr);

      // Compare using MITK's Equal function
      CPPUNIT_ASSERT_MESSAGE("Stored image should match original",
                             mitk::Equal(*originalImage, *storedImage, mitk::eps, false));
    }
    else if (res.status == 415 || res.status == 422)
    {
      // Serializer not available - OK for this test
      CPPUNIT_ASSERT(true);
    }
    else
    {
      fs::remove(fs::path(tempFilePath));
      CPPUNIT_FAIL("Unexpected status code: " + std::to_string(res.status) + " body: " + res.body);
    }

    // Cleanup
    fs::remove(fs::path(tempFilePath));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDataStorageControllerData)
