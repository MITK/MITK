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

#ifndef QmitkDoseValueDelegate_h
#define QmitkDoseValueDelegate_h


#include <QStyledItemDelegate>

#include "MitkRTUIExports.h"

/** \class QmitkDoseValueDelegate
\brief An item delegate for rendering and editing dose values.
The delegate assumes that the model uses the role Qt::UserRole+1
to indicate if the returned dose value is an absolute (data(Qt::UserRole+1) == true)
or an relative dose (data(Qt::UserRole+1) == false).*/
class MITKRTUI_EXPORT QmitkDoseValueDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  ///
  /// Creates a new PropertyDelegate.
  ///
  QmitkDoseValueDelegate(QObject *parent = 0);

  ///
  /// Renders a specific property  (overwritten from QItemDelegate)
  ///
  void paint(QPainter *painter, const QStyleOptionViewItem &option
    , const QModelIndex &index) const;

  ///
  /// Create an editor for a specific property  (overwritten from QItemDelegate)
  ///
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option
    , const QModelIndex &index) const;

  ///
  /// Create an editor for a specific property  (overwritten from QItemDelegate)
  ///
  void setEditorData(QWidget *editor, const QModelIndex &index) const;

  ///
  /// When the user accepts input this func commits the data to the model  (overwritten from QItemDelegate)
  ///
  void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex &index) const;
};

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
