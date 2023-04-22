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
\brief An item delegate for rendering and editing properties that can be toggled (e.g. visibility).
*/
class MITKSEGMENTATIONUI_EXPORT QmitkLabelToggleItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  ///
  /// Creates a new PropertyDelegate.
  ///
  explicit QmitkLabelToggleItemDelegate(const QIcon& onIcon, const QIcon& offIcon, QObject* parent = nullptr);

  bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
    const QModelIndex& index) override;

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

protected:
  QIcon m_OnIcon;
  QIcon m_OffIcon;
};

#endif
