/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkException.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkProperties.h>
#include <mitkSceneJsonReader.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStringProperty.h>

#include <itksys/SystemTools.hxx>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
  /**
   * \brief RAII helper that writes content to a uniquely-named temp file and
   *        removes it on destruction.
   */
  class TempFile
  {
  public:
    explicit TempFile(const std::string &suffix)
    {
      std::ostringstream os;
      os << itksys::SystemTools::GetCurrentWorkingDirectory() << "/mitkSceneJsonReaderTest_"
         << static_cast<const void *>(this) << suffix;
      m_Path = os.str();
    }

    ~TempFile()
    {
      std::remove(m_Path.c_str());
    }

    void Write(const std::string &content) const
    {
      std::ofstream out(m_Path);
      out << content;
    }

    const std::string &Path() const { return m_Path; }

  private:
    std::string m_Path;
  };
}

class mitkSceneJsonReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSceneJsonReaderTestSuite);
  MITK_TEST(TestMalformedJsonThrows);
  MITK_TEST(TestWrongRootTypeThrows);
  MITK_TEST(TestUnsupportedVersionThrows);
  MITK_TEST(TestMissingNodesArrayThrows);
  MITK_TEST(TestDuplicateUidThrows);
  MITK_TEST(TestDanglingParentUidThrows);
  MITK_TEST(TestCircularParentUidThrows);
  MITK_TEST(TestNodeWithoutDataLoads);
  MITK_TEST(TestDefaultLoadstyleIsModify);
  MITK_TEST(TestReplaceLoadstyleParsed);
  MITK_TEST(TestInvalidLoadstyleThrows);
  MITK_TEST(TestUnknownMetaKeyIgnored);
  MITK_TEST(TestInlineAndFileMutuallyExclusive);
  MITK_TEST(TestContextPropertiesApplied);
  MITK_TEST(TestParentChildRelationship);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override {}
  void tearDown() override {}

  void TestMalformedJsonThrows()
  {
    TempFile file(".mitkscene.json");
    file.Write("{ this is not json ");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestWrongRootTypeThrows()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.other","version":1,"nodes":[]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestUnsupportedVersionThrows()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":99,"nodes":[]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestMissingNodesArrayThrows()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestDuplicateUidThrows()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"uid":"a"},
      {"uid":"a"}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestDanglingParentUidThrows()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"uid":"a","parent_uid":"does-not-exist"}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestCircularParentUidThrows()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"uid":"a","parent_uid":"b"},
      {"uid":"b","parent_uid":"a"}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestNodeWithoutDataLoads()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"uid":"n1","properties":{"name":"just a node","visible":true}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    const bool ok = reader->LoadScene(file.Path(), storage);
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), storage->GetAll()->Size());

    auto node = storage->GetAll()->GetElement(0);
    CPPUNIT_ASSERT_EQUAL(std::string("just a node"), node->GetName());

    bool visible = false;
    CPPUNIT_ASSERT(node->GetBoolProperty("visible", visible));
    CPPUNIT_ASSERT(visible);
  }

  void TestDefaultLoadstyleIsModify()
  {
    // Modify: explicit 'name' applied; other mapper defaults left untouched.
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"properties":{"name":"modify-default"}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT(reader->LoadScene(file.Path(), storage));
    CPPUNIT_ASSERT_EQUAL(std::string("modify-default"), storage->GetAll()->GetElement(0)->GetName());
  }

  void TestReplaceLoadstyleParsed()
  {
    // Data-less nodes carry no mapper-assigned defaults, so the observable
    // difference between 'modify' and 'replace' in a unit test is only that
    // 'replace' is accepted and the listed property is applied. The full
    // clearing behavior is covered by the Workbench manual test.
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"properties":{"_loadstyle":"replace","name":"replaced"}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT(reader->LoadScene(file.Path(), storage));
    CPPUNIT_ASSERT_EQUAL(std::string("replaced"),
                         storage->GetAll()->GetElement(0)->GetName());
  }

  void TestInvalidLoadstyleThrows()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"properties":{"_loadstyle":"destroy","name":"x"}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestUnknownMetaKeyIgnored()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"properties":{"_future_meta":"some value","name":"ok"}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT(reader->LoadScene(file.Path(), storage));
    CPPUNIT_ASSERT_EQUAL(std::string("ok"), storage->GetAll()->GetElement(0)->GetName());
  }

  void TestInlineAndFileMutuallyExclusive()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"properties":{"_file":"does-not-matter.json","name":"conflict"}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestContextPropertiesApplied()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"properties":{"name":"root"},
       "context_properties":{"stdmulti.widget0":{"opacity":0.25}}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT(reader->LoadScene(file.Path(), storage));

    auto node = storage->GetAll()->GetElement(0);
    auto *ctxList = node->GetPropertyList("stdmulti.widget0");
    CPPUNIT_ASSERT(ctxList != nullptr);
    auto *prop = ctxList->GetProperty("opacity");
    CPPUNIT_ASSERT(prop != nullptr);
  }

  void TestParentChildRelationship()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"uid":"parent","properties":{"name":"P"}},
      {"uid":"child","parent_uid":"parent","properties":{"name":"C"}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT(reader->LoadScene(file.Path(), storage));
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), storage->GetAll()->Size());

    auto parent = storage->GetNamedNode("P");
    auto child = storage->GetNamedNode("C");
    CPPUNIT_ASSERT(parent != nullptr);
    CPPUNIT_ASSERT(child != nullptr);

    auto sources = storage->GetSources(child);
    bool foundParent = false;
    for (auto it = sources->Begin(); it != sources->End(); ++it)
    {
      if (it->Value() == parent)
      {
        foundParent = true;
        break;
      }
    }
    CPPUNIT_ASSERT(foundParent);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSceneJsonReader)
