/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>
#include <QmitkRenderWindowUtilityWidget.h>

#include <mitkBaseRenderer.h>
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
 *     enforces window-id uniqueness and group-reference validity (strict),
 *     tears down existing cells, and rolls back to a single default cell on
 *     construction failure.
 *   - View-direction parsing throws on unknown strings (no silent fallback)
 *     and on type mismatches; nlohmann::json::exception subtypes surface as
 *     mitk::Exception at the apply boundary.
 */
class QmitkMxNLayoutV2TestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNLayoutV2TestSuite);

  // --- Serialize / Apply round-trip ---
  MITK_TEST(RoundTrip_Golden_TwoByThreeGrid);
  MITK_TEST(RoundTrip_Recursive_StrictModeFixture);
  MITK_TEST(RoundTrip_LazyModeFixture);
  MITK_TEST(MultiGroup_RoundTrip_PreservesSelectAll);

  // --- Validation ---
  MITK_TEST(StrictMode_MissingGroupReference_Throws);
  MITK_TEST(CustomIds_RegisterUnderEditorPrefix);
  MITK_TEST(DuplicateWindowIds_Throws);
  MITK_TEST(Version_RejectsAllNonV2);
  MITK_TEST(Version_AcceptsExactly_2_0);

  // --- Engine-state semantics ---
  MITK_TEST(TearDown_DestroysAllOldCells);
  MITK_TEST(Apply_Failure_RollsBackToDefault);
  MITK_TEST(Serialize_GroupNaming_Deterministic);
  MITK_TEST(Serialize_RegisteredIds_StripPrefix);
  MITK_TEST(Apply_NestedSplits_RoundTrip);
  MITK_TEST(Apply_NullJson_Throws);

  // --- Strict parsing and exception boundary ---
  MITK_TEST(ViewDirection_TypoSagittal_Throws);
  MITK_TEST(ViewDirection_TypeMismatch_Throws);
  MITK_TEST(ApplyLayout_OutOfRange_Wraps);
  MITK_TEST(ApplyLayout_ParseError_NotPossibleAtThisLayer);

  // --- Post-load grid-state invalidation + active-widget reset ---
  MITK_TEST(SetLayout_PopulatesRowAndColumn);
  MITK_TEST(ApplyLayout_InvalidatesRowAndColumn);
  MITK_TEST(ApplyLayout_AssignsActiveWidgetFromNewMap);
  MITK_TEST(ApplyLayout_RollbackKeepsActiveWidget);

  // --- Group seeding rule + post-apply consistency ---
  MITK_TEST(ApplyLayout_GroupMembersAgreeOnVisibility);

  // --- Optional size field + default weight 1 ---
  MITK_TEST(Size_Omitted_LoadsWithDefaultWeight);
  MITK_TEST(Size_PartiallyOmitted_MixedSiblings);
  MITK_TEST(Size_Zero_Throws);
  MITK_TEST(Size_Negative_Throws);

  // --- ListWindowDescriptors (engine query consumed by REST WP3) ---
  MITK_TEST(ListWindowDescriptors_DefaultGrid_ReturnsBareIds);
  MITK_TEST(ListWindowDescriptors_AfterApply_PreOrderTraversal);
  MITK_TEST(ListWindowDescriptors_CarriesViewDirectionAndSelectionGroup);

  // --- Optional display 'name' (free-form, non-unique label) ---
  MITK_TEST(WindowName_OptionalDisplay_RoundTrips);
  MITK_TEST(WindowName_DisplayOmitted_RoundTrips);
  MITK_TEST(WindowName_DisplayDuplicates_OK);
  MITK_TEST(WindowName_DisplayFreeForm_OK);
  MITK_TEST(WindowName_DisplayEmptyString_Throws);
  MITK_TEST(WindowName_DisplayNotString_Throws);
  MITK_TEST(ListWindowDescriptors_CarriesDisplayName);

  // --- Qualified-id contract: id must start with this editor's prefix ---
  MITK_TEST(ApplyLayout_UnprefixedId_Throws);
  MITK_TEST(ApplyLayout_WrongEditorPrefix_Throws);
  MITK_TEST(Construct_BadMultiWidgetName_Throws);

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
  // Round-trip golden -- SetLayout(2, 3) -> SerializeLayout
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
  // Round-trip recursive -- apply strict fixture, re-serialize
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
          { "type": "window", "id": "mxn__alpha", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__beta",  "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
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
      CPPUNIT_ASSERT_EQUAL(f.at("id"),             r.at("id"));
      CPPUNIT_ASSERT_EQUAL(f.at("view_direction"), r.at("view_direction"));
      CPPUNIT_ASSERT_EQUAL(f.at("links"),          r.at("links"));
      // Lower-bound check on emitted sizes - guards against a future regression
      // that emits zero or negative splitter sizes.
      CPPUNIT_ASSERT_MESSAGE("Round-trip child size must be > 0",
                             r.at("size").get<int>() > 0);
    }
  }

  // ====================================================================
  // Lazy-mode fixture (no `groups` block); defaults applied
  // ====================================================================
  void RoundTrip_LazyModeFixture()
  {
    const auto lazy = nlohmann::json::parse(R"json({
      "version": "2.0",
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
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
  // Multi-group round-trip -- different select_all per group
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
          { "type": "split", "orientation": "horizontal", "size": 1, "children": [
            { "type": "window", "id": "mxn__w0", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
            { "type": "window", "id": "mxn__w1", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
          ]},
          { "type": "split", "orientation": "horizontal", "size": 1, "children": [
            { "type": "window", "id": "mxn__w2", "view_direction": "axial",    "links": { "selection": "row2" }, "size": 1 },
            { "type": "window", "id": "mxn__w3", "view_direction": "coronal",  "links": { "selection": "row2" }, "size": 1 }
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

    // The non-default group's original document label is preserved through the
    // round-trip (the engine's group-name registry records the layout-document
    // label at AddSynchronizationGroup time and SerializeLayout reads from
    // there). Find the non-'main' group and confirm select_all=false.
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
    // count of distinct engine groups. Group labels are also preserved across
    // round-trip via the engine's group-name registry, but this test pins the
    // partition only - the label-preservation invariant is covered by the
    // assertions on the 'groups' dict above.
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
      const auto id = QString::fromStdString(node.at("id").get<std::string>());
      const auto label = node.at("links").at("selection").get<std::string>();
      auto cell = editor->GetRenderWindowWidget(id);
      CPPUNIT_ASSERT_MESSAGE("Round-trip cell must be addressable by its canonical id",
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
  // Strict-mode missing reference throws
  // ====================================================================
  void StrictMode_MissingGroupReference_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__w0", "view_direction": "axial", "links": { "selection": "phantom" }, "size": 1 }
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
  // Custom ids register verbatim under their canonical fully-qualified
  // form. The engine does not prepend a prefix at apply time; the
  // document's id is the registration key.
  // ====================================================================
  void CustomIds_RegisterUnderEditorPrefix()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__alpha", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);
    CPPUNIT_ASSERT_MESSAGE("Cell must register under the document's canonical id verbatim",
                           nullptr != editor->GetRenderWindowWidget(QString("mxn__alpha")));
  }

  // ====================================================================
  // Duplicate window ids throws
  // ====================================================================
  void DuplicateWindowIds_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__widget0", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    try
    {
      editor->ApplyLayout(fixture);
      CPPUNIT_FAIL("ApplyLayout must throw on duplicate window id");
    }
    catch (const mitk::Exception& e)
    {
      const std::string msg = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE("Exception message must name the duplicate",
                             msg.find("widget0") != std::string::npos);
    }
  }

  // ====================================================================
  // Version rejection across many non-2.0 strings
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
            { "type": "window", "id": "mxn__w0", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
          ]
        }
      })json");
      fixture["version"] = bad;
      try
      {
        editor->ApplyLayout(fixture);
        CPPUNIT_FAIL((std::string("Expected throw on version '") + bad + "'").c_str());
      }
      catch (const mitk::Exception& e)
      {
        // v1.x rejects must reference the migration script so the
        // QMessageBox wrapper surfaces the exact path the user needs to run.
        const std::string sBad(bad);
        const bool looksV1 = sBad.size() >= 2 && sBad[0] == '1' && sBad[1] == '.';
        if (looksV1)
        {
          const std::string msg = e.GetDescription();
          CPPUNIT_ASSERT_MESSAGE(
            "v1.x version-rejection message must reference migrate-mxn-layout-v1-to-v2",
            msg.find("migrate-mxn-layout-v1-to-v2") != std::string::npos);
        }
      }
    }
  }

  // ====================================================================
  // Single-cell v2.0 fixture applies cleanly
  // ====================================================================
  void Version_AcceptsExactly_2_0()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__only", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);
    CPPUNIT_ASSERT_EQUAL(1u, editor->GetNumberOfRenderWindowWidgets());
    CPPUNIT_ASSERT(nullptr != editor->GetRenderWindowWidget(QString("mxn__only")));
  }

  // ====================================================================
  // Tear-down evidence -- old cells destroyed before construction
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
    for ([[maybe_unused]] const auto& [name, widget] : editor->GetRenderWindowWidgets())
    {
      weakCells.emplace_back(widget);
    }
    CPPUNIT_ASSERT_EQUAL(std::size_t{4}, weakCells.size());

    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__only", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
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
  // Construction failure rolls back to single default cell
  // ====================================================================
  void Apply_Failure_RollsBackToDefault()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__ok",   "view_direction": "axial",     "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__bad",  "view_direction": "saggital",  "links": { "selection": "main" }, "size": 1 }
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
  // Group naming is deterministic across two consecutive serializations
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
          { "type": "window", "id": "mxn__a", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__b", "view_direction": "axial", "links": { "selection": "row2" }, "size": 1 },
          { "type": "window", "id": "mxn__c", "view_direction": "axial", "links": { "selection": "row3" }, "size": 1 }
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
  // Serialize emits each cell's id verbatim: the canonical fully-qualified
  // form goes in, the same string comes out (no prefix prepend, no strip).
  // ====================================================================
  void Serialize_RegisteredIds_StripPrefix()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__alpha",   "view_direction": "axial", "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);
    const auto doc = editor->SerializeLayout();
    std::set<std::string> emittedIds;
    for (const auto& c : doc.at("root").at("children"))
    {
      emittedIds.insert(c.at("id").get<std::string>());
    }
    CPPUNIT_ASSERT_EQUAL(std::size_t{2}, emittedIds.size());
    CPPUNIT_ASSERT(emittedIds.count("mxn__alpha")   == 1);
    CPPUNIT_ASSERT(emittedIds.count("mxn__widget0") == 1);
  }

  // ====================================================================
  // Nested splits round-trip cleanly
  // ====================================================================
  void Apply_NestedSplits_RoundTrip()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "vertical",
        "children": [
          { "type": "split", "orientation": "horizontal", "size": 1, "children": [
            { "type": "window", "id": "mxn__tl", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
            { "type": "window", "id": "mxn__tr", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
          ]},
          { "type": "split", "orientation": "horizontal", "size": 1, "children": [
            { "type": "window", "id": "mxn__bl", "view_direction": "coronal",  "links": { "selection": "main" }, "size": 1 },
            { "type": "window", "id": "mxn__br", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 }
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
  // LoadLayout(nullptr) throws (covers the wrapper)
  // ====================================================================
  void Apply_NullJson_Throws()
  {
    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->LoadLayout(nullptr), mitk::Exception);
  }

  // ====================================================================
  // View-direction typo throws strictly (no silent fallback)
  // ====================================================================
  void ViewDirection_TypoSagittal_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__w0", "view_direction": "saggital", "links": { "selection": "main" }, "size": 1 }
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
  // View-direction type mismatch (int instead of string) throws
  // ====================================================================
  void ViewDirection_TypeMismatch_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__w0", "view_direction": 42, "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    // Either the prewalk type check or the boundary catch on
    // nlohmann::json::type_error must surface as mitk::Exception.
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(fixture), mitk::Exception);
  }

  // ====================================================================
  // Missing 'root' field surfaces as mitk::Exception
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
  // Layering note -- ApplyLayout takes parsed JSON, so parse_error
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

  // ====================================================================
  // SetLayout(r, c) populates row + column count
  // ====================================================================
  void SetLayout_PopulatesRowAndColumn()
  {
    auto editor = MakeEditor();
    editor->SetLayout(2, 3);
    CPPUNIT_ASSERT_EQUAL(2, editor->GetRowCount());
    CPPUNIT_ASSERT_EQUAL(3, editor->GetColumnCount());
  }

  // ====================================================================
  // ApplyLayout invalidates the row/column sentinel to 0/0
  // ====================================================================
  void ApplyLayout_InvalidatesRowAndColumn()
  {
    auto editor = MakeEditor();
    editor->SetLayout(2, 3);
    CPPUNIT_ASSERT_EQUAL(2, editor->GetRowCount());
    CPPUNIT_ASSERT_EQUAL(3, editor->GetColumnCount());

    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__a", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__b", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");
    editor->ApplyLayout(fixture);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "ApplyLayout must invalidate the grid-row sentinel to 0 - the loaded layout is not a regular grid",
      0, editor->GetRowCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "ApplyLayout must invalidate the grid-column sentinel to 0",
      0, editor->GetColumnCount());
  }

  // ====================================================================
  // ApplyLayout points the active widget at a cell of the new map
  // ====================================================================
  void ApplyLayout_AssignsActiveWidgetFromNewMap()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__alpha", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__beta",  "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);

    const auto active = editor->GetActiveRenderWindowWidget();
    CPPUNIT_ASSERT_MESSAGE(
      "ApplyLayout must leave the editor with a non-null active render-window widget",
      active != nullptr);

    bool found = false;
    for ([[maybe_unused]] const auto& [name, widget] : editor->GetRenderWindowWidgets())
    {
      if (widget == active)
      {
        found = true;
        break;
      }
    }
    CPPUNIT_ASSERT_MESSAGE(
      "Active render-window widget must be a member of the post-apply cell map",
      found);
  }

  // ====================================================================
  // After ApplyLayout, every cell of a group reports the same
  //     per-renderer visibility for each selected node (the seeding pass
  //     normalises divergence to the seed cell's values).
  //
  //     Note: on a fresh editor, all cells start in lock-step against a
  //     single connector, so this assertion mostly pins the consistency
  //     contract rather than a divergence-resolution outcome. Direct
  //     'seed-wins' divergence assertions are deferred - on fresh cells
  //     no divergence exists to resolve through the public API.
  //     SeedFromMember's selection-replacement contract is unit-tested
  //     in QmitkSynchronizedWidgetConnectorTest.
  // ====================================================================
  void ApplyLayout_GroupMembersAgreeOnVisibility()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__widget1", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);

    auto cell0 = editor->GetRenderWindowWidget(QString("mxn__widget0"));
    auto cell1 = editor->GetRenderWindowWidget(QString("mxn__widget1"));
    CPPUNIT_ASSERT(cell0 != nullptr);
    CPPUNIT_ASSERT(cell1 != nullptr);

    auto* renderer0 = mitk::BaseRenderer::GetInstance(cell0->GetRenderWindow()->GetVtkRenderWindow());
    auto* renderer1 = mitk::BaseRenderer::GetInstance(cell1->GetRenderWindow()->GetVtkRenderWindow());
    CPPUNIT_ASSERT(renderer0 != nullptr);
    CPPUNIT_ASSERT(renderer1 != nullptr);

    for (const auto& node : { m_Node1, m_Node2 })
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "After ApplyLayout, group members must agree on per-renderer visibility",
        node->IsVisible(renderer0), node->IsVisible(renderer1));
    }
  }

  void ApplyLayout_RollbackKeepsActiveWidget()
  {
    // Forces rollback via the unknown-view-direction path (strictness).
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__ok",  "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__bad", "view_direction": "saggital", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(fixture), mitk::Exception);
    CPPUNIT_ASSERT_MESSAGE(
      "After rollback the editor must expose a usable active cell",
      nullptr != editor->GetActiveRenderWindowWidget());
  }

  // ====================================================================
  // 'size' is optional. When omitted on every child, the loader assigns
  // each child the default weight 1 and the layout applies cleanly.
  // ====================================================================
  void Size_Omitted_LoadsWithDefaultWeight()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "view_direction": "axial",    "links": { "selection": "main" } },
          { "type": "window", "id": "mxn__widget1", "view_direction": "sagittal", "links": { "selection": "main" } },
          { "type": "window", "id": "mxn__widget2", "view_direction": "coronal",  "links": { "selection": "main" } }
        ]
      }
    })json");

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_NO_THROW(editor->ApplyLayout(fixture));

    CPPUNIT_ASSERT(editor->GetRenderWindowWidget(QString("mxn__widget0")) != nullptr);
    CPPUNIT_ASSERT(editor->GetRenderWindowWidget(QString("mxn__widget1")) != nullptr);
    CPPUNIT_ASSERT(editor->GetRenderWindowWidget(QString("mxn__widget2")) != nullptr);
  }

  // ====================================================================
  // 'size' may be omitted on some siblings while others declare a value.
  // The loader treats omitted children as weight 1 and the layout applies
  // cleanly; ratios resolve as e.g. [size: 3, default, default] -> 3:1:1.
  // ====================================================================
  void Size_PartiallyOmitted_MixedSiblings()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "view_direction": "axial",    "links": { "selection": "main" }, "size": 3 },
          { "type": "window", "id": "mxn__widget1", "view_direction": "sagittal", "links": { "selection": "main" } },
          { "type": "window", "id": "mxn__widget2", "view_direction": "coronal",  "links": { "selection": "main" } }
        ]
      }
    })json");

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_NO_THROW(editor->ApplyLayout(fixture));
    CPPUNIT_ASSERT(editor->GetRenderWindowWidget(QString("mxn__widget0")) != nullptr);
    CPPUNIT_ASSERT(editor->GetRenderWindowWidget(QString("mxn__widget1")) != nullptr);
    CPPUNIT_ASSERT(editor->GetRenderWindowWidget(QString("mxn__widget2")) != nullptr);
  }

  // ====================================================================
  // size = 0 is rejected by the loader. The format has no documented
  // 'hide this cell' semantics, so 0 is not a valid stand-in.
  // ====================================================================
  void Size_Zero_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": 0 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(fixture), mitk::Exception);
  }

  // ====================================================================
  // Negative size is rejected by the loader.
  // ====================================================================
  void Size_Negative_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": -3 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(fixture), mitk::Exception);
  }

  // ====================================================================
  // ListWindowDescriptors -- engine query consumed by REST WP3 bindings.
  //
  // This is the public engine surface the REST window-list provider calls
  // (instead of going through SerializeLayout-then-parse). The descriptor
  // shape -- canonical fully-qualified id, view direction, selection group
  // -- mirrors what the v2 layout document persists per cell, so per-cell
  // field values agree by construction with what GET /editors/mxn/layout
  // would emit.
  // ====================================================================
  void ListWindowDescriptors_DefaultGrid_ReturnsBareIds()
  {
    auto editor = MakeEditor();
    editor->SetLayout(1, 2);

    const auto descriptors = editor->ListWindowDescriptors();

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), descriptors.size());
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget0"), descriptors[0].id.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget1"), descriptors[1].id.toStdString());
    // Default grid uses the conventional 'main' group label.
    CPPUNIT_ASSERT_EQUAL(std::string("main"), descriptors[0].selectionGroup.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("main"), descriptors[1].selectionGroup.toStdString());
  }

  void ListWindowDescriptors_AfterApply_PreOrderTraversal()
  {
    // Two-row layout with three cells per row; pre-order traversal over the
    // splitter tree yields widget0..widget5 in order.
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true }, "row2": { "select_all": false } },
      "root": {
        "type": "split", "orientation": "vertical",
        "children": [
          { "type": "split", "orientation": "horizontal", "size": 1, "children": [
            { "type": "window", "id": "mxn__widget0", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
            { "type": "window", "id": "mxn__widget1", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 },
            { "type": "window", "id": "mxn__widget2", "view_direction": "coronal",  "links": { "selection": "main" }, "size": 1 }
          ]},
          { "type": "split", "orientation": "horizontal", "size": 1, "children": [
            { "type": "window", "id": "mxn__widget3", "view_direction": "axial",    "links": { "selection": "row2" }, "size": 1 },
            { "type": "window", "id": "mxn__widget4", "view_direction": "sagittal", "links": { "selection": "row2" }, "size": 1 },
            { "type": "window", "id": "mxn__widget5", "view_direction": "coronal",  "links": { "selection": "row2" }, "size": 1 }
          ]}
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);

    const auto descriptors = editor->ListWindowDescriptors();
    CPPUNIT_ASSERT_EQUAL(std::size_t(6), descriptors.size());
    for (size_t i = 0; i < 6; ++i)
    {
      const auto expected = std::string("mxn__widget") + std::to_string(i);
      CPPUNIT_ASSERT_EQUAL(expected, descriptors[i].id.toStdString());
    }
    // Row 1 is "main", row 2 is "row2".
    CPPUNIT_ASSERT_EQUAL(std::string("main"), descriptors[0].selectionGroup.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("main"), descriptors[2].selectionGroup.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("row2"), descriptors[3].selectionGroup.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("row2"), descriptors[5].selectionGroup.toStdString());
  }

  void ListWindowDescriptors_CarriesViewDirectionAndSelectionGroup()
  {
    // Apply a fixture with three view directions and verify each survives
    // round-trip through the descriptor query. Custom ids exercise the
    // explicit-id path (no positional reassignment).
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__axView",    "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__sagView",   "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__corView",   "view_direction": "coronal",  "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__originalV", "view_direction": "original", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);

    const auto descriptors = editor->ListWindowDescriptors();
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), descriptors.size());

    CPPUNIT_ASSERT_EQUAL(std::string("mxn__axView"),    descriptors[0].id.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("axial"),          descriptors[0].viewDirection.toStdString());

    CPPUNIT_ASSERT_EQUAL(std::string("mxn__sagView"),   descriptors[1].id.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("sagittal"),       descriptors[1].viewDirection.toStdString());

    CPPUNIT_ASSERT_EQUAL(std::string("mxn__corView"),   descriptors[2].id.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("coronal"),        descriptors[2].viewDirection.toStdString());

    CPPUNIT_ASSERT_EQUAL(std::string("mxn__originalV"), descriptors[3].id.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("original"),       descriptors[3].viewDirection.toStdString());

    for (const auto& d : descriptors)
      CPPUNIT_ASSERT_EQUAL(std::string("main"), d.selectionGroup.toStdString());
  }

  // ====================================================================
  // Optional display 'name' round-trips through Apply -> Serialize.
  // ====================================================================
  void WindowName_OptionalDisplay_RoundTrips()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "name": "Tumor axial", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);

    const auto roundTrip = editor->SerializeLayout();
    const auto& child = roundTrip.at("root").at("children").at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget0"), child.at("id").get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("Tumor axial"),  child.at("name").get<std::string>());
  }

  // ====================================================================
  // Display 'name' omitted on input -> not emitted on output (no empty
  // string lands on disk).
  // ====================================================================
  void WindowName_DisplayOmitted_RoundTrips()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);

    const auto roundTrip = editor->SerializeLayout();
    const auto& child = roundTrip.at("root").at("children").at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget0"), child.at("id").get<std::string>());
    CPPUNIT_ASSERT_MESSAGE(
      "Cells without a display label must NOT emit a 'name' key on serialize",
      !child.contains("name"));
  }

  // ====================================================================
  // Display 'name' is not subject to uniqueness; two cells may share the
  // same display label as long as their 'id' differs.
  // ====================================================================
  void WindowName_DisplayDuplicates_OK()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "name": "View", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__widget1", "name": "View", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_NO_THROW(editor->ApplyLayout(fixture));
  }

  // ====================================================================
  // Display 'name' is free-form: spaces, punctuation, and non-ASCII are
  // accepted.
  // ====================================================================
  void WindowName_DisplayFreeForm_OK()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "name": "Brain - axial (T1, FLAIR)", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_NO_THROW(editor->ApplyLayout(fixture));

    const auto roundTrip = editor->SerializeLayout();
    CPPUNIT_ASSERT_EQUAL(std::string("Brain - axial (T1, FLAIR)"),
                         roundTrip.at("root").at("children").at(0).at("name").get<std::string>());
  }

  // ====================================================================
  // Empty 'name' is rejected. Tools must omit the field instead.
  // ====================================================================
  void WindowName_DisplayEmptyString_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "name": "", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    try
    {
      editor->ApplyLayout(fixture);
      CPPUNIT_FAIL("ApplyLayout must throw on empty display 'name'");
    }
    catch (const mitk::Exception& e)
    {
      const std::string msg = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE("Exception message must name the offending cell id",
                             msg.find("widget0") != std::string::npos);
    }
  }

  // ====================================================================
  // Non-string 'name' is rejected.
  // ====================================================================
  void WindowName_DisplayNotString_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "name": 42, "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    CPPUNIT_ASSERT_THROW(editor->ApplyLayout(fixture), mitk::Exception);
  }

  // ====================================================================
  // ListWindowDescriptors surfaces the optional display label via
  // WindowDescriptor::displayName; cells without one carry an empty
  // string.
  // ====================================================================
  void ListWindowDescriptors_CarriesDisplayName()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__widget0", "name": "Tumor axial", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__widget1",                          "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    editor->ApplyLayout(fixture);

    const auto descriptors = editor->ListWindowDescriptors();
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), descriptors.size());
    CPPUNIT_ASSERT_EQUAL(std::string("Tumor axial"), descriptors[0].displayName.toStdString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Cells without a display label must surface an empty displayName",
      std::string{}, descriptors[1].displayName.toStdString());
  }

  // ====================================================================
  // A document whose id has no '<editor>__' prefix is rejected up-front.
  // The loader's instance check runs before any engine state mutation,
  // so the existing layout survives the failed apply.
  // ====================================================================
  void ApplyLayout_UnprefixedId_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    try
    {
      editor->ApplyLayout(fixture);
      CPPUNIT_FAIL("ApplyLayout must throw on an unprefixed window id");
    }
    catch (const mitk::Exception& e)
    {
      const std::string msg = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE("Exception message must name the offending id",
                             msg.find("widget0") != std::string::npos);
      CPPUNIT_ASSERT_MESSAGE("Exception message must name the required prefix",
                             msg.find("mxn__") != std::string::npos);
    }
    // Pre-mutation throw: the existing single-cell layout from MakeEditor
    // must still be intact.
    CPPUNIT_ASSERT_EQUAL(1u, editor->GetNumberOfRenderWindowWidgets());
  }

  // ====================================================================
  // A document whose id carries a different editor's prefix is rejected.
  // ====================================================================
  void ApplyLayout_WrongEditorPrefix_Throws()
  {
    const auto fixture = nlohmann::json::parse(R"json({
      "version": "2.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "stdmulti__widget0", "view_direction": "axial", "links": { "selection": "main" }, "size": 1 }
        ]
      }
    })json");

    auto editor = MakeEditor();
    try
    {
      editor->ApplyLayout(fixture);
      CPPUNIT_FAIL("ApplyLayout must throw on a wrong-editor prefix");
    }
    catch (const mitk::Exception& e)
    {
      const std::string msg = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE("Exception message must name the offending id",
                             msg.find("stdmulti__widget0") != std::string::npos);
    }
    CPPUNIT_ASSERT_EQUAL(1u, editor->GetNumberOfRenderWindowWidgets());
  }

  // ====================================================================
  // The constructor rejects multiWidgetName values that would produce
  // schema-invalid ids (containing '_' / '__', empty, leading digit, ...).
  // ====================================================================
  void Construct_BadMultiWidgetName_Throws()
  {
    for (const auto& bad : { QString("bad__name"),
                             QString("bad_name"),
                             QString(""),
                             QString("1abc"),
                             QString(".abc"),
                             QString("ab cd") })
    {
      try
      {
        QmitkMxNMultiWidget invalid(/*parent=*/nullptr, /*flags=*/{}, bad);
        CPPUNIT_FAIL((std::string("Constructor must throw on multiWidgetName '")
                      + bad.toStdString() + "'").c_str());
      }
      catch (const mitk::Exception&)
      {
        // expected
      }
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNLayoutV2)
