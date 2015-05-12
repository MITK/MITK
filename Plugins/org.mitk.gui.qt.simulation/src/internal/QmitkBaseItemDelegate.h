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

#ifndef QmitkBaseItemDelegate_h
#define QmitkBaseItemDelegate_h

#include <QStyledItemDelegate>
#include <sofa/core/objectmodel/BaseData.h>

class QmitkBaseItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkBaseItemDelegate(QObject* parent = nullptr);
  ~QmitkBaseItemDelegate();

  QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};

Q_DECLARE_METATYPE(sofa::core::objectmodel::BaseData*)

#endif
