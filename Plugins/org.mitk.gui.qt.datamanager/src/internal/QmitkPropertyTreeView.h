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

#ifndef QMITKPROPERTYTREEVIEW_H
#define QMITKPROPERTYTREEVIEW_H

#include <org_mitk_gui_qt_datamanager_Export.h>
#include <QmitkAbstractView.h>

class QmitkLineEdit;
class QmitkPropertyDelegate;
class QmitkPropertyTreeModel;
class QmitkPropertyTreeFilterProxyModel;
class QTreeView;

class MITK_QT_DATAMANAGER QmitkPropertyTreeView : public QmitkAbstractView
{
  Q_OBJECT

public:
  berryObjectMacro(QmitkPropertyTreeView)

  static const std::string VIEW_ID;

  QmitkPropertyTreeView();
  ~QmitkPropertyTreeView();

  void CreateQtPartControl(QWidget *parent);
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes);

protected:
  void SetFocus();

private slots:
  void OnFilterChanged(const QString &filter);

private:
  QmitkPropertyTreeView(const QmitkPropertyTreeView &);
  QmitkPropertyTreeView & operator=(const QmitkPropertyTreeView &);

  QmitkLineEdit *m_Filter;
  QmitkPropertyTreeModel *m_Model;
  QmitkPropertyTreeFilterProxyModel *m_ProxyModel;
  QmitkPropertyDelegate *m_Delegate;
  QTreeView *m_TreeView;
};

#endif
