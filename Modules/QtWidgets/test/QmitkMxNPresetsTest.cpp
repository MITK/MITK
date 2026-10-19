/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNDynamicLayoutDialog.h>
#include <QmitkMxNLayoutEditorWidget.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNPresets.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkBaseRenderer.h>
#include <mitkImageGenerator.h>
#include <mitkSliceNavigationController.h>
#include <mitkSliceNavigationHelper.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStepper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <nlohmann/json.hpp>

#include <vtkCamera.h>
#include <vtkRenderer.h>

#include <QApplication>
#include <QCoreApplication>
#include <QMenu>
#include <QTimer>
#include <QStringList>
#include <QToolButton>

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

/**
 * The layout presets MITK ships, applied the way the Layout Editor view
 * applies them: an entry of a layout editor widget's "Presets" menu, then the
 * document its LoadLayout signal carries. Each preset's specification is
 * written out here, independently of the preset files, and checked on the
 * default-named editor and on one with another name.
 */
class QmitkMxNPresetsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNPresetsTestSuite);

  MITK_TEST(Menu_ListsThePresetsInOrder);
  MITK_TEST(Menu_EndsWithTheDynamicLayoutEntry);
  MITK_TEST(DynamicLayoutEntry_NeedsImageData);
  MITK_TEST(DynamicLayoutEntry_OpensTheDialog);
  MITK_TEST(Presets_AreIdentifiedByTheirFileNames);
  MITK_TEST(DocumentFor_QualifiesTheWindowIds);
  MITK_TEST(Presets_MatchTheirSpecification_OnTheDefaultNamedEditor);
  MITK_TEST(Presets_MatchTheirSpecification_OnAnotherNamedEditor);
  MITK_TEST(Lightbox_StartsOneSliceApartPerWindow);
  MITK_TEST(OverviewDetail_StartsTheDetailAtFourTimesTheZoom);

  CPPUNIT_TEST_SUITE_END();

  using Dimension = QmitkMxNSyncDimension;
  using ViewDirection = QmitkMxNMultiWidget::ViewDirection;

  struct Link
  {
    Dimension dimension;
    std::string group;
    int sliceOffset = 0;
    double zoomOffset = 1.0;
  };

  struct Window
  {
    QString name;
    ViewDirection direction;
    bool threeD;
    std::string selection;
    std::vector<Link> links;
  };

  struct Preset
  {
    QString menuName;
    std::vector<Window> windows;               // pre-order
    std::map<std::string, std::string> groups;  // id -> display name; exactly the editor's groups
  };

  mitk::DataStorage::Pointer m_DataStorage;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;
  std::unique_ptr<QmitkMxNLayoutEditorWidget> m_LayoutEditor;

