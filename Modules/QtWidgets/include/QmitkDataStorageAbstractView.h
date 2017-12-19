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

#ifndef QMITKDATASTORAGEABSTRACTVIEW_H
#define QMITKDATASTORAGEABSTRACTVIEW_H

#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

// qt widgets module
#include <QmitkIDataStorageViewModel.h>

// qt
#include <QAbstractItemView>
#include <QWidget>

/*
* @brief This abstract widget provides base functionality for each concrete data storage viewer widget.
*
*   It sets the viewer widget as a node event listener of the data storage. It accepts a node predicate
*   and forwards this filter to the 'QmitkIDataStorageViewModel', the actual model of the data storage viewer.
*
*   The 'QmitkDataStorageAbstractView' offers a public slot and signal that can be used to set / propagate the selected
*   nodes in the current view:
*   The 'SetCurrentSelection'-slot finds the model indices of the given selected nodes and changes the selection of the
*   internal view's selection model accordingly.
*   The 'CurrentSelectionChanged'-signal sends a list of selected nodes to it's environment.
*   The 'CurrentSelectionChanged'-signal is emitted by the 'ModelSelectionChanged'-function, which transforms a model
*   selection into a data node list. The 'ModelSelectionChanged'-function is called when the selection of view's
*   selection model changes.
*
*   The 'QmitkDataStorageAbstractView' provides three empty functions, 'NodeAdded', 'NodeChanged' and 'NodeRemoved', that
*   may be implemented by the subclasses. These functions allow to react to the 'AddNodeEvent', 'ChangedNodeEvent' and
*   'RemoveNodeEvent' of the data storage. This might be useful to force an update on the custom view to correctly
*   represent the content of the data storage.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageAbstractView : public QWidget
{
  Q_OBJECT

public:

  virtual ~QmitkDataStorageAbstractView() = 0;
  /*
  * @brief Sets the data storage and adds listener for node events.
  *
  * @par dataStorage      A pointer to the data storage to set.
  */
  void SetDataStorage(mitk::DataStorage* dataStorage);
  /*
  * @brief Sets the node predicate and updates the model data, according to the node predicate.
  *
  * @par nodePredicate    A pointer to node predicate.
  */
  void SetNodePredicate(mitk::NodePredicateBase* nodePredicate);
  /*
  * @brief Set the widget to change the current selection according to the filter.
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

Q_SIGNALS:
  /*
  * @brief A base signal that can be emitted in any subclass of this widget
  *
  * @par	nodes		A list of data nodes that are newly selected.
  */
  void CurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  public Q_SLOTS:
  /*
  * @brief Transform a data node list into a model selection and set this as a new selection of the selection model of
  *        the private member view.
  *
  *   The function filters the given list of nodes according to the 'm_SelectOnlyVisibleNodes' member variable. If
  *   necessary, the non-visible nodes are stored. This is done if 'm_SelectOnlyVisibleNodes' is false: In this case
  *   the selection may be filtered and only a subset of the selected nodes may be visible and therefore (de-)selectable
  *   in the data storage viewer. By storing the non-visible nodes it is possible to send the new, modified selection
  *   but also include the selected nodes from the original selection that could not be modified (see 'SetSelectOnlyVisibleNodes').
  *
  * @par	nodes		A list of data nodes that should be newly selected.
  */
  void SetCurrentSelection(QList<mitk::DataNode::Pointer> selectedNodes);

  private Q_SLOTS:
  /*
  * @brief Transform a model selection into a data node list and emit the
  *        "CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)"-signal.
  *
  *   The function adds the selected nodes from the original selection that could not be modified, if
  *   'm_SelectOnlyVisibleNodes' is false.
  *   This slot is internally connected to the 'selectionChanged'-signal of the selection model of the private member view.
  *
  * @par	selected	The newly selected items.
  * @par	deselected	The newly deselected items.
  */
  void ModelSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
  /*
  * @brief Set the model that should be used for the view in this widget.
  *
  *		 This model must return mitk::DataNode::Pointer objects for model indexes
  *		 when the role is QmitkDataNodeRole.
  *		 This model must return mitk::DataNode* objects for model indexes
  *		 when the role is QmitkDataNodeRawPointerRole.
  *
  * @par	model	The model to set.
  */
  void SetModel(QmitkIDataStorageViewModel* model);
  /*
  * @brief Set the view that is included in this widget.
  *
  * @par	view	The view to set.
  */
  void SetView(QAbstractItemView* view);

  QmitkDataStorageAbstractView(QWidget* parent = nullptr);
  QmitkDataStorageAbstractView(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);
  mitk::DataStorage* m_DataStorage;
  mitk::NodePredicateBase* m_NodePredicate;

  bool m_SelectOnlyVisibleNodes;

  QList<mitk::DataNode::Pointer> m_NonVisibleSelection;

  QmitkIDataStorageViewModel* m_Model;
  QAbstractItemView* m_View;

private:
  /*
  * @brief Resets the model member after the data storage has been set /changed.
  */
  void DataStorageChanged();
  /*
  * @brief Update the model data after the node predicate has been set / changed.
  */
  void NodePredicateChanged();
  /*
  * @brief Callback for the data storage node added event. May be reimplemented
  *        by subclasses.
  *
  * @par		node	The data node that was added.
  */
  virtual void NodeAdded(const mitk::DataNode* node);
  /*
  * @brief Callback for the data storage node changed event. May be reimplemented
  *        by subclasses.
  *
  * @par		node	The data node that was changed.
  */
  virtual void NodeChanged(const mitk::DataNode* node);
  /*
  * @brief Callback for the data storage node removed event. May be reimplemented
  *        by subclasses.
  *
  * @par		node	The data node that was removed.
  */
  virtual void NodeRemoved(const mitk::DataNode* node);

  QList<mitk::DataNode::Pointer> GetSelectedNodes() const;

  QList<mitk::DataNode::Pointer> FilterNodeList(const QList<mitk::DataNode::Pointer>& nodes) const;

};

#endif // QMITKDATASTORAGEABSTRACTVIEW_H
