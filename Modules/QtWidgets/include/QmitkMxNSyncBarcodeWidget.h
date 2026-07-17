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

#include <QColor>
#include <QList>
#include <QWidget>

/**
 * \brief A compact per-dimension synchronization barcode: one slot per MxN
 *        sync dimension, filled with the group's hue when the cell is linked
 *        on that dimension and left as a gap when it is not.
 *
 * The same idiom as the layout-editor tile barcode, sized for a render
 * window's utility strip. It is a passive presenter: the owner computes the
 * per-dimension colors (an invalid QColor marks an unsynced gap) and pushes
 * them via SetSlots, so the widget stays decoupled from the group model.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNSyncBarcodeWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkMxNSyncBarcodeWidget(QWidget* parent = nullptr);
  ~QmitkMxNSyncBarcodeWidget() override;

  /** \brief Set one color per dimension slot; an invalid color is a gap. */
  void SetSlots(const QList<QColor>& slotColors);

  /** \brief The current slot colors (an invalid color is a gap). */
  QList<QColor> Slots() const;

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

protected:

  void paintEvent(QPaintEvent* event) override;

private:

  QList<QColor> m_Slots;
};

#endif
