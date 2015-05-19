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

#ifndef QmitkDoseColorDelegate_h
#define QmitkDoseColorDelegate_h


#include <QStyledItemDelegate>

#include "MitkRTUIExports.h"

/** \class QmitkDoseColorDelegate
\brief An item delegate for rendering and editing dose color in a QTableView.*/
class MITKRTUI_EXPORT QmitkDoseColorDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  ///
  /// Creates a new PropertyDelegate.
  ///
  QmitkDoseColorDelegate(QObject *parent = 0);

  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
    const QModelIndex &index);

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const;
};

#endif
