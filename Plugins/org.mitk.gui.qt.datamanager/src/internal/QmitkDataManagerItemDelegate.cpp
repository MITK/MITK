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
