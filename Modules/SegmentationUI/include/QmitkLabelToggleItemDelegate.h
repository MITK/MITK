/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLabelToggleItemDelegate_h
#define QmitkLabelToggleItemDelegate_h


#include <QStyledItemDelegate>
#include <QIcon>

#include <MitkSegmentationUIExports.h>

/**
 * \brief Item delegate for rendering and toggling boolean properties (e.g., visibility, lock) in a QmitkMultiLabelTreeView.
 *
 * Displays an on/off icon based on the current boolean state and toggles the value
 * on mouse click. Used for visibility and lock columns in the multi-label tree view.
 *
 * \sa QmitkMultiLabelTreeView
 * \sa QmitkMultiLabelTreeModel
 * \sa QmitkLabelColorItemDelegate
 */
class MITKSEGMENTATIONUI_EXPORT QmitkLabelToggleItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the toggle item delegate with on/off icons.
   * \param[in] onIcon The icon displayed when the property is true/on.
   * \param[in] offIcon The icon displayed when the property is false/off.
   * \param[in] parent Optional parent QObject.
   */
  explicit QmitkLabelToggleItemDelegate(const QIcon& onIcon, const QIcon& offIcon, QObject* parent = nullptr);

  /**
   * \brief Handles mouse click events to toggle the boolean property.
   * \param[in] event The editor event.
   * \param[in] model The model being edited.
   * \param[in] option The style option for the item.
   * \param[in] index The model index of the item.
   * \return True if the event was handled; false otherwise.
   */
  bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
    const QModelIndex& index) override;

  /**
   * \brief Paints the on or off icon based on the current value.
   * \param[in] painter The painter used for rendering.
   * \param[in] option The style option for the item.
   * \param[in] index The model index of the item.
   */
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;

  /**
   * \brief Returns the preferred size for the toggle icon.
   * \param[in] option The style option for the item.
   * \param[in] index The model index of the item.
   * \return The size hint for the delegate.
   */
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

protected:
  QIcon m_OnIcon;
  QIcon m_OffIcon;
};

#endif
