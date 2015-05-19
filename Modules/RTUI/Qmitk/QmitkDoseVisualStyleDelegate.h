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

  QmitkDoseVisualStyleDelegate(QObject *parent = 0);

  void paint(QPainter *painter, const QStyleOptionViewItem &option
    , const QModelIndex &index) const;

  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
    const QModelIndex &index);

};

#endif
