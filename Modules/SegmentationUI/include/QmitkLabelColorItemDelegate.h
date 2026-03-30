/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLabelColorItemDelegate_h
#define QmitkLabelColorItemDelegate_h


#include <QStyledItemDelegate>

#include <MitkSegmentationUIExports.h>

/**
 * \brief Item delegate for rendering and editing label colors in a QmitkMultiLabelTreeView.
 *
 * Renders the label color as a filled rectangle in the color column and handles
 * mouse click events to open a color picker dialog for changing the label color.
 *
 * \sa QmitkMultiLabelTreeView
 * \sa QmitkMultiLabelTreeModel
 * \sa QmitkLabelToggleItemDelegate
 */
class MITKSEGMENTATIONUI_EXPORT QmitkLabelColorItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the color item delegate.
   * \param[in] parent Optional parent QObject.
   */
  explicit QmitkLabelColorItemDelegate(QObject *parent = nullptr);

  /**
   * \brief Handles mouse events for opening the color picker dialog.
   * \param[in] event The editor event.
   * \param[in] model The model being edited.
   * \param[in] option The style option for the item.
   * \param[in] index The model index of the item.
   * \return True if the event was handled; false otherwise.
   */
  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
    const QModelIndex &index) override;

  /**
   * \brief Paints the label color as a filled rectangle.
   * \param[in] painter The painter used for rendering.
   * \param[in] option The style option for the item.
   * \param[in] index The model index of the item.
   */
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const override;
};

#endif
