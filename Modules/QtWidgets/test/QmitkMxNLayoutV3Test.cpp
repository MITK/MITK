/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNMultiWidget.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <nlohmann/json.hpp>

/**
 * Tests the v3 layout format on QmitkMxNMultiWidget:
 *   - The loader accepts "2.0" and "3.0" in one code path; a v2 document
 *     keeps its lenient treatment of unknown link keys while a "3.0"
 *     document has a closed `links` object (unknown keys, unknown
 *     modifiers, and misplaced or mistyped offsets throw).
 *   - All eight link dimensions parse in string and object form and
 *     round-trip through SerializeLayout / ApplyLayout, including the
 *     slice / zoom / pan offsets.
 *   - Strict mode requires every referenced group declared, across all
 *     dimensions; lazy mode does not.
 */
class QmitkMxNLayoutV3TestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNLayoutV3TestSuite);

  MITK_TEST(V2Doc_UnknownLinkKey_Tolerated);
  MITK_TEST(V3Doc_UnknownLinkKey_Throws);
  MITK_TEST(V3Doc_MisplacedOffset_Throws);
  MITK_TEST(V3Doc_UnknownModifier_Throws);
  MITK_TEST(V3Doc_MistypedOffset_Throws);
  MITK_TEST(V3Doc_SelectionObjectForm_OK);
  MITK_TEST(V3_RoundTrip_AllDimensionsAndOffsets);
  MITK_TEST(V3_StrictMode_NavGroupUndeclared_Throws);
  MITK_TEST(V3_LazyMode_NavLinks_OK);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Node1;
  mitk::DataNode::Pointer m_Node2;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    // QmitkRenderWindowDataNodeTableModel comparator workaround (see
    // QmitkMxNSyncGroupApiTest for the long-form rationale).
    m_Node1 = mitk::DataNode::New();
    m_Node1->SetName("node1");
    m_Node1->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_Node1);

    m_Node2 = mitk::DataNode::New();
    m_Node2->SetName("node2");
    m_Node2->SetIntProperty("layer", 1);
    m_DataStorage->Add(m_Node2);
  }

  void tearDown() override
  {
    m_Node1 = nullptr;
    m_Node2 = nullptr;
    m_DataStorage = nullptr;
  }

  /** Make a usable, initialized MxN editor wired to the test data storage. */
  std::unique_ptr<QmitkMxNMultiWidget> MakeEditor()
  {
    auto editor = std::make_unique<QmitkMxNMultiWidget>();
    editor->SetDataStorage(m_DataStorage);
    editor->InitializeMultiWidget();
    return editor;
  }

  /** Single-window document skeleton; callers patch version / links / groups. */
  static nlohmann::json SingleWindowDoc()
  {
    return nlohmann::json::parse(R"json({
      "version": "3.0",
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__w0", "view_direction": "axial",
            "links": { "selection": "main" } }
        ]
      }
    })json");
  }

  static nlohmann::json& WindowLinks(nlohmann::json& doc, std::size_t childIndex = 0)
  {
    return doc["root"]["children"][childIndex]["links"];
  }

  void V2Doc_UnknownLinkKey_Tolerated()
  {
    auto doc = SingleWindowDoc();
    doc["version"] = "2.0";
    WindowLinks(doc)["slize"] = "nav";  // typo'd key: v2 files must keep loading

    auto editor = MakeEditor();
    editor->ApplyLayout(doc);
    CPPUNIT_ASSERT_EQUAL(1u, editor->GetNumberOfRenderWindowWidgets());
    CPPUNIT_ASSERT_MESSAGE("The ignored key must not materialize as a link",
      !editor->GetSyncLink("mxn__w0", QmitkMxNSyncDimension::Slice).has_value());
  }

  void V3Doc_UnknownLinkKey_Throws()
  {
    auto doc = SingleWindowDoc();
    WindowLinks(doc)["slize"] = "nav";

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(doc), mitk::Exception);
  }

  void V3Doc_MisplacedOffset_Throws()
  {
    // Crosshair accepts no offset (absolute propagation), nor does selection.
    for (const auto* dimension : { "crosshair", "orientation", "windowing", "lut", "selection" })
    {
      auto doc = SingleWindowDoc();
      WindowLinks(doc)[dimension] = nlohmann::json{ { "target", "nav" }, { "offset", 1 } };

      auto editor = MakeEditor();
      CPPUNIT_ASSERT_THROW_MESSAGE(std::string("offset on '") + dimension + "' must throw",
                                   editor->ApplyLayout(doc), mitk::Exception);
    }
  }

  void V3Doc_UnknownModifier_Throws()
  {
    auto doc = SingleWindowDoc();
    WindowLinks(doc)["slice"] = nlohmann::json{ { "target", "nav" }, { "mirror", true } };

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(doc), mitk::Exception);
  }

  void V3Doc_MistypedOffset_Throws()
  {
    const nlohmann::json badOffsets[] = {
      { "slice", nlohmann::json::array({ 1, 2 }) },   // array on slice (wants integer)
      { "slice", 1.5 },                               // fraction on slice
      { "zoom", "double" },                           // string on zoom (wants number > 0)
      { "zoom", 0.0 },                                // zero factor is not a zoom relation
      { "zoom", -2.0 },
      { "pan", 5.0 },                                 // scalar on pan (wants [x, y])
      { "pan", nlohmann::json::array({ 1.0 }) },
      { "pan", nlohmann::json::array({ 1.0, 2.0, 3.0 }) },
      { "pan", nlohmann::json::array({ "a", "b" }) },
    };
    for (const auto& spec : badOffsets)
    {
      const auto dimension = spec[0].get<std::string>();
      auto doc = SingleWindowDoc();
      WindowLinks(doc)[dimension] = nlohmann::json{ { "target", "nav" }, { "offset", spec[1] } };

      auto editor = MakeEditor();
      CPPUNIT_ASSERT_THROW_MESSAGE("mistyped offset on '" + dimension + "' must throw",
                                   editor->ApplyLayout(doc), mitk::Exception);
    }
  }

  void V3Doc_SelectionObjectForm_OK()
  {
    auto doc = SingleWindowDoc();
    WindowLinks(doc)["selection"] = nlohmann::json{ { "target", "main" } };

    auto editor = MakeEditor();
    editor->ApplyLayout(doc);

    // The object form is accepted on load; the writer normalizes to the
    // string shorthand (selection carries no modifiers).
    const auto serialized = editor->SerializeLayout();
    CPPUNIT_ASSERT_EQUAL(std::string("main"),
      serialized.at("root").at("children").at(0).at("links").at("selection").get<std::string>());
  }

  void V3_RoundTrip_AllDimensionsAndOffsets()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "3.0",
      "groups": {
        "main": { "select_all": true },
        "nav": {},
        "planes": {},
        "wl": {}
      },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__w0", "view_direction": "axial",
            "links": { "selection": "main", "pan": "nav", "zoom": "nav", "slice": "nav",
                       "crosshair": "nav", "orientation": "planes",
                       "windowing": "wl", "lut": "wl" } },
          { "type": "window", "id": "mxn__w1", "view_direction": "axial",
            "links": { "selection": "main",
                       "pan":   { "target": "nav", "offset": [12.5, -3.0] },
                       "zoom":  { "target": "nav", "offset": 2.0 },
                       "slice": { "target": "nav", "offset": -1 },
                       "crosshair": "nav" } }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);

    const auto roundTrip = editor->SerializeLayout();
    CPPUNIT_ASSERT_EQUAL(std::string("3.0"), roundTrip.at("version").get<std::string>());

    // Every referenced group is declared on the way out (strict mode).
    for (const auto* group : { "main", "nav", "planes", "wl" })
    {
      CPPUNIT_ASSERT_MESSAGE(std::string("groups dict must declare '") + group + "'",
                             roundTrip.at("groups").contains(group));
    }

    const auto& w0 = roundTrip.at("root").at("children").at(0).at("links");
    const auto& w1 = roundTrip.at("root").at("children").at(1).at("links");

    CPPUNIT_ASSERT_EQUAL(fixture.at("root").at("children").at(0).at("links"), w0);
    CPPUNIT_ASSERT_EQUAL(fixture.at("root").at("children").at(1).at("links"), w1);
  }

  void V3_StrictMode_NavGroupUndeclared_Throws()
  {
    auto doc = SingleWindowDoc();
    doc["groups"] = nlohmann::json{ { "main", { { "select_all", true } } } };
    WindowLinks(doc)["slice"] = "nav";  // 'nav' not declared

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(doc), mitk::Exception);
  }

  void V3_LazyMode_NavLinks_OK()
  {
    auto doc = SingleWindowDoc();
    WindowLinks(doc)["slice"] = "nav";

    auto editor = MakeEditor();
    editor->ApplyLayout(doc);

    const auto link = editor->GetSyncLink("mxn__w0", QmitkMxNSyncDimension::Slice);
    CPPUNIT_ASSERT(link.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("nav"), link->group);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNLayoutV3)
