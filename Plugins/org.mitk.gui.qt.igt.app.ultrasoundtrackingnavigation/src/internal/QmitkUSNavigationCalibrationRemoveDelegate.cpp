/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
