/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFileSystem.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkImage.h>
#include <mitkMessage.h>
#include <mitkPropertyList.h>
#include <mitkSceneJsonReader.h>
#include <mitkStandaloneDataStorage.h>

#include <string>
#include <vector>

namespace
{
  /**
   * \brief Listener that records the DataNodes added to a DataStorage in
   *        the order `AddNodeEvent` fires, for layer-ordering assertions.
   *
   * Stores raw pointers rather than names: SceneJsonReader fires
   * AddNodeEvent (via `storage->Add`) before it applies the node's
   * `properties` map, so `GetName()` would be empty at event time. The
   * test resolves names after the load completes, when the name property
   * is in place.
   */
  class AddOrderRecorder
  {
  public:
    void OnAdd(const mitk::DataNode *node)
    {
      if (node != nullptr)
      {
        m_Order.push_back(node);
      }
    }

    const std::vector<const mitk::DataNode *> &Order() const { return m_Order; }

  private:
    std::vector<const mitk::DataNode *> m_Order;
  };
}

/**
 * \brief Integration tests for mitk::SceneJsonReader.
 *
 * All tests load static scene fixtures from
 * `<MITK_DATA_DIR>/SceneSerialization/Json/` via `GetTestDataFilePath()`.
 */
class mitkSceneJsonReaderIntegrationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSceneJsonReaderIntegrationTestSuite);
  MITK_TEST(TestLoadRealImage);
  MITK_TEST(TestDataPropertiesReplace);
  MITK_TEST(TestFileIndirection);
  MITK_TEST(TestRelativePathSubdir);
  MITK_TEST(TestLayerOrdering);
  MITK_TEST(TestDataTypeMismatch);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string Fixture(const std::string &relative) const
  {
    return GetTestDataFilePath(std::string("SceneSerialization/Json/") + relative);
  }

public:
  void setUp() override {}
  void tearDown() override {}

  /**
   * \brief 1. Load an actual image via `transfer.file_path`.
   *
   * Fixture: `SceneSerialization/Json/basic.mitkscene.json`.
   */
  void TestLoadRealImage()
  {
    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_MESSAGE("basic.mitkscene.json loads without non-fatal errors",
                           reader->LoadScene(Fixture("basic.mitkscene.json"), storage));

    auto node = storage->GetNamedNode("loaded");
    CPPUNIT_ASSERT_MESSAGE("Node 'loaded' present in storage", node != nullptr);
    auto *image = dynamic_cast<mitk::Image *>(node->GetData());
    CPPUNIT_ASSERT_MESSAGE("Loaded BaseData is an mitk::Image", image != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Image has at least two dimensions", image->GetDimension() >= 2);
  }

  /**
   * \brief 2. `data_properties` with `_loadstyle: replace` leaves only the
   *           author-declared key on the loaded BaseData's property list.
   */
  void TestDataPropertiesReplace()
  {
    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_MESSAGE("data-properties-replace scene loads",
                           reader->LoadScene(Fixture("data-properties-replace.mitkscene.json"), storage));

    auto node = storage->GetNamedNode("replaced");
    CPPUNIT_ASSERT_MESSAGE("Node 'replaced' carries a BaseData",
                           node != nullptr && node->GetData() != nullptr);
    auto dataProps = node->GetData()->GetPropertyList();
    CPPUNIT_ASSERT_MESSAGE("BaseData has a property list", dataProps != nullptr);

    std::string marker;
    CPPUNIT_ASSERT_MESSAGE("Author marker applied to BaseData",
                           dataProps->GetStringProperty("author.marker", marker) && marker == "ok");
    CPPUNIT_ASSERT_MESSAGE("BaseData property list contains only the marker after 'replace'",
                           dataProps->GetMap()->size() == 1);
  }

  /**
   * \brief 3. `_file` indirection resolves a sibling JSON file and applies
   *           its keys to the node.
   */
  void TestFileIndirection()
  {
    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_MESSAGE("file-indirection scene loads",
                           reader->LoadScene(Fixture("file-indirection/scene.mitkscene.json"), storage));

    auto node = storage->GetNamedNode("from-file");
    CPPUNIT_ASSERT_MESSAGE("Node named by the external property map is present", node != nullptr);

    std::string tag;
    CPPUNIT_ASSERT_MESSAGE("Key from the external property map applied to the node",
                           node->GetStringProperty("author.tag", tag) && tag == "external");
  }

  /**
   * \brief 4. A scene file in a subdirectory resolves `file_path` against
   *           its own parent directory, not against any fixed root.
   */
  void TestRelativePathSubdir()
  {
    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_MESSAGE("Scene nested in a subdirectory loads with a relative file_path",
                           reader->LoadScene(Fixture("subdir-relative/sub/scene.mitkscene.json"), storage));

    auto node = storage->GetNamedNode("relative");
    CPPUNIT_ASSERT_MESSAGE("Image reached via ../-relative file_path from a subdirectory",
                           node != nullptr && dynamic_cast<mitk::Image *>(node->GetData()) != nullptr);
  }

  /**
   * \brief 5. Layer ordering across multiple data-bearing siblings.
   */
  void TestLayerOrdering()
  {
    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();

    AddOrderRecorder recorder;
    auto listener = mitk::MessageDelegate1<AddOrderRecorder, const mitk::DataNode *>(
      &recorder, &AddOrderRecorder::OnAdd);
    storage->AddNodeEvent.AddListener(listener);

    const bool ok = reader->LoadScene(Fixture("layer-ordering.mitkscene.json"), storage);
    storage->AddNodeEvent.RemoveListener(listener);
    CPPUNIT_ASSERT_MESSAGE("layer-ordering scene loads", ok);

    const auto &order = recorder.Order();
    CPPUNIT_ASSERT_MESSAGE("Three nodes were added", order.size() == 3);
    std::vector<std::string> names;
    names.reserve(order.size());
    for (const auto *n : order)
    {
      names.push_back(n->GetName());
    }
    CPPUNIT_ASSERT_MESSAGE("Nodes added in ascending-layer order (bot, mid, top)",
                           names[0] == "bot" && names[1] == "mid" && names[2] == "top");
  }

  /**
   * \brief 6. `data_type` mismatch produces a non-fatal warning; the real
   *           class wins.
   */
  void TestDataTypeMismatch()
  {
    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();

    const bool ok = reader->LoadScene(Fixture("data-type-mismatch.mitkscene.json"), storage);
    CPPUNIT_ASSERT_MESSAGE("data_type mismatch reports non-fatal errors (returns false)", !ok);

    auto node = storage->GetNamedNode("misdeclared");
    CPPUNIT_ASSERT_MESSAGE("Node is present after the non-fatal warning",
                           node != nullptr && node->GetData() != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Loaded class follows IOUtil::Load, not the declared data_type",
                           std::string(node->GetData()->GetNameOfClass()) == "Image");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSceneJsonReaderIntegration)
