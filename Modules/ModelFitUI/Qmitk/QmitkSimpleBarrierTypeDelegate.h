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

#ifndef QmitkSimpleBarrierTypeDelegate_h
#define QmitkSimpleBarrierTypeDelegate_h


#include <QStyledItemDelegate>

#include "MitkModelFitUIExports.h"


/** \class QmitkSimpleBarrierTypeDelegate
\brief An item delegate for rendering and editing the type of a simple barrier constraint.
It assumes that the type is encoded as int. 0: lower border, 1: upper border.*/
class MITKMODELFITUI_EXPORT QmitkSimpleBarrierTypeDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  QmitkSimpleBarrierTypeDelegate(QObject* parent = nullptr);

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option
                        , const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};

#endif
