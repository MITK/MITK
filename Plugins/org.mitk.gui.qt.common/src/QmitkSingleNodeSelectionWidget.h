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
* \brief Widget that represents a node selection. It acts like a button. Clicking on it
* allows to change the selection.
*/
class MITK_QT_COMMON QmitkSingleNodeSelectionWidget : public QmitkAbstractNodeSelectionWidget
{
  Q_OBJECT

public:
  explicit QmitkSingleNodeSelectionWidget(QWidget* parent = nullptr);
  ~QmitkSingleNodeSelectionWidget();

  mitk::DataNode::Pointer GetSelectedNode() const;

  using NodeList = QmitkAbstractNodeSelectionWidget::NodeList;

public Q_SLOTS:
  virtual void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes) override;
  virtual void SetCurrentSelection(NodeList selectedNodes) override;

protected Q_SLOTS:
  virtual void OnClearSelection();

protected:
  mitk::DataNode::Pointer ExtractCurrentValidSelection(const NodeList& nodes) const;
  NodeList CompileEmitSelection() const;

  bool eventFilter(QObject *obj, QEvent *ev) override;
  void EditSelection();
  void UpdateInfo() override;

  void OnNodePredicateChanged(mitk::NodePredicateBase* newPredicate) override;
  void OnDataStorageChanged() override;
  void NodeRemovedFromStorage(const mitk::DataNode* node) override;

  NodeList m_ExternalSelection;
  mitk::DataNode::Pointer m_SelectedNode;

  Ui_QmitkSingleNodeSelectionWidget m_Controls;
};

#endif // QmitkSingleNodeSelectionWidget_H
