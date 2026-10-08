/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestPopupProbe.h"
#include "QmitkTestQApplication.h"

#include <QmitkMxNCellOverlay.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowProximity.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>
#include <QmitkSynchronizedNodeSelectionWidget.h>

#include <mitkException.h>
#include <mitkImageGenerator.h>
#include <mitkRenderingManager.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QContextMenuEvent>
#include <QDir>
#include <QImage>
#include <QMenu>
#include <QMouseEvent>
#include <QSplitter>
#include <QToolButton>

#include <fstream>
#include <memory>
#include <vector>

/**
 * Tests the MxN layout lock: the declaration parsers that turn plugin.xml
 * values into a lock set and a widget name, the lock state on the widget, and
 * every user-facing seam a lock closes, for cells present when the lock is set,
 * cells created later and a live lock change in both directions. Programmatic
 * calls must keep working on a locked widget.
 *
 * Also applies the layouts the example plugin org.mitk.example.gui.mxneditor
 * ships, whose resource directory is the last command-line argument. Nothing
 * starts that example automatically, so this is what catches window-ID or
 * schema drift in its files.
 */
class QmitkMxNLayoutLockTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNLayoutLockTestSuite);
  MITK_TEST(Parse_EachTokenAlone);
  MITK_TEST(Parse_AllTokens);
  MITK_TEST(Parse_TrimsWhitespace);
  MITK_TEST(Parse_EmptyOrBlank_YieldsNoLock);
  MITK_TEST(Parse_UnknownToken_Throws);
  MITK_TEST(Parse_WrongCase_Throws);
  MITK_TEST(Parse_EmptyToken_Throws);
  MITK_TEST(ValidName_Accepted);
  MITK_TEST(InvalidName_Rejected);
  MITK_TEST(Locks_DefaultToNone);
  MITK_TEST(Signal_FiresOncePerActualChange);
  MITK_TEST(ExampleLayouts_ApplyToTheDemoEditor);
  MITK_TEST(Unlocked_AllSeamsOpen);
  MITK_TEST(LockedBeforeInitialize_FirstCellIsLocked);
  MITK_TEST(LockedCells_FromSetLayout);
  MITK_TEST(LockedCells_FromGridOperation);
  MITK_TEST(LockedCells_FromApplyLayout);
  MITK_TEST(LockedSplitters_FromDataBasedLayout);
  MITK_TEST(LiveLock_ClosesAndReopensExistingCells);
  MITK_TEST(SingleAspects_CloseOnlyTheirSeams);
  MITK_TEST(Barcode_LiveLockUnderThePointerDropsTheHighlight);
  MITK_TEST(Barcode_PeekStillReportsWhileLocked);
  MITK_TEST(PlaneLabel_LiveLockUnderThePointerDropsTheHighlight);
  MITK_TEST(ProgrammaticCalls_WorkOnALockedWidget);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_Image;
  mitk::DataNode::Pointer m_ImageNode;

  static QmitkMxNLayoutLocks AllLocks()
  {
    return QmitkMxNLayoutLock::Structure | QmitkMxNLayoutLock::Direction
         | QmitkMxNLayoutLock::Proportions | QmitkMxNLayoutLock::Content;
  }

