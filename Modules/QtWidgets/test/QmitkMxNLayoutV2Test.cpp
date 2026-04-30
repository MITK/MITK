/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindowWidget.h>
#include <QmitkRenderWindowUtilityWidget.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <nlohmann/json.hpp>

#include <set>
#include <string>

/**
 * Tests the v2 layout I/O on QmitkMxNMultiWidget:
 *   - SerializeLayout produces a strict-mode v2.0 document.
 *   - ApplyLayout accepts both strict and lazy mode, rejects non-2.0 versions,
 *     enforces window-name uniqueness and group-reference validity (strict),
 *     tears down existing cells, and rolls back to a single default cell on
 *     construction failure.
 *   - View-direction parsing throws on unknown strings (no silent fallback)
 *     and on type mismatches; nlohmann::json::exception subtypes surface as
 *     mitk::Exception at the apply boundary.
 */
class QmitkMxNLayoutV2TestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNLayoutV2TestSuite);

  // --- T3: Serialize / Apply round-trip ---
  MITK_TEST(RoundTrip_Golden_TwoByThreeGrid);
  MITK_TEST(RoundTrip_Recursive_StrictModeFixture);
  MITK_TEST(RoundTrip_LazyModeFixture);
  MITK_TEST(MultiGroup_RoundTrip_PreservesSelectAll);

  // --- T3: Validation ---
  MITK_TEST(StrictMode_MissingGroupReference_Throws);
  MITK_TEST(CustomNames_RegisterUnderEditorPrefix);
  MITK_TEST(DuplicateWindowNames_Throws);
  MITK_TEST(Version_RejectsAllNonV2);
  MITK_TEST(Version_AcceptsExactly_2_0);

  // --- T3: Engine-state semantics ---
  MITK_TEST(TearDown_DestroysAllOldCells);
  MITK_TEST(Apply_Failure_RollsBackToDefault);
  MITK_TEST(Serialize_GroupNaming_Deterministic);
  MITK_TEST(Serialize_RegisteredNames_StripPrefix);
  MITK_TEST(Apply_NestedSplits_RoundTrip);
  MITK_TEST(Apply_NullJson_Throws);

  // --- T4: Strict parsing and exception boundary ---
  MITK_TEST(ViewDirection_TypoSagittal_Throws);
  MITK_TEST(ViewDirection_TypeMismatch_Throws);
  MITK_TEST(ApplyLayout_OutOfRange_Wraps);
  MITK_TEST(ApplyLayout_ParseError_NotPossibleAtThisLayer);

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
    editor->InitializeMultiWidget();   // 1 default cell + group 1
    return editor;
  }

  // ====================================================================
  // T3 #1: Round-trip golden — SetLayout(2, 3) → SerializeLayout
  // ====================================================================
  void RoundTrip_Golden_TwoByThreeGrid()
  {
    auto editor = MakeEditor();
    editor->SetLayout(2, 3);

    const auto doc = editor->SerializeLayout();

    CPPUNIT_ASSERT_EQUAL(std::string("2.0"), doc.at("version").get<std::string>());
    CPPUNIT_ASSERT(doc.contains("groups"));
    CPPUNIT_ASSERT(doc.at("groups").contains("main"));
    CPPUNIT_ASSERT_EQUAL(true, doc.at("groups").at("main").at("select_all").get<bool>());

    // Six windows total, all linking to "main".
    int windowCount = 0;
    std::function<void(const nlohmann::json&)> count = [&](const nlohmann::json& n)
    {
      const auto type = n.at("type").get<std::string>();
      if (type == "split")
      {
        for (const auto& c : n.at("children")) count(c);
      }
      else
      {
        ++windowCount;
        CPPUNIT_ASSERT_EQUAL(std::string("main"),
                             n.at("links").at("selection").get<std::string>());
      }
    };
    count(doc.at("root"));
    CPPUNIT_ASSERT_EQUAL(6, windowCount);
  }

  // ====================================================================
  // T3 #2: Round-trip recursive — apply strict fixture, re-serialize
  // ====================================================================
  void RoundTrip_Recursive_StrictModeFixture()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "name": "Custom Layout",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "alpha", "view_direction": "axial",    "links": { "selection": "main" }, "size": 100 },
          { "type": "window", "name": "beta",  "view_direction": "sagittal", "links": { "selection": "main" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);

    const auto roundTrip = editor->SerializeLayout();

    CPPUNIT_ASSERT_EQUAL(fixture.at("version"),       roundTrip.at("version"));
    CPPUNIT_ASSERT_EQUAL(fixture.at("groups"),        roundTrip.at("groups"));
    // Compare the root subtree without splitter sizes (Qt may redistribute).
    CPPUNIT_ASSERT_EQUAL(fixture.at("root").at("orientation"),
                         roundTrip.at("root").at("orientation"));
    CPPUNIT_ASSERT_EQUAL(fixture.at("root").at("children").size(),
                         roundTrip.at("root").at("children").size());
    for (std::size_t i = 0; i < fixture.at("root").at("children").size(); ++i)
    {
      const auto& f = fixture.at("root").at("children").at(i);
      const auto& r = roundTrip.at("root").at("children").at(i);
      CPPUNIT_ASSERT_EQUAL(f.at("type"),           r.at("type"));
      CPPUNIT_ASSERT_EQUAL(f.at("name"),           r.at("name"));
      CPPUNIT_ASSERT_EQUAL(f.at("view_direction"), r.at("view_direction"));
      CPPUNIT_ASSERT_EQUAL(f.at("links"),          r.at("links"));
      // Lower-bound check on emitted sizes - guards against a future regression
      // that emits zero or negative splitter sizes.
      CPPUNIT_ASSERT_MESSAGE("Round-trip child size must be > 0",
                             r.at("size").get<int>() > 0);
    }
  }

  // ====================================================================
  // T3 #3: Lazy-mode fixture (no `groups` block); defaults applied
  // ====================================================================
  void RoundTrip_LazyModeFixture()
  {
    const auto lazy = nlohmann::json::parse(R"json({
      "version": "2.0",
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(lazy);

    const auto roundTrip = editor->SerializeLayout();
    // SerializeLayout always emits strict mode, so 'groups' must be present
    // with default select_all=true for the implicit 'main' group.
    CPPUNIT_ASSERT(roundTrip.contains("groups"));
    CPPUNIT_ASSERT(roundTrip.at("groups").contains("main"));
    CPPUNIT_ASSERT_EQUAL(true, roundTrip.at("groups").at("main").at("select_all").get<bool>());

    // Re-applying the strict-mode result must succeed and be a no-op.
    editor->ApplyLayout(roundTrip);
    CPPUNIT_ASSERT_EQUAL(1u, editor->GetNumberOfRenderWindowWidgets());

    // Strict-mode round-trip must be a fixpoint: serializing again yields the
    // same document the second apply consumed. Ignore 'size', which Qt may
    // redistribute on rebuild.
    auto roundTrip2 = editor->SerializeLayout();
    auto stripSizes = [](nlohmann::json& node)
    {
      auto recurse = [](nlohmann::json& n, auto& self) -> void
      {
        n.erase("size");
        if (n.contains("children"))
        {
          for (auto& c : n.at("children")) self(c, self);
        }
      };
      recurse(node.at("root"), recurse);
    };
    auto a = roundTrip;
    auto b = roundTrip2;
    stripSizes(a);
    stripSizes(b);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Strict-mode round-trip must be a fixpoint (modulo splitter sizes)", a, b);
  }

  // ====================================================================
  // T3 #4: Multi-group round-trip — different select_all per group
  // ====================================================================
  void MultiGroup_RoundTrip_PreservesSelectAll()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": {
        "main": { "select_all": true  },
        "row2": { "select_all": false }
      },
      "root": {
        "type": "split", "orientation": "vertical",
        "children": [
          { "type": "split", "orientation": "horizontal", "size": 100, "children": [
            { "type": "window", "name": "w0", "view_direction": "axial",    "links": { "selection": "main" }, "size": 100 },
            { "type": "window", "name": "w1", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 100 }
          ]},
          { "type": "split", "orientation": "horizontal", "size": 100, "children": [
            { "type": "window", "name": "w2", "view_direction": "axial",    "links": { "selection": "row2" }, "size": 100 },
            { "type": "window", "name": "w3", "view_direction": "coronal",  "links": { "selection": "row2" }, "size": 100 }
          ]}
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);

    const auto roundTrip = editor->SerializeLayout();
    CPPUNIT_ASSERT_EQUAL(std::size_t{2}, roundTrip.at("groups").size());
    // Both groups must be present and must preserve select_all.
    CPPUNIT_ASSERT_EQUAL(true,  roundTrip.at("groups").at("main").at("select_all").get<bool>());

    // The non-default group's name on serialize is deterministic (engine index 1
    // = "main"; next allocated index gets bare name "g_1"). Find that other
    // group and confirm select_all=false.
    bool foundFalse = false;
    for (auto it = roundTrip.at("groups").begin(); it != roundTrip.at("groups").end(); ++it)
    {
      if (it.key() == "main") continue;
      foundFalse = (it.value().at("select_all").get<bool>() == false);
    }
    CPPUNIT_ASSERT_MESSAGE("Non-default group's select_all must be preserved as false", foundFalse);

    // Per-window cell-to-group mapping must survive the round-trip. Walk the
    // round-trip leaves (cell names are preserved) and group their engine sync
    // indices by 'links.selection' label. Cells sharing a label must land in
    // the same engine group; the set of distinct labels must produce the same
    // count of distinct engine groups. (Group labels themselves may be renamed
    // on serialize - 'main' is stable, others are reassigned to 'g_<n>' - so
    // this test cannot pin the post-round-trip label, only the partition.)
    std::map<std::string, QmitkMxNMultiWidget::GroupSyncIndexType> labelToEngineGroup;
    std::set<QmitkMxNMultiWidget::GroupSyncIndexType> distinctEngineGroups;
    std::function<void(const nlohmann::json&)> walk = [&](const nlohmann::json& node)
    {
      const auto type = node.at("type").get<std::string>();
      if (type == "split")
      {
        for (const auto& c : node.at("children")) walk(c);
        return;
      }
      const auto bareName = QString::fromStdString(node.at("name").get<std::string>());
      const auto label = node.at("links").at("selection").get<std::string>();
      auto cell = editor->GetRenderWindowWidget(QString("mxn.") + bareName);
      CPPUNIT_ASSERT_MESSAGE("Round-trip cell must be addressable by its qualified name",
                             cell != nullptr);
      const auto engineGroup = cell->GetUtilityWidget()->GetSyncGroup();
      distinctEngineGroups.insert(engineGroup);
      auto [it, inserted] = labelToEngineGroup.emplace(label, engineGroup);
      if (!inserted)
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
          "Cells with the same 'links.selection' must map to the same engine sync group",
          it->second, engineGroup);
      }
    };
    walk(roundTrip.at("root"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Two distinct 'links.selection' labels must surface during the walk",
                                 std::size_t{2}, labelToEngineGroup.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Distinct 'links.selection' labels must map to distinct engine sync groups",
      std::size_t{2}, distinctEngineGroups.size());
  }

  // ====================================================================
  // T3 #5: Strict-mode missing reference throws
  // ====================================================================
  void StrictMode_MissingGroupReference_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "w0", "view_direction": "axial", "links": { "selection": "phantom" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    try
    {
      editor->ApplyLayout(fixture);
      CPPUNIT_FAIL("ApplyLayout must throw on missing group reference in strict mode");
    }
    catch (const mitk::Exception& e)
    {
      const std::string msg = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE("Exception message must name the missing group",
                             msg.find("phantom") != std::string::npos);
    }
  }

  // ====================================================================
  // T3 #6: Custom names register under editor prefix
  // ====================================================================
  void CustomNames_RegisterUnderEditorPrefix()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "alpha", "view_direction": "axial", "links": { "selection": "main" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);
    CPPUNIT_ASSERT_MESSAGE("Cell must register under '<editorName>.<bareName>'",
                           nullptr != editor->GetRenderWindowWidget(QString("mxn.alpha")));
  }

  // ====================================================================
  // T3 #7: Duplicate window names throws
  // ====================================================================
  void DuplicateWindowNames_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": 100 },
          { "type": "window", "name": "widget0", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    try
    {
      editor->ApplyLayout(fixture);
      CPPUNIT_FAIL("ApplyLayout must throw on duplicate window name");
    }
    catch (const mitk::Exception& e)
    {
      const std::string msg = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE("Exception message must name the duplicate",
                             msg.find("widget0") != std::string::npos);
    }
  }

  // ====================================================================
  // T3 #8: Version rejection across many non-2.0 strings
  // ====================================================================
  void Version_RejectsAllNonV2()
  {
    auto editor = MakeEditor();
    for (const auto& bad : { "1.0", "1.1", "100", "1abc", "3.0", "abc" })
    {
      auto fixture = nlohmann::json::parse(R"json({
        "version": "PLACEHOLDER",
        "root": {
          "type": "split", "orientation": "horizontal",
          "children": [
            { "type": "window", "name": "w0", "view_direction": "axial", "links": { "selection": "main" }, "size": 100 }
          ]
        }
      })json");
      fixture["version"] = bad;
      try
      {
        editor->ApplyLayout(fixture);
        CPPUNIT_FAIL((std::string("Expected throw on version '") + bad + "'").c_str());
      }
      catch (const mitk::Exception&)
      {
        // expected
      }
    }
  }

  // ====================================================================
  // T3 #9: Single-cell v2.0 fixture applies cleanly
  // ====================================================================
  void Version_AcceptsExactly_2_0()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "only", "view_direction": "axial", "links": { "selection": "main" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);
    CPPUNIT_ASSERT_EQUAL(1u, editor->GetNumberOfRenderWindowWidgets());
    CPPUNIT_ASSERT(nullptr != editor->GetRenderWindowWidget(QString("mxn.only")));
  }

  // ====================================================================
  // T3 #10: Tear-down evidence — old cells destroyed before construction
  // ====================================================================
  void TearDown_DestroysAllOldCells()
  {
    auto editor = MakeEditor();
    editor->SetLayout(2, 2);
    CPPUNIT_ASSERT_EQUAL(4u, editor->GetNumberOfRenderWindowWidgets());

    // Hold weak references to the four cells. Connecting to QObject::destroyed
    // would be the natural observer, but RemoveRenderWindowWidget calls
    // disconnect(widget, 0, 0, 0) which severs the destroyed-signal handler
    // before the widget actually dies. weak_ptr expiry directly tracks shared_ptr
    // refcount hitting zero (= ~QmitkRenderWindowWidget ran), which is exactly
    // the invariant we want to verify.
    std::vector<std::weak_ptr<QmitkRenderWindowWidget>> weakCells;
    for (const auto& [name, widget] : editor->GetRenderWindowWidgets())
    {
      (void)name;
      weakCells.emplace_back(widget);
    }
    CPPUNIT_ASSERT_EQUAL(std::size_t{4}, weakCells.size());

    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "only", "view_direction": "axial", "links": { "selection": "main" }, "size": 100 }
        ]
      }
    })json");

    editor->ApplyLayout(fixture);
    QApplication::processEvents();

    int destroyedCount = 0;
    for (const auto& weak : weakCells)
    {
      if (weak.expired())
      {
        ++destroyedCount;
      }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Every existing cell must be destroyed during tear-down, before the new layout is built",
      4, destroyedCount);
    CPPUNIT_ASSERT_EQUAL(1u, editor->GetNumberOfRenderWindowWidgets());
  }

  // ====================================================================
  // T3 #11: Construction failure rolls back to single default cell
  // ====================================================================
  void Apply_Failure_RollsBackToDefault()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "ok",   "view_direction": "axial",     "links": { "selection": "main" }, "size": 100 },
          { "type": "window", "name": "bad",  "view_direction": "saggital",  "links": { "selection": "main" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->SetLayout(2, 2);
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(fixture), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "After a failed apply the editor must be left with exactly one default cell",
      1u, editor->GetNumberOfRenderWindowWidgets());
    CPPUNIT_ASSERT_MESSAGE(
      "After rollback the editor must expose a usable active cell, "
      "not just a dangling single-cell placeholder",
      nullptr != editor->GetActiveRenderWindowWidget());
  }

  // ====================================================================
  // T3 #12: Group naming is deterministic across two consecutive serializations
  // ====================================================================
  void Serialize_GroupNaming_Deterministic()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": {
        "main": { "select_all": true },
        "row2": { "select_all": false },
        "row3": { "select_all": true }
      },
      "root": {
        "type": "split", "orientation": "vertical",
        "children": [
          { "type": "window", "name": "a", "view_direction": "axial", "links": { "selection": "main" }, "size": 100 },
          { "type": "window", "name": "b", "view_direction": "axial", "links": { "selection": "row2" }, "size": 100 },
          { "type": "window", "name": "c", "view_direction": "axial", "links": { "selection": "row3" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);
    const auto first  = editor->SerializeLayout();
    const auto second = editor->SerializeLayout();
    CPPUNIT_ASSERT_EQUAL(first.at("groups"), second.at("groups"));
    CPPUNIT_ASSERT_EQUAL(first.at("root"),   second.at("root"));
  }

  // ====================================================================
  // T3 #13: Bare names in JSON have no editor prefix
  // ====================================================================
  void Serialize_RegisteredNames_StripPrefix()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "alpha",   "view_direction": "axial", "links": { "selection": "main" }, "size": 100 },
          { "type": "window", "name": "widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);
    const auto doc = editor->SerializeLayout();
    std::set<std::string> emittedNames;
    for (const auto& c : doc.at("root").at("children"))
    {
      emittedNames.insert(c.at("name").get<std::string>());
    }
    CPPUNIT_ASSERT_EQUAL(std::size_t{2}, emittedNames.size());
    CPPUNIT_ASSERT(emittedNames.count("alpha")   == 1);
    CPPUNIT_ASSERT(emittedNames.count("widget0") == 1);
  }

  // ====================================================================
  // T3 #14: Nested splits round-trip cleanly
  // ====================================================================
  void Apply_NestedSplits_RoundTrip()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "vertical",
        "children": [
          { "type": "split", "orientation": "horizontal", "size": 100, "children": [
            { "type": "window", "name": "tl", "view_direction": "axial",    "links": { "selection": "main" }, "size": 100 },
            { "type": "window", "name": "tr", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 100 }
          ]},
          { "type": "split", "orientation": "horizontal", "size": 100, "children": [
            { "type": "window", "name": "bl", "view_direction": "coronal",  "links": { "selection": "main" }, "size": 100 },
            { "type": "window", "name": "br", "view_direction": "axial",    "links": { "selection": "main" }, "size": 100 }
          ]}
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);
    CPPUNIT_ASSERT_EQUAL(4u, editor->GetNumberOfRenderWindowWidgets());

    const auto roundTrip = editor->SerializeLayout();
    CPPUNIT_ASSERT_EQUAL(std::string("vertical"),
                         roundTrip.at("root").at("orientation").get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::size_t{2},
                         roundTrip.at("root").at("children").size());
    for (const auto& row : roundTrip.at("root").at("children"))
    {
      CPPUNIT_ASSERT_EQUAL(std::string("split"),     row.at("type").get<std::string>());
      CPPUNIT_ASSERT_EQUAL(std::string("horizontal"), row.at("orientation").get<std::string>());
      CPPUNIT_ASSERT_EQUAL(std::size_t{2},           row.at("children").size());
    }
  }

  // ====================================================================
  // T3 #15: LoadLayout(nullptr) throws (covers the wrapper)
  // ====================================================================
  void Apply_NullJson_Throws()
  {
    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->LoadLayout(nullptr), mitk::Exception);
  }

  // ====================================================================
  // T4 #16: View-direction typo throws strictly (no silent fallback)
  // ====================================================================
  void ViewDirection_TypoSagittal_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "w0", "view_direction": "saggital", "links": { "selection": "main" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    try
    {
      editor->ApplyLayout(fixture);
      CPPUNIT_FAIL("ApplyLayout must throw on unknown view_direction");
    }
    catch (const mitk::Exception& e)
    {
      const std::string msg = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE("Exception message must name the offending value",
                             msg.find("saggital") != std::string::npos);
    }
  }

  // ====================================================================
  // T4 #17: View-direction type mismatch (int instead of string) throws
  // ====================================================================
  void ViewDirection_TypeMismatch_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "name": "w0", "view_direction": 42, "links": { "selection": "main" }, "size": 100 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    // Either the prewalk type check or the boundary catch on
    // nlohmann::json::type_error must surface as mitk::Exception.
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(fixture), mitk::Exception);
  }

  // ====================================================================
  // T4 #18: Missing 'root' field surfaces as mitk::Exception
  // (rewrap of nlohmann::json::out_of_range, layered defence above prewalk)
  // ====================================================================
  void ApplyLayout_OutOfRange_Wraps()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } }
    })json");

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(fixture), mitk::Exception);
  }

  // ====================================================================
  // T4 #19: Layering note — ApplyLayout takes parsed JSON, so parse_error
  //         from nlohmann::json::parse cannot reach this layer; it is the
  //         popup wrapper's responsibility to catch parse_error. This test
  //         simply documents the layering: applying a valid JSON object
  //         that does not contain anything ApplyLayout can use just throws
  //         a normal validation error.
  // ====================================================================
  void ApplyLayout_ParseError_NotPossibleAtThisLayer()
  {
    const auto emptyObject = nlohmann::json::object();
    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(emptyObject), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNLayoutV2)
