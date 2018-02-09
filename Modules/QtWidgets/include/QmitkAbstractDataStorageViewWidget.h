/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKABSTRACTDATASTORAGEVIEWWIDGET_H
#define QMITKABSTRACTDATASTORAGEVIEWWIDGET_H

#include <QmitkModelViewSelectionConnector.h>

#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

// qt
#include <QWidget>

class QAbstractItemVew;

/*
* @brief This abstract class extends the 'QAbstractItemModel' to accept an 'mitk::DataStorage' and a 'mitk::NodePredicateBase'.,
*   It registers itself as a node event listener of the data storage.
*   The 'QmitkAbstractDataStorageViewWidget' provides three empty functions, 'NodeAdded', 'NodeChanged' and 'NodeRemoved', that
*   may be implemented by the subclasses. These functions allow to react to the 'AddNodeEvent', 'ChangedNodeEvent' and
*   'RemoveNodeEvent' of the data storage. This might be useful to force an update on the custom view to correctly
*   represent the content of the data storage.
*
*   A concrete implementations of this class is used to store the temporarily shown data nodes of the data storage.
*   These nodes may be a subset of all the nodes inside the data storage, if a specific node predicate is set.
*
*   A model that implements this class has to return mitk::DataNode::Pointer objects for model indexes when the
*   role is QmitkDataNodeRole.
*/
class MITKQTWIDGETS_EXPORT QmitkAbstractDataStorageViewWidget : public QWidget
{
  Q_OBJECT

public:
  virtual ~QmitkAbstractDataStorageViewWidget();

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
  virtual void SetNodePredicate(mitk::NodePredicateBase* nodePredicate);

  mitk::NodePredicateBase* GetNodePredicate() const;

  using NodeList = QList<mitk::DataNode::Pointer>;

  NodeList GetSelectedNodes() const;

  virtual QAbstractItemView* GetView() = 0;
  virtual QAbstractItemView* GetView() const = 0;

  bool GetSelectOnlyVisibleNodes() const;

  using SelectionMode = QAbstractItemView::SelectionMode;
  virtual void SetSelectionMode(SelectionMode mode) = 0;
  virtual SelectionMode GetSelectionMode() const = 0;

Q_SIGNALS:
  /*
  * @brief A signal that will be emitted if the selected node has changed.
  *
  * @par	nodes		A list of data nodes that are newly selected.
  */
  void CurrentSelectionChanged(NodeList nodes);

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

protected Q_SLOTS:
  void OnSelectionChanged(NodeList selectedNodes);

protected:
  /** Helper function is called if data storage or predicate is changed to (re) inizialze the widget correctly.
   Implement the function in derived classes.*/
  virtual void Initialize() = 0;

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::Pointer m_NodePredicate;

  std::unique_ptr<QmitkModelViewSelectionConnector> m_Connector;

  QmitkAbstractDataStorageViewWidget(QWidget* parent = nullptr);

};

#endif // QMITKABSTRACTDATASTORAGEMODEL_H
