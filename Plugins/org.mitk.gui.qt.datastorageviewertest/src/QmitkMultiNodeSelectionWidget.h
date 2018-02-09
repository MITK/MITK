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


#ifndef QMITK_MULTI_NODE_SELECTION_WIDGET_H
#define QMITK_MULTI_NODE_SELECTION_WIDGET_H

#include <QmitkModelViewSelectionConnector.h>

#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkNodePredicateBase.h>

#include "org_mitk_gui_qt_datastorageviewertest_Export.h"

#include "ui_QmitkMultiNodeSelectionWidget.h"

#include <QmitkAbstractNodeSelectionWidget.h>

class QmitkAbstractDataStorageModel;
class QAbstractItemVew;

/**
* \class QmitkMultiNodeSelectionWidget
* \brief Widget that allows to show and edit the content of an mitk::IsoDoseLevel instance.
*/
class DATASTORAGEVIEWERTEST_EXPORT QmitkMultiNodeSelectionWidget : public QmitkAbstractNodeSelectionWidget
{
  Q_OBJECT

public:
  explicit QmitkMultiNodeSelectionWidget(QWidget* parent = nullptr);

  using NodeList = QmitkAbstractNodeSelectionWidget::NodeList;

  NodeList GetSelectedNodes() const;

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
  void OnEditSelection();

protected:
  NodeList CompileEmitSelection() const;

  virtual void UpdateInfo() override;
  virtual void UpdateList();

  virtual void OnNodePredicateChanged(mitk::NodePredicateBase* newPredicate);

  NodeList m_CurrentSelection;

  Ui_QmitkMultiNodeSelectionWidget m_Controls;
};
#endif // QmitkMultiNodeSelectionWidget_H
