/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
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
