/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkInitialValuesTypeDelegate_h
#define QmitkInitialValuesTypeDelegate_h


#include <QStyledItemDelegate>

#include "MitkModelFitUIExports.h"


/** \class QmitkInitialValuesTypeDelegate
\brief An item delegate for rendering and editing the type of a initial value.
It assumes that the type is encoded as int. 0: simple scalar, 1: image.*/
class MITKMODELFITUI_EXPORT QmitkInitialValuesTypeDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  QmitkInitialValuesTypeDelegate(QObject* parent = nullptr);

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option
                        , const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};

#endif
