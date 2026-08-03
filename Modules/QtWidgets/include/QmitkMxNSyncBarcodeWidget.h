/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNSyncBarcodeWidget_h
#define QmitkMxNSyncBarcodeWidget_h

#include <MitkQtWidgetsExports.h>

#include <QmitkMxNAxisGlyph.h>

#include <QColor>
#include <QList>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QWidget>

class QPainter;

/**
 * \brief A compact synchronization barcode: one slot per synchronization axis,
 *        rendered as the group's hue when the cell is linked on that axis and
 *        left as a gap when it is not.
 *
 * The widget is a passive presenter: the owner computes one slot per axis
 * (which axis glyph, the group hue, a tooltip) and pushes them via SetSlots.
 * When the strip is wide enough it draws the self-describing axis glyphs
 * (tinted to the hue when linked, grayed when not); when it is too narrow it
 * collapses to plain color slots.
 *
 * The whole strip doubles as a button: it emits Clicked so the owner can open
 * the layout editor, and it hints "not synchronized" when nothing is linked.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNSyncBarcodeWidget : public QWidget
{
  Q_OBJECT

public:

  /**
   * \brief One barcode slot. The owner supplies the axis glyph and its state;
   *        the widget stays ignorant of what the axes mean.
   */
  struct AxisSlot
  {
    QmitkMxNAxisGlyph glyph = QmitkMxNAxisGlyph::Pan;  // which axis icon to draw
    QColor color;     // invalid == this cell is unsynced on the axis (a gap)
    QString tooltip;  // per-slot hover text

    // Heterogeneous state for the group perspective: the color is the group hue
    // but only some of the group's member windows are linked on this axis. The
    // window perspective (a single cell) never sets this - a cell is linked or
    // not. Rendered as a broken variant of the hue so "some" reads distinctly
    // from "all" (solid) and "none" (gap).
    bool partial = false;
  };

  /**
   * \brief How the strip renders its slots for a given geometry.
   *
   * Glyphs: self-describing axis glyphs in a grid (a single row when the strip
   * is wide and short, wrapping to more rows when it is squarer, e.g. a cell-map
   * tile). ColorBar: the compact positional color slots, used when even a
   * wrapped grid cannot show legible glyphs.
   */
  struct BarcodeLayout
  {
    enum class Mode { Glyphs, ColorBar };
    Mode mode = Mode::ColorBar;
    int columns = 0;  // glyph grid columns (Glyphs mode)
    int rows = 0;     // glyph grid rows (Glyphs mode)
    int box = 0;      // glyph box side in px (Glyphs mode)
  };

  /**
   * \brief Decide the render mode purely from the granted geometry and slot
   *        count: prefer the largest legible glyph box across all row/column
   *        wrappings, else collapse to color slots. Static and side-effect-free
   *        so the wrap/collapse decision is unit-testable without a realized
   *        widget.
   */
  static BarcodeLayout ComputeLayout(int width, int height, int slotCount);

  /**
   * \brief Paint the given slots into an arbitrary rect: the wrapping glyph grid
   *        or the collapsed color bar, chosen by ComputeLayout for that rect's
   *        size. Static so a surface that custom-paints its own tiles (the
   *        layout editor's cell map) renders the identical barcode without
   *        embedding a child widget per tile. 'gapColor' fills the unsynced
   *        hairline; 'hovered' brightens the glyph frames to white.
   */
  static void PaintInto(QPainter& painter, const QRect& target,
                        const QList<AxisSlot>& axisSlots, bool hovered, const QColor& gapColor,
                        int hoveredSlot = -1);

  explicit QmitkMxNSyncBarcodeWidget(QWidget* parent = nullptr);
  ~QmitkMxNSyncBarcodeWidget() override;

  /** \brief Set one slot per axis. */
  void SetSlots(const QList<AxisSlot>& axisSlots);

  /** \brief The current slots. */
  QList<AxisSlot> Slots() const;

  /**
   * \brief Whether a click targets a single axis (AxisClicked) or the whole
   *        strip (Clicked). Off by default: the per-cell strip is one button
   *        onto the layout editor. The layout editor's group header turns it on
   *        so each axis glyph toggles that axis for the group.
   */
  void SetAxisClickable(bool clickable);

  /**
   * \brief True when no slot is synchronized (the list is empty or every color
   *        is invalid); the widget then paints the "not synchronized" hint
   *        instead of a row of gaps.
   */
  bool IsEmptyState() const;

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

Q_SIGNALS:

  /** \brief The whole strip acts as a button onto the layout editor (emitted
   *         only when axis-clickable mode is off). */
  void Clicked();

  /** \brief A single axis glyph was clicked (index into the slot list). Emitted
   *         only in axis-clickable mode. */
  void AxisClicked(int index);

protected:

  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;

  /** \brief Tracks hover over the glyph area (lights the strip to advertise it
   *         is clickable) and the pointing cursor; the strip may be wider than
   *         its glyphs, so the empty part stays inert. */
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;

  /** \brief Serves per-slot tooltips. */
  bool event(QEvent* event) override;

private:

  /** \brief The bounding rect the drawn slots occupy within 'target', centered
   *         vertically and left-aligned; the surrounding padding stays inert. */
  static QRect ContentRectIn(const QRect& target, const BarcodeLayout& layout, int slotCount);

  /** \brief The box rect of one glyph-grid slot within a content rect. */
  static QRect GlyphBoxRectIn(const QRect& contentRect, const BarcodeLayout& layout, int index);

  /** \brief This widget's content rect (hit-testing hover/click/tooltips). */
  QRect ContentRect(const BarcodeLayout& layout) const;

  /** \brief The slot index under a point in this widget, or -1. */
  int SlotAt(const BarcodeLayout& layout, const QPoint& pos) const;

  QList<AxisSlot> m_Slots;
  bool m_Hovered = false;         // whole-strip hover (passive per-cell strip)
  int m_HoveredSlot = -1;         // single hovered axis (axis-clickable mode)
  bool m_AxisClickable = false;
  QPoint m_PressPos;
};

#endif
