/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAnimationItemDelegate_h
#define QmitkAnimationItemDelegate_h

#include <QStyledItemDelegate>

class QmitkAnimationItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkAnimationItemDelegate(QObject* parent = nullptr);
  ~QmitkAnimationItemDelegate() override;

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif
