/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDoseVisualStyleDelegate_h
#define QmitkDoseVisualStyleDelegate_h


/// Toolkit includes.
#include <QStyledItemDelegate>

#include "MitkRTUIExports.h"


/** \class QmitkDoseVisualStyleDelegate
\brief An item delegate for rendering and editing dose visualization options.
The delegate is used to handle aspects of a isodose level like visualization
of the isodose lines or colorwash display.*/
class MITKRTUI_EXPORT QmitkDoseVisualStyleDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  explicit QmitkDoseVisualStyleDelegate(QObject *parent = nullptr);

  void paint(QPainter *painter, const QStyleOptionViewItem &option
    , const QModelIndex &index) const override;

  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
    const QModelIndex &index) override;

};

#endif
