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
#include "QmitkUSNavigationCalibrationRemoveDelegate.h"

#include <QEvent>

QmitkUSNavigationCalibrationRemoveDelegate::QmitkUSNavigationCalibrationRemoveDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QmitkUSNavigationCalibrationRemoveDelegate::~QmitkUSNavigationCalibrationRemoveDelegate()
{
}

bool QmitkUSNavigationCalibrationRemoveDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem& option, const QModelIndex &index)
{
  if( event->type() == QEvent::MouseButtonPress )
  {
    model->setData(index, QVariant(false));
    return true;
  }
  else
  {
    return QStyledItemDelegate::editorEvent(event, model, option, index);
  }
}
