/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNArrangeMode.h>
#include <QmitkMxNAxisGlyph.h>
#include <QmitkMxNCellOverlay.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkImageGenerator.h>
#include <mitkRenderingManager.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QApplication>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <string>

using PeekAxis = std::optional<QmitkMxNSyncAxis>;

namespace CppUnit
{
  /** Lets CPPUNIT_ASSERT_EQUAL compare and print a peek emphasis. */
  template <>
  struct assertion_traits<PeekAxis>
  {
    static bool equal(const PeekAxis& expected, const PeekAxis& actual)
    {
      return expected == actual;
    }

    static std::string toString(const PeekAxis& axis)
    {
      return axis.has_value() ? "axis " + std::to_string(QmitkMxNSyncAxisToSlot(*axis)) : "no axis";
    }
  };
}

/**
 * Headless tests for the MxN sync peek: the plate geometry, the one glyph box
 * the whole layout shares, the broadcast into the cells, the pointer state
 * machine, and the barcode's new reporting.
 *
 * Everything perceptual - legibility over anatomy, whether the pump reads at a
 * glance, whether the dwell feels right - is manual acceptance and not covered
 * here.
 */
class QmitkMxNSyncPeekTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNSyncPeekTestSuite);
  MITK_TEST(PeekPlate_GlyphsFitThePlateAndEachOther);
  MITK_TEST(PeekPlate_PumpedGlyphCarriesTheScaledSide);
  MITK_TEST(PeekPlate_PlateIsTheSameWhicheverAxisIsPumped);
  MITK_TEST(PeekPlate_NoAxisEmphasisedKeepsTheSamePlate);
  MITK_TEST(PeekPlate_CellTooSmallHasNoPlate);
  MITK_TEST(PeekPlate_PumpingLeavesEveryOtherGlyphInPlace);
  MITK_TEST(PeekPlate_ThreeRowsInFamilyOrder);
  MITK_TEST(PeekPlate_SizeAtTheMinimumBox);
  MITK_TEST(PeekPlate_LineSitsAboveTheGrid);
  MITK_TEST(PeekPlate_RibbonAnchoredAtTheGlyphCorner);
  MITK_TEST(PeekPlate_RibbonGrowsALittleWhenPumped);
  MITK_TEST(Barcode_GlyphBoxFillsTheHeightItIsGranted);
  MITK_TEST(Barcode_WidthAskedForFollowsTheGrantedHeight);
  MITK_TEST(Barcode_FrameStaysInsideTheTargetAtEveryPixelRatio);
  MITK_TEST(MaxPeekGlyphBox_ZeroBelowTheFloorAndNeverShrinksWithTheCell);
  MITK_TEST(ResolvePeekGeometry_LegibleAndDrivenByTheDensestGrid);
  MITK_TEST(ResolvePeekGeometry_ZeroWhenNoCellQualifies);
  MITK_TEST(ResolvePeekGeometry_IgnoresHiddenCells);
  MITK_TEST(MaxPeekGlyphBox_HostsDownTo135x214);
  MITK_TEST(Broadcast_EveryVisibleCellSharesOneGeometry);
  MITK_TEST(Broadcast_NegativeAxisClears);
  MITK_TEST(Broadcast_HiddenCellsShowNoPlate);
  MITK_TEST(Gesture_DwellRaisesAndGraceLowers);
  MITK_TEST(Gesture_AxisSwitchIsImmediateWhilePeekIsUp);
  MITK_TEST(Gesture_NewAxisWithinTheGraceKeepsThePeek);
  MITK_TEST(Gesture_GapBetweenGlyphsKeepsTheEmphasis);
  MITK_TEST(Gesture_EnteringOnAGapRaisesWithoutEmphasis);
  MITK_TEST(Gesture_LeavingBeforeTheDwellNeverRaises);
  MITK_TEST(Gesture_MaximizingLowersThePeek);
  MITK_TEST(Gesture_LayoutChangeLowersThePeek);
  MITK_TEST(Gesture_AddingACellLowersThePeek);
  MITK_TEST(Gesture_CleanViewLowersThePeek);
  MITK_TEST(Arrange_PlatesAreUpWhileArranging);
  MITK_TEST(Arrange_StripGraceExpiryKeepsThePlates);
  MITK_TEST(Arrange_LayoutChangesKeepThePlatesAndReachNewCells);
  MITK_TEST(Arrange_MaximizeAndCleanViewKeepThePlate);
  MITK_TEST(Arrange_ResizingTheCellsChangesTheSharedBox);
  MITK_TEST(Arrange_LeavingLowersEveryPlateAtOnce);
  MITK_TEST(Arrange_StripLeavingClearsOnlyItsOwnEmphasis);
  MITK_TEST(Arrange_StripHoverRingsTheCellsSharingItsAxis);
  MITK_TEST(Glyph_LargeRenderIsNotAnUpscaledResource);
  MITK_TEST(Glyph_RepeatedRenderIsServedFromTheCache);
  MITK_TEST(Glyph_StickerRingsTheGlyphWhiteOutsideDarkInside);
  MITK_TEST(Glyph_StickerOutlineStaysCrispWhereTheGlyphFades);
  MITK_TEST(Barcode_PassiveStripReportsTheGlyphUnderThePointer);
  MITK_TEST(Barcode_PassiveStripReportsLeavingTheGlyphsButNotTheStrip);
  MITK_TEST(Barcode_PassiveStripReportsColorBarModeWithoutAnAxis);
  MITK_TEST(Barcode_PassiveStripStillOpensTheLayoutEditor);
  MITK_TEST(Barcode_ClickableStripReportsInBothRenderModes);
  MITK_TEST(Barcode_NotApplicableSlotIsInert);
  MITK_TEST(Barcode_HoveredSlotRaisedOthersFade);
  MITK_TEST(Barcode_OffsetFillsTheLowerRightCorner);
  MITK_TEST(Barcode_PassiveStripHoverIsAWashBehindTheContent);
  MITK_TEST(PaintPath_PlateDoesNotCrash);
  MITK_TEST(Peek_FadeOutKeepsThePlateGeometry);
  CPPUNIT_TEST_SUITE_END();

  // Large enough that a single cell and a 1x3 grid host a plate and a 5x5 grid
  // none - the cases the sizing rule has to tell apart.
  static constexpr int EditorWidth = 1024;
  static constexpr int EditorHeight = 768;

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
    // The gesture is driven by signals here, not by the clock.
    m_Editor->SetSyncPeekTimings(0, 0);
    m_Editor->resize(EditorWidth, EditorHeight);
    m_Editor->show();
    Pump();
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_ImageNode = nullptr;
    m_Image = nullptr;
    m_DataStorage = nullptr;
  }

  static void Pump()
  {
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
  }

  static QString CellId(int index)
  {
    return QStringLiteral("mxn__widget") + QString::number(index);
  }

  /** Lay the editor out and let Qt give the cells their geometry. */
  void Arrange(int rows, int columns) const
  {
    m_Editor->SetLayout(rows, columns);
    Pump();
  }

  QmitkMxNCellOverlay* Overlay(int index) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(index));
    CPPUNIT_ASSERT(nullptr != cell);
    auto* overlay = cell->findChild<QmitkMxNCellOverlay*>();
    CPPUNIT_ASSERT_MESSAGE("Every cell carries a composite overlay", nullptr != overlay);
    return overlay;
  }

  static int LineHeight()
  {
    return QmitkMxNCellOverlay::PeekTextLineHeight(QApplication::font());
  }

  /**
   * Records AxisHovered / Clicked emissions. QSignalSpy needs Qt::Test, which
   * MitkQtWidgets does not link.
   */
  struct BarcodeRecorder
  {
    QList<int> axes;       // AxisHovered, the clickable path
    QList<bool> onStrip;   // PeekHovered's first argument, the passive path
    QList<int> peekAxes;   // PeekHovered's second argument
    int clicks = 0;

    explicit BarcodeRecorder(QmitkMxNSyncBarcodeWidget& strip)
    {
      QObject::connect(&strip, &QmitkMxNSyncBarcodeWidget::AxisHovered,
                       [this](int index) { axes.append(index); });
      QObject::connect(&strip, &QmitkMxNSyncBarcodeWidget::PeekHovered,
                       [this](bool overStrip, int index)
                       { onStrip.append(overStrip); peekAxes.append(index); });
      QObject::connect(&strip, &QmitkMxNSyncBarcodeWidget::Clicked,
                       [this]() { ++clicks; });
    }
  };

  /** One plain slot per axis, enough for the strip's hit testing and its paint. */
  static QList<QmitkMxNSyncBarcodeWidget::AxisSlot> PlainSlots()
  {
    QList<QmitkMxNSyncBarcodeWidget::AxisSlot> axisSlots;
    for (int axis = 0; axis < QmitkMxNCellOverlay::PeekAxisCount; ++axis)
    {
      QmitkMxNSyncBarcodeWidget::AxisSlot slot;
      slot.color = QColor(200, 120, 60);
      axisSlots.append(slot);
    }
    return axisSlots;
  }

  static void MoveTo(QWidget& target, const QPoint& position)
  {
    const QPointF local(position);
    QMouseEvent move(QEvent::MouseMove, local, local, local, Qt::NoButton, Qt::NoButton,
                     Qt::NoModifier);
    static_cast<QObject&>(target).event(&move);
  }

  static void ClickAt(QWidget& target, const QPoint& position)
  {
    const QPointF local(position);
    QMouseEvent press(QEvent::MouseButtonPress, local, local, local, Qt::LeftButton,
                      Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease, local, local, local, Qt::LeftButton,
                        Qt::NoButton, Qt::NoModifier);
    static_cast<QObject&>(target).event(&press);
    static_cast<QObject&>(target).event(&release);
  }

  // ---------- Plate geometry ----------

  void PeekPlate_GlyphsFitThePlateAndEachOther()
  {
    const QSize cell(900, 700);
    for (int box = QmitkMxNCellOverlay::PeekGlyphBoxMin;
         box <= QmitkMxNCellOverlay::PeekGlyphBoxMax; ++box)
    {
      // -1 is the state while the pointer rests on the strip between two glyphs.
      for (int pumped = -1; pumped < QmitkMxNCellOverlay::PeekAxisCount; ++pumped)
      {
        const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, pumped, LineHeight());
        CPPUNIT_ASSERT_MESSAGE("A cell this size hosts a plate at every legible box",
                               layout.plate.isValid());

        for (int axis = 0; axis < QmitkMxNCellOverlay::PeekAxisCount; ++axis)
        {
          CPPUNIT_ASSERT_MESSAGE("Every glyph lies inside the plate",
                                 layout.plate.contains(layout.glyphs[axis]));
          // The pumped glyph may overlap its neighbours; the resting ones never
          // touch each other.
          for (int other = axis + 1; other < QmitkMxNCellOverlay::PeekAxisCount; ++other)
          {
            if (axis == pumped || other == pumped)
            {
              continue;
            }
            CPPUNIT_ASSERT_MESSAGE("No two resting glyphs overlap",
                                   !layout.glyphs[axis].intersects(layout.glyphs[other]));
          }
        }
        CPPUNIT_ASSERT_MESSAGE("The top line lies inside the plate", layout.plate.contains(layout.line));
      }
    }
  }

  void PeekPlate_PumpedGlyphCarriesTheScaledSide()
  {
    const QSize cell(900, 700);
    for (int box = QmitkMxNCellOverlay::PeekGlyphBoxMin;
         box <= QmitkMxNCellOverlay::PeekGlyphBoxMax; ++box)
    {
      const int expected = qRound(1.75 * box);
      for (int pumped = 0; pumped < QmitkMxNCellOverlay::PeekAxisCount; ++pumped)
      {
        const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, pumped, LineHeight());
        CPPUNIT_ASSERT_EQUAL(expected, layout.glyphs[pumped].width());
        CPPUNIT_ASSERT_EQUAL(expected, layout.glyphs[pumped].height());
        for (int axis = 0; axis < QmitkMxNCellOverlay::PeekAxisCount; ++axis)
        {
          if (axis != pumped)
          {
            CPPUNIT_ASSERT_EQUAL(box, layout.glyphs[axis].width());
          }
        }
      }
    }
  }

  void PeekPlate_PlateIsTheSameWhicheverAxisIsPumped()
  {
    const QSize cell(900, 700);
    for (int box = QmitkMxNCellOverlay::PeekGlyphBoxMin;
         box <= QmitkMxNCellOverlay::PeekGlyphBoxMax; ++box)
    {
      const auto reference = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, 0, LineHeight());
      for (int pumped = 1; pumped < QmitkMxNCellOverlay::PeekAxisCount; ++pumped)
      {
        const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, pumped, LineHeight());
        CPPUNIT_ASSERT_MESSAGE("Switching the pumped axis moves and resizes nothing",
                               layout.plate == reference.plate);
        CPPUNIT_ASSERT_MESSAGE("The top line keeps its place across a switch", layout.line == reference.line);
      }
    }
  }

  void PeekPlate_NoAxisEmphasisedKeepsTheSamePlate()
  {
    const QSize cell(900, 700);
    for (int box = QmitkMxNCellOverlay::PeekGlyphBoxMin;
         box <= QmitkMxNCellOverlay::PeekGlyphBoxMax; ++box)
    {
      const auto none = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, -1, LineHeight());
      const auto pumped = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, 3, LineHeight());
      CPPUNIT_ASSERT_MESSAGE("Resting between two glyphs must not resize the plate",
                             none.plate == pumped.plate);
      for (int axis = 0; axis < QmitkMxNCellOverlay::PeekAxisCount; ++axis)
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("With nothing emphasised every glyph is a plain box",
                                     box, none.glyphs[axis].width());
        CPPUNIT_ASSERT_MESSAGE("Every glyph still lies inside the plate",
                               none.plate.contains(none.glyphs[axis]));
      }
    }
  }

  void PeekPlate_CellTooSmallHasNoPlate()
  {
    const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(
      QSize(80, 60), QmitkMxNCellOverlay::PeekGlyphBoxMin, 0, LineHeight());
    CPPUNIT_ASSERT_MESSAGE("A cell that cannot hold the plate gets an invalid rect",
                           !layout.plate.isValid());

    const auto belowFloor = QmitkMxNCellOverlay::ComputePeekPlate(
      QSize(900, 700), QmitkMxNCellOverlay::PeekGlyphBoxMin - 1, 0, LineHeight());
    CPPUNIT_ASSERT_MESSAGE("A box below the legible floor gets no plate either",
                           !belowFloor.plate.isValid());

    const auto noSuchAxis = QmitkMxNCellOverlay::ComputePeekPlate(
      QSize(900, 700), QmitkMxNCellOverlay::PeekGlyphBoxMin, QmitkMxNCellOverlay::PeekAxisCount, LineHeight());
    CPPUNIT_ASSERT_MESSAGE("An axis past the last gets no plate", !noSuchAxis.plate.isValid());
  }

  void PeekPlate_PumpingLeavesEveryOtherGlyphInPlace()
  {
    const QSize cell(900, 700);
    for (int box = QmitkMxNCellOverlay::PeekGlyphBoxMin;
         box <= QmitkMxNCellOverlay::PeekGlyphBoxMax; ++box)
    {
      const auto nominal = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, -1, LineHeight());
      for (int pumped = -1; pumped < QmitkMxNCellOverlay::PeekAxisCount; ++pumped)
      {
        const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, pumped, LineHeight());
        for (int axis = 0; axis < QmitkMxNCellOverlay::PeekAxisCount; ++axis)
        {
          if (axis != pumped)
          {
            CPPUNIT_ASSERT_MESSAGE("A resting glyph sits at its nominal slot whatever is pumped",
                                   layout.glyphs[axis] == nominal.glyphs[axis]);
          }
        }
        if (pumped < 0)
        {
          continue;
        }

        const QRect pumpedRect = layout.glyphs[pumped];
        const QRect slot = nominal.glyphs[pumped];
        CPPUNIT_ASSERT_MESSAGE("The pumped glyph lies inside the plate",
                               layout.plate.contains(pumpedRect));
        // The line spans exactly the glyph grid, so this is the check that the
        // grid's reserve covers the overhang - the plate padding alone would
        // hide a one-pixel overrun.
        CPPUNIT_ASSERT_MESSAGE("The pumped glyph stays within the grid's reserve",
                               pumpedRect.left() >= layout.line.left()
                                 && pumpedRect.right() <= layout.line.right());
        CPPUNIT_ASSERT_MESSAGE("and below the top line", pumpedRect.top() > layout.line.bottom());
        const QPointF offset = QRectF(pumpedRect).center() - QRectF(slot).center();
        CPPUNIT_ASSERT_MESSAGE("The pumped glyph grows around its own slot",
                               std::abs(offset.x()) <= 1.0 && std::abs(offset.y()) <= 1.0);
      }
    }
  }

  void PeekPlate_ThreeRowsInFamilyOrder()
  {
    // View, Position and Content, one family per row, in slot order.
    const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(QSize(900, 700), 24, -1, LineHeight());
    CPPUNIT_ASSERT(layout.plate.isValid());
    CPPUNIT_ASSERT_EQUAL(9, QmitkMxNCellOverlay::PeekAxisCount);
    for (int slot = 0; slot < QmitkMxNCellOverlay::PeekAxisCount; ++slot)
    {
      const QRect glyph = layout.glyphs[slot];
      const QRect rowStart = layout.glyphs[slot / 3 * 3];
      const QRect columnTop = layout.glyphs[slot % 3];
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A family shares its row", rowStart.top(), glyph.top());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A slot keeps its column across the rows", columnTop.left(), glyph.left());
      if (slot % 3 > 0)
      {
        CPPUNIT_ASSERT_MESSAGE("Axes run left to right within a row", layout.glyphs[slot - 1].right() < glyph.left());
      }
      if (slot >= 3)
      {
        CPPUNIT_ASSERT_MESSAGE("Families run top to bottom", layout.glyphs[slot - 3].bottom() < glyph.top());
      }
    }
    const auto row = [&layout](QmitkMxNSyncAxis axis) {
      return layout.glyphs[QmitkMxNSyncAxisToSlot(axis)].top();
    };
    CPPUNIT_ASSERT(row(QmitkMxNSyncAxis::Orientation) == row(QmitkMxNSyncAxis::Zoom));
    CPPUNIT_ASSERT(row(QmitkMxNSyncAxis::Slice) == row(QmitkMxNSyncAxis::Time)
                   && row(QmitkMxNSyncAxis::Time) == row(QmitkMxNSyncAxis::Crosshair));
    CPPUNIT_ASSERT(row(QmitkMxNSyncAxis::Selection) == row(QmitkMxNSyncAxis::Lut));
  }

  void PeekPlate_SizeAtTheMinimumBox()
  {
    // At a 16 px line: 2 * 12 + the grid wide and 8 + line + 4 + the grid + 8
    // tall.
    const auto layout =
      QmitkMxNCellOverlay::ComputePeekPlate(QSize(900, 700), QmitkMxNCellOverlay::PeekGlyphBoxMin, -1, 16);
    CPPUNIT_ASSERT_EQUAL(116, layout.plate.width());
    CPPUNIT_ASSERT_EQUAL(128, layout.plate.height());

    // Only the line height follows the font.
    const auto here =
      QmitkMxNCellOverlay::ComputePeekPlate(QSize(900, 700), QmitkMxNCellOverlay::PeekGlyphBoxMin, -1, LineHeight());
    CPPUNIT_ASSERT_EQUAL(116, here.plate.width());
    CPPUNIT_ASSERT_EQUAL(112 + LineHeight(), here.plate.height());
  }

  void PeekPlate_LineSitsAboveTheGrid()
  {
    const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(QSize(900, 700), 24, -1, LineHeight());
    CPPUNIT_ASSERT(layout.plate.isValid());
    CPPUNIT_ASSERT_EQUAL(LineHeight(), layout.line.height());
    for (int slot = 0; slot < QmitkMxNCellOverlay::PeekAxisCount; ++slot)
    {
      CPPUNIT_ASSERT_MESSAGE("The line sits above every glyph", layout.line.bottom() < layout.glyphs[slot].top());
      CPPUNIT_ASSERT_MESSAGE("and spans the grid", layout.line.left() <= layout.glyphs[slot].left()
                                                     && layout.line.right() >= layout.glyphs[slot].right());
    }
  }

  void PeekPlate_RibbonAnchoredAtTheGlyphCorner()
  {
    const QFont font = QApplication::font();
    const QRect glyph(100, 50, 20, 20);
    const QRect ribbon = QmitkMxNCellOverlay::PeekRibbonRect(glyph, 20, QStringLiteral("+1"), font);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Its right edge sits 3 px past the glyph's", glyph.right() + 3, ribbon.right());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Its bottom 3 px below the glyph's", glyph.bottom() + 3, ribbon.bottom());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A 9 px text plus 3", 12, ribbon.height());

    const QRect longer = QmitkMxNCellOverlay::PeekRibbonRect(glyph, 20, QStringLiteral("(12|-4)"), font);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A longer offset keeps the corner", ribbon.right(), longer.right());
    CPPUNIT_ASSERT_MESSAGE("and grows to the left", longer.left() < ribbon.left());

    // Text and offset grow with the box only until the text is 11 px, which
    // it is from 11/9 of the smallest box on.
    const QRect big(100, 50, 40, 40);
    const QRect capped = QmitkMxNCellOverlay::PeekRibbonRect(big, 40, QStringLiteral("+1"), font);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("An 11 px text plus 3", 14, capped.height());
    CPPUNIT_ASSERT_EQUAL(big.right() + 4, capped.right());
    CPPUNIT_ASSERT_EQUAL(big.bottom() + 4, capped.bottom());

    // The corner glyph of the bottom row keeps its ribbon on the plate, also
    // when it is enlarged at the largest box.
    for (const int box : { QmitkMxNCellOverlay::PeekGlyphBoxMin, QmitkMxNCellOverlay::PeekGlyphBoxMax })
    {
      const int last = QmitkMxNCellOverlay::PeekAxisCount - 1;
      const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(QSize(900, 700), box, last, LineHeight());
      const QRect cornerRibbon =
        QmitkMxNCellOverlay::PeekRibbonRect(layout.glyphs[last], box, QStringLiteral("(12|-4)"), font);
      CPPUNIT_ASSERT_MESSAGE("The last glyph's ribbon stays on the plate at box " + std::to_string(box),
                             layout.plate.contains(cornerRibbon));
    }
  }

  void PeekPlate_RibbonGrowsALittleWhenPumped()
  {
    const QFont font = QApplication::font();
    const int box = 24;
    const int slice = QmitkMxNSyncAxisToSlot(QmitkMxNSyncAxis::Slice);
    const auto resting = QmitkMxNCellOverlay::ComputePeekPlate(QSize(900, 700), box, -1, LineHeight());
    const auto pumped = QmitkMxNCellOverlay::ComputePeekPlate(QSize(900, 700), box, slice, LineHeight());

    const QRect atRest = QmitkMxNCellOverlay::PeekRibbonRect(resting.glyphs[slice], box, QStringLiteral("-1"), font);
    const QRect enlarged = QmitkMxNCellOverlay::PeekRibbonRect(pumped.glyphs[slice], box, QStringLiteral("-1"), font);
    // A ribbon is its text plus 3 px; the text grows by a quarter, the glyph by
    // three quarters.
    const double textGrowth = (enlarged.height() - 3.0) / (atRest.height() - 3.0);
    CPPUNIT_ASSERT_MESSAGE("The ribbon grows a little with its glyph", textGrowth > 1.15 && textGrowth < 1.35);

    // The glyph artwork is drawn inside 4.5..19.5 of its 24 unit grid, so its
    // corner moves out less than its box's corner when the glyph is enlarged.
    // The ribbon's outer corner follows it, give or take a pixel of rounding
    // and the pixel by which the ribbon's overhang grows.
    const auto artCorner = [](const QRect& glyph)
    {
      const double inset = 4.5 / 24.0 * glyph.width();
      return QPointF(glyph.right() - inset, glyph.bottom() - inset);
    };
    const QPointF artMove = artCorner(pumped.glyphs[slice]) - artCorner(resting.glyphs[slice]);
    const QPoint ribbonMove = enlarged.bottomRight() - atRest.bottomRight();
    CPPUNIT_ASSERT_MESSAGE("and moves with the drawn glyph's corner, as if pinned to it",
                           std::abs(ribbonMove.x() - artMove.x()) <= 2.0
                             && std::abs(ribbonMove.y() - artMove.y()) <= 2.0);
    CPPUNIT_ASSERT_MESSAGE("not as far as the box's corner",
                           ribbonMove.x() < pumped.glyphs[slice].right() - resting.glyphs[slice].right());
  }

  /** The ink in one device row of a painted strip, summed over the row. */
  static int RowInk(const QImage& canvas, int row)
  {
    int ink = 0;
    for (int x = 0; x < canvas.width(); ++x)
    {
      ink += qGray(canvas.pixel(x, row));
    }
    return ink;
  }

  void Barcode_FrameStaysInsideTheTargetAtEveryPixelRatio()
  {
    // A box sits flush against the target's edge whenever the strip fills the
    // height of its chrome. A frame stroked along that edge puts half its width
    // outside the canvas, which loses the top line. Stroked half a pixel inside,
    // the top line survives and matches the bottom one.
    constexpr int slotCount = QmitkMxNSyncAxisCount;
    QList<QmitkMxNSyncBarcodeWidget::AxisSlot> axisSlots;
    for (int axis = 0; axis < slotCount; ++axis)
    {
      QmitkMxNSyncBarcodeWidget::AxisSlot slot;
      slot.color = QColor(Qt::red);
      axisSlots.append(slot);
    }

    for (const qreal ratio : { qreal(1.0), qreal(1.5), qreal(2.0) })
    {
      const QSize logical(240, 24);
      QImage canvas(QSize(qRound(logical.width() * ratio), qRound(logical.height() * ratio)),
                    QImage::Format_ARGB32_Premultiplied);
      canvas.setDevicePixelRatio(ratio);
      canvas.fill(Qt::black);
      {
        QPainter painter(&canvas);
        QmitkMxNSyncBarcodeWidget::PaintInto(painter, QRect(QPoint(0, 0), logical), axisSlots,
                                             false, QColor(Qt::gray), -1);
      }

      const auto layout =
        QmitkMxNSyncBarcodeWidget::ComputeLayout(logical.width(), logical.height(), slotCount);
      const QRect content = QRect(QPoint(0, 0), logical);

      // Every edge of every box lands inside the target, whatever the ratio, so
      // no part of a frame depends on what happens at the canvas boundary.
      const int gridTop = qRound((content.top() + 1) * ratio);
      const int gridBottom = qRound((content.top() + 1 + layout.box - 1) * ratio);
      CPPUNIT_ASSERT_MESSAGE("The grid starts inside the target", gridTop >= 1);
      CPPUNIT_ASSERT_MESSAGE("and ends inside it", gridBottom <= canvas.height() - 1);
      CPPUNIT_ASSERT_MESSAGE("The top frame line carries ink", RowInk(canvas, gridTop) > 0);
      CPPUNIT_ASSERT_MESSAGE("and the canvas edge above it is untouched",
                             0 == RowInk(canvas, 0));
    }
  }

  void Barcode_WidthAskedForFollowsTheGrantedHeight()
  {
    // A strip parked at a row's trailing edge is granted exactly the width it
    // asks for. Asking for font-sized boxes therefore capped the glyphs at the
    // font's height however tall the row was - the width, not the height, was
    // what kept the strip's glyphs smaller than its chrome.
    QmitkMxNSyncBarcodeWidget strip;
    strip.SetPreferGlyphWidth(true);
    QList<QmitkMxNSyncBarcodeWidget::AxisSlot> axisSlots;
    for (int axis = 0; axis < QmitkMxNSyncAxisCount; ++axis)
    {
      QmitkMxNSyncBarcodeWidget::AxisSlot slot;
      slot.color = QColor(Qt::red);
      axisSlots.append(slot);
    }
    strip.SetSlots(axisSlots);

    strip.resize(strip.sizeHint().width(), 16);
    const int shortWidth = strip.sizeHint().width();

    strip.resize(shortWidth, 32);
    const int tallWidth = strip.sizeHint().width();

    CPPUNIT_ASSERT_MESSAGE("A taller strip asks for more width", tallWidth > shortWidth);
    // Enough for boxes as tall as the row allows - all of it but the pixel each
    // frame keeps above and below itself.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("enough for boxes as tall as the row allows", 30,
                                 QmitkMxNSyncBarcodeWidget::ComputeLayout(tallWidth, 32, QmitkMxNSyncAxisCount).box);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A short strip still renders glyphs at its own height", 14,
                                 QmitkMxNSyncBarcodeWidget::ComputeLayout(shortWidth, 16, QmitkMxNSyncAxisCount).box);
  }

  void Barcode_GlyphBoxFillsTheHeightItIsGranted()
  {
    // A strip that left slack above and below its glyphs wasted the chrome it
    // was given and made the axes harder to tell apart; the box takes the
    // height on offer, bounded only by the width it has to share.
    constexpr int slotCount = QmitkMxNSyncAxisCount;
    constexpr int wide = 1000;  // wide enough that height is the binding limit

    for (const int height : { 18, 24, 32, 48 })
    {
      const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(wide, height, slotCount);
      CPPUNIT_ASSERT_MESSAGE("A wide strip renders glyphs",
                             layout.mode == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs);
      // All of it but the pixel each frame needs above and below itself, which
      // is what keeps a box's own edges inside the canvas.
      CPPUNIT_ASSERT_EQUAL_MESSAGE("and nothing beyond the frames' own room is left over",
                                   height - 2, layout.box);
    }

    // Width still bounds it: every box plus the gaps have to fit.
    const auto narrow = QmitkMxNSyncBarcodeWidget::ComputeLayout(160, 200, slotCount);
    CPPUNIT_ASSERT_MESSAGE("A narrow, tall strip is bounded by its width",
                           narrow.box < 200);
  }

  void MaxPeekGlyphBox_ZeroBelowTheFloorAndNeverShrinksWithTheCell()
  {
    const int lineHeight = LineHeight();
    CPPUNIT_ASSERT_EQUAL(0, QmitkMxNCellOverlay::MaxPeekGlyphBox(QSize(120, 90), lineHeight));

    int previous = 0;
    for (int width = 120; width <= 1600; width += 20)
    {
      const int box = QmitkMxNCellOverlay::MaxPeekGlyphBox(QSize(width, width), lineHeight);
      CPPUNIT_ASSERT_MESSAGE("A larger cell never admits a smaller box", box >= previous);
      CPPUNIT_ASSERT_MESSAGE("A hosted box is at or above the legible floor",
                             box == 0 || box >= QmitkMxNCellOverlay::PeekGlyphBoxMin);
      previous = box;
    }
    CPPUNIT_ASSERT_MESSAGE("A large cell hosts a plate", previous > 0);
  }

  void MaxPeekGlyphBox_HostsDownTo135x214()
  {
    // The plate may take 86 % of the width and 60 % of the height; at a 16 px
    // line its minimum, 116 x 128, fits a 135 x 214 window and no smaller.
    CPPUNIT_ASSERT_EQUAL(QmitkMxNCellOverlay::PeekGlyphBoxMin,
                         QmitkMxNCellOverlay::MaxPeekGlyphBox(QSize(135, 214), 16));
    CPPUNIT_ASSERT_EQUAL(0, QmitkMxNCellOverlay::MaxPeekGlyphBox(QSize(134, 214), 16));
    CPPUNIT_ASSERT_EQUAL(0, QmitkMxNCellOverlay::MaxPeekGlyphBox(QSize(135, 213), 16));
  }

  // ---------- The one box the layout shares ----------

  void ResolvePeekGeometry_LegibleAndDrivenByTheDensestGrid()
  {
    this->Arrange(1, 1);
    const int single = m_Editor->ResolvePeekGeometry().glyphBox;
    CPPUNIT_ASSERT_MESSAGE("A full-editor cell hosts a plate", single > 0);
    CPPUNIT_ASSERT_MESSAGE("The box stays in the legible range",
                           single >= QmitkMxNCellOverlay::PeekGlyphBoxMin
                             && single <= QmitkMxNCellOverlay::PeekGlyphBoxMax);

    this->Arrange(1, 3);
    const int thirds = m_Editor->ResolvePeekGeometry().glyphBox;
    CPPUNIT_ASSERT_MESSAGE("Three cells across still host a plate", thirds > 0);
    CPPUNIT_ASSERT_MESSAGE("Narrower cells never ask for a larger box", thirds <= single);
    CPPUNIT_ASSERT_MESSAGE("The box stays in the legible range",
                           thirds >= QmitkMxNCellOverlay::PeekGlyphBoxMin
                             && thirds <= QmitkMxNCellOverlay::PeekGlyphBoxMax);
  }

  void ResolvePeekGeometry_ZeroWhenNoCellQualifies()
  {
    this->Arrange(5, 5);
    CPPUNIT_ASSERT_EQUAL(0, m_Editor->ResolvePeekGeometry().glyphBox);
  }

  void ResolvePeekGeometry_IgnoresHiddenCells()
  {
    this->Arrange(5, 5);
    CPPUNIT_ASSERT_EQUAL(0, m_Editor->ResolvePeekGeometry().glyphBox);

    // Maximizing leaves the slivers registered but hidden; sizing from one of
    // them would shrink the only plate the user can see.
    m_Editor->SetMaximizedCell(CellId(0));
    Pump();
    CPPUNIT_ASSERT_MESSAGE("The maximized cell alone decides the box",
                           m_Editor->ResolvePeekGeometry().glyphBox > 0);
  }

  // ---------- The broadcast ----------

  void Broadcast_EveryVisibleCellSharesOneGeometry()
  {
    this->Arrange(1, 3);
    m_Editor->SetSyncPeek(true, QmitkMxNSyncAxis::Slice);

    const int box = this->Overlay(0)->SyncPeekGlyphBox();
    CPPUNIT_ASSERT_MESSAGE("The layout hosts a plate", box > 0);
    for (int index = 0; index < 3; ++index)
    {
      CPPUNIT_ASSERT_MESSAGE("Every visible cell raises its plate",
                             this->Overlay(index)->IsSyncPeekVisible());
      CPPUNIT_ASSERT_EQUAL(PeekAxis(QmitkMxNSyncAxis::Slice), this->Overlay(index)->SyncPeekAxis());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Every plate of a layout uses one glyph box",
                                   box, this->Overlay(index)->SyncPeekGlyphBox());
    }
    CPPUNIT_ASSERT_EQUAL(PeekAxis(QmitkMxNSyncAxis::Slice), m_Editor->GetSyncPeekAxis());
  }

  void Broadcast_NegativeAxisClears()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, QmitkMxNSyncAxis::Windowing);
    CPPUNIT_ASSERT_EQUAL(PeekAxis(QmitkMxNSyncAxis::Windowing), this->Overlay(0)->SyncPeekAxis());

    m_Editor->SetSyncPeek(false, std::nullopt);
    CPPUNIT_ASSERT_MESSAGE("Lowering clears the editor's own state",
                           !m_Editor->IsSyncPeekVisible());
    for (int index = 0; index < 2; ++index)
    {
      CPPUNIT_ASSERT_MESSAGE("Lowering clears every cell",
                             !this->Overlay(index)->IsSyncPeekVisible());
      CPPUNIT_ASSERT_EQUAL(0, this->Overlay(index)->SyncPeekGlyphBox());
    }
  }

  void Broadcast_HiddenCellsShowNoPlate()
  {
    this->Arrange(1, 2);
    m_Editor->SetMaximizedCell(CellId(0));
    Pump();

    m_Editor->SetSyncPeek(true, QmitkMxNSyncAxis::Zoom);
    CPPUNIT_ASSERT_MESSAGE("The visible cell shows the peek",
                           this->Overlay(0)->IsSyncPeekVisible());
    CPPUNIT_ASSERT_MESSAGE("A hidden cell shows no plate",
                           !this->Overlay(1)->IsSyncPeekVisible());
  }

  // ---------- The pointer gesture ----------

  void Gesture_DwellRaisesAndGraceLowers()
  {
    this->Arrange(1, 2);

    m_Editor->OnSyncPeekHovered(true, QmitkMxNSyncAxis::Crosshair);
    CPPUNIT_ASSERT_MESSAGE("The dwell has not elapsed yet", !m_Editor->IsSyncPeekVisible());
    Pump();
    CPPUNIT_ASSERT(m_Editor->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL(PeekAxis(QmitkMxNSyncAxis::Crosshair), m_Editor->GetSyncPeekAxis());

    m_Editor->OnSyncPeekHovered(false, std::nullopt);
    CPPUNIT_ASSERT_MESSAGE("The grace has not elapsed yet", m_Editor->IsSyncPeekVisible());
    Pump();
    CPPUNIT_ASSERT_MESSAGE("Leaving the strip lowers the peek", !m_Editor->IsSyncPeekVisible());
  }

  void Gesture_AxisSwitchIsImmediateWhilePeekIsUp()
  {
    this->Arrange(1, 2);

    m_Editor->OnSyncPeekHovered(true, QmitkMxNSyncAxis::Crosshair);
    Pump();
    CPPUNIT_ASSERT_EQUAL(PeekAxis(QmitkMxNSyncAxis::Crosshair), m_Editor->GetSyncPeekAxis());

    m_Editor->OnSyncPeekHovered(true, QmitkMxNSyncAxis::Lut);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comparing axes must not wait out a second dwell",
                                 PeekAxis(QmitkMxNSyncAxis::Lut), m_Editor->GetSyncPeekAxis());
    CPPUNIT_ASSERT_EQUAL(PeekAxis(QmitkMxNSyncAxis::Lut), this->Overlay(0)->SyncPeekAxis());
  }

  void Gesture_NewAxisWithinTheGraceKeepsThePeek()
  {
    this->Arrange(1, 2);

    m_Editor->OnSyncPeekHovered(true, QmitkMxNSyncAxis::Crosshair);
    Pump();

    // Clipping a corner off the strip and coming straight back must not tear the
    // peek down; the grace window covers it.
    m_Editor->OnSyncPeekHovered(false, std::nullopt);
    m_Editor->OnSyncPeekHovered(true, QmitkMxNSyncAxis::Orientation);
    Pump();
    CPPUNIT_ASSERT(m_Editor->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL(PeekAxis(QmitkMxNSyncAxis::Orientation), m_Editor->GetSyncPeekAxis());
  }

  void Gesture_GapBetweenGlyphsKeepsTheEmphasis()
  {
    this->Arrange(1, 2);

    m_Editor->OnSyncPeekHovered(true, QmitkMxNSyncAxis::Crosshair);
    Pump();
    CPPUNIT_ASSERT_EQUAL(PeekAxis(QmitkMxNSyncAxis::Crosshair), m_Editor->GetSyncPeekAxis());

    // The pointer is still on the strip, just in the 2 px gap between two glyph
    // boxes. Dropping the emphasis there would flicker axis - none - axis at
    // every boundary of a slide along the row, so it latches instead.
    m_Editor->OnSyncPeekHovered(true, std::nullopt);
    Pump();
    CPPUNIT_ASSERT_MESSAGE("The strip is one surface; its gaps do not lower the peek",
                           m_Editor->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A gap does not clear the emphasis",
                                 PeekAxis(QmitkMxNSyncAxis::Crosshair), m_Editor->GetSyncPeekAxis());
    CPPUNIT_ASSERT_EQUAL(PeekAxis(QmitkMxNSyncAxis::Crosshair), this->Overlay(0)->SyncPeekAxis());

    // Only another glyph moves it.
    m_Editor->OnSyncPeekHovered(true, QmitkMxNSyncAxis::Lut);
    CPPUNIT_ASSERT_EQUAL(PeekAxis(QmitkMxNSyncAxis::Lut), m_Editor->GetSyncPeekAxis());

    // And only leaving the strip drops it.
    m_Editor->OnSyncPeekHovered(false, std::nullopt);
    Pump();
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());
  }

  void Gesture_EnteringOnAGapRaisesWithoutEmphasis()
  {
    this->Arrange(1, 2);

    // Reaching the strip between two glyphs is still an engagement: the plate
    // comes up, with nothing emphasised until a glyph is actually reached.
    m_Editor->OnSyncPeekHovered(true, std::nullopt);
    Pump();
    CPPUNIT_ASSERT(m_Editor->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL(PeekAxis(), m_Editor->GetSyncPeekAxis());
    CPPUNIT_ASSERT_MESSAGE("The cells raise a plate with no axis pumped",
                           this->Overlay(0)->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL(PeekAxis(), this->Overlay(0)->SyncPeekAxis());
  }

  void Gesture_LeavingBeforeTheDwellNeverRaises()
  {
    this->Arrange(1, 2);

    m_Editor->OnSyncPeekHovered(true, QmitkMxNSyncAxis::Crosshair);
    m_Editor->OnSyncPeekHovered(false, std::nullopt);
    Pump();
    CPPUNIT_ASSERT_MESSAGE("A pointer that crosses without resting raises nothing",
                           !m_Editor->IsSyncPeekVisible());
  }

  void Gesture_MaximizingLowersThePeek()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, QmitkMxNSyncAxis::Slice);
    CPPUNIT_ASSERT(m_Editor->IsSyncPeekVisible());

    m_Editor->SetMaximizedCell(CellId(0));
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());
    CPPUNIT_ASSERT(!this->Overlay(0)->IsSyncPeekVisible());
  }

  void Gesture_LayoutChangeLowersThePeek()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, QmitkMxNSyncAxis::Slice);

    this->Arrange(2, 1);
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());
  }

  void Gesture_AddingACellLowersThePeek()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, QmitkMxNSyncAxis::Slice);

    // A grid op changes the set of cells the comparison was made of, and it
    // reaches the peek through the same LayoutChanged the grid rebuild emits.
    m_Editor->AddGridColumn();
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());

    m_Editor->SetSyncPeek(true, QmitkMxNSyncAxis::Slice);
    m_Editor->RemoveGridColumn();
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());
  }

  void Gesture_CleanViewLowersThePeek()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, QmitkMxNSyncAxis::Slice);

    m_Editor->SetCleanView(true);
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());
    m_Editor->SetCleanView(false);
  }

  // ---------- Arrange mode ----------

  /** Every registered cell's overlay reports its plate up (or down). */
  bool AllPlates(bool up) const
  {
    for (const auto& [windowId, cell] : m_Editor->GetRenderWindowWidgets())
    {
      auto* overlay = cell->findChild<QmitkMxNCellOverlay*>();
      if (nullptr == overlay || overlay->IsSyncPeekVisible() != up)
      {
        return false;
      }
    }
    return true;
  }

  void Arrange_PlatesAreUpWhileArranging()
  {
    this->Arrange(1, 2);
    CPPUNIT_ASSERT_MESSAGE("Without arrange mode or a hover there is no plate", this->AllPlates(false));

    m_Editor->GetArrangeMode()->SetActive(true);
    CPPUNIT_ASSERT_MESSAGE("Arrange mode raises every plate without any dwell", this->AllPlates(true));
    CPPUNIT_ASSERT_MESSAGE("Arrange mode is not the hover peek", !m_Editor->IsSyncPeekVisible());
  }

  void Arrange_StripGraceExpiryKeepsThePlates()
  {
    this->Arrange(1, 2);
    m_Editor->GetArrangeMode()->SetActive(true);

    m_Editor->OnSyncPeekHovered(true, QmitkMxNSyncAxis::Slice);
    CPPUNIT_ASSERT(PeekAxis(QmitkMxNSyncAxis::Slice) == this->Overlay(1)->SyncPeekAxis());

    m_Editor->OnSyncPeekHovered(false, std::nullopt);
    Pump();  // the grace is zero here, so its expiry has run
    CPPUNIT_ASSERT_MESSAGE("Leaving the strip lowers only the hover term", this->AllPlates(true));
    CPPUNIT_ASSERT_MESSAGE("...and drops the emphasis the strip set",
                           !this->Overlay(1)->SyncPeekAxis().has_value());
  }

  void Arrange_LayoutChangesKeepThePlatesAndReachNewCells()
  {
    this->Arrange(1, 2);
    m_Editor->GetArrangeMode()->SetActive(true);

    this->Arrange(1, 3);
    Pump();
    CPPUNIT_ASSERT_MESSAGE("A rebuilt layout comes back with plates", this->AllPlates(true));

    m_Editor->AddGridRow();
    Pump();
    Pump();
    CPPUNIT_ASSERT_MESSAGE("A cell added while arranging shows its plate like the others",
                           this->AllPlates(true));
  }

  void Arrange_MaximizeAndCleanViewKeepThePlate()
  {
    this->Arrange(1, 2);
    m_Editor->GetArrangeMode()->SetActive(true);

    m_Editor->SetMaximizedCell(CellId(0));
    Pump();
    CPPUNIT_ASSERT_MESSAGE("The visible cell keeps its plate while maximized",
                           this->Overlay(0)->IsSyncPeekVisible());
    m_Editor->SetMaximizedCell(QString());
    Pump();

    m_Editor->SetCleanView(true);
    CPPUNIT_ASSERT_MESSAGE("Clean view does not lower an arranging plate", this->AllPlates(true));
    m_Editor->SetCleanView(false);
  }

  void Arrange_ResizingTheCellsChangesTheSharedBox()
  {
    this->Arrange(1, 2);
    m_Editor->GetArrangeMode()->SetActive(true);
    const int wide = this->Overlay(0)->SyncPeekGlyphBox();
    CPPUNIT_ASSERT(wide > 0);

    // The view appearing beside the editor narrows it; the plates must follow.
    // Half the size is small enough that no arrangement keeps the old box.
    m_Editor->resize(EditorWidth / 2, EditorHeight / 2);
    Pump();
    Pump();
    const int narrow = this->Overlay(0)->SyncPeekGlyphBox();
    CPPUNIT_ASSERT_MESSAGE("A smaller editor gets a smaller shared box", narrow < wide);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Every plate shares the re-resolved box",
                                 narrow, this->Overlay(1)->SyncPeekGlyphBox());
  }

  void Arrange_LeavingLowersEveryPlateAtOnce()
  {
    this->Arrange(1, 2);
    m_Editor->GetArrangeMode()->SetActive(true);
    CPPUNIT_ASSERT(this->AllPlates(true));

    m_Editor->GetArrangeMode()->SetActive(false);
    CPPUNIT_ASSERT_MESSAGE("Leaving arrange mode lowers every plate synchronously", this->AllPlates(false));
  }

  void Arrange_StripLeavingClearsOnlyItsOwnEmphasis()
  {
    this->Arrange(1, 2);
    auto* arrangeMode = m_Editor->GetArrangeMode();
    arrangeMode->SetActive(true);

    m_Editor->OnSyncPeekHovered(true, QmitkMxNSyncAxis::Slice);
    // Another surface takes over the emphasis before the strip's grace runs out.
    arrangeMode->SetHighlight(QmitkMxNArrangeMode::HighlightSource::Editor, QmitkMxNSyncAxis::Zoom,
                              QStringList(), QColor());
    m_Editor->OnSyncPeekHovered(false, std::nullopt);
    Pump();
    CPPUNIT_ASSERT_MESSAGE("The strip's leave must not clear another surface's emphasis",
                           PeekAxis(QmitkMxNSyncAxis::Zoom) == this->Overlay(0)->SyncPeekAxis());
  }

  void Arrange_StripHoverRingsTheCellsSharingItsAxis()
  {
    this->Arrange(1, 3);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "sliceGroup");
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "sliceGroup");
    auto* arrangeMode = m_Editor->GetArrangeMode();
    arrangeMode->SetActive(true);

    auto* strip = m_Editor->GetRenderWindowWidget(CellId(2))->GetUtilityWidget();
    emit strip->SyncPeekHovered(true, QmitkMxNSyncAxis::Slice);

    QStringList ringed = arrangeMode->GetHighlightedWindowIds();
    ringed.sort();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The strip rings every cell sharing its cell's synchronization",
                                 (CellId(0) + QLatin1Char(',') + CellId(2)).toStdString(),
                                 ringed.join(QLatin1Char(',')).toStdString());
  }

  // ---------- The barcode's report ----------

  void Barcode_PassiveStripReportsTheGlyphUnderThePointer()
  {
    QmitkMxNSyncBarcodeWidget strip;
    strip.SetSlots(PlainSlots());
    strip.resize(240, 24);
    BarcodeRecorder recorder(strip);

    const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(240, 24, QmitkMxNSyncAxisCount);
    CPPUNIT_ASSERT_MESSAGE("This geometry must render glyphs for the test to mean anything",
                           layout.mode == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs);

    // The glyph boxes start one pixel in and repeat every box + 2 px gap.
    const int step = layout.box + 2;
    MoveTo(strip, QPoint(1 + layout.box / 2, 12));
    MoveTo(strip, QPoint(1 + 3 * step + layout.box / 2, 12));
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(recorder.peekAxes.size()));
    CPPUNIT_ASSERT_EQUAL(0, recorder.peekAxes[0]);
    CPPUNIT_ASSERT_EQUAL(3, recorder.peekAxes[1]);
    CPPUNIT_ASSERT(recorder.onStrip[0] && recorder.onStrip[1]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A passive strip leaves AxisHovered to the clickable mode",
                                 0, static_cast<int>(recorder.axes.size()));

    QEvent leave(QEvent::Leave);
    static_cast<QObject&>(strip).event(&leave);
    CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(recorder.peekAxes.size()));
    CPPUNIT_ASSERT_MESSAGE("Leaving reports the pointer off the strip", !recorder.onStrip[2]);
  }

  void Barcode_PassiveStripReportsLeavingTheGlyphsButNotTheStrip()
  {
    QmitkMxNSyncBarcodeWidget strip;
    strip.SetSlots(PlainSlots());
    strip.resize(240, 24);
    BarcodeRecorder recorder(strip);

    const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(240, 24, QmitkMxNSyncAxisCount);
    const QRect content(1, (24 - layout.box) / 2, QmitkMxNSyncAxisCount * layout.box + (QmitkMxNSyncAxisCount - 1) * 2, layout.box);

    MoveTo(strip, QPoint(1 + layout.box / 2, content.center().y()));
    CPPUNIT_ASSERT_EQUAL(0, recorder.peekAxes.back());

    // The 2 px gap between the first two glyph boxes: still on the strip, so the
    // receiver must hear "no axis" rather than "gone".
    MoveTo(strip, QPoint(1 + layout.box, content.center().y()));
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(recorder.peekAxes.size()));
    CPPUNIT_ASSERT_MESSAGE("A gap between glyphs is still the strip", recorder.onStrip.back());
    CPPUNIT_ASSERT_EQUAL(-1, recorder.peekAxes.back());

    // Past the drawn slots the strip is inert, and the pointer counts as off it.
    MoveTo(strip, QPoint(content.right() + 20, content.center().y()));
    CPPUNIT_ASSERT_MESSAGE("The inert trailing space is not the strip",
                           !recorder.onStrip.back());
  }

  void Barcode_PassiveStripReportsColorBarModeWithoutAnAxis()
  {
    // The peek answers for the whole window, so a strip too narrow for glyphs
    // still has something to show. It just names no axis: a featureless colour
    // slot gives the user nothing to point at, so the answer comes up with none
    // emphasised rather than not at all.
    QmitkMxNSyncBarcodeWidget strip;
    strip.SetSlots(PlainSlots());
    strip.resize(71, 16);
    BarcodeRecorder recorder(strip);

    const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(71, 16, QmitkMxNSyncAxisCount);
    CPPUNIT_ASSERT_MESSAGE("This geometry must collapse to color slots",
                           layout.mode == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::ColorBar);

    MoveTo(strip, QPoint(3, 8));
    MoveTo(strip, QPoint(27, 8));

    CPPUNIT_ASSERT_MESSAGE("Being on a collapsed strip is still reported",
                           !recorder.onStrip.isEmpty() && recorder.onStrip.back());
    for (const int axis : recorder.peekAxes)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("but it names no axis", -1, axis);
    }
  }

  void Barcode_PassiveStripStillOpensTheLayoutEditor()
  {
    QmitkMxNSyncBarcodeWidget strip;
    strip.SetSlots(PlainSlots());
    strip.resize(240, 24);
    BarcodeRecorder recorder(strip);

    ClickAt(strip, QPoint(20, 12));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The whole strip is still one button onto the editor",
                                 1, recorder.clicks);
  }

  void Barcode_ClickableStripReportsInBothRenderModes()
  {
    for (const QSize size : { QSize(240, 24), QSize(71, 16) })
    {
      QmitkMxNSyncBarcodeWidget strip;
      strip.SetAxisClickable(true);
      strip.SetSlots(PlainSlots());
      strip.resize(size);
      BarcodeRecorder recorder(strip);

      const auto layout =
        QmitkMxNSyncBarcodeWidget::ComputeLayout(size.width(), size.height(), QmitkMxNSyncAxisCount);
      const bool glyphs = layout.mode == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs;
      const QPoint firstSlot = glyphs ? QPoint(1 + layout.box / 2, size.height() / 2)
                                      : QPoint(3, size.height() / 2);
      MoveTo(strip, firstSlot);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("The editor's group cards report in either rendering",
                                   1, static_cast<int>(recorder.axes.size()));
      CPPUNIT_ASSERT_EQUAL(0, recorder.axes[0]);
    }
  }

  void Barcode_NotApplicableSlotIsInert()
  {
    constexpr int notApplicable = 2;
    constexpr int applicable = 5;

    auto axisSlots = PlainSlots();
    axisSlots[notApplicable].applicable = false;

    QmitkMxNSyncBarcodeWidget strip;
    strip.SetAxisClickable(true);
    strip.SetSlots(axisSlots);
    strip.resize(240, 24);

    QList<int> clicked;
    QObject::connect(&strip, &QmitkMxNSyncBarcodeWidget::AxisClicked,
                     [&clicked](int index) { clicked.append(index); });

    const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(240, 24, QmitkMxNSyncAxisCount);
    CPPUNIT_ASSERT_MESSAGE("This geometry must render glyphs for the test to mean anything",
                           layout.mode == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs);
    const int step = layout.box + 2;
    const QPoint notApplicablePoint(1 + notApplicable * step + layout.box / 2, 12);
    const QPoint applicablePoint(1 + applicable * step + layout.box / 2, 12);

    MoveTo(strip, applicablePoint);
    CPPUNIT_ASSERT_MESSAGE("An applicable slot invites a click",
                           strip.cursor().shape() == Qt::PointingHandCursor);
    MoveTo(strip, notApplicablePoint);
    CPPUNIT_ASSERT_MESSAGE("A slot that does not apply gives no click affordance",
                           strip.cursor().shape() == Qt::ArrowCursor);

    ClickAt(strip, notApplicablePoint);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A click on a slot that does not apply reports nothing",
                                 0, static_cast<int>(clicked.size()));

    ClickAt(strip, applicablePoint);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A click on an applicable slot is reported once",
                                 1, static_cast<int>(clicked.size()));
    CPPUNIT_ASSERT_EQUAL(applicable, clicked[0]);
  }

  // ---------- The barcode's highlight ----------

  /** The strip as PaintInto draws it on black, with a gray gap colour. */
  static QImage PaintStrip(const QSize& size, const QList<QmitkMxNSyncBarcodeWidget::AxisSlot>& axisSlots,
                           bool hovered, int hoveredSlot)
  {
    QImage canvas(size, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::black);
    QPainter painter(&canvas);
    QmitkMxNSyncBarcodeWidget::PaintInto(painter, QRect(QPoint(0, 0), size), axisSlots, hovered,
                                         QColor(100, 100, 100), hoveredSlot);
    return canvas;
  }

  /** Where the strip draws slot 'index': a glyph box, or a 7 px colour slot. */
  static QRect StripSlotRect(const QSize& size, int slotCount, int index)
  {
    const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(size.width(), size.height(), slotCount);
    if (layout.mode == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs)
    {
      const int top = 1 + std::max(0, (size.height() - 2 - layout.box) / 2);
      return QRect(1 + index * (layout.box + 2), top, layout.box, layout.box);
    }
    return QRect(index * 8, (size.height() - 16) / 2, 7, 16);
  }

  static qint64 Ink(const QImage& image, const QRect& rect)
  {
    qint64 ink = 0;
    for (int y = rect.top(); y <= rect.bottom(); ++y)
    {
      for (int x = rect.left(); x <= rect.right(); ++x)
      {
        const QRgb pixel = image.pixel(x, y);
        ink += qRed(pixel) + qGreen(pixel) + qBlue(pixel);
      }
    }
    return ink;
  }

  static int MaxGreen(const QImage& image, const QRect& rect)
  {
    int green = 0;
    for (int y = rect.top(); y <= rect.bottom(); ++y)
    {
      for (int x = rect.left(); x <= rect.right(); ++x)
      {
        green = std::max(green, qGreen(image.pixel(x, y)));
      }
    }
    return green;
  }

  void Barcode_HoveredSlotRaisedOthersFade()
  {
    // Pure red slots: any green in a linked slot is a white frame.
    constexpr int hovered = 2;
    constexpr int unlinked = 5;
    auto axisSlots = PlainSlots();
    for (auto& slot : axisSlots)
    {
      slot.color = QColor(255, 0, 0);
    }
    axisSlots[unlinked].color = QColor();
    const int slotCount = static_cast<int>(axisSlots.size());

    for (const QSize size : { QSize(240, 24), QSize(71, 16) })
    {
      const bool glyphs = QmitkMxNSyncBarcodeWidget::ComputeLayout(size.width(), size.height(), slotCount).mode
                          == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs;
      const std::string mode = glyphs ? " (glyphs)" : " (colour slots)";
      const QImage resting = PaintStrip(size, axisSlots, false, -1);
      const QImage pointed = PaintStrip(size, axisSlots, false, hovered);

      const QRect hoveredRect = StripSlotRect(size, slotCount, hovered);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A hovered linked slot keeps its resting look" + mode,
                                   Ink(resting, hoveredRect), Ink(pointed, hoveredRect));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("in its own colour, nothing white" + mode, 0, MaxGreen(pointed, hoveredRect));

      for (int slot = 0; slot < slotCount; ++slot)
      {
        if (slot == hovered)
        {
          continue;
        }
        const QRect rect = StripSlotRect(size, slotCount, slot);
        const double ratio = static_cast<double>(Ink(pointed, rect)) / static_cast<double>(Ink(resting, rect));
        CPPUNIT_ASSERT_MESSAGE("Slot " + std::to_string(slot) + " fades to 35 % of its resting look" + mode,
                               ratio > 0.28 && ratio < 0.42);
      }

      if (glyphs)
      {
        const QImage pointedAtGap = PaintStrip(size, axisSlots, false, unlinked);
        const QRect gapRect = StripSlotRect(size, slotCount, unlinked);
        CPPUNIT_ASSERT_MESSAGE("A hovered unlinked slot is raised",
                               Ink(pointedAtGap, gapRect) > 3 * Ink(resting, gapRect) / 2);
        CPPUNIT_ASSERT_MESSAGE("in the gap colour, not in white", MaxGreen(pointedAtGap, gapRect) <= 100);
      }
    }
  }

  void Barcode_OffsetFillsTheLowerRightCorner()
  {
    // The plate's ribbon sits at the glyph's lower right, so the barcode's
    // small version of it does too; the upper right is left to warnings.
    const QSize size(240, 24);
    constexpr int shifted = 0;
    auto axisSlots = PlainSlots();
    const int slotCount = static_cast<int>(axisSlots.size());
    const QImage plain = PaintStrip(size, axisSlots, false, -1);
    axisSlots[shifted].hasOffset = true;
    const QImage marked = PaintStrip(size, axisSlots, false, -1);

    const QRect box = StripSlotRect(size, slotCount, shifted);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the strip draws glyphs",
                           QmitkMxNSyncBarcodeWidget::ComputeLayout(size.width(), size.height(), slotCount).mode
                             == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs);
    const QRect lowerRight(box.right() - 2, box.bottom() - 2, 2, 2);
    const QRect upperRight(box.right() - 2, box.top() + 1, 2, 2);
    CPPUNIT_ASSERT_MESSAGE("An offset fills the box's lower-right corner",
                           Ink(marked, lowerRight) > Ink(plain, lowerRight));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("and leaves its upper-right corner as it is",
                                 Ink(plain, upperRight), Ink(marked, upperRight));
  }

  void Barcode_PassiveStripHoverIsAWashBehindTheContent()
  {
    auto axisSlots = PlainSlots();
    for (auto& slot : axisSlots)
    {
      slot.color = QColor(255, 0, 0);
    }
    const int slotCount = static_cast<int>(axisSlots.size());

    for (const QSize size : { QSize(240, 24), QSize(71, 16) })
    {
      const bool glyphs = QmitkMxNSyncBarcodeWidget::ComputeLayout(size.width(), size.height(), slotCount).mode
                          == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs;
      const std::string mode = glyphs ? " (glyphs)" : " (colour slots)";
      const QImage resting = PaintStrip(size, axisSlots, false, -1);
      const QImage washed = PaintStrip(size, axisSlots, true, -1);

      const QRect first = StripSlotRect(size, slotCount, 0);
      const QPoint between(first.right() + 1, first.center().y());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("At rest the gap between two slots is empty" + mode, 0,
                                   qGray(resting.pixel(between)));
      const int wash = qGray(washed.pixel(between));
      CPPUNIT_ASSERT_MESSAGE("Hovered, the button wash lies behind the slots" + mode, wash >= 20 && wash <= 32);

      const QPoint past(StripSlotRect(size, slotCount, slotCount - 1).right() + 4, first.center().y());
      if (past.x() < size.width())
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("but not past them" + mode, 0, qGray(washed.pixel(past)));
      }

      for (int slot = 0; slot < slotCount; ++slot)
      {
        CPPUNIT_ASSERT_MESSAGE("The slots keep their own colour, nothing turns white" + mode,
                               MaxGreen(washed, StripSlotRect(size, slotCount, slot)) <= 40);
      }
    }
  }

  void Glyph_LargeRenderIsNotAnUpscaledResource()
  {
    // The resources declare a 24 px intrinsic size, and a peek plate draws the
    // emphasised axis at nearly three times that. Rasterising at 24 and scaling
    // the bitmap up reads as a blurred glyph, so the renderer must hand the size
    // to the SVG renderer instead - which is exactly what makes a large render
    // differ from an upscaled small one.
    const QColor color(200, 120, 60);
    const QPixmap large = QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph::Crosshair, color, 96);
    const QPixmap small = QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph::Crosshair, color, 24);
    CPPUNIT_ASSERT(!large.isNull() && !small.isNull());
    CPPUNIT_ASSERT_EQUAL(96, large.width());

    const QImage vector = large.toImage().convertToFormat(QImage::Format_ARGB32);
    const QImage upscaled = small.toImage()
                              .scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                              .convertToFormat(QImage::Format_ARGB32);

    double difference = 0.0;
    for (int y = 0; y < 96; ++y)
    {
      for (int x = 0; x < 96; ++x)
      {
        difference += std::abs(qAlpha(vector.pixel(x, y)) - qAlpha(upscaled.pixel(x, y)));
      }
    }
    difference /= 96.0 * 96.0;

    // An upscale of the 24 px raster would be the very image this compares
    // against, so any real separation here means the glyph was drawn as vector
    // art at the requested size.
    CPPUNIT_ASSERT_MESSAGE("A large glyph must be rendered, not upscaled", difference > 2.0);
  }

  void Glyph_RepeatedRenderIsServedFromTheCache()
  {
    // A persistent plate repaints on every render-end value change, so the
    // artwork must not be re-read and re-rasterised on each call.
    const QColor color(200, 120, 60);
    const QPixmap first = QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph::Zoom, color, 35);
    const QPixmap second = QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph::Zoom, color, 35);
    CPPUNIT_ASSERT(!first.isNull());
    CPPUNIT_ASSERT_EQUAL(first.cacheKey(), second.cacheKey());

    const QPixmap otherColor = QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph::Zoom, QColor(60, 120, 200), 35);
    const QPixmap otherSize = QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph::Zoom, color, 36);
    CPPUNIT_ASSERT(first.cacheKey() != otherColor.cacheKey());
    CPPUNIT_ASSERT(first.cacheKey() != otherSize.cacheKey());

    const QPixmap sticker = QmitkMxNRenderAxisGlyphSticker(QmitkMxNAxisGlyph::Zoom, color, 35, 2.0);
    const QPixmap stickerAgain = QmitkMxNRenderAxisGlyphSticker(QmitkMxNAxisGlyph::Zoom, color, 35, 2.0);
    CPPUNIT_ASSERT(!sticker.isNull());
    CPPUNIT_ASSERT_EQUAL(sticker.cacheKey(), stickerAgain.cacheKey());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The sticker comes with its pixel ratio already set",
                                 2.0, sticker.devicePixelRatio());
    const QPixmap otherRatio = QmitkMxNRenderAxisGlyphSticker(QmitkMxNAxisGlyph::Zoom, color, 35, 1.0);
    CPPUNIT_ASSERT(sticker.cacheKey() != otherRatio.cacheKey());
  }

  void Glyph_StickerRingsTheGlyphWhiteOutsideDarkInside()
  {
    const QColor hue(200, 120, 60);
    constexpr int side = 35;
    const QPixmap sticker = QmitkMxNRenderAxisGlyphSticker(QmitkMxNAxisGlyph::Crosshair, hue, side, 1.0);
    CPPUNIT_ASSERT(!sticker.isNull());
    const int margin = QmitkMxNAxisGlyphStickerMargin(side);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The rings grow outward, around the full-size glyph",
                                 side + 2 * margin, sticker.width());

    const auto opaque = [](QRgb pixel) { return qAlpha(pixel) > 200; };
    const auto white = [&](QRgb pixel) { return opaque(pixel) && qRed(pixel) > 220 && qGreen(pixel) > 220 && qBlue(pixel) > 220; };
    const auto dark = [&](QRgb pixel) { return opaque(pixel) && qRed(pixel) < 40 && qGreen(pixel) < 40 && qBlue(pixel) < 40; };
    const auto inHue = [&](QRgb pixel)
    {
      return opaque(pixel) && std::abs(qRed(pixel) - hue.red()) < 30
             && std::abs(qGreen(pixel) - hue.green()) < 30 && std::abs(qBlue(pixel) - hue.blue()) < 30;
    };

    // Walking in from the left edge, a line through the glyph must meet the
    // outer ring, then the inner ring, then the glyph itself.
    const QImage image = sticker.toImage().convertToFormat(QImage::Format_ARGB32);
    bool found = false;
    for (int y = 0; y < image.height() && !found; ++y)
    {
      int stage = 0;
      for (int x = 0; x < image.width() && stage < 3; ++x)
      {
        const QRgb pixel = image.pixel(x, y);
        if ((0 == stage && white(pixel)) || (1 == stage && dark(pixel)) || (2 == stage && inHue(pixel)))
        {
          ++stage;
        }
      }
      found = 3 == stage;
    }
    CPPUNIT_ASSERT_MESSAGE("White outer ring, dark inner ring, then the glyph", found);
  }

  void Glyph_StickerOutlineStaysCrispWhereTheGlyphFades()
  {
    // The lookup-table glyph is a gradient that fades out towards its left end.
    // Its outline must still be solid there, or the sticker loses its edge on
    // exactly the side where the glyph itself is faint.
    const QPixmap sticker =
      QmitkMxNRenderAxisGlyphSticker(QmitkMxNAxisGlyph::Lut, QColor(80, 140, 240), 70, 1.0);
    CPPUNIT_ASSERT(!sticker.isNull());
    const QImage image = sticker.toImage().convertToFormat(QImage::Format_ARGB32);
    const int y = image.height() / 2;
    int first = -1;
    for (int x = 0; x < image.width() && first < 0; ++x)
    {
      if (qAlpha(image.pixel(x, y)) > 0)
      {
        first = x;
      }
    }
    CPPUNIT_ASSERT(first >= 0);
    int peak = 0;
    for (int x = first; x < std::min(image.width(), first + 4); ++x)
    {
      peak = std::max(peak, qAlpha(image.pixel(x, y)));
    }
    CPPUNIT_ASSERT_MESSAGE("The outer ring is opaque at the faded end", peak > 240);
  }

  void Peek_FadeOutKeepsThePlateGeometry()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, QmitkMxNSyncAxis::Slice);
    auto* overlay = this->Overlay(0);
    overlay->SetPeekProgress(1.0);  // faded in, whether or not the style animates
    const QRect raised = overlay->SyncPeekPlateRect();
    CPPUNIT_ASSERT(raised.isValid());

    // A frame of the fade-out, whether or not the style animates it.
    m_Editor->SetSyncPeek(false, std::nullopt);
    overlay->SetPeekProgress(0.5);
    CPPUNIT_ASSERT_MESSAGE("The peek reports itself down at once", !overlay->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL(0, overlay->SyncPeekGlyphBox());
    CPPUNIT_ASSERT_MESSAGE("...but the fading plate keeps the geometry it is painted at",
                           raised == overlay->SyncPeekPlateRect());

    overlay->SetPeekProgress(0.0);
    CPPUNIT_ASSERT_MESSAGE("A finished fade leaves no plate", !overlay->SyncPeekPlateRect().isValid());
  }

  void PaintPath_PlateDoesNotCrash()
  {
    this->Arrange(1, 2);
    for (const auto& [windowId, cell] : m_Editor->GetRenderWindowWidgets())
    {
      mitk::RenderingManager::GetInstance()->InitializeView(
        cell->GetRenderWindow()->GetVtkRenderWindow(), m_Image->GetTimeGeometry());
    }

    // An axis this cell does not synchronize still raises the peek: the plate
    // draws it in the absent state rather than skipping it.
    m_Editor->ClearSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing);
    m_Editor->SetSyncPeek(true, QmitkMxNSyncAxis::Windowing);
    CPPUNIT_ASSERT_MESSAGE("The layout must host a plate for the paint smoke to mean anything",
                           this->Overlay(0)->SyncPeekGlyphBox() > 0);

    auto* overlay = this->Overlay(0);
    overlay->SetPeekProgress(1.0);
    overlay->grab();  // forces a synchronous paintEvent

    // The plate with nothing emphasised drives the empty-caption branch.
    m_Editor->SetSyncPeek(true, std::nullopt);
    overlay->SetPeekProgress(1.0);
    overlay->grab();

    // Clean view short-circuits the whole paint, the plate included.
    m_Editor->SetCleanView(true);
    overlay->grab();
    m_Editor->SetCleanView(false);

    CPPUNIT_ASSERT(true);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNSyncPeek)
