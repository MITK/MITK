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
 * (tinted to the hue when linked, grayed when not), marking an axis that carries
 * an offset by filling its box's upper-right corner; when it is too narrow it
 * collapses to plain color slots, which have no room for that mark and so omit
 * it.
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
    QString label;    // the axis's display name, for a surface that names it in words

    // Heterogeneous state for the group perspective: the color is the group hue
    // but only some of the group's member windows are linked on this axis. The
    // window perspective (a single cell) never sets this - a cell is linked or
    // not. Rendered as a broken variant of the hue so "some" reads distinctly
    // from "all" (solid) and "none" (gap).
    bool partial = false;

    // The axis carries an offset relative to its group's seed. Marked by filling
    // the box's upper-right corner, because an offset is otherwise invisible
    // outside the layout editor's matrix, and a window parked at slice -1 looks
    // exactly like one sitting on the group.
    bool hasOffset = false;

    // That offset in words, where a single window's is meant. Empty on the group
    // perspective, which speaks for several windows at once and so can mark that
    // an offset exists but not say which.
    QString offsetText;
  };

  /**
   * \brief What a host will allow the strip to do with the rect it grants.
   *
   * Wrapping trades vertical room for larger glyphs. A strip in a chrome row
   * must stay one line whatever happens, so it forbids it; a host with room to
   * spare would rather break the row than shrink the glyphs, so it allows it
   * and caps their size instead - unbounded, a large host rect would render
   * glyphs larger than anything else on screen.
   */
  struct BarcodeFit
  {
    bool allowWrap = true;  // may the row wrap into a grid
    int maxBox = 0;         // glyph box ceiling; 0 = bounded only by the rect
  };

  /**
   * \brief The fit a caller gets when it grants a rect and says nothing about
   *        it: wrapping allowed, glyph size bounded only by that rect.
   *
   * Named rather than spelled '= {}' at each default argument below: a nested
   * class's default member initializers are not available inside the enclosing
   * class, so a braced default argument there is ill-formed and GCC rejects it.
   */
  static const BarcodeFit DefaultFit;

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
  static BarcodeLayout ComputeLayout(int width, int height, int slotCount,
                                     const BarcodeFit& fit = DefaultFit);

  /**
   * \brief Paint the given slots into an arbitrary rect: the wrapping glyph grid
   *        or the collapsed color bar, chosen by ComputeLayout for that rect's
   *        size. Static so a surface that custom-paints several barcodes renders
   *        the identical barcode without embedding a child widget per barcode. 'gapColor' fills the unsynced
   *        hairline; 'hovered' brightens the glyph frames to white.
   */
  static void PaintInto(QPainter& painter, const QRect& target,
                        const QList<AxisSlot>& axisSlots, bool hovered, const QColor& gapColor,
                        int hoveredSlot = -1, const BarcodeFit& fit = DefaultFit);

  /**
   * \brief The slot index under 'pos' when 'slotCount' slots are painted into
   *        'target' by PaintInto, or -1. The hit-test counterpart to PaintInto,
   *        for a surface that custom-paints the barcode into its own rect and
   *        needs to know which axis glyph the pointer is
   *        over. Uses the same ComputeLayout geometry as the render, so hit-test
   *        and paint cannot drift.
   */
  static int SlotAtIn(const QRect& target, int slotCount, const QPoint& pos,
                      const BarcodeFit& fit = DefaultFit);

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
   * \brief Ask for the width the glyph rendering needs instead of the compact
   *        color-bar width.
   *
   *        A strip that shares a row's slack grows into the glyph rendering on
   *        its own; one parked against the row's trailing edge never does and
   *        would be granted the color bar forever. The width asked for follows
   *        the height the host grants, so the boxes come out square and the
   *        strip fills its chrome. The minimum size hint stays the color bar
   *        either way, so a narrow host still collapses to it.
   */
  void SetPreferGlyphWidth(bool prefer);

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

  /** \brief The axis glyph under the pointer changed: its slot index, or -1 when
   *         the pointer leaves the glyphs. Emitted only in axis-clickable mode
   *         (the layout editor's group cards), so a hover can light up every cell
   *         sharing that axis's synchronization. */
  void AxisHovered(int index);

  /**
   * \brief Where the pointer is on a passive strip: 'overStrip' is true while it
   *        is anywhere on the drawn slots - the same area the click reacts to -
   *        and 'axisIndex' names the glyph under it, or -1 between glyphs.
   *
   *        The two are deliberately separate. A receiver that answers the strip
   *        (the editor's sync peek) stays up for as long as the pointer is on it
   *        and only changes which axis it emphasises, so crossing the gap between
   *        two glyphs does not tear the answer down. Being on the strip is
   *        reported in either render mode; a collapsed colour-bar slot simply
   *        names no axis, since a featureless 7 px column shows nothing to point
   *        at, and the receiver answers with none emphasised.
   */
  void PeekHovered(bool overStrip, int axisIndex);

protected:

  void paintEvent(QPaintEvent* event) override;

  /** \brief Re-asks for the width the glyphs need whenever the granted height
   *         changes, since the box is square and follows that height. */
  void resizeEvent(QResizeEvent* event) override;
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
  int SlotAt(const QPoint& pos) const;

  QList<AxisSlot> m_Slots;
  bool m_Hovered = false;         // whole-strip hover (passive per-cell strip)
  int m_HoveredSlot = -1;         // single hovered axis (axis-clickable mode)
  // What a passive strip last reported. Deliberately not m_Hovered /
  // m_HoveredSlot: those drive the paint, and the passive strip's pixels must
  // stay exactly what they were before it began reporting.
  bool m_ReportedOverStrip = false;
  int m_ReportedSlot = -1;
  bool m_AxisClickable = false;
  bool m_PreferGlyphWidth = false;
  QPoint m_PressPos;
};

#endif