public:
  void setUp() override
  {
    EnsureQApplication();

    // Deep enough that the lightbox's eight slices around the fitted middle
    // stay clear of both ends of the stack.
    m_DataStorage = mitk::StandaloneDataStorage::New();
    auto imageNode = mitk::DataNode::New();
    imageNode->SetName("image");
    imageNode->SetData(mitk::ImageGenerator::GenerateGradientImage<short>(32, 32, 32, 1.0f, 1.0f, 1.0f));
    imageNode->SetIntProperty("layer", 0);
    m_DataStorage->Add(imageNode);

    m_Editor = this->MakeEditor(QStringLiteral("mxn"));
    m_LayoutEditor = std::make_unique<QmitkMxNLayoutEditorWidget>();
    m_LayoutEditor->SetMultiWidget(m_Editor.get());
  }

  void tearDown() override
  {
    m_LayoutEditor.reset();
    m_Editor.reset();
    m_DataStorage = nullptr;
  }

  std::unique_ptr<QmitkMxNMultiWidget> MakeEditor(const QString& name) const
  {
    auto editor = std::make_unique<QmitkMxNMultiWidget>(nullptr, Qt::WindowFlags{}, name);
    editor->SetDataStorage(m_DataStorage);
    editor->InitializeMultiWidget();
    editor->resize(1200, 800);
    editor->show();
    Pump();
    return editor;
  }

  static void Pump()
  {
    for (int i = 0; i < 20; ++i)
    {
      QCoreApplication::processEvents();
    }
  }

  static QMenu* PresetMenu(QmitkMxNLayoutEditorWidget& layoutEditor)
  {
    for (auto* button : layoutEditor.findChildren<QToolButton*>())
    {
      if (QStringLiteral("Presets") == button->text())
      {
        CPPUNIT_ASSERT_MESSAGE("The Presets button has a menu", nullptr != button->menu());
        return button->menu();
      }
    }
    CPPUNIT_FAIL("The layout editor offers a Presets button");
    return nullptr;
  }

  /** Trigger the menu entry 'menuName' and apply what it emits, as the view does. */
  static void ApplyPreset(QmitkMxNLayoutEditorWidget& layoutEditor, QmitkMxNMultiWidget& editor,
                          const QString& menuName)
  {
    QAction* entry = nullptr;
    for (auto* action : PresetMenu(layoutEditor)->actions())
    {
      if (action->text() == menuName)
      {
        entry = action;
      }
    }
    CPPUNIT_ASSERT_MESSAGE("The Presets menu offers '" + menuName.toStdString() + "'", nullptr != entry);

    bool applied = false;
    const auto connection = QObject::connect(
      &layoutEditor, &QmitkMxNLayoutEditorWidget::LoadLayout,
      [&applied, &editor](const nlohmann::json* doc)
      {
        editor.ApplyLayout(*doc);
        applied = true;
      });
    entry->trigger();
    QObject::disconnect(connection);
    CPPUNIT_ASSERT_MESSAGE("The entry emits a layout", applied);
    Pump();
  }

  static std::vector<Preset> Specification()
  {
    const auto mpr = [](const char* name, ViewDirection direction) {
      return Window{ name, direction, false, "main",
                     { { Dimension::Crosshair, "main" }, { Dimension::Windowing, "main" }, { Dimension::Lut, "main" } } };
    };
    const auto look = [](const char* name, ViewDirection direction, const char* selection, const char* column) {
      return Window{ name, direction, false, selection,
                     { { Dimension::Crosshair, "nav" }, { Dimension::Slice, column }, { Dimension::Zoom, column },
                       { Dimension::Pan, column }, { Dimension::Windowing, selection }, { Dimension::Lut, selection } } };
    };
    const auto side = [](const char* name, ViewDirection direction) {
      return Window{ name, direction, true, "main", { { Dimension::Zoom, "main" }, { Dimension::Pan, "main" } } };
    };
    const std::map<std::string, std::string> mainOnly = { { "main", "main" } };

    std::vector<Window> lightbox;
    for (int i = 0; i < 9; ++i)
    {
      lightbox.push_back({ QString(), ViewDirection::Axial, false, "main",
                           { { Dimension::Crosshair, "main" }, { Dimension::Slice, "main", i - 4 },
                             { Dimension::Zoom, "main" },
                             { Dimension::Pan, "main" }, { Dimension::Windowing, "main" },
                             { Dimension::Lut, "main" } } });
    }

    return {
      { "MPR + 3D",
        { mpr("Axial", ViewDirection::Axial), mpr("Sagittal", ViewDirection::Sagittal),
          mpr("Coronal", ViewDirection::Coronal),
          { "3D", ViewDirection::Coronal, true, "main", {} } },
        mainOnly },
      { "Focus + context",
        { { "Working plane", ViewDirection::Axial, false, "main",
            { { Dimension::Crosshair, "main" }, { Dimension::Zoom, "main" }, { Dimension::Windowing, "main" },
              { Dimension::Lut, "main" } } },
          mpr("Context 1", ViewDirection::Sagittal), mpr("Context 2", ViewDirection::Coronal),
          { "3D check", ViewDirection::Coronal, true, "main", { { Dimension::Zoom, "main" } } } },
        mainOnly },
      { "Same data, two looks",
        { look("Look A", ViewDirection::Axial, "main", "ax"), look("Look A", ViewDirection::Sagittal, "main", "sag"),
          look("Look A", ViewDirection::Coronal, "main", "cor"), look("Look B", ViewDirection::Axial, "lookB", "ax"),
          look("Look B", ViewDirection::Sagittal, "lookB", "sag"),
          look("Look B", ViewDirection::Coronal, "lookB", "cor") },
        { { "main", "Look A" }, { "lookB", "Look B" }, { "nav", "Crosshair" }, { "ax", "Axial column" },
          { "sag", "Sagittal column" }, { "cor", "Coronal column" } } },
      { "Lightbox 3x3", lightbox, mainOnly },
      { "Slice context",
        { { "Previous slice", ViewDirection::Axial, false, "main",
            { { Dimension::Crosshair, "main" }, { Dimension::Slice, "ctx", -1 }, { Dimension::Zoom, "ctx" }, { Dimension::Pan, "ctx" },
              { Dimension::Windowing, "main" }, { Dimension::Lut, "main" } } },
          { "Working slice", ViewDirection::Axial, false, "main",
            { { Dimension::Crosshair, "main" }, { Dimension::Slice, "ctx" }, { Dimension::Zoom, "ctx" },
              { Dimension::Pan, "ctx" }, { Dimension::Windowing, "main" }, { Dimension::Lut, "main" } } },
          { "Next slice", ViewDirection::Axial, false, "main",
            { { Dimension::Crosshair, "main" }, { Dimension::Slice, "ctx", 1 }, { Dimension::Zoom, "ctx" }, { Dimension::Pan, "ctx" },
              { Dimension::Windowing, "main" }, { Dimension::Lut, "main" } } },
          mpr("Context 1", ViewDirection::Sagittal), mpr("Context 2", ViewDirection::Coronal) },
        { { "main", "main" }, { "ctx", "Slice context" } } },
      { "2D | 3D",
        { { "Edit", ViewDirection::Axial, false, "main",
            { { Dimension::Zoom, "main" }, { Dimension::Windowing, "main" }, { Dimension::Lut, "main" } } },
          { "3D check", ViewDirection::Coronal, true, "main", { { Dimension::Zoom, "main" } } } },
        mainOnly },
      { "3D from three sides",
        { side("Coronal", ViewDirection::Coronal), side("Sagittal", ViewDirection::Sagittal),
          side("Axial", ViewDirection::Axial), side("Free", ViewDirection::Free) },
        mainOnly },
      { "Overview + detail",
        { { "Overview", ViewDirection::Axial, false, "main",
            { { Dimension::Crosshair, "main" }, { Dimension::Slice, "main" }, { Dimension::Zoom, "main" },
              { Dimension::Pan, "main" }, { Dimension::Windowing, "main" }, { Dimension::Lut, "main" } } },
          { "Detail", ViewDirection::Axial, false, "main",
            { { Dimension::Crosshair, "main" }, { Dimension::Slice, "main" }, { Dimension::Zoom, "main", 0, 4.0 },
              { Dimension::Pan, "main" }, { Dimension::Windowing, "main" }, { Dimension::Lut, "main" } } } },
        mainOnly },
    };
  }

  static void CheckPreset(const QmitkMxNMultiWidget& editor, const Preset& preset)
  {
    const std::string where = " (" + preset.menuName.toStdString() + " on '"
                              + editor.GetMultiWidgetName().toStdString() + "')";
    const auto descriptors = editor.ListWindowDescriptors();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The window count" + where, preset.windows.size(), descriptors.size());

    for (std::size_t i = 0; i < preset.windows.size(); ++i)
    {
      const auto& expected = preset.windows[i];
      const auto& id = descriptors[i].id;
      const std::string at = " of window " + std::to_string(i) + where;

      CPPUNIT_ASSERT_EQUAL_MESSAGE("The id" + at,
                                   (editor.GetMultiWidgetName() + "__widget" + QString::number(i)).toStdString(),
                                   id.toStdString());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The name" + at, expected.name.toStdString(),
                                   descriptors[i].displayName.toStdString());
      CPPUNIT_ASSERT_MESSAGE("The direction" + at, expected.direction == editor.GetViewDirection(id));
      CPPUNIT_ASSERT_MESSAGE("The render mode" + at,
                             (expected.threeD ? QmitkMxNMultiWidget::RenderMode::ThreeD
                                              : QmitkMxNMultiWidget::RenderMode::TwoD) == editor.GetRenderMode(id));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The selection group" + at, expected.selection,
                                   editor.GetCellSelectionGroup(id));

      for (const auto dimension : { Dimension::Pan, Dimension::Zoom, Dimension::Slice, Dimension::Crosshair,
                                    Dimension::Orientation, Dimension::Windowing, Dimension::Lut })
      {
        const std::string on = std::string(" on ") + QmitkMxNSyncDimensionToLinkKey(dimension) + at;
        const auto link = editor.GetSyncLink(id, dimension);
        const auto spec = std::find_if(expected.links.begin(), expected.links.end(),
                                       [dimension](const Link& l) { return l.dimension == dimension; });
        if (spec == expected.links.end())
        {
          CPPUNIT_ASSERT_MESSAGE("No link" + on, !link.has_value());
          continue;
        }
        CPPUNIT_ASSERT_MESSAGE("A link" + on, link.has_value());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The group" + on, spec->group, link->group);
        if (Dimension::Slice == dimension)
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("The slice offset" + on, spec->sliceOffset, std::get<int>(link->offset));
        }
        else if (Dimension::Zoom == dimension)
        {
          CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The zoom offset" + on, spec->zoomOffset,
                                               std::get<double>(link->offset), 1e-9);
        }
      }
    }

    std::map<std::string, std::string> groups;
    for (const auto& info : editor.GetSyncGroupInfos())
    {
      groups[info.id] = info.displayName;
    }
    CPPUNIT_ASSERT_MESSAGE("Exactly the preset's groups, with their display names" + where, preset.groups == groups);
  }

  // --- The menu --------------------------------------------------------------

  void Menu_ListsThePresetsInOrder()
  {
    QStringList expected;
    for (const auto& preset : Specification())
    {
      expected << preset.menuName;
    }
    QStringList listed;
    for (const auto* action : PresetMenu(*m_LayoutEditor)->actions())
    {
      if (action->isSeparator())
      {
        break;
      }
      listed << action->text();
    }
    CPPUNIT_ASSERT_EQUAL(expected.join(QStringLiteral(" | ")).toStdString(),
                         listed.join(QStringLiteral(" | ")).toStdString());
  }

  // --- The dynamic layout entry ----------------------------------------------

  /** The last entry of the Presets menu, as it is when the menu opens. */
  static QAction* DynamicLayoutEntry(QmitkMxNLayoutEditorWidget& layoutEditor)
  {
    auto* menu = PresetMenu(layoutEditor);
    emit menu->aboutToShow();
    const auto actions = menu->actions();
    CPPUNIT_ASSERT(actions.size() >= 2);
    return actions.back();
  }

  void Menu_EndsWithTheDynamicLayoutEntry()
  {
    const auto actions = PresetMenu(*m_LayoutEditor)->actions();
    CPPUNIT_ASSERT_MESSAGE("A separator sets the entry apart from the presets",
                           actions[actions.size() - 2]->isSeparator());
    CPPUNIT_ASSERT_EQUAL(std::string("Dynamic layout..."), actions.back()->text().toStdString());
  }

  void DynamicLayoutEntry_NeedsImageData()
  {
    auto empty = mitk::StandaloneDataStorage::New();
    m_LayoutEditor->SetDataStorage(empty);
    CPPUNIT_ASSERT_MESSAGE("Without an image there is nothing to lay out",
                           !DynamicLayoutEntry(*m_LayoutEditor)->isEnabled());

    m_LayoutEditor->SetDataStorage(m_DataStorage);
    CPPUNIT_ASSERT_MESSAGE("An image is enough", DynamicLayoutEntry(*m_LayoutEditor)->isEnabled());
  }

  void DynamicLayoutEntry_OpensTheDialog()
  {
    m_LayoutEditor->SetDataStorage(m_DataStorage);
    auto* entry = DynamicLayoutEntry(*m_LayoutEditor);

    // The dialog runs its own loop; the poll answers it.
    bool dialogFound = false;
    QTimer poll;
    poll.setInterval(5);
    QObject::connect(&poll, &QTimer::timeout, [&dialogFound]()
    {
      if (auto* dialog = qobject_cast<QmitkMxNDynamicLayoutDialog*>(QApplication::activeModalWidget()))
      {
        dialogFound = true;
        dialog->reject();
      }
    });
    poll.start();
    entry->trigger();
    poll.stop();

    CPPUNIT_ASSERT_MESSAGE("The entry opens the dynamic layout dialog", dialogFound);
  }

  // --- The presets as a module service ---------------------------------------

  void Presets_AreIdentifiedByTheirFileNames()
  {
    // Preferences store a preset by its id, so the ids must not follow the
    // display names.
    const QStringList expected = { "mxnLayout_01_mprAnd3d",     "mxnLayout_02_focusContext",
                                   "mxnLayout_03_twoLooks",     "mxnLayout_04_lightbox",
                                   "mxnLayout_05_sliceContext", "mxnLayout_06_2dAnd3d",
                                   "mxnLayout_07_3dThreeSides", "mxnLayout_08_overviewDetail" };
    QStringList ids;
    QStringList names;
    for (const auto& preset : QmitkMxNPresets::All())
    {
      ids << preset.id;
      names << preset.name;
    }
    CPPUNIT_ASSERT_EQUAL(expected.join(QStringLiteral(" | ")).toStdString(),
                         ids.join(QStringLiteral(" | ")).toStdString());

    QStringList menuNames;
    for (const auto& preset : Specification())
    {
      menuNames << preset.menuName;
    }
    CPPUNIT_ASSERT_EQUAL(menuNames.join(QStringLiteral(" | ")).toStdString(),
                         names.join(QStringLiteral(" | ")).toStdString());
  }

  /** The window ids below 'node'. */
  static void CollectWindowIds(const nlohmann::json& node, std::vector<std::string>& ids)
  {
    if ("window" == node.value("type", std::string()))
    {
      ids.push_back(node.at("id").get<std::string>());
    }
    for (const auto& child : node.value("children", nlohmann::json::array()))
    {
      CollectWindowIds(child, ids);
    }
  }

  void DocumentFor_QualifiesTheWindowIds()
  {
    for (const auto& preset : QmitkMxNPresets::All())
    {
      std::vector<std::string> ids;
      CollectWindowIds(QmitkMxNPresets::DocumentFor(preset, QStringLiteral("other")).at("root"), ids);
      CPPUNIT_ASSERT_MESSAGE("Every preset has windows", !ids.empty());
      for (const auto& id : ids)
      {
        CPPUNIT_ASSERT_MESSAGE(id + " is qualified for 'other'", 0 == id.rfind("other__", 0));
      }

      ids.clear();
      CollectWindowIds(preset.document.at("root"), ids);
      for (const auto& id : ids)
      {
        CPPUNIT_ASSERT_MESSAGE(id + " stays as shipped", 0 == id.rfind("mxn__", 0));
      }
    }
  }

  // --- Every preset ----------------------------------------------------------

  void Presets_MatchTheirSpecification_OnTheDefaultNamedEditor()
  {
    for (const auto& preset : Specification())
    {
      ApplyPreset(*m_LayoutEditor, *m_Editor, preset.menuName);
      CheckPreset(*m_Editor, preset);
    }
  }

  void Presets_MatchTheirSpecification_OnAnotherNamedEditor()
  {
    auto custom = this->MakeEditor(QStringLiteral("custom"));
    QmitkMxNLayoutEditorWidget customLayoutEditor;
    customLayoutEditor.SetMultiWidget(custom.get());

    for (const auto& preset : Specification())
    {
      ApplyPreset(customLayoutEditor, *custom, preset.menuName);
      CheckPreset(*custom, preset);
    }
  }

  // --- The initial relative state ------------------------------------------

  mitk::BaseRenderer* Renderer(std::size_t index) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(QStringLiteral("mxn__widget") + QString::number(index));
    CPPUNIT_ASSERT(nullptr != cell);
    return mitk::BaseRenderer::GetInstance(cell->GetRenderWindow()->GetVtkRenderWindow());
  }

  /** The displayed slice index: the one the navigator shows and slice offsets count. */
  unsigned int ShownSlice(std::size_t index) const
  {
    auto* renderer = this->Renderer(index);
    const auto* stepper = renderer->GetSliceNavigationController()->GetStepper();
    const unsigned int last = stepper->GetSteps() - 1;
    return mitk::SliceNavigationHelper::IsDisplayedSliceInverted(renderer) ? last - stepper->GetPos()
                                                                          : stepper->GetPos();
  }

  void Lightbox_StartsOneSliceApartPerWindow()
  {
    ApplyPreset(*m_LayoutEditor, *m_Editor, QStringLiteral("Lightbox 3x3"));

    for (std::size_t i = 1; i < 9; ++i)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Window " + std::to_string(i) + " shows the slice after its predecessor's",
                                   this->ShownSlice(i - 1) + 1, this->ShownSlice(i));
    }
  }

  void OverviewDetail_StartsTheDetailAtFourTimesTheZoom()
  {
    ApplyPreset(*m_LayoutEditor, *m_Editor, QStringLiteral("Overview + detail"));

    const double overview = this->Renderer(0)->GetVtkRenderer()->GetActiveCamera()->GetParallelScale();
    const double detail = this->Renderer(1)->GetVtkRenderer()->GetActiveCamera()->GetParallelScale();
    CPPUNIT_ASSERT_MESSAGE("The overview is fitted", overview > 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The detail shows a quarter of the overview's height",
                                         overview / 4.0, detail, 1e-6 * overview);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNPresets)
