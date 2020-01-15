/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkUSZoneManagementColorDialogDelegate.h"

#include <QEvent>
#include <QColorDialog>

QmitkUSZoneManagementColorDialogDelegate::QmitkUSZoneManagementColorDialogDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QmitkUSZoneManagementColorDialogDelegate::~QmitkUSZoneManagementColorDialogDelegate()
{
}

bool QmitkUSZoneManagementColorDialogDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem& option, const QModelIndex &index)
{
  if( event->type() == QEvent::MouseButtonDblClick )
  {
    // open the color dialog with the value currently in the cell
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QColor color = QColorDialog::getColor(QColor(value));
    if (color.isValid()) { model->setData(index, QVariant(color)); }

    return true;
  }
  else
  {
    return QStyledItemDelegate::editorEvent(event, model, option, index);
  }
}