public:
  void setUp() override
  {
    EnsureQApplication();
    m_DataStorage = mitk::StandaloneDataStorage::New();

    // A plane label and a level/window need an image under the cells.
    m_Image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);
    m_ImageNode = mitk::DataNode::New();
    m_ImageNode->SetName("image");
    m_ImageNode->SetData(m_Image);
    m_ImageNode->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_ImageNode);
  }

  void tearDown() override
  {
    m_ImageNode = nullptr;
    m_Image = nullptr;
    m_DataStorage = nullptr;
  }

  /** A shown widget named 'mxn' with 'locks' set before InitializeMultiWidget,
   *  as the editor does for declared locks. */
  std::unique_ptr<QmitkMxNMultiWidget> MakeWidget(QmitkMxNLayoutLocks locks) const
  {
    auto widget = std::make_unique<QmitkMxNMultiWidget>();
    widget->SetDataStorage(m_DataStorage);
    widget->SetLayoutLocks(locks);
    widget->InitializeMultiWidget();
    widget->resize(640, 480);
    widget->show();
    this->InitializeViews(*widget);
    return widget;
  }

  void InitializeViews(QmitkMxNMultiWidget& widget) const
  {
    for (const auto& [name, cell] : widget.GetRenderWindowWidgets())
    {
      mitk::RenderingManager::GetInstance()->InitializeView(
        cell->GetRenderWindow()->GetVtkRenderWindow(), m_Image->GetTimeGeometry());
    }
    QmitkTestPopupProbe::Pump(100);
  }

  static QmitkRenderWindowWidget* Cell(const QmitkMxNMultiWidget& widget, const QString& id)
  {
    auto cell = widget.GetRenderWindowWidget(id);
    CPPUNIT_ASSERT_MESSAGE("Cell must exist: " + id.toStdString(), nullptr != cell);
    return cell.get();
  }

  static QmitkMxNSyncBarcodeWidget* Barcode(const QmitkMxNMultiWidget& widget, const QString& id)
  {
    auto* barcode = Cell(widget, id)->findChild<QmitkMxNSyncBarcodeWidget*>();
    CPPUNIT_ASSERT(nullptr != barcode);
    CPPUNIT_ASSERT_MESSAGE("Precondition: the barcode has slots", !barcode->Slots().isEmpty());
    // The strip auto-hides, so it may never have been laid out, and its layout
    // resizes it as the strip changes; a fixed size gives its drawn slots an
    // extent and keeps grabs of it comparable.
    barcode->setFixedSize(200, 22);
    return barcode;
  }

  /** A point on the barcode's drawn slots, which are left-aligned and centered
   *  vertically. */
  static QPoint OnBarcodeSlots(const QmitkMxNSyncBarcodeWidget* barcode)
  {
    return QPoint(4, barcode->height() / 2);
  }

  static QToolButton* DataButton(const QmitkMxNMultiWidget& widget, const QString& id)
  {
    for (auto* button : Cell(widget, id)->GetUtilityWidget()->findChildren<QToolButton*>())
    {
      if (button->text() == QStringLiteral("Data"))
      {
        return button;
      }
    }
    CPPUNIT_FAIL("The utility strip must have a Data button");
    return nullptr;
  }

  static QmitkMxNCellOverlay* Overlay(const QmitkMxNMultiWidget& widget, const QString& id)
  {
    auto* overlay = Cell(widget, id)->findChild<QmitkMxNCellOverlay*>();
    CPPUNIT_ASSERT(nullptr != overlay);
    return overlay;
  }

  static void SendMouse(QWidget* target, QEvent::Type type, const QPoint& position, Qt::MouseButton button)
  {
    const Qt::MouseButtons buttons =
      QEvent::MouseButtonPress == type ? Qt::MouseButtons(button) : Qt::MouseButtons(Qt::NoButton);
    QMouseEvent event(type, QPointF(position), QPointF(position), QPointF(target->mapToGlobal(position)), button,
                      buttons, Qt::NoModifier);
    QCoreApplication::sendEvent(target, &event);
  }

  static void Click(QWidget* target, const QPoint& position)
  {
    SendMouse(target, QEvent::MouseButtonPress, position, Qt::LeftButton);
    SendMouse(target, QEvent::MouseButtonRelease, position, Qt::LeftButton);
  }

  /** The top-level entries of the cell's context menu, opened as from the
   *  keyboard. */
  static QStringList ContextMenuEntries(const QmitkMxNMultiWidget& widget, const QString& id)
  {
    auto* renderWindow = Cell(widget, id)->GetRenderWindow();
    QStringList entries;
    QmitkTestPopupProbe probe;
    probe.inspect = [&entries](QWidget* popup)
    {
      if (auto* menu = qobject_cast<QMenu*>(popup))
      {
        for (const auto* action : menu->actions())
        {
          entries << action->text();
        }
      }
    };
    QContextMenuEvent request(QContextMenuEvent::Keyboard, renderWindow->rect().center(),
                              renderWindow->mapToGlobal(renderWindow->rect().center()));
    QCoreApplication::sendEvent(renderWindow, &request);
    QmitkTestPopupProbe::Pump();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The context menu opens", 1, probe.Count());
    return entries;
  }

  /** Puts the overlay into its interactive state (it is proximity-driven), so
   *  a click on the plane label reaches it instead of falling through. */
  static QmitkMxNCellOverlay* InteractiveOverlay(const QmitkMxNMultiWidget& widget, const QString& id)
  {
    auto* cell = Cell(widget, id);
    auto* proximity = cell->findChild<QmitkRenderWindowProximity*>();
    CPPUNIT_ASSERT(nullptr != proximity);
    const QRect area = cell->GetRenderWindow()->geometry();
    proximity->HandlePointerMoved(QPoint(area.center().x(), area.bottom() - 1));
    auto* overlay = Overlay(widget, id);
    CPPUNIT_ASSERT_MESSAGE("Precondition: the overlay takes input",
                           !overlay->testAttribute(Qt::WA_TransparentForMouseEvents));
    CPPUNIT_ASSERT_MESSAGE("Precondition: the plane label is shown", overlay->PlaneLabelRect().isValid());
    return overlay;
  }

  /** Number of popups a left click on the plane label opens. */
  static int PopupsFromPlaneLabelClick(const QmitkMxNMultiWidget& widget, const QString& id)
  {
    auto* overlay = InteractiveOverlay(widget, id);
    QmitkTestPopupProbe probe;
    Click(overlay, overlay->PlaneLabelRect().center());
    QmitkTestPopupProbe::Pump();
    return probe.Count();
  }

  /** Requests the barcode emits for a click on its drawn slots. */
  static std::size_t RequestsFromBarcodeClick(QmitkMxNMultiWidget& widget, const QString& id)
  {
    std::size_t requests = 0;
    const auto connection = QObject::connect(&widget, &QmitkMxNMultiWidget::LayoutEditorRequested,
                                             [&requests](QmitkMxNMultiWidget::LayoutEditorRequest) { ++requests; });
    auto* barcode = Barcode(widget, id);
    Click(barcode, OnBarcodeSlots(barcode));
    QObject::disconnect(connection);
    return requests;
  }

  static bool AllSplitterHandlesEnabled(const QmitkMxNMultiWidget& widget, bool enabled)
  {
    const auto splitters = widget.findChildren<QSplitter*>();
    CPPUNIT_ASSERT_MESSAGE("Precondition: the layout has splitters", !splitters.isEmpty());
    for (const auto* splitter : splitters)
    {
      for (int i = 0; i < splitter->count(); ++i)
      {
        if (splitter->handle(i)->isEnabled() != enabled)
        {
          return false;
        }
      }
    }
    return true;
  }

  /** Every seam of one cell matches 'locks': closed where locked, open
   *  otherwise. */
  void CheckCellSeams(QmitkMxNMultiWidget& widget, const QString& id, QmitkMxNLayoutLocks locks)
  {
    const std::string where = " (cell " + id.toStdString() + ")";

    const bool structure = locks.testFlag(QmitkMxNLayoutLock::Structure);
    const auto entries = ContextMenuEntries(widget, id);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Open layout editor' follows Structure" + where,
                                 !structure, entries.contains(QStringLiteral("Open layout editor")));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A barcode click follows Structure" + where,
                                 std::size_t(structure ? 0 : 1), RequestsFromBarcodeClick(widget, id));
    auto* barcode = Barcode(widget, id);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The barcode tooltip follows Structure" + where,
                                 structure, barcode->toolTip().isEmpty());

    const bool direction = locks.testFlag(QmitkMxNLayoutLock::Direction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The view-direction entries follow Direction" + where,
                                 !direction, entries.contains(QStringLiteral("View direction")));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The plane-label direction picker follows Direction" + where,
                                 direction ? 0 : 1, PopupsFromPlaneLabelClick(widget, id));

    const bool content = locks.testFlag(QmitkMxNLayoutLock::Content);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Data...' follows Content" + where,
                                 !content, entries.contains(QStringLiteral("Data...")));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The Data button follows Content" + where,
                                 content, DataButton(widget, id)->isHidden());
  }

  void CheckAllSeams(QmitkMxNMultiWidget& widget, QmitkMxNLayoutLocks locks)
  {
    for (const auto& [id, cell] : widget.GetRenderWindowWidgets())
    {
      this->CheckCellSeams(widget, id, locks);
    }
    CPPUNIT_ASSERT_MESSAGE("The splitter handles follow Proportions",
                           AllSplitterHandlesEnabled(widget, !locks.testFlag(QmitkMxNLayoutLock::Proportions)));
  }

  /** A two-cell layout document for the widget named 'mxn'. */
  static nlohmann::json TwoCellLayout()
  {
    return nlohmann::json::parse(R"({
      "version": "3.0",
      "root": { "type": "split", "orientation": "horizontal", "children": [
        { "type": "window", "id": "mxn__left", "view_direction": "axial", "links": { "selection": "main" } },
        { "type": "window", "id": "mxn__right", "view_direction": "coronal", "links": { "selection": "main" } }
      ] }
    })");
  }

  /** The plane label's pixels, to compare its highlight across states. */
  static QImage PlaneLabelImage(QmitkMxNCellOverlay* overlay)
  {
    overlay->SetRevealProgress(1.0);
    return overlay->grab(overlay->PlaneLabelRect()).toImage();
  }

  void Parse_EachTokenAlone()
  {
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks(QmitkMxNLayoutLock::Structure) == QmitkMxNParseLayoutLocks("structure"));
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks(QmitkMxNLayoutLock::Direction) == QmitkMxNParseLayoutLocks("direction"));
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks(QmitkMxNLayoutLock::Proportions) == QmitkMxNParseLayoutLocks("proportions"));
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks(QmitkMxNLayoutLock::Content) == QmitkMxNParseLayoutLocks("content"));
  }

  void Parse_AllTokens()
  {
    const auto expected = QmitkMxNLayoutLock::Structure | QmitkMxNLayoutLock::Direction
                        | QmitkMxNLayoutLock::Proportions | QmitkMxNLayoutLock::Content;
    CPPUNIT_ASSERT(expected == QmitkMxNParseLayoutLocks("structure,direction,proportions,content"));
  }

  void Parse_TrimsWhitespace()
  {
    const auto expected = QmitkMxNLayoutLock::Structure | QmitkMxNLayoutLock::Content;
    CPPUNIT_ASSERT(expected == QmitkMxNParseLayoutLocks("  structure ,\tcontent  "));
  }

  void Parse_EmptyOrBlank_YieldsNoLock()
  {
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks() == QmitkMxNParseLayoutLocks(""));
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks() == QmitkMxNParseLayoutLocks("   "));
  }

  void Parse_UnknownToken_Throws()
  {
    CPPUNIT_ASSERT_THROW(QmitkMxNParseLayoutLocks("structure,maximize"), mitk::Exception);
  }

  void Parse_WrongCase_Throws()
  {
    CPPUNIT_ASSERT_THROW(QmitkMxNParseLayoutLocks("Structure"), mitk::Exception);
  }

  void Parse_EmptyToken_Throws()
  {
    CPPUNIT_ASSERT_THROW(QmitkMxNParseLayoutLocks("structure,,direction"), mitk::Exception);
    CPPUNIT_ASSERT_THROW(QmitkMxNParseLayoutLocks("structure,"), mitk::Exception);
  }

  void ValidName_Accepted()
  {
    CPPUNIT_ASSERT(QmitkMxNIsValidMultiWidgetName("mxn"));
    CPPUNIT_ASSERT(QmitkMxNIsValidMultiWidgetName("org.mitk.editors.longitrack"));
    CPPUNIT_ASSERT(QmitkMxNIsValidMultiWidgetName("a-b.c"));
  }

  void InvalidName_Rejected()
  {
    CPPUNIT_ASSERT(!QmitkMxNIsValidMultiWidgetName(""));
    CPPUNIT_ASSERT(!QmitkMxNIsValidMultiWidgetName("1abc"));
    CPPUNIT_ASSERT(!QmitkMxNIsValidMultiWidgetName("a_b"));
    CPPUNIT_ASSERT(!QmitkMxNIsValidMultiWidgetName("a b"));
  }

  void Locks_DefaultToNone()
  {
    QmitkMxNMultiWidget widget;
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks() == widget.GetLayoutLocks());
  }

  void Signal_FiresOncePerActualChange()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    std::vector<QmitkMxNLayoutLocks> emitted;
    QObject::connect(&widget, &QmitkMxNMultiWidget::LayoutLocksChanged,
                     [&emitted](QmitkMxNLayoutLocks locks) { emitted.push_back(locks); });

    const QmitkMxNLayoutLocks locks = QmitkMxNLayoutLock::Structure | QmitkMxNLayoutLock::Direction;
    widget.SetLayoutLocks(locks);
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, emitted.size());
    CPPUNIT_ASSERT(locks == emitted[0]);
    CPPUNIT_ASSERT(locks == widget.GetLayoutLocks());

    widget.SetLayoutLocks(locks);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Setting the current locks again must not emit", std::size_t{1}, emitted.size());

    widget.SetLayoutLocks(QmitkMxNLayoutLocks());
    CPPUNIT_ASSERT_EQUAL(std::size_t{2}, emitted.size());
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks() == emitted[1]);
  }

  void ExampleLayouts_ApplyToTheDemoEditor()
  {
    CPPUNIT_ASSERT_MESSAGE("The example's resource directory must be passed as the last argument",
                           !globalCmdLineArgs.empty());
    const QDir resources(QString::fromStdString(globalCmdLineArgs.back()));

    // Named and locked as the example's declaration does.
    QmitkMxNMultiWidget widget(nullptr, {}, QStringLiteral("org.mitk.example.editors.mxndemo"));
    widget.SetDataStorage(m_DataStorage);
    widget.SetLayoutLocks(QmitkMxNParseLayoutLocks("structure,direction,proportions,content"));
    widget.InitializeMultiWidget();

    const std::pair<const char*, unsigned int> layouts[] = {
      { "MxNDemoLayout.json", 2 },
      { "MxNDemoLayoutAlt.json", 3 }
    };
    for (const auto& [fileName, cellCount] : layouts)
    {
      const auto path = resources.filePath(QString::fromUtf8(fileName)).toStdString();
      std::ifstream file(path);
      CPPUNIT_ASSERT_MESSAGE("Example layout must exist: " + path, file.good());

      const auto document = nlohmann::json::parse(file);
      CPPUNIT_ASSERT_NO_THROW_MESSAGE("Example layout must apply: " + path, widget.ApplyLayout(document));
      CPPUNIT_ASSERT_EQUAL_MESSAGE(path, cellCount, widget.GetNumberOfRenderWindowWidgets());
    }
  }

  void Unlocked_AllSeamsOpen()
  {
    auto widget = this->MakeWidget({});
    widget->SetLayout(1, 2);
    this->InitializeViews(*widget);
    this->CheckAllSeams(*widget, {});
  }

  void LockedBeforeInitialize_FirstCellIsLocked()
  {
    auto widget = this->MakeWidget(AllLocks());
    this->CheckAllSeams(*widget, AllLocks());
  }

  void LockedCells_FromSetLayout()
  {
    auto widget = this->MakeWidget(AllLocks());
    widget->SetLayout(2, 2);
    this->InitializeViews(*widget);
    this->CheckAllSeams(*widget, AllLocks());
  }

  void LockedCells_FromGridOperation()
  {
    auto widget = this->MakeWidget(AllLocks());
    widget->AddGridColumn();
    this->InitializeViews(*widget);
    CPPUNIT_ASSERT_EQUAL(2u, widget->GetNumberOfRenderWindowWidgets());
    this->CheckAllSeams(*widget, AllLocks());
  }

  void LockedCells_FromApplyLayout()
  {
    auto widget = this->MakeWidget(AllLocks());
    widget->ApplyLayout(TwoCellLayout());
    this->InitializeViews(*widget);
    CPPUNIT_ASSERT_EQUAL(2u, widget->GetNumberOfRenderWindowWidgets());
    this->CheckAllSeams(*widget, AllLocks());
  }

  void LockedSplitters_FromDataBasedLayout()
  {
    auto widget = this->MakeWidget(AllLocks());
    widget->SetDataBasedLayout({ m_ImageNode });
    CPPUNIT_ASSERT_MESSAGE("A data-based layout's handles are disabled",
                           AllSplitterHandlesEnabled(*widget, false));
  }

  void LiveLock_ClosesAndReopensExistingCells()
  {
    auto widget = this->MakeWidget({});
    widget->SetLayout(1, 2);
    this->InitializeViews(*widget);

    widget->SetLayoutLocks(AllLocks());
    this->CheckAllSeams(*widget, AllLocks());

    widget->SetLayoutLocks({});
    this->CheckAllSeams(*widget, {});
  }

  void SingleAspects_CloseOnlyTheirSeams()
  {
    auto widget = this->MakeWidget({});
    widget->SetLayout(1, 2);
    this->InitializeViews(*widget);
    for (const auto lock : { QmitkMxNLayoutLock::Structure, QmitkMxNLayoutLock::Direction,
                             QmitkMxNLayoutLock::Proportions, QmitkMxNLayoutLock::Content })
    {
      widget->SetLayoutLocks(lock);
      this->CheckAllSeams(*widget, lock);
    }
  }

  void Barcode_LiveLockUnderThePointerDropsTheHighlight()
  {
    auto widget = this->MakeWidget({});
    const auto id = widget->GetRenderWindowWidgets().begin()->first;
    auto* barcode = Barcode(*widget, id);
    const QImage idle = barcode->grab().toImage();

    SendMouse(barcode, QEvent::MouseMove, OnBarcodeSlots(barcode), Qt::NoButton);
    CPPUNIT_ASSERT_MESSAGE("Fixture: hovering the unlocked strip highlights it", idle != barcode->grab().toImage());
    CPPUNIT_ASSERT(Qt::PointingHandCursor == barcode->cursor().shape());

    // The pointer stays where it is; only the lock changes.
    widget->SetLayoutLocks(QmitkMxNLayoutLock::Structure);
    CPPUNIT_ASSERT_MESSAGE("A live lock drops the highlight under a resting pointer",
                           idle == barcode->grab().toImage());
    CPPUNIT_ASSERT(Qt::PointingHandCursor != barcode->cursor().shape());
    CPPUNIT_ASSERT(barcode->toolTip().isEmpty());

    SendMouse(barcode, QEvent::MouseMove, OnBarcodeSlots(barcode) + QPoint(1, 0), Qt::NoButton);
    CPPUNIT_ASSERT_MESSAGE("A locked strip is not highlighted on hover", idle == barcode->grab().toImage());

    widget->SetLayoutLocks({});
    CPPUNIT_ASSERT_MESSAGE("A live unlock restores the highlight under a resting pointer",
                           idle != barcode->grab().toImage());
    CPPUNIT_ASSERT(Qt::PointingHandCursor == barcode->cursor().shape());
    CPPUNIT_ASSERT(!barcode->toolTip().isEmpty());
  }

  void Barcode_PeekStillReportsWhileLocked()
  {
    auto widget = this->MakeWidget(QmitkMxNLayoutLock::Structure);
    const auto id = widget->GetRenderWindowWidgets().begin()->first;
    auto* barcode = Barcode(*widget, id);

    std::vector<bool> reports;
    QObject::connect(barcode, &QmitkMxNSyncBarcodeWidget::PeekHovered,
                     [&reports](bool overStrip, int) { reports.push_back(overStrip); });
    SendMouse(barcode, QEvent::MouseMove, OnBarcodeSlots(barcode), Qt::NoButton);

    CPPUNIT_ASSERT_MESSAGE("The sync peek still answers a locked strip", !reports.empty() && reports.back());
  }

  void PlaneLabel_LiveLockUnderThePointerDropsTheHighlight()
  {
    auto widget = this->MakeWidget({});
    const auto id = widget->GetRenderWindowWidgets().begin()->first;
    auto* overlay = InteractiveOverlay(*widget, id);
    const QPoint onLabel = overlay->PlaneLabelRect().center();
    const QImage idle = PlaneLabelImage(overlay);

    SendMouse(overlay, QEvent::MouseMove, onLabel, Qt::NoButton);
    CPPUNIT_ASSERT_MESSAGE("Fixture: hovering the plane label highlights it", idle != PlaneLabelImage(overlay));

    widget->SetLayoutLocks(QmitkMxNLayoutLock::Direction);
    CPPUNIT_ASSERT_MESSAGE("A live lock drops the highlight under a resting pointer",
                           idle == PlaneLabelImage(overlay));

    SendMouse(overlay, QEvent::MouseMove, onLabel + QPoint(1, 0), Qt::NoButton);
    CPPUNIT_ASSERT_MESSAGE("A locked plane label is not highlighted on hover", idle == PlaneLabelImage(overlay));
  }

  void ProgrammaticCalls_WorkOnALockedWidget()
  {
    auto widget = this->MakeWidget(AllLocks());

    widget->ApplyLayout(TwoCellLayout());
    this->InitializeViews(*widget);
    CPPUNIT_ASSERT_EQUAL(2u, widget->GetNumberOfRenderWindowWidgets());
    const QString left = QStringLiteral("mxn__left");
    const QString right = QStringLiteral("mxn__right");

    widget->SetSyncLink(left, QmitkMxNSyncDimension::Slice, "nav");
    widget->SetSyncLink(right, QmitkMxNSyncDimension::Slice, "nav");
    CPPUNIT_ASSERT(widget->GetSyncLink(right, QmitkMxNSyncDimension::Slice).has_value());

    widget->SetCellSelectionGroup(right, "other");
    CPPUNIT_ASSERT_EQUAL(std::string("other"), widget->GetCellSelectionGroup(right));

    widget->SetViewDirection(left, mitk::AnatomicalPlane::Sagittal);
    CPPUNIT_ASSERT_EQUAL(std::string("Sagittal"), Overlay(*widget, left)->PlaneLabel().toStdString());

    widget->SetMaximizedCell(left);
    CPPUNIT_ASSERT(left == widget->GetMaximizedCell());
    widget->SetMaximizedCell(QString());
    CPPUNIT_ASSERT(widget->GetMaximizedCell().isEmpty());

    auto* selection = Cell(*widget, left)->GetUtilityWidget()->GetNodeSelectionWidget();
    selection->SetSelectAll(false);
    selection->SetCurrentSelection({});
    CPPUNIT_ASSERT(selection->GetSelectedNodes().isEmpty());
    selection->SetCurrentSelection({ m_ImageNode });
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(selection->GetSelectedNodes().size()));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNLayoutLock)
