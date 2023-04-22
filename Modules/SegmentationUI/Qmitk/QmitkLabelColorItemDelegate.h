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
\brief An item delegate for rendering and editing label color in a QmitkMultiLabelTreeView.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkLabelColorItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  ///
  /// Creates a new PropertyDelegate.
  ///
  explicit QmitkLabelColorItemDelegate(QObject *parent = nullptr);

  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
    const QModelIndex &index) override;

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const override;
};

#endif
