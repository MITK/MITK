/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  explicit QmitkDoseValueDelegate(QObject *parent = nullptr);

  ///
  /// Renders a specific property  (overwritten from QItemDelegate)
  ///
  void paint(QPainter *painter, const QStyleOptionViewItem &option
    , const QModelIndex &index) const override;

  ///
  /// Create an editor for a specific property  (overwritten from QItemDelegate)
  ///
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option
    , const QModelIndex &index) const override;

  ///
  /// Create an editor for a specific property  (overwritten from QItemDelegate)
  ///
  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  ///
  /// When the user accepts input this func commits the data to the model  (overwritten from QItemDelegate)
  ///
  void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex &index) const override;
};

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
