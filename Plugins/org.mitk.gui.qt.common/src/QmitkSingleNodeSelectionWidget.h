/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITK_SINGLE_NODE_SELECTION_WIDGET_H
#define QMITK_SINGLE_NODE_SELECTION_WIDGET_H

#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkNodePredicateBase.h>

#include "org_mitk_gui_qt_common_Export.h"

#include "ui_QmitkSingleNodeSelectionWidget.h"

#include <QmitkAbstractNodeSelectionWidget.h>
#include <QmitkNodeSelectionButton.h>

class QmitkAbstractDataStorageModel;

/**
* \class QmitkSingleNodeSelectionWidget
* \brief Widget that represents a node selection of (max) one node. It acts like a button. Clicking on it
* allows to change the selection.
*/
class MITK_QT_COMMON QmitkSingleNodeSelectionWidget : public QmitkAbstractNodeSelectionWidget
{
  Q_OBJECT

public:
  explicit QmitkSingleNodeSelectionWidget(QWidget* parent = nullptr);
  ~QmitkSingleNodeSelectionWidget() override;

  mitk::DataNode::Pointer GetSelectedNode() const;
  bool GetAutoSelectNewNodes() const;

  using NodeList = QmitkAbstractNodeSelectionWidget::NodeList;

public Q_SLOTS:
  void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes) override;
  void SetCurrentSelection(NodeList selectedNodes) override;
  void SetCurrentSelectedNode(mitk::DataNode* selectedNode);

  /** Sets the auto selection mode (Default is false).
  If auto select is true and the following conditions are fullfilled, the widget will
  select a node automatically from the data storage:
  - a data storage is set
  - data storage contains at least one node that matches the given predicate
  - no selection is set.*/
  void SetAutoSelectNewNodes(bool autoSelect);

protected Q_SLOTS:
  virtual void OnClearSelection();

protected:
  mitk::DataNode::Pointer ExtractCurrentValidSelection(const NodeList& nodes) const;
  NodeList CompileEmitSelection() const;

  bool eventFilter(QObject *obj, QEvent *ev) override;
  void EditSelection();
  void UpdateInfo() override;

  void OnNodePredicateChanged(const mitk::NodePredicateBase* newPredicate) override;
  void OnDataStorageChanged() override;
  void NodeAddedToStorage(const mitk::DataNode* node) override;
  void NodeRemovedFromStorage(const mitk::DataNode* node) override;

  void DoAutoSelectIfNeeded(const mitk::DataNode* ignoreNode = nullptr);
  void EmitAndUpdateIfNeeded(const NodeList& lastEmission);

  NodeList m_ExternalSelection;
  mitk::DataNode::Pointer m_SelectedNode;
  /** See documentation of SetAutoSelectNewNodes for details*/
  bool m_AutoSelectNewNodes;

  Ui_QmitkSingleNodeSelectionWidget m_Controls;
};

#endif // QmitkSingleNodeSelectionWidget_H
