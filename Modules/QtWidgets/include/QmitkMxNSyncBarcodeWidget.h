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
#include <QString>
#include <QWidget>

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
  };

  explicit QmitkMxNSyncBarcodeWidget(QWidget* parent = nullptr);
  ~QmitkMxNSyncBarcodeWidget() override;

  /** \brief Set one slot per axis. */
  void SetSlots(const QList<AxisSlot>& axisSlots);

  /** \brief The current slots. */
  QList<AxisSlot> Slots() const;

  /**
   * \brief True when no slot is synchronized (the list is empty or every color
   *        is invalid); the widget then paints the "not synchronized" hint
   *        instead of a row of gaps.
   */
  bool IsEmptyState() const;

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

Q_SIGNALS:

  /** \brief The whole strip acts as a button onto the layout editor. */
  void Clicked();

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

  /** \brief Whether the current width and slots warrant the icon rendering. */
  bool UseIconMode() const;

  /** \brief The square glyph-box side in icon mode, sized to fill the strip
   *         height so the glyphs are not lost in padding. */
  int IconBox() const;

  /** \brief The width the glyphs actually occupy (icon or color mode); the
   *         strip is only interactive within this, not any trailing space. */
  int ContentWidth() const;

  /** \brief The slot index under an x coordinate, or -1. */
  int SlotAtX(int x) const;

  QList<AxisSlot> m_Slots;
  bool m_Hovered = false;
  QPoint m_PressPos;
};

#endif
