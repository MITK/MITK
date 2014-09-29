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

#include "QmitkNoEditItemDelegate.h"

QmitkNoEditItemDelegate::QmitkNoEditItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QmitkNoEditItemDelegate::~QmitkNoEditItemDelegate()
{
}

QWidget* QmitkNoEditItemDelegate::createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const
{
  return NULL;
}
