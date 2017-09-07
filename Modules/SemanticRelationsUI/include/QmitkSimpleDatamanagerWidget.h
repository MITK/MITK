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

#ifndef QMITKSIMPLEDATAMANAGERWIDGET_H
#define QMITKSIMPLEDATAMANAGERWIDGET_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"
#include <ui_QmitkSimpleDatamanagerWidgetControls.h>
#include "QmitkSelectionWidget.h"

// mitk core
#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

// mitk qt widgets
#include <QmitkDataStorageTreeModel.h>
#include <QmitkDataStorageFilterProxyModel.h>

/*
* @brief The QmitkSimpleDatamanagerWidget is a widget that shows the currently available data of the data storage in a hierarchy tree view.
*
*   The QmitkSimpleDatamanagerWidget is derived from 'QmitkSelectionWidget', so that the "SelectionChanged"-signal can be emitted.
*   This is done in order to inform other widgets about a changed selection in this widget (e.g. the 'QmitkSelectNodeDialog').
*   The QmitkSimpleDatamanagerWidget is currently injected in the 'QmitkSelectNodeDialog' in the 'QmitkSemanticRelationsView' class.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkSimpleDatamanagerWidget : public QmitkSelectionWidget
{
  Q_OBJECT

public:

  QmitkSimpleDatamanagerWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);
  virtual ~QmitkSimpleDatamanagerWidget();

private Q_SLOTS:

  /*
  * @brief A slot that is connected with the 'selectionChanged'-signal of the underlying model.
  *
  *       This function gets the currently selected data node and emits the 'SelectionChanged(mitk::DataNode*)'-signal of the injected 'QmitkSelectionWidget'.
  */
  void OnNodeTreeViewSelectionChanged(const QItemSelection&, const QItemSelection&);

private:

  void Init();
  void SetUpConnections();

  Ui::QmitkSimpleDatamanagerWidgetControls m_Controls;

  std::unique_ptr<QmitkDataStorageTreeModel> m_NodeTreeModel;
  std::unique_ptr<QmitkDataStorageFilterProxyModel> m_FilterModel;

  mitk::NodePredicateBase::Pointer m_HelperObjectFilterPredicate;
  mitk::NodePredicateBase::Pointer m_NodeWithNoDataFilterPredicate;

  mitk::DataNode* m_SelectedDataNode;

};

#endif // QMITKSIMPLEDATAMANAGERWIDGET_H
