/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QLineEdit>
#include "QmitkDataManagerItemDelegate.h"

QmitkDataManagerItemDelegate::QmitkDataManagerItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QmitkDataManagerItemDelegate::~QmitkDataManagerItemDelegate()
{
}

void QmitkDataManagerItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QVariant data = index.data();

  if (!data.isValid())
    return;

  if (data.type() == QVariant::String)
  {
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    lineEdit->setText(data.toString());
  }
  else
  {
    QStyledItemDelegate::setEditorData(editor, index);
  }
}
