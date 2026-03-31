/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkModelViewSelectionConnector_h
#define QmitkModelViewSelectionConnector_h

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

 // qt
#include <QAbstractItemView>

/**
 * \brief Bridges the selection between a Qt model-view pair and external MITK node selections.
 *
 * This class accepts a QAbstractItemView (whose model must be a QmitkAbstractDataStorageModel)
 * and translates between the view's QItemSelectionModel and lists of mitk::DataNode::Pointer.
 * It can both propagate selection changes outward (via the CurrentSelectionChanged signal) and
 * accept incoming selections (via the SetCurrentSelection slot).
 *
 * When selectOnlyVisibleNodes is false, the connector also stores non-visible nodes from
 * an incoming selection so they can be merged into outgoing selection signals, preserving
 * the full original selection even when the view can only display a subset.
 *
 * \sa QmitkAbstractDataStorageInspector
 * \sa QmitkAbstractDataStorageModel
 */
class MITKQTWIDGETS_EXPORT QmitkModelViewSelectionConnector : public QObject
{
  Q_OBJECT

public:

  QmitkModelViewSelectionConnector();

  /**
   * \brief Sets the view whose selection model is used for selection synchronization.
   *
   * The view's data model is extracted and must be a QmitkAbstractDataStorageModel.
   * The model must return mitk::DataNode::Pointer objects for the QmitkDataNodeRole.
   *
   * \param[in] view The view to connect to.
   * \pre The view must be valid and its model must be a QmitkAbstractDataStorageModel.
   * \throw mitk::Exception if the view is invalid or its model is not a QmitkAbstractDataStorageModel.
   */
  void SetView(QAbstractItemView* view);

  /**
   * \brief Returns the currently selected data nodes.
   * \return A list of currently selected mitk::DataNode::Pointer objects.
   */
  QList<mitk::DataNode::Pointer> GetSelectedNodes() const;

  /**
   * \brief Returns whether only visible nodes can be selected.
   * \return True if only visible nodes are selectable.
   * \sa SetSelectOnlyVisibleNodes
   */
  bool GetSelectOnlyVisibleNodes() const;

Q_SIGNALS:
  /**
   * \brief Emitted when the view's selection changes.
   *
   * The node list includes non-visible nodes from a previous SetCurrentSelection
   * call if selectOnlyVisibleNodes is false.
   *
   * \param[in] nodes The newly selected data nodes.
   */
  void CurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

public Q_SLOTS:
  /**
   * \brief Controls whether selections are restricted to visible nodes.
   *
   * If true, incoming selections are filtered to only those nodes matching the
   * model's predicate (i.e., visible in the view). If false, non-visible nodes
   * are stored and included in outgoing selection signals.
   *
   * \param[in] selectOnlyVisibleNodes True to restrict to visible nodes.
   * \sa GetSelectOnlyVisibleNodes
   */
  void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes);

  /**
   * \brief Applies the given node list as the view's selection.
   *
   * The nodes are looked up in the model to find matching indexes. If
   * selectOnlyVisibleNodes is false, nodes not present in the model are stored
   * and merged into subsequent outgoing CurrentSelectionChanged signals.
   *
   * \param[in] selectedNodes The data nodes to select.
   * \sa SetSelectOnlyVisibleNodes
   */
  void SetCurrentSelection(QList<mitk::DataNode::Pointer> selectedNodes);

private Q_SLOTS:
  /**
  * \brief Transform a model selection into a data node list and emit the 'CurrentSelectionChanged'-signal.
  *
  *   The function adds the selected nodes from the original selection that could not be modified, if
  *   'm_SelectOnlyVisibleNodes' is false.
  *   This slot is internally connected to the 'selectionChanged'-signal of the selection model of the private member item view.
  *
  * \param selected	The newly selected items.
  * \param deselected	The newly deselected items.
  */
  void ChangeModelSelection(const QItemSelection& selected, const QItemSelection& deselected);

private:

  QmitkAbstractDataStorageModel* m_Model;
  QAbstractItemView* m_View;

  bool m_SelectOnlyVisibleNodes;
  QList<mitk::DataNode::Pointer> m_NonVisibleSelection;

  /**
  * \brief 	Retrieve the currently selected nodes from the selection model of the private member item view by
  *         transforming the selection indexes into a data node list.
  *
  *   In order to transform the indices into data nodes, the private data storage model must return
  *   'mitk::DataNode::Pointer' objects for model indexes if the role is QmitkDataNodeRole.
  */
  QList<mitk::DataNode::Pointer> GetInternalSelectedNodes() const;
  /**
  * \brief Filter the list of given nodes such that only those nodes are used that are valid
  *        when using the data storage model's node predicate.
  *        If no node predicate was set or the data storage model is invalid, the input list
  *        of given nodes is returned.
  */
  QList<mitk::DataNode::Pointer> FilterNodeList(const QList<mitk::DataNode::Pointer>& nodes) const;
};

/**
 * \brief Compares two node selections for equality regardless of order.
 *
 * \param[in] selection1 The first selection list.
 * \param[in] selection2 The second selection list.
 * \return True if both selections contain the same nodes (in any order), false otherwise.
 */
bool MITKQTWIDGETS_EXPORT EqualNodeSelections(const QList<mitk::DataNode::Pointer>& selection1, const QList<mitk::DataNode::Pointer>& selection2);

#endif
