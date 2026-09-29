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

#include <QColor>

#include <algorithm>

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
  MITK_TEST(GroupCosmetics_RoundTripVerbatim);
  MITK_TEST(GroupCosmetics_AbsentStaysAbsent);
  MITK_TEST(GroupCosmetics_MalformedColorIgnored);
  MITK_TEST(GroupCosmetics_UnknownEntryKeyTolerated);
  MITK_TEST(GroupCosmetics_SetWritesRoundTripAndLeaveLinksAlone);
  MITK_TEST(EmptyGroup_SurvivesSaveAndLoad);
  MITK_TEST(DeclaredEmptyGroup_KeepsItsCosmeticsToItself);

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

  /** Document with a selection group and a nav group, both carrying cosmetics. */
  static nlohmann::json CosmeticsDoc()
  {
    auto doc = SingleWindowDoc();
    doc["groups"] = nlohmann::json{
      { "main", { { "select_all", true }, { "color", "#6FA8DC" }, { "name", "Navigation" } } },
      { "nav", { { "color", "#E1707A" } } }
    };
    WindowLinks(doc)["slice"] = "nav";
    return doc;
  }

  void GroupCosmetics_RoundTripVerbatim()
  {
    auto editor = MakeEditor();
    editor->ApplyLayout(CosmeticsDoc());

    CPPUNIT_ASSERT_EQUAL(std::string("Navigation"), editor->GetSyncGroupDisplayName("main"));
    CPPUNIT_ASSERT(QColor("#6FA8DC") == editor->GetSyncGroupColor("main"));
    CPPUNIT_ASSERT_EQUAL(std::string("nav"), editor->GetSyncGroupDisplayName("nav"));
    CPPUNIT_ASSERT(QColor("#E1707A") == editor->GetSyncGroupColor("nav"));

    const auto roundTrip = editor->SerializeLayout();
    // Verbatim: the hex string survives byte-for-byte, including case.
    CPPUNIT_ASSERT_EQUAL(std::string("#6FA8DC"),
      roundTrip.at("groups").at("main").at("color").get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("Navigation"),
      roundTrip.at("groups").at("main").at("name").get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("#E1707A"),
      roundTrip.at("groups").at("nav").at("color").get<std::string>());
    CPPUNIT_ASSERT_MESSAGE("No display name was set for 'nav'; none may be emitted",
      !roundTrip.at("groups").at("nav").contains("name"));
  }

  void GroupCosmetics_AbsentStaysAbsent()
  {
    auto doc = SingleWindowDoc();
    doc["groups"] = nlohmann::json{ { "main", { { "select_all", true } } } };

    auto editor = MakeEditor();
    editor->ApplyLayout(doc);

    CPPUNIT_ASSERT_EQUAL(std::string("main"), editor->GetSyncGroupDisplayName("main"));
    CPPUNIT_ASSERT_MESSAGE("Default hue must be assigned without a persisted color",
      editor->GetSyncGroupColor("main").isValid());

    const auto roundTrip = editor->SerializeLayout();
    CPPUNIT_ASSERT(!roundTrip.at("groups").at("main").contains("color"));
    CPPUNIT_ASSERT(!roundTrip.at("groups").at("main").contains("name"));
  }

  void GroupCosmetics_MalformedColorIgnored()
  {
    auto doc = SingleWindowDoc();
    doc["groups"] = nlohmann::json{
      { "main", { { "select_all", true }, { "color", "red" }, { "name", 42 } } }
    };

    auto editor = MakeEditor();
    // A cosmetic field must never make a layout unloadable.
    editor->ApplyLayout(doc);

    CPPUNIT_ASSERT_EQUAL(1u, editor->GetNumberOfRenderWindowWidgets());
    CPPUNIT_ASSERT_MESSAGE("Malformed color falls back to the default hue",
      editor->GetSyncGroupColor("main").isValid());
    CPPUNIT_ASSERT_EQUAL(std::string("main"), editor->GetSyncGroupDisplayName("main"));

    const auto roundTrip = editor->SerializeLayout();
    CPPUNIT_ASSERT(!roundTrip.at("groups").at("main").contains("color"));
    CPPUNIT_ASSERT(!roundTrip.at("groups").at("main").contains("name"));
  }

  void GroupCosmetics_UnknownEntryKeyTolerated()
  {
    // The loader stays lenient on group-entry keys (unlike the closed v3
    // 'links' object); the schema documents the shape for authors.
    auto doc = SingleWindowDoc();
    doc["groups"] = nlohmann::json{
      { "main", { { "select_all", true }, { "frobnicate", 1 } } }
    };

    auto editor = MakeEditor();
    editor->ApplyLayout(doc);
    CPPUNIT_ASSERT_EQUAL(1u, editor->GetNumberOfRenderWindowWidgets());
  }

  void GroupCosmetics_SetWritesRoundTripAndLeaveLinksAlone()
  {
    auto editor = MakeEditor();
    editor->ApplyLayout(CosmeticsDoc());

    const auto linkBefore = editor->GetSyncLink("mxn__w0", QmitkMxNSyncDimension::Slice);

    editor->SetSyncGroupDisplayName("nav", "Detail");
    editor->SetSyncGroupColor("nav", QColor("#93C47D"));

    const auto linkAfter = editor->GetSyncLink("mxn__w0", QmitkMxNSyncDimension::Slice);
    CPPUNIT_ASSERT_MESSAGE("Cosmetic writes must not touch links",
      linkAfter.has_value() && linkAfter->group == linkBefore->group);

    const auto roundTrip = editor->SerializeLayout();
    CPPUNIT_ASSERT_EQUAL(std::string("Detail"),
      roundTrip.at("groups").at("nav").at("name").get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("#93c47d"),
      roundTrip.at("groups").at("nav").at("color").get<std::string>());
    CPPUNIT_ASSERT_MESSAGE("The URL-safe id itself never changes",
      roundTrip.at("groups").contains("nav"));

    // Empty display name reverts to the id.
    editor->SetSyncGroupDisplayName("nav", "");
    CPPUNIT_ASSERT_EQUAL(std::string("nav"), editor->GetSyncGroupDisplayName("nav"));

    CPPUNIT_ASSERT_THROW(editor->SetSyncGroupDisplayName("no-such-group", "x"), mitk::Exception);
    CPPUNIT_ASSERT_THROW(editor->SetSyncGroupColor("no-such-group", QColor("#000000")), mitk::Exception);
    CPPUNIT_ASSERT_THROW(editor->SetSyncGroupColor("nav", QColor()), mitk::Exception);
  }

  static const QmitkMxNMultiWidget::SyncGroupInfo* FindGroup(
    const std::vector<QmitkMxNMultiWidget::SyncGroupInfo>& infos, const std::string& id)
  {
    const auto it = std::find_if(infos.begin(), infos.end(), [&id](const auto& info) { return info.id == id; });
    return it != infos.end() ? &*it : nullptr;
  }

  void EmptyGroup_SurvivesSaveAndLoad()
  {
    // A group no window is in yet is still a card in the editor, so Save keeps it.
    auto editor = MakeEditor();
    editor->ApplyLayout(SingleWindowDoc());
    const auto index = editor->NextFreeSyncGroupIndex();
    editor->AddSynchronizationGroup(index);
    const auto id = editor->GetSyncGroupName(index);
    editor->SetSyncGroupDisplayName(id, "Tumor");
    editor->SetSyncGroupColor(id, QColor("#ff0000"));

    const auto saved = editor->SerializeLayout();
    CPPUNIT_ASSERT_MESSAGE("Save declares the empty group", saved.at("groups").contains(id));

    auto reloaded = MakeEditor();
    reloaded->ApplyLayout(saved);
    const auto infos = reloaded->GetSyncGroupInfos();
    const auto* group = FindGroup(infos, id);
    CPPUNIT_ASSERT_MESSAGE("The empty group is a card again after loading", nullptr != group);
    CPPUNIT_ASSERT_EQUAL(std::string("Tumor"), group->displayName);
    CPPUNIT_ASSERT(QColor("#ff0000") == group->color);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The default group stays the referenced one",
                                 std::string("main"), reloaded->GetDefaultSyncGroupName());
  }

  void DeclaredEmptyGroup_KeepsItsCosmeticsToItself()
  {
    // A declared group whose name a new group would derive from its index must
    // not lend that new group its color and name.
    auto doc = SingleWindowDoc();
    doc["groups"] = nlohmann::json{
      { "main", { { "select_all", true } } },
      { "g_3", { { "color", "#ff0000" }, { "name", "Tumor" } } }
    };
    auto editor = MakeEditor();
    editor->ApplyLayout(doc);

    const auto infos = editor->GetSyncGroupInfos();
    const auto* declared = FindGroup(infos, "g_3");
    CPPUNIT_ASSERT_MESSAGE("The declared group is registered", nullptr != declared);
    CPPUNIT_ASSERT_EQUAL(std::string("Tumor"), declared->displayName);

    for (int created = 0; created < 2; ++created)
    {
      const auto index = editor->NextFreeSyncGroupIndex();
      editor->AddSynchronizationGroup(index);
      const auto id = editor->GetSyncGroupName(index);
      CPPUNIT_ASSERT_MESSAGE("A new group does not alias the declared one", "g_3" != id);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A new group carries no borrowed display name",
                                   id, editor->GetSyncGroupDisplayName(id));
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNLayoutV3)
