/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNAxisGlyph.h>
#include <QmitkMxNCellOverlay.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>
#include <QmitkRenderWindow.h>
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
  MITK_TEST(PeekPlate_ValueBandSitsUnderTheRowAndAboveTheName);
  MITK_TEST(PeekPlate_ValueBandIsReservedWhicheverAxisIsPumped);
  MITK_TEST(Barcode_GlyphBoxFillsTheHeightItIsGranted);
  MITK_TEST(Barcode_WidthAskedForFollowsTheGrantedHeight);
  MITK_TEST(Barcode_WrapsRatherThanShrinksWhenTheHostAllowsIt);
  MITK_TEST(Barcode_WrapsEvenlyAndNeverWhenForbidden);
  MITK_TEST(Barcode_HostCeilingBoundsTheGlyph);
  MITK_TEST(Barcode_FrameStaysInsideTheTargetAtEveryPixelRatio);
  MITK_TEST(MaxPeekGlyphBox_ZeroBelowTheFloorAndNeverShrinksWithTheCell);
  MITK_TEST(ResolvePeekGlyphBox_LegibleAndDrivenByTheDensestGrid);
  MITK_TEST(ResolvePeekGlyphBox_ZeroWhenNoCellQualifies);
  MITK_TEST(ResolvePeekGlyphBox_IgnoresHiddenCells);
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
  MITK_TEST(Glyph_LargeRenderIsNotAnUpscaledResource);
  MITK_TEST(Barcode_PassiveStripReportsTheGlyphUnderThePointer);
  MITK_TEST(Barcode_PassiveStripReportsLeavingTheGlyphsButNotTheStrip);
  MITK_TEST(Barcode_PassiveStripIsSilentInColorBarMode);
  MITK_TEST(Barcode_PassiveStripStillOpensTheLayoutEditor);
  MITK_TEST(Barcode_ClickableStripReportsInBothRenderModes);
  MITK_TEST(PaintPath_PlateDoesNotCrash);
  CPPUNIT_TEST_SUITE_END();

  // Wide enough that a single cell, and a 1x3 grid, both host a plate while a
  // 1x5 grid cannot - the three cases the sizing rule has to tell apart.
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

  /** Eight plain slots, enough for the strip's hit testing and its paint. */
  static QList<QmitkMxNSyncBarcodeWidget::AxisSlot> EightSlots()
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
          for (int other = axis + 1; other < QmitkMxNCellOverlay::PeekAxisCount; ++other)
          {
            CPPUNIT_ASSERT_MESSAGE("No two glyphs overlap",
                                   !layout.glyphs[axis].intersects(layout.glyphs[other]));
          }
        }
        CPPUNIT_ASSERT_MESSAGE("The caption lies inside the plate",
                               layout.plate.contains(layout.caption));
        CPPUNIT_ASSERT_MESSAGE("The name line lies inside the plate",
                               layout.plate.contains(layout.name));
        const QRect emphasised = layout.glyphs[pumped < 0 ? 0 : pumped];
        CPPUNIT_ASSERT_MESSAGE("The caption sits above the glyph row",
                               layout.caption.bottom() < emphasised.top());
        CPPUNIT_ASSERT_MESSAGE("The name line sits below the glyph row",
                               layout.name.top() > emphasised.bottom());
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
        CPPUNIT_ASSERT_MESSAGE("The caption keeps its place across a switch",
                               layout.caption == reference.caption);
        CPPUNIT_ASSERT_MESSAGE("The name line keeps its place across a switch",
                               layout.name == reference.name);
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
      QSize(900, 700), QmitkMxNCellOverlay::PeekGlyphBoxMin,
      QmitkMxNCellOverlay::PeekAxisCount, LineHeight());
    CPPUNIT_ASSERT_MESSAGE("An axis past the eight gets no plate",
                           !noSuchAxis.plate.isValid());
  }

  void Barcode_WrapsRatherThanShrinksWhenTheHostAllowsIt()
  {
    // A cell-map tile has vertical room to spare, and a legible glyph matters
    // more there than a shallow band: breaking the row buys a far larger box
    // than squeezing eight of them into the width.
    constexpr int slotCount = 8;
    const QSize band(120, 56);

    const auto wrapped = QmitkMxNSyncBarcodeWidget::ComputeLayout(
      band.width(), band.height(), slotCount, { true, 0 });
    const auto flat = QmitkMxNSyncBarcodeWidget::ComputeLayout(
      band.width(), band.height(), slotCount, { false, 0 });

    CPPUNIT_ASSERT_MESSAGE("Allowed to wrap, it does", wrapped.rows > 1);
    CPPUNIT_ASSERT_MESSAGE("and comes out with a larger glyph than one row allows",
                           wrapped.box > flat.box);
  }

  void Barcode_WrapsEvenlyAndNeverWhenForbidden()
  {
    // An even split reads calmer than a ragged last row, and costs the same.
    constexpr int slotCount = 8;
    for (const QSize band : { QSize(120, 56), QSize(90, 80), QSize(70, 90) })
    {
      const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(
        band.width(), band.height(), slotCount, { true, 0 });
      if (layout.mode != QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs)
      {
        continue;
      }
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Every row holds the same number of glyphs", 0,
                                   slotCount % layout.columns);
    }

    // A chrome row must stay one line whatever the rect would allow.
    const auto forbidden = QmitkMxNSyncBarcodeWidget::ComputeLayout(120, 90, slotCount,
                                                                   { false, 0 });
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Forbidden to wrap, it stays one line", 1, forbidden.rows);
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
    // outside the canvas, which loses the top line - and loses it worst when the
    // frame is lit, because a lit frame is drawn part-transparent. Stroked half a
    // pixel inside, the top line survives and matches the bottom one.
    constexpr int slotCount = 8;
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
        // hovered: every frame lit, which is the state the top line vanished in
        QmitkMxNSyncBarcodeWidget::PaintInto(painter, QRect(QPoint(0, 0), logical), axisSlots,
                                             true, QColor(Qt::gray), -1, { false, 0 });
      }

      const auto layout =
        QmitkMxNSyncBarcodeWidget::ComputeLayout(logical.width(), logical.height(), slotCount,
                                                 { false, 0 });
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

  void Barcode_HostCeilingBoundsTheGlyph()
  {
    // Unbounded, a two-cell layout's enormous tiles would render glyphs larger
    // than anything else in the editor.
    constexpr int slotCount = 8;
    const auto capped = QmitkMxNSyncBarcodeWidget::ComputeLayout(400, 200, slotCount,
                                                                 { true, 24 });
    const auto free = QmitkMxNSyncBarcodeWidget::ComputeLayout(400, 200, slotCount,
                                                               { true, 0 });
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The ceiling binds", 24, capped.box);
    CPPUNIT_ASSERT_MESSAGE("and without one the rect alone does", free.box > 24);
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
    for (int axis = 0; axis < 8; ++axis)
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
                                 QmitkMxNSyncBarcodeWidget::ComputeLayout(tallWidth, 32, 8).box);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A short strip still renders glyphs at its own height", 14,
                                 QmitkMxNSyncBarcodeWidget::ComputeLayout(shortWidth, 16, 8).box);
  }

  void Barcode_GlyphBoxFillsTheHeightItIsGranted()
  {
    // A strip that left slack above and below its glyphs wasted the chrome it
    // was given and made the axes harder to tell apart; the box takes the
    // height on offer, bounded only by the width it has to share.
    constexpr int slotCount = 8;
    constexpr int wide = 1000;  // wide enough that height is the binding limit

    for (const int height : { 18, 24, 32, 48 })
    {
      const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(wide, height, slotCount);
      CPPUNIT_ASSERT_MESSAGE("A wide strip renders glyphs",
                             layout.mode == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("One row, so the box takes the height", 1, layout.rows);
      // All of it but the pixel each frame needs above and below itself, which
      // is what keeps a box's own edges inside the canvas.
      CPPUNIT_ASSERT_EQUAL_MESSAGE("and nothing beyond the frames' own room is left over",
                                   height - 2, layout.box);
    }

    // Width still bounds it: eight boxes plus their gaps have to fit.
    const auto narrow = QmitkMxNSyncBarcodeWidget::ComputeLayout(160, 200, slotCount);
    CPPUNIT_ASSERT_MESSAGE("A narrow, tall strip is bounded by its width",
                           narrow.box < 200);
  }

  void PeekPlate_ValueBandSitsUnderTheRowAndAboveTheName()
  {
    // The offsets are read off the row, so their band belongs between the
    // glyphs and the window name - and inside the plate, like every other part.
    const QSize cell(600, 400);
    const int box = QmitkMxNCellOverlay::MaxPeekGlyphBox(cell, LineHeight());
    CPPUNIT_ASSERT(box >= QmitkMxNCellOverlay::PeekGlyphBoxMin);

    constexpr int pumped = 2;
    const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, pumped, LineHeight());
    CPPUNIT_ASSERT(layout.plate.isValid());

    // Each value hugs its own glyph, so the band reaches from just under a
    // resting glyph down past the pumped one - which therefore overlaps it.
    for (int axis = 0; axis < QmitkMxNCellOverlay::PeekAxisCount; ++axis)
    {
      if (axis == pumped)
      {
        continue;
      }
      CPPUNIT_ASSERT_MESSAGE("A resting glyph sits above the values",
                             layout.glyphs[axis].bottom() <= layout.values.top());
      CPPUNIT_ASSERT_MESSAGE("and has room for its value below it",
                             layout.values.top() - layout.glyphs[axis].bottom() < LineHeight());
    }
    CPPUNIT_ASSERT_MESSAGE("The band reaches below the pumped glyph",
                           layout.values.bottom() > layout.glyphs[pumped].bottom());
    CPPUNIT_ASSERT_MESSAGE("with room there for a value grown to match it",
                           layout.values.bottom() - layout.glyphs[pumped].bottom()
                             >= LineHeight());
    CPPUNIT_ASSERT_MESSAGE("The window name sits below them all",
                           layout.values.bottom() <= layout.name.top());
    CPPUNIT_ASSERT_MESSAGE("The band stays inside the plate",
                           layout.plate.contains(layout.values));
  }

  void PeekPlate_ValueBandIsReservedWhicheverAxisIsPumped()
  {
    // The band is reserved whether or not this window is offset anywhere, so a
    // plate never resizes as the pointer moves along the row or as offsets are
    // authored.
    const QSize cell(600, 400);
    const int box = QmitkMxNCellOverlay::MaxPeekGlyphBox(cell, LineHeight());
    const auto reference = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, -1, LineHeight());
    CPPUNIT_ASSERT(reference.plate.isValid());

    for (int pumped = 0; pumped < QmitkMxNCellOverlay::PeekAxisCount; ++pumped)
    {
      const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(cell, box, pumped, LineHeight());
      CPPUNIT_ASSERT_MESSAGE("The plate does not move or resize with the pumped axis",
                             layout.plate == reference.plate);
      CPPUNIT_ASSERT_MESSAGE("nor does the value band",
                             layout.values == reference.values);
    }
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

  // ---------- The one box the layout shares ----------

  void ResolvePeekGlyphBox_LegibleAndDrivenByTheDensestGrid()
  {
    this->Arrange(1, 1);
    const int single = m_Editor->ResolvePeekGlyphBox();
    CPPUNIT_ASSERT_MESSAGE("A full-editor cell hosts a plate", single > 0);
    CPPUNIT_ASSERT_MESSAGE("The box stays in the legible range",
                           single >= QmitkMxNCellOverlay::PeekGlyphBoxMin
                             && single <= QmitkMxNCellOverlay::PeekGlyphBoxMax);

    this->Arrange(1, 3);
    const int thirds = m_Editor->ResolvePeekGlyphBox();
    CPPUNIT_ASSERT_MESSAGE("Three cells across still host a plate", thirds > 0);
    CPPUNIT_ASSERT_MESSAGE("Narrower cells never ask for a larger box", thirds <= single);
    CPPUNIT_ASSERT_MESSAGE("The box stays in the legible range",
                           thirds >= QmitkMxNCellOverlay::PeekGlyphBoxMin
                             && thirds <= QmitkMxNCellOverlay::PeekGlyphBoxMax);
  }

  void ResolvePeekGlyphBox_ZeroWhenNoCellQualifies()
  {
    this->Arrange(1, 5);
    CPPUNIT_ASSERT_EQUAL(0, m_Editor->ResolvePeekGlyphBox());
  }

  void ResolvePeekGlyphBox_IgnoresHiddenCells()
  {
    this->Arrange(1, 5);
    CPPUNIT_ASSERT_EQUAL(0, m_Editor->ResolvePeekGlyphBox());

    // Maximizing leaves the slivers registered but hidden; sizing from one of
    // them would shrink the only plate the user can see.
    m_Editor->SetMaximizedCell(CellId(0));
    Pump();
    CPPUNIT_ASSERT_MESSAGE("The maximized cell alone decides the box",
                           m_Editor->ResolvePeekGlyphBox() > 0);
  }

  // ---------- The broadcast ----------

  void Broadcast_EveryVisibleCellSharesOneGeometry()
  {
    this->Arrange(1, 3);
    m_Editor->SetSyncPeek(true, 2);

    const int box = this->Overlay(0)->SyncPeekGlyphBox();
    CPPUNIT_ASSERT_MESSAGE("The layout hosts a plate", box > 0);
    for (int index = 0; index < 3; ++index)
    {
      CPPUNIT_ASSERT_MESSAGE("Every visible cell raises its plate",
                             this->Overlay(index)->IsSyncPeekVisible());
      CPPUNIT_ASSERT_EQUAL(2, this->Overlay(index)->SyncPeekAxis());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Every plate of a layout uses one glyph box",
                                   box, this->Overlay(index)->SyncPeekGlyphBox());
    }
    CPPUNIT_ASSERT_EQUAL(2, m_Editor->GetSyncPeekAxis());
  }

  void Broadcast_NegativeAxisClears()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, 5);
    CPPUNIT_ASSERT_EQUAL(5, this->Overlay(0)->SyncPeekAxis());

    m_Editor->SetSyncPeek(false, -1);
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

    m_Editor->SetSyncPeek(true, 1);
    CPPUNIT_ASSERT_MESSAGE("The visible cell shows the peek",
                           this->Overlay(0)->IsSyncPeekVisible());
    CPPUNIT_ASSERT_MESSAGE("A hidden cell shows no plate",
                           !this->Overlay(1)->IsSyncPeekVisible());
  }

  // ---------- The pointer gesture ----------

  void Gesture_DwellRaisesAndGraceLowers()
  {
    this->Arrange(1, 2);

    m_Editor->OnSyncPeekHovered(true, 3);
    CPPUNIT_ASSERT_MESSAGE("The dwell has not elapsed yet", !m_Editor->IsSyncPeekVisible());
    Pump();
    CPPUNIT_ASSERT(m_Editor->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL(3, m_Editor->GetSyncPeekAxis());

    m_Editor->OnSyncPeekHovered(false, -1);
    CPPUNIT_ASSERT_MESSAGE("The grace has not elapsed yet", m_Editor->IsSyncPeekVisible());
    Pump();
    CPPUNIT_ASSERT_MESSAGE("Leaving the strip lowers the peek", !m_Editor->IsSyncPeekVisible());
  }

  void Gesture_AxisSwitchIsImmediateWhilePeekIsUp()
  {
    this->Arrange(1, 2);

    m_Editor->OnSyncPeekHovered(true, 3);
    Pump();
    CPPUNIT_ASSERT_EQUAL(3, m_Editor->GetSyncPeekAxis());

    m_Editor->OnSyncPeekHovered(true, 6);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comparing axes must not wait out a second dwell",
                                 6, m_Editor->GetSyncPeekAxis());
    CPPUNIT_ASSERT_EQUAL(6, this->Overlay(0)->SyncPeekAxis());
  }

  void Gesture_NewAxisWithinTheGraceKeepsThePeek()
  {
    this->Arrange(1, 2);

    m_Editor->OnSyncPeekHovered(true, 3);
    Pump();

    // Clipping a corner off the strip and coming straight back must not tear the
    // peek down; the grace window covers it.
    m_Editor->OnSyncPeekHovered(false, -1);
    m_Editor->OnSyncPeekHovered(true, 4);
    Pump();
    CPPUNIT_ASSERT(m_Editor->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL(4, m_Editor->GetSyncPeekAxis());
  }

  void Gesture_GapBetweenGlyphsKeepsTheEmphasis()
  {
    this->Arrange(1, 2);

    m_Editor->OnSyncPeekHovered(true, 3);
    Pump();
    CPPUNIT_ASSERT_EQUAL(3, m_Editor->GetSyncPeekAxis());

    // The pointer is still on the strip, just in the 2 px gap between two glyph
    // boxes. Dropping the emphasis there would flicker axis - none - axis at
    // every boundary of a slide along the row, so it latches instead.
    m_Editor->OnSyncPeekHovered(true, -1);
    Pump();
    CPPUNIT_ASSERT_MESSAGE("The strip is one surface; its gaps do not lower the peek",
                           m_Editor->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A gap does not clear the emphasis",
                                 3, m_Editor->GetSyncPeekAxis());
    CPPUNIT_ASSERT_EQUAL(3, this->Overlay(0)->SyncPeekAxis());

    // Only another glyph moves it.
    m_Editor->OnSyncPeekHovered(true, 6);
    CPPUNIT_ASSERT_EQUAL(6, m_Editor->GetSyncPeekAxis());

    // And only leaving the strip drops it.
    m_Editor->OnSyncPeekHovered(false, -1);
    Pump();
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());
  }

  void Gesture_EnteringOnAGapRaisesWithoutEmphasis()
  {
    this->Arrange(1, 2);

    // Reaching the strip between two glyphs is still an engagement: the plate
    // comes up, with nothing emphasised until a glyph is actually reached.
    m_Editor->OnSyncPeekHovered(true, -1);
    Pump();
    CPPUNIT_ASSERT(m_Editor->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL(-1, m_Editor->GetSyncPeekAxis());
    CPPUNIT_ASSERT_MESSAGE("The cells raise a plate with no axis pumped",
                           this->Overlay(0)->IsSyncPeekVisible());
    CPPUNIT_ASSERT_EQUAL(-1, this->Overlay(0)->SyncPeekAxis());
  }

  void Gesture_LeavingBeforeTheDwellNeverRaises()
  {
    this->Arrange(1, 2);

    m_Editor->OnSyncPeekHovered(true, 3);
    m_Editor->OnSyncPeekHovered(false, -1);
    Pump();
    CPPUNIT_ASSERT_MESSAGE("A pointer that crosses without resting raises nothing",
                           !m_Editor->IsSyncPeekVisible());
  }

  void Gesture_MaximizingLowersThePeek()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, 2);
    CPPUNIT_ASSERT(m_Editor->IsSyncPeekVisible());

    m_Editor->SetMaximizedCell(CellId(0));
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());
    CPPUNIT_ASSERT(!this->Overlay(0)->IsSyncPeekVisible());
  }

  void Gesture_LayoutChangeLowersThePeek()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, 2);

    this->Arrange(2, 1);
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());
  }

  void Gesture_AddingACellLowersThePeek()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, 2);

    // A grid op changes the set of cells the comparison was made of, and it
    // reaches the peek through the same LayoutChanged the grid rebuild emits.
    m_Editor->AddGridColumn();
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());

    m_Editor->SetSyncPeek(true, 2);
    m_Editor->RemoveGridColumn();
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());
  }

  void Gesture_CleanViewLowersThePeek()
  {
    this->Arrange(1, 2);
    m_Editor->SetSyncPeek(true, 2);

    m_Editor->SetCleanView(true);
    CPPUNIT_ASSERT(!m_Editor->IsSyncPeekVisible());
    m_Editor->SetCleanView(false);
  }

  // ---------- The barcode's report ----------

  void Barcode_PassiveStripReportsTheGlyphUnderThePointer()
  {
    QmitkMxNSyncBarcodeWidget strip;
    strip.SetSlots(EightSlots());
    strip.resize(240, 24);
    BarcodeRecorder recorder(strip);

    const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(240, 24, 8);
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
    strip.SetSlots(EightSlots());
    strip.resize(240, 24);
    BarcodeRecorder recorder(strip);

    const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(240, 24, 8);
    const QRect content(1, (24 - layout.box) / 2, 8 * layout.box + 7 * 2, layout.box);

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

  void Barcode_PassiveStripIsSilentInColorBarMode()
  {
    QmitkMxNSyncBarcodeWidget strip;
    strip.SetSlots(EightSlots());
    strip.resize(63, 16);
    BarcodeRecorder recorder(strip);

    const auto layout = QmitkMxNSyncBarcodeWidget::ComputeLayout(63, 16, 8);
    CPPUNIT_ASSERT_MESSAGE("This geometry must collapse to color slots",
                           layout.mode == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::ColorBar);

    MoveTo(strip, QPoint(3, 8));
    MoveTo(strip, QPoint(27, 8));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A featureless color slot gives the user nothing to point at",
                                 0, static_cast<int>(recorder.peekAxes.size()));
  }

  void Barcode_PassiveStripStillOpensTheLayoutEditor()
  {
    QmitkMxNSyncBarcodeWidget strip;
    strip.SetSlots(EightSlots());
    strip.resize(240, 24);
    BarcodeRecorder recorder(strip);

    ClickAt(strip, QPoint(20, 12));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The whole strip is still one button onto the editor",
                                 1, recorder.clicks);
  }

  void Barcode_ClickableStripReportsInBothRenderModes()
  {
    for (const QSize size : { QSize(240, 24), QSize(63, 16) })
    {
      QmitkMxNSyncBarcodeWidget strip;
      strip.SetAxisClickable(true);
      strip.SetSlots(EightSlots());
      strip.resize(size);
      BarcodeRecorder recorder(strip);

      const auto layout =
        QmitkMxNSyncBarcodeWidget::ComputeLayout(size.width(), size.height(), 8);
      const bool glyphs = layout.mode == QmitkMxNSyncBarcodeWidget::BarcodeLayout::Mode::Glyphs;
      const QPoint firstSlot = glyphs ? QPoint(1 + layout.box / 2, size.height() / 2)
                                      : QPoint(3, size.height() / 2);
      MoveTo(strip, firstSlot);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("The editor's group cards report in either rendering",
                                   1, static_cast<int>(recorder.axes.size()));
      CPPUNIT_ASSERT_EQUAL(0, recorder.axes[0]);
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
    m_Editor->SetSyncPeek(true, 5);
    CPPUNIT_ASSERT_MESSAGE("The layout must host a plate for the paint smoke to mean anything",
                           this->Overlay(0)->SyncPeekGlyphBox() > 0);

    auto* overlay = this->Overlay(0);
    overlay->SetPeekProgress(1.0);
    overlay->grab();  // forces a synchronous paintEvent

    // The plate with nothing emphasised drives the empty-caption branch.
    m_Editor->SetSyncPeek(true, -1);
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
