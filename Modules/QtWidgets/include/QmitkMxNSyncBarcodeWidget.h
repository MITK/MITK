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
 *        rendered as the group's hue when the cell is linked on that axis,
 *        left as a gap when it is not, and struck through when the axis does
 *        not apply to the cell.
 *
 * The widget is a passive presenter: the owner computes one slot per axis
 * (which axis, the group hue, a tooltip) and pushes them via SetSlots.
 * When the strip is wide enough it draws the self-describing axis glyphs
 * (tinted to the hue when linked, grayed when not), marking an axis that carries
 * an offset by filling its box's lower-right corner; when it is too narrow it
 * collapses to plain color slots, which have no room for that mark and so omit
 * it.
 *
 * A slot whose axis does not apply to the window (applicable == false) is
 * struck through in the gap color in both modes, with no offset mark.
 *
 * By default the whole strip doubles as a button: it emits Clicked so the
 * owner can open the layout editor. 'SetClickable' withdraws that button.
 * 'SetAxisInteraction' makes the single axes take the pointer instead.
 *
 * Highlighting never recolours a slot, since a white frame or a brightened
 * hue disappears on light group colours (the reserved group's light gray
 * among them). A strip that is one button as a whole shows, under the
 * pointer, the hover wash of the tool buttons beside it behind its slots; a
 * strip whose axes take the pointer keeps the pointed-at slot as it is and
 * fades the others to QmitkMxNHighlightFade.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNSyncBarcodeWidget : public QWidget
{
  Q_OBJECT

public:

  /**
   * \brief One barcode slot. The owner supplies the axis and its state; the
   *        widget draws the axis's glyph and stays ignorant of what the axes
   *        mean.
   */
  struct AxisSlot
  {
    QmitkMxNSyncAxis axis = QmitkMxNSyncAxis::Pan;  // which axis, and so which glyph
    // The group hue; invalid == this cell is unsynced on the axis (a gap). On an
    // axis that does not apply, a valid hue is a kept link, not drawn until the
    // axis applies.
    QColor color;
    QString tooltip;  // per-slot hover text, shown while the axes take the pointer
    QString label;    // the axis's display name, for a surface that names it in words

    // Heterogeneous state for the group perspective: the color is the group hue
    // but only some of the group's member windows are linked on this axis. The
    // window perspective (a single cell) never sets this - a cell is linked or
    // not. Rendered as a broken variant of the hue so "some" reads distinctly
    // from "all" (solid) and "none" (gap). Meaningful only where the axis applies.
    bool partial = false;

    // The axis carries an offset relative to its group's reference. Marked by
    // filling the box's lower-right corner, because an offset is otherwise
    // invisible outside the layout editor's matrix, and a window parked at
    // slice -1 looks exactly like one sitting on the group. Not marked on a slot
    // whose axis does not apply.
    bool hasOffset = false;

    // That offset in words, where a single window's is meant. Empty on the group
    // perspective, which speaks for several windows at once and so can mark that
    // an offset exists but not say which.
    QString offsetText;

    // The axis has no meaning for the window (e.g. slice in a 3D view). Drawn as
    // a third state, struck through, whether or not a link is kept: a positional
    // strip keeps the slot, so the axes sit at the same place in every window.
    // A kept link's hue stays in 'color' and shows again once the axis applies.
    bool applicable = true;
  };

  /**
   * \brief How the strip renders its slots for a given geometry.
   *
   * Glyphs: self-describing axis glyphs in one row; the strip lives in a chrome
   * row or a group card header, where a second line would push the whole row
   * taller. ColorBar: the compact positional color slots, used when the row
   * cannot show legible glyphs.
   */
  struct BarcodeLayout
  {
    enum class Mode { Glyphs, ColorBar };
    Mode mode = Mode::ColorBar;
    int box = 0;      // glyph box side in px (Glyphs mode)
  };

  /**
   * \brief Decide the render mode purely from the granted geometry and slot
   *        count: the largest legible glyph box one row allows, else the color
   *        slots. Static and side-effect-free so the decision is unit-testable
   *        without a realized widget.
   */
  static BarcodeLayout ComputeLayout(int width, int height, int slotCount);

  /**
   * \brief Paint the given slots into 'target' as the strip does: the glyph row
   *        or the collapsed color bar, chosen by ComputeLayout for that rect's
   *        size. 'gapColor' fills the unsynced hairline; 'hovered' lays the
   *        button hover wash behind the drawn slots; 'hoveredSlot', when not
   *        -1, fades every other slot. Static so the rendering can be checked
   *        on a QImage at any device pixel ratio.
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

  /** \brief What the single axes do with the pointer. */
  enum class AxisInteraction
  {
    None,   // the strip is one button as a whole (Clicked)
    Hover,  // each axis reports the pointer (AxisHovered) but takes no click
    Click   // each axis reports the pointer and is a button (AxisClicked)
  };

  /**
   * \brief Whether the pointer targets the whole strip or the single axes.
   *        None by default: the per-cell strip is one button onto the layout
   *        editor. The layout editor's group cards use Click, so each axis
   *        glyph toggles that axis for the group; the reserved group's card
   *        uses Hover, since its axes are not the user's to toggle. With Hover
   *        or Click the strip emits no Clicked, and a slot that does not apply
   *        (applicable == false) emits no AxisClicked.
   */
  void SetAxisInteraction(AxisInteraction interaction);

  /**
   * \brief Whether a click on the passive strip is offered. When false,
   *        'Clicked' is not emitted, and the click tooltip, the pointing-hand
   *        cursor and the whole-strip hover highlight are not shown. A change
   *        takes effect at once, also while the pointer rests on the strip.
   *        'PeekHovered' keeps firing either way. Default: true.
   */
  void SetClickable(bool clickable);

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

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

Q_SIGNALS:

  /** \brief The whole strip acts as a button onto the layout editor (emitted
   *         only with AxisInteraction::None). */
  void Clicked();

  /** \brief A single axis glyph was clicked (index into the slot list). Emitted
   *         only with AxisInteraction::Click. */
  void AxisClicked(int index);

  /** \brief The axis glyph under the pointer changed: its slot index, or -1 when
   *         the pointer leaves the glyphs. Emitted only with
   *         AxisInteraction::Hover or ::Click (the layout editor's group cards),
   *         so a hover can light up every cell sharing that axis's
   *         synchronization. */
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

  /** \brief Tracks hover over the glyph area (the hover wash and the pointing
   *         cursor advertise a click, where one is offered); the strip may be
   *         wider than its glyphs, so the empty part stays inert. */
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;

  /** \brief Serves per-slot tooltips. */
  bool event(QEvent* event) override;

private:

  /** \brief The bounding rect the drawn slots occupy within 'target', centered
   *         vertically and left-aligned; the surrounding padding stays inert. */
  static QRect ContentRectIn(const QRect& target, const BarcodeLayout& layout, int slotCount);

  /** \brief The box rect of one glyph slot within a content rect. */
  static QRect GlyphBoxRectIn(const QRect& contentRect, const BarcodeLayout& layout, int index);

  /** \brief This widget's content rect (hit-testing hover/click/tooltips). */
  QRect ContentRect(const BarcodeLayout& layout) const;

  /** \brief The slot index under a point in this widget, or -1. */
  int SlotAt(const QPoint& pos) const;

  QList<AxisSlot> m_Slots;
  bool m_Hovered = false;         // whole-strip hover (passive per-cell strip)
  int m_HoveredSlot = -1;         // single hovered axis (axes take the pointer)
  // What a passive strip last reported. Deliberately not m_Hovered /
  // m_HoveredSlot: those drive the paint, and reporting must not change what
  // a passive strip paints.
  bool m_ReportedOverStrip = false;
  int m_ReportedSlot = -1;
  AxisInteraction m_AxisInteraction = AxisInteraction::None;
  bool m_Clickable = true;
  bool m_PreferGlyphWidth = false;
  QPoint m_PressPos;
};

#endif
