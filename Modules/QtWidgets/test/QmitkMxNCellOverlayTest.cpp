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
#include <QmitkMxNSyncDimension.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowProximity.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkAnatomicalPlanes.h>
#include <mitkBaseRenderer.h>
#include <mitkException.h>
#include <mitkImageGenerator.h>
#include <mitkLevelWindow.h>
#include <mitkRenderingManager.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QApplication>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QWheelEvent>

#include <memory>
#include <utility>
#include <vector>

/**
 * Headless behavior tests for the MxN cell overlay's information
 * architecture: the plane label tracks the cell's view direction (and the
 * VTK cell-id annotation is blanked so the two do not double up), and the
 * group-identity dot follows the mono / complex rule over the cell's
 * per-dimension synchronization. Presentation, animation, and gesture are
 * manual acceptance, not covered here.
 */
class QmitkMxNCellOverlayTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNCellOverlayTestSuite);
  MITK_TEST(CellIdAnnotation_IsBlank);
  MITK_TEST(PlaneLabel_TracksViewDirection);
  MITK_TEST(FrameIdentity_FreshCellIsMonoMain);
  MITK_TEST(FrameIdentity_MonoWhenSingleGroup);
  MITK_TEST(FrameIdentity_ComplexAcrossGroups);
  MITK_TEST(FrameIdentity_SelectionParticipates);
  MITK_TEST(PaintPath_DoesNotCrash);
  MITK_TEST(GroupState_ShownOnEveryCellAfterGridGrows);
  MITK_TEST(GroupState_ShownOnEveryCellAfterAddGridRow);
  MITK_TEST(RightClick_OnMaskedFurnitureOpensTheMenuAndReachesVtk);
  MITK_TEST(Wheel_OverMaskedFurnitureReachesTheRenderWindow);
  MITK_TEST(CleanView_LeavingNearFurnitureRevealsTheFrame);
  MITK_TEST(Resize_RebuildsTheMask);
  MITK_TEST(Destruction_UnregistersRegions);
  MITK_TEST(Popup_FromTheFurnitureKeepsTheFrameUp);
  MITK_TEST(Popup_ContextMenuDoesNotRevealTheFrame);
  MITK_TEST(LostGrab_LeavesTheColorbarDragWorking);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_Image;
  mitk::DataNode::Pointer m_ImageNode;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);

    m_ImageNode = mitk::DataNode::New();
    m_ImageNode->SetName("image");
    m_ImageNode->SetData(m_Image);
    m_ImageNode->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_ImageNode);

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(1, 3); // cells: mxn__widget0 .. mxn__widget2

    for (const auto& [name, cell] : m_Editor->GetRenderWindowWidgets())
    {
      mitk::RenderingManager::GetInstance()->InitializeView(
        cell->GetRenderWindow()->GetVtkRenderWindow(), m_Image->GetTimeGeometry());
    }
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_ImageNode = nullptr;
    m_Image = nullptr;
    m_DataStorage = nullptr;
  }

  static QString CellId(std::size_t index)
  {
    return QStringLiteral("mxn__widget") + QString::number(index);
  }

  QmitkMxNCellOverlay* Overlay(std::size_t index) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(index));
    CPPUNIT_ASSERT(nullptr != cell);
    auto* overlay = cell->findChild<QmitkMxNCellOverlay*>();
    CPPUNIT_ASSERT_MESSAGE("Every cell carries a composite overlay", nullptr != overlay);
    return overlay;
  }

  /** Records the mouse and wheel events a watched widget receives. */
  class EventRecorder : public QObject
  {
  public:
    std::vector<std::pair<QEvent::Type, Qt::MouseButton>> events;

    bool Saw(QEvent::Type type, Qt::MouseButton button = Qt::NoButton) const
    {
      for (const auto& [seenType, seenButton] : events)
      {
        if (seenType == type && seenButton == button)
        {
          return true;
        }
      }
      return false;
    }

  protected:
    bool eventFilter(QObject* /*watched*/, QEvent* event) override
    {
      switch (event->type())
      {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
          events.emplace_back(event->type(), static_cast<QMouseEvent*>(event)->button());
          break;
        case QEvent::Wheel:
          events.emplace_back(event->type(), Qt::NoButton);
          break;
        default:
          break;
      }
      return false;
    }
  };

  /** Give the editor real geometry; -platform minimal makes show() work
   *  without a display. */
  void ShowEditor()
  {
    m_Editor->resize(640, 480);
    m_Editor->show();
    // Long enough for the first render and the value refresh it triggers, so
    // a test sees only what its own action causes.
    QmitkTestPopupProbe::Pump(200);
  }

  QmitkRenderWindowProximity* Proximity(std::size_t index) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(index));
    auto* proximity = cell->findChild<QmitkRenderWindowProximity*>();
    CPPUNIT_ASSERT(nullptr != proximity);
    return proximity;
  }

  /** A point on the bottom furniture of cell 'index', in cell coordinates. */
  QPoint BottomFurniturePoint(std::size_t index) const
  {
    const QRect area = m_Editor->GetRenderWindowWidget(CellId(index))->GetRenderWindow()->geometry();
    return QPoint(area.center().x(), area.bottom() - 1);
  }

  /** Rest the pointer on the bottom furniture of cell 'index' so its overlay
   *  takes input there. */
  QmitkMxNCellOverlay* MaskOverlay(std::size_t index) const
  {
    this->Proximity(index)->HandlePointerMoved(this->BottomFurniturePoint(index));
    auto* overlay = this->Overlay(index);
    CPPUNIT_ASSERT_MESSAGE("Precondition: the overlay takes input over the furniture",
                           !overlay->testAttribute(Qt::WA_TransparentForMouseEvents));
    return overlay;
  }

  static void SendMouse(QWidget* target, QEvent::Type type, const QPoint& position, Qt::MouseButton button)
  {
    const Qt::MouseButtons buttons =
      QEvent::MouseButtonRelease == type ? Qt::MouseButtons(Qt::NoButton) : Qt::MouseButtons(button);
    QMouseEvent event(type, QPointF(position), QPointF(position), QPointF(target->mapToGlobal(position)), button,
                      buttons, Qt::NoModifier);
    QCoreApplication::sendEvent(target, &event);
  }

  void RightClick_OnMaskedFurnitureOpensTheMenuAndReachesVtk()
  {
    this->ShowEditor();
    auto* overlay = this->MaskOverlay(0);
    auto* renderWindow = m_Editor->GetRenderWindowWidget(CellId(0))->GetRenderWindow();
    EventRecorder recorder;
    renderWindow->installEventFilter(&recorder);

    QmitkTestPopupProbe probe;
    const QPoint at = this->BottomFurniturePoint(0);
    SendMouse(overlay, QEvent::MouseButtonPress, at, Qt::RightButton);
    SendMouse(overlay, QEvent::MouseButtonRelease, at, Qt::RightButton);
    QmitkTestPopupProbe::Pump();
    renderWindow->removeEventFilter(&recorder);

    CPPUNIT_ASSERT_MESSAGE("The right-button gesture reaches the render window",
                           recorder.Saw(QEvent::MouseButtonPress, Qt::RightButton)
                             && recorder.Saw(QEvent::MouseButtonRelease, Qt::RightButton));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A right click on the furniture opens the context menu", 1, probe.Count());
  }

  void Wheel_OverMaskedFurnitureReachesTheRenderWindow()
  {
    this->ShowEditor();
    auto* overlay = this->MaskOverlay(0);
    auto* renderWindow = m_Editor->GetRenderWindowWidget(CellId(0))->GetRenderWindow();
    EventRecorder recorder;
    renderWindow->installEventFilter(&recorder);

    const QPointF at(this->BottomFurniturePoint(0));
    QWheelEvent wheel(at, QPointF(overlay->mapToGlobal(at.toPoint())), QPoint(), QPoint(0, 120), Qt::NoButton,
                      Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(overlay, &wheel);
    renderWindow->removeEventFilter(&recorder);

    CPPUNIT_ASSERT_MESSAGE("A wheel over the furniture scrolls the render window", recorder.Saw(QEvent::Wheel));
  }

  void CleanView_LeavingNearFurnitureRevealsTheFrame()
  {
    // The pointer rests on the furniture through the whole round trip, so no
    // region changes state when clean view ends; the frame must still come back.
    this->ShowEditor();
    auto* overlay = this->MaskOverlay(0);
    auto* utility = m_Editor->GetRenderWindowWidget(CellId(0))->GetUtilityWidget();

    m_Editor->SetCleanView(true);
    QmitkTestPopupProbe::Pump(400);
    m_Editor->SetCleanView(false);
    QmitkTestPopupProbe::Pump(400);

    CPPUNIT_ASSERT_MESSAGE("The furniture is revealed again", overlay->RevealProgress() > 0.99);
    CPPUNIT_ASSERT_MESSAGE("...and so is the utility strip", utility->isVisible());
  }

  void Resize_RebuildsTheMask()
  {
    this->ShowEditor();
    auto* overlay = this->MaskOverlay(0);
    auto* renderWindow = m_Editor->GetRenderWindowWidget(CellId(0))->GetRenderWindow();
    const QRect before = renderWindow->geometry();

    // Let the reveal animation settle: each of its frames rebuilds the mask.
    QmitkTestPopupProbe::Pump(300);

    // Wider only, so the pointer stays on the bottom furniture.
    m_Editor->resize(900, 480);
    QmitkTestPopupProbe::Pump(50);
    const QRect after = renderWindow->geometry();
    CPPUNIT_ASSERT_MESSAGE("Precondition: the cell grew", after.right() > before.right() + 20);
    CPPUNIT_ASSERT_MESSAGE("Precondition: the overlay still takes input",
                           !overlay->testAttribute(Qt::WA_TransparentForMouseEvents));

    // The active-cell corner squares are part of the mask.
    CPPUNIT_ASSERT_MESSAGE("The mask follows the new geometry",
                           overlay->mask().contains(after.bottomRight() - QPoint(2, 2)));
  }

  void Destruction_UnregistersRegions()
  {
    auto* proximity = this->Proximity(0);
    // The overlay is the cell's only registrant, and ids start at 0.
    CPPUNIT_ASSERT_NO_THROW(proximity->GetRegionState(0));

    delete this->Overlay(0);

    CPPUNIT_ASSERT_THROW_MESSAGE("A destroyed overlay leaves no region behind that calls into it",
                                 proximity->GetRegionState(0), mitk::Exception);
  }

  double LevelIn(std::size_t index) const
  {
    auto* renderWindow = m_Editor->GetRenderWindowWidget(CellId(index))->GetRenderWindow();
    mitk::LevelWindow levelWindow;
    CPPUNIT_ASSERT(m_ImageNode->GetLevelWindow(
      levelWindow, mitk::BaseRenderer::GetInstance(renderWindow->GetVtkRenderWindow())));
    return levelWindow.GetLevel();
  }

  void LostGrab_LeavesTheColorbarDragWorking()
  {
    this->ShowEditor();
    auto* overlay = this->MaskOverlay(0);
    const QPoint furniture = this->BottomFurniturePoint(0);

    // A gesture forwarded to the render window whose release never arrives:
    // the grab went elsewhere (a window switch, a popup), and the pointer comes
    // back with no button held.
    SendMouse(overlay, QEvent::MouseButtonPress, furniture, Qt::RightButton);
    SendMouse(overlay, QEvent::MouseMove, furniture + QPoint(0, -1), Qt::NoButton);

    // The colorbar body drags the level.
    const QRect area = m_Editor->GetRenderWindowWidget(CellId(0))->GetRenderWindow()->geometry();
    const QPoint onColorbar(area.right() - 2, area.center().y());
    const double before = this->LevelIn(0);
    SendMouse(overlay, QEvent::MouseButtonPress, onColorbar, Qt::LeftButton);
    QMouseEvent drag(QEvent::MouseMove, QPointF(onColorbar - QPoint(0, 40)), QPointF(onColorbar - QPoint(0, 40)),
                     QPointF(overlay->mapToGlobal(onColorbar - QPoint(0, 40))), Qt::NoButton, Qt::LeftButton,
                     Qt::NoModifier);
    QCoreApplication::sendEvent(overlay, &drag);
    SendMouse(overlay, QEvent::MouseButtonRelease, onColorbar - QPoint(0, 40), Qt::LeftButton);

    CPPUNIT_ASSERT_MESSAGE("A colorbar drag works after a gesture lost its release", this->LevelIn(0) > before);
  }

  void Popup_FromTheFurnitureKeepsTheFrameUp()
  {
    // A popup takes a pointer grab, and the cell reads that as the pointer
    // leaving; the furniture it was opened from must stay up beneath it.
    this->ShowEditor();
    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Axial);
    auto* overlay = this->MaskOverlay(0);
    auto* proximity = this->Proximity(0);
    const QRect planeLabel = overlay->PlaneLabelRect();
    CPPUNIT_ASSERT(planeLabel.isValid());

    bool pinnedWhileOpen = false;
    QmitkTestPopupProbe probe;
    probe.inspect = [&](QWidget*) { pinnedWhileOpen = proximity->IsPinned(); };
    SendMouse(overlay, QEvent::MouseButtonPress, planeLabel.center(), Qt::LeftButton);
    SendMouse(overlay, QEvent::MouseButtonRelease, planeLabel.center(), Qt::LeftButton);
    QmitkTestPopupProbe::Pump();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The plane label opens the direction picker", 1, probe.Count());
    CPPUNIT_ASSERT_MESSAGE("The frame is pinned while the picker is open", pinnedWhileOpen);
    CPPUNIT_ASSERT_MESSAGE("...and released once it closes", !proximity->IsPinned());
  }

  void Popup_ContextMenuDoesNotRevealTheFrame()
  {
    // The context menu opens over the image, not from the furniture; pinning
    // would pop the whole frame in under it.
    auto* renderWindow = m_Editor->GetRenderWindowWidget(CellId(0))->GetRenderWindow();
    auto* proximity = this->Proximity(0);
    bool pinnedWhileOpen = false;
    QmitkTestPopupProbe probe;
    probe.inspect = [&](QWidget*) { pinnedWhileOpen = proximity->IsPinned(); };
    QContextMenuEvent request(QContextMenuEvent::Keyboard, renderWindow->rect().center(),
                              renderWindow->mapToGlobal(renderWindow->rect().center()));
    QCoreApplication::sendEvent(renderWindow, &request);
    QmitkTestPopupProbe::Pump();

    CPPUNIT_ASSERT_EQUAL(1, probe.Count());
    CPPUNIT_ASSERT_MESSAGE("The context menu leaves the frame as it was", !pinnedWhileOpen);
  }

  void CellIdAnnotation_IsBlank()
  {
    for (std::size_t i = 0; i < 3; ++i)
    {
      const auto cell = m_Editor->GetRenderWindowWidget(CellId(i));
      CPPUNIT_ASSERT(nullptr != cell);
      CPPUNIT_ASSERT_MESSAGE("MxN blanks the VTK cell-id corner annotation; the plane "
                             "label lives in the Qt overlay instead",
                             cell->GetCornerAnnotationText().empty());
    }
  }

  void PlaneLabel_TracksViewDirection()
  {
    auto* overlay = this->Overlay(0);

    // The label is resolved live from the cell's view direction, so setting a
    // direction is reflected without a render or a separate signal.
    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Axial);
    CPPUNIT_ASSERT_EQUAL(std::string("Axial"), overlay->PlaneLabel().toStdString());

    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Coronal);
    CPPUNIT_ASSERT_EQUAL(std::string("Coronal"), overlay->PlaneLabel().toStdString());

    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Sagittal);
    CPPUNIT_ASSERT_EQUAL(std::string("Sagittal"), overlay->PlaneLabel().toStdString());
  }

  void FrameIdentity_FreshCellIsMonoMain()
  {
    // A fresh cell links Windowing, LUT, and selection to "main", so its frame is
    // a solid "main" hue - there is no group-less "None" resting state.
    const auto identity = m_Editor->ResolveCellGroupIdentity(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("A fresh cell's frame is Mono",
      QmitkMxNMultiWidget::CellGroupIdentityKind::Mono == identity.kind);
    CPPUNIT_ASSERT_MESSAGE("A fresh cell's frame hue is 'main's color",
      identity.hue == m_Editor->GetSyncGroupColor("main"));
  }

  void FrameIdentity_MonoWhenSingleGroup()
  {
    // Wholly one group across every tied axis (the appearance defaults cleared,
    // navigation and selection all on "nav") reads Mono.
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      m_Editor->ClearSyncLink(CellId(0), dimension);
    }
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "nav");
    m_Editor->SetCellSelectionGroup(CellId(0), "nav");

    const auto identity = m_Editor->ResolveCellGroupIdentity(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("A cell whose tied axes all name one group is mono",
      QmitkMxNMultiWidget::CellGroupIdentityKind::Mono == identity.kind);
    CPPUNIT_ASSERT_MESSAGE("A mono cell carries the group hue", identity.hue.isValid());
  }

  void FrameIdentity_ComplexAcrossGroups()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "detail");

    CPPUNIT_ASSERT_MESSAGE("A cell spanning more than one group is complex, not a single hue",
      QmitkMxNMultiWidget::CellGroupIdentityKind::Complex
        == m_Editor->ResolveCellGroupIdentity(CellId(0)).kind);
  }

  void FrameIdentity_SelectionParticipates()
  {
    // Selection is the 8th frame axis. A cell whose only group tie is a deliberate
    // data-selection group (the appearance defaults cleared) reads Mono of that
    // group - selection drives the frame like any other axis.
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      m_Editor->ClearSyncLink(CellId(0), dimension);
    }
    m_Editor->SetCellSelectionGroup(CellId(0), "sel");

    const auto identity = m_Editor->ResolveCellGroupIdentity(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("A selection-only cell is Mono of its selection group",
      QmitkMxNMultiWidget::CellGroupIdentityKind::Mono == identity.kind);
    CPPUNIT_ASSERT_MESSAGE("The frame hue is the selection group's color",
      identity.hue == m_Editor->GetSyncGroupColor("sel"));
  }

  /** Every cell's furniture shows its group state: the barcode slots of the
   *  appearance axes carry the "main" hue, and the frame is drawn in it. */
  void AssertEveryCellShowsMainGroup(const std::string& when) const
  {
    const auto mainHue = m_Editor->GetSyncGroupColor("main");
    for (const auto& [windowId, cell] : m_Editor->GetRenderWindowWidgets())
    {
      const auto where = when + ", cell " + windowId.toStdString();
      auto* utility = cell->GetUtilityWidget();
      CPPUNIT_ASSERT(nullptr != utility);
      auto* barcode = utility->findChild<QmitkMxNSyncBarcodeWidget*>();
      CPPUNIT_ASSERT(nullptr != barcode);
      const auto axisSlots = barcode->Slots();
      // One slot per dimension in dimension order, then the data selection.
      CPPUNIT_ASSERT(axisSlots.size() > static_cast<int>(QmitkMxNAllSyncDimensions.size()));
      for (std::size_t i = 0; i < QmitkMxNAllSyncDimensions.size(); ++i)
      {
        const auto dimension = QmitkMxNAllSyncDimensions[i];
        if (QmitkMxNSyncDimension::Windowing == dimension || QmitkMxNSyncDimension::Lut == dimension)
        {
          CPPUNIT_ASSERT_MESSAGE("The barcode shows the appearance link: " + where,
                                 axisSlots[static_cast<int>(i)].color == mainHue);
        }
      }
      CPPUNIT_ASSERT_MESSAGE("The frame carries the group hue: " + where,
                             cell->styleSheet().contains(mainHue.name(QColor::HexRgb)));
    }
  }

  void GroupState_ShownOnEveryCellAfterGridGrows()
  {
    m_Editor->SetLayout(2, 3);
    QCoreApplication::processEvents();
    CPPUNIT_ASSERT_EQUAL(std::size_t(6), m_Editor->GetRenderWindowWidgets().size());
    this->AssertEveryCellShowsMainGroup("after SetLayout grows the grid");
  }

  void GroupState_ShownOnEveryCellAfterAddGridRow()
  {
    m_Editor->AddGridRow();
    QCoreApplication::processEvents();
    CPPUNIT_ASSERT_EQUAL(std::size_t(6), m_Editor->GetRenderWindowWidgets().size());
    this->AssertEveryCellShowsMainGroup("after AddGridRow");
  }

  void PaintPath_DoesNotCrash()
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(0));
    CPPUNIT_ASSERT(nullptr != cell);

    // Give the cell real geometry offscreen so paintEvent exercises its
    // arithmetic instead of early-returning on an invalid rect
    // (-platform minimal makes show() work without a display).
    m_Editor->resize(640, 480);
    m_Editor->show();
    QApplication::processEvents();
    CPPUNIT_ASSERT_MESSAGE("The cell must have real geometry for the paint smoke to be meaningful",
      cell->GetRenderWindow()->width() > 0 && cell->GetRenderWindow()->height() > 0);

    // A heterogeneous cell (complex dot bands) with a fully revealed, expanded
    // navigator drives the widest set of paint branches: colorbar tick scale,
    // colormap chip, navigator slider rows, coordinate line, and barcode.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "detail");
    m_Editor->SetNavigatorExpanded(true);

    auto* overlay = this->Overlay(0);
    overlay->SetRevealProgress(1.0);
    overlay->grab();  // forces a synchronous paintEvent
    m_Editor->SetNavigatorExpanded(false);
    overlay->SetRevealProgress(1.0);
    overlay->grab();

    m_Editor->hide();

    // Reaching here means the paint path did not crash across those branches.
    CPPUNIT_ASSERT(true);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNCellOverlay)
