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

#ifndef QmitkAnimationItemDelegate_h
#define QmitkAnimationItemDelegate_h

#include <QStyledItemDelegate>

class QmitkAnimationItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkAnimationItemDelegate(QObject* parent = NULL);
  ~QmitkAnimationItemDelegate();

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif
