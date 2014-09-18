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

#ifndef QmitkNoEditItemDelegate_h
#define QmitkNoEditItemDelegate_h

#include <QStyledItemDelegate>

class QmitkNoEditItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkNoEditItemDelegate(QObject* parent = NULL);
  ~QmitkNoEditItemDelegate();

  QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif
