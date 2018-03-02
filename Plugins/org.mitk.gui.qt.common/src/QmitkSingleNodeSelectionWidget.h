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

#include <QmitkModelViewSelectionConnector.h>

#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkNodePredicateBase.h>

#include "org_mitk_gui_qt_common_Export.h"

#include "ui_QmitkSingleNodeSelectionWidget.h"

#include <QmitkAbstractNodeSelectionWidget.h>

class QmitkAbstractDataStorageModel;
class QAbstractItemVew;

/**
* \class QmitkSingleNodeSelectionWidget
* \brief Widget that allows to show and edit the content of an mitk::IsoDoseLevel instance.
*/
class MITK_QT_COMMON QmitkSingleNodeSelectionWidget : public QmitkAbstractNodeSelectionWidget
{
  Q_OBJECT

public:
  explicit QmitkSingleNodeSelectionWidget(QWidget* parent = nullptr);

  mitk::DataNode::Pointer GetSelectedNode() const;

  using NodeList = QmitkAbstractNodeSelectionWidget::NodeList;

Q_SIGNALS:
  /*
  * @brief A signal that will be emitted if the selected node has changed.
  *
  * @par	nodes		A list of data nodes that are newly selected.
  */
  void CurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

public Q_SLOTS:
  virtual void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes) override;
  virtual void SetCurrentSelection(NodeList selectedNodes) override;

protected:
  mitk::DataNode::Pointer ExtractCurrentValidSelection(const NodeList& nodes) const;
  NodeList CompileEmitSelection() const;

  virtual bool eventFilter(QObject *obj, QEvent *ev) override;
  void EditSelection();
  virtual void UpdateInfo() override;

  virtual void OnNodePredicateChanged(mitk::NodePredicateBase* newPredicate);

  NodeList m_ExternalSelection;
  mitk::DataNode::Pointer m_SelectedNode;

  Ui_QmitkSingleNodeSelectionWidget m_Controls;
};
#endif // QmitkSingleNodeSelectionWidget_H
