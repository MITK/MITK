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

#include "org_mitk_gui_qt_datastorageviewertest_Export.h"

#include "ui_QmitkSingleNodeSelectionWidget.h"

#include <QWidget>

class QmitkAbstractDataStorageModel;
class QAbstractItemVew;

/**
* \class QmitkSingleNodeSelectionWidget
* \brief Widget that allows to show and edit the content of an mitk::IsoDoseLevel instance.
*/
class DATASTORAGEVIEWERTEST_EXPORT QmitkSingleNodeSelectionWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkSingleNodeSelectionWidget(QWidget* parent = nullptr);

  /*
  * @brief Sets the data storage that will be used /monitored by widget.
  *
  * @par dataStorage      A pointer to the data storage to set.
  */
  void SetDataStorage(mitk::DataStorage* dataStorage);
 
  /*
  * @brief Sets the node predicate and updates the widget, according to the node predicate.
  *
  * @par nodePredicate    A pointer to node predicate.
  */
  void SetNodePredicate(mitk::NodePredicateBase* nodePredicate);

  mitk::NodePredicateBase* GetNodePredicate() const;
  
  mitk::DataNode::Pointer GetSelectedNode() const;

  QString GetInvalidInfo() const;

  bool GetSelectionIsOptional() const;

  bool GetSelectOnlyVisibleNodes() const;

  using NodeList = QList<mitk::DataNode::Pointer>;

Q_SIGNALS:
  /*
  * @brief A signal that will be emitted if the selected node has changed.
  *
  * @par	nodes		A list of data nodes that are newly selected.
  */
  void CurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  bool SelectionLock(bool lock);

  bool SelectionUnlock(bool unlock);

  void InvalidInfo(QString info);

  /** Set the widget into an optional mode. Optional means that the selection of no valid
  node does not mean an invalid state. Thus no node is a valid "node" selection too.*/
  void SelectionIsOptional(bool isOptional);

  public Q_SLOTS:
  /*
  * @brief Change the selection modus of the item view's selection model.
  *
  *   If true, an incoming selection will be filtered (reduced) to only those nodes that are visible by the current view.
  *   An outgoing selection can then at most contain the filtered nodes.
  *   If false, the incoming non-visible selection will be stored and later added to the outgoing selection,
  *   to include the original selection that could not be modified.
  *   The part of the original selection, that is non-visible are the nodes that are not
  *
  * @par selectOnlyVisibleNodes   The bool value to define the selection modus.
  */
  void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes);

  /*
  * @brief Transform a list of data nodes into a model selection and set this as a new selection of the
  *        selection model of the private member item view.
  *
  *   The function filters the given list of nodes according to the 'm_SelectOnlyVisibleNodes' member variable. If
  *   necessary, the non-visible nodes are stored. This is done if 'm_SelectOnlyVisibleNodes' is false: In this case
  *   the selection may be filtered and only a subset of the selected nodes may be visible and therefore (de-)selectable
  *   in the data storage viewer. By storing the non-visible nodes it is possible to send the new, modified selection
  *   but also include the selected nodes from the original selection that could not be modified (see 'SetSelectOnlyVisibleNodes').
  *
  * @par	nodes		A list of data nodes that should be newly selected.
  */
  void SetCurrentSelection(NodeList selectedNodes);

  /** Slot can be used to lock the selection. If the selection is locked
  SetCurrentSelection will be ignored and the selection stays untouched.*/
  void SetSelectionLock(bool locked);
  /** Slot can be used to unlock the selection. If the selection is locked
  SetCurrentSelection will be ignored and the selection stays untouched.*/
  void SetSelectionUnlock(bool unlocked);

  /** Set the info text that should be displayed if no valid node is selected,
   * but a selection is mandatory.
   * The string can contain HTML code. if wanted*/
  void SetInvalidInfo(QString info);

  /** Set the widget into an optional mode. Optional means that the selection of no valid
   node does not mean an invalid state. Thus no node is a valid "node" selection too.*/
  void SetSelectionIsOptional(bool isOptional);

protected Q_SLOTS:
  void OnLock(bool locked);

protected:

  bool eventFilter(QObject *obj, QEvent *ev) override;
  void EditSelection();
  void UpdateInfo();

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::Pointer m_NodePredicate;

  mitk::DataNode::Pointer m_SelectedNode;

  QString m_InvalidInfo;
  bool m_IsOptional;
  bool m_SelectOnlyVisibleNodes;

  Ui_QmitkSingleNodeSelectionWidget m_Controls;
};
#endif // QmitkSingleNodeSelectionWidget_H
