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

#ifndef QmitkVisibilityDelegate_h
#define QmitkVisibilityDelegate_h

// render window manager
#include "MitkRenderWindowManagerExports.h"

// qt
#include <QStyledItemDelegate>
#include <QSize>

#include <QObject>

/*
* @brief 
*/
class MITKRENDERWINDOWMANAGER_EXPORT QmitkVisibilityDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  QmitkVisibilityDelegate(QObject* parent = 0);

  // Create Editor when we construct
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  //// Then, we set the Editor
  //void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
  //
  //// When we modify data, this model reflect the change
  //void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE;

  // Give the SpinBox the info on size and location
  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  bool editorEvent(QEvent *event, QAbstractItemModel* model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

#endif //QmitkVisabilityDelegate_h
