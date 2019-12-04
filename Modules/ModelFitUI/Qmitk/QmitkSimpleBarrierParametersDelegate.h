/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSimpleBarrierParametersDelegate_h
#define QmitkSimpleBarrierParametersDelegate_h


/// Toolkit includes.
#include <QStyledItemDelegate>

#include "MitkModelFitUIExports.h"


/** \class QmitkSimpleBarrierParametersDelegate
\brief An item delegate for rendering and editing the parameters relevant for
a barrier constraint. The delegate assumes the following: 1) if the data is requested with the
edit role, it gets a string list of all possible options. 2) if the data is requested with the
display role it gets only a list of all currently selected options.
If the data is transfered back to the model it contains all selected parameter names in a string list.*/
class MITKMODELFITUI_EXPORT QmitkSimpleBarrierParametersDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  QmitkSimpleBarrierParametersDelegate(QObject* parent = nullptr);

  void paint(QPainter* painter, const QStyleOptionViewItem& option
             , const QModelIndex& index) const override;

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option
                        , const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

};

#endif
