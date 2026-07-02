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
  MITK_TEST(TestTransientPropertyStrippedOnLoad);
  MITK_TEST(TestParentChildRelationship);
  MITK_TEST(TestEmptyContextKeyRejected);
  MITK_TEST(TestNullContextKeyRejected);
  MITK_TEST(TestTransferRequiresFilePath);
  MITK_TEST(TestTransferRejectsUnknownMode);
  MITK_TEST(TestOrphanDataPropertiesWarnedNonFatal);
  MITK_TEST(TestLayerAcceptsPrimitiveAndTaggedForms);
  MITK_TEST(TestResolvePathEscapeIsNonFatal);
  MITK_TEST(TestMalformedPropertyValueDowngraded);
  MITK_TEST(TestEmptyNodesArrayLoadsCleanly);
  MITK_TEST(TestUnicodeUidPropertyKeyAndValue);
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

  void TestTransientPropertyStrippedOnLoad()
  {
    // 'selected' is registered transient for any data type and must be dropped
    // on load, from both the default and named (context) property lists, so a
    // reloaded scene does not resurrect UI state. Mirrors SceneReaderV1 (XML).
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"properties":{"name":"n","selected":true},
       "context_properties":{"stdmulti.widget0":{"selected":true}}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT(reader->LoadScene(file.Path(), storage));

    auto node = storage->GetNamedNode("n");
    CPPUNIT_ASSERT(node != nullptr);

    bool selected = false;
    CPPUNIT_ASSERT_MESSAGE("Transient 'selected' must not be applied from the default list",
                           !node->GetBoolProperty("selected", selected) || !selected);

    auto *ctxList = node->GetPropertyList("stdmulti.widget0");
    CPPUNIT_ASSERT(ctxList != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Transient 'selected' must not be applied from a context list",
                           ctxList->GetProperty("selected") == nullptr);
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

  void TestEmptyContextKeyRejected()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"context_properties":{"":{"opacity":0.5}}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestNullContextKeyRejected()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"context_properties":{"null":{"opacity":0.5}}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestTransferRequiresFilePath()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"transfer":{"mode":"file-reference"}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestTransferRejectsUnknownMode()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"transfer":{"mode":"shared-memory","file_path":"does-not-matter.nrrd"}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT_THROW(reader->LoadScene(file.Path(), storage), mitk::Exception);
  }

  void TestLayerAcceptsPrimitiveAndTaggedForms()
  {
    // Layer is read pre-Add to determine wave ordering. Both the primitive
    // form ("layer": 2) and the tagged IntProperty form must produce the
    // same ordering, otherwise authors using the shorthand would get
    // non-deterministic stacking.
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"uid":"a","properties":{"name":"primitive","layer":5}},
      {"uid":"b","properties":{"name":"tagged","layer":{"type":"IntProperty","value":1}}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT(reader->LoadScene(file.Path(), storage));
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), storage->GetAll()->Size());

    auto primitive = storage->GetNamedNode("primitive");
    auto tagged = storage->GetNamedNode("tagged");
    CPPUNIT_ASSERT(primitive != nullptr);
    CPPUNIT_ASSERT(tagged != nullptr);

    int layer = 0;
    CPPUNIT_ASSERT(primitive->GetIntProperty("layer", layer));
    CPPUNIT_ASSERT_EQUAL(5, layer);
    CPPUNIT_ASSERT(tagged->GetIntProperty("layer", layer));
    CPPUNIT_ASSERT_EQUAL(1, layer);
  }

  void TestOrphanDataPropertiesWarnedNonFatal()
  {
    // data_properties on a node without 'transfer' is an orphan: ignored,
    // warned, and the overall load reports non-fatal errors (returns false).
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"data_properties":{"modality":"CT"},"properties":{"name":"orphan"}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    const bool ok = reader->LoadScene(file.Path(), storage);
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), storage->GetAll()->Size());
    CPPUNIT_ASSERT_EQUAL(std::string("orphan"), storage->GetAll()->GetElement(0)->GetName());
  }

  void TestResolvePathEscapeIsNonFatal()
  {
    // A property-map `_file` that escapes the scene directory via `..` exercises
    // the ResolvePath escape branch (warning emitted, load proceeds). The
    // missing target file is then a non-fatal per-node error: the node is added,
    // the overall load returns false. The warning itself is not asserted (no
    // log capture facility); this test only asserts the observable side
    // effects of routing through ResolvePath's escape branch.
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"uid":"escapes","properties":{"_file":"../no-such-external-map.json"}}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    const bool ok = reader->LoadScene(file.Path(), storage);
    CPPUNIT_ASSERT_MESSAGE("load reports non-fatal errors (missing _file target)", !ok);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), storage->GetAll()->Size());
  }

  void TestMalformedPropertyValueDowngraded()
  {
    // A malformed property value (here: a ColorProperty whose 'value' array is
    // truncated) is not catchable by structural pre-validation; it surfaces
    // inside ConvertPropertyFromSelfContainedJson during Pass 2. The reader
    // must downgrade this to a per-node non-fatal error so storage is never
    // left partially populated after storage->Add.
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[
      {"uid":"bad-color",
       "properties":{
         "color":{"type":"ColorProperty","value":[1.0,0.0]}
       }}
    ]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    const bool ok = reader->LoadScene(file.Path(), storage);
    CPPUNIT_ASSERT_MESSAGE("malformed property value downgrades to non-fatal", !ok);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), storage->GetAll()->Size());
    // Note: no assertion on which sibling properties were applied. The bad key
    // aborts the property-map iteration; whether a co-listed property is
    // applied depends on iteration order and is not part of the contract.
  }

  void TestEmptyNodesArrayLoadsCleanly()
  {
    TempFile file(".mitkscene.json");
    file.Write(R"({"type":"org.mitk.scene","version":1,"nodes":[]})");

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT(reader->LoadScene(file.Path(), storage));
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), storage->GetAll()->Size());
  }

  void TestUnicodeUidPropertyKeyAndValue()
  {
    // UTF-8 must round-trip through node uids, parent_uid references, property
    // keys and string-property values. Bytes are written as explicit hex
    // escapes in plain (non-raw) string literals so the test compiles
    // identically under both C++17 (where u8"" is char[]) and C++20 (where
    // u8"" is char8_t[] and no longer converts to std::string), and is
    // independent of the compiler's source/execution charset.
    //
    //   ä = 0xC3 0xA4  ö = 0xC3 0xB6  ü = 0xC3 0xBC  ß = 0xC3 0x9F
    //
    // Note on hex escapes: in C++, \xHH... consumes every following hex digit,
    // so "\xC3\xBCber" would be read as \xC3 + \xBCbe + r (and \xBCbe is out of
    // range for char). Wherever a [0-9a-fA-F] character follows a 2-digit \xHH
    // sequence, the literal is split with `" "` to terminate the escape.
    const std::string sceneJson =
      "{\"type\":\"org.mitk.scene\",\"version\":1,\"nodes\":["
        "{\"uid\":\"node-\xC3\xA4hnlich\","
         "\"properties\":{"
           "\"name\":\"Patient \xC3\xA4 \xC3\xB6 \xC3\xBC\","
           "\"\xC3\xBC" "ber\":\"Wert \xC3\x9F\""
         "}},"
        "{\"uid\":\"child\",\"parent_uid\":\"node-\xC3\xA4hnlich\","
         "\"properties\":{\"name\":\"Kind\"}}"
      "]}";

    TempFile file(".mitkscene.json");
    file.Write(sceneJson);

    auto reader = mitk::SceneJsonReader::New();
    auto storage = mitk::StandaloneDataStorage::New();
    CPPUNIT_ASSERT(reader->LoadScene(file.Path(), storage));
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), storage->GetAll()->Size());

    auto parent = storage->GetNamedNode(std::string("Patient \xC3\xA4 \xC3\xB6 \xC3\xBC"));
    CPPUNIT_ASSERT_MESSAGE("parent located by Unicode name", parent != nullptr);

    std::string ueber;
    CPPUNIT_ASSERT_MESSAGE("Unicode property key holds Unicode value",
                           parent->GetStringProperty("\xC3\xBC" "ber", ueber));
    CPPUNIT_ASSERT_EQUAL(std::string("Wert \xC3\x9F"), ueber);

    auto child = storage->GetNamedNode("Kind");
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
    CPPUNIT_ASSERT_MESSAGE("parent_uid resolved across the Unicode UID", foundParent);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSceneJsonReader)
