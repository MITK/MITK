/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITK_ABSTRACT_NODE_SELECTION_WIDGET_H
#define QMITK_ABSTRACT_NODE_SELECTION_WIDGET_H

#include <MitkQtWidgetsExports.h>

#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkNodePredicateBase.h>

#include <QWidget>

class QmitkAbstractDataStorageModel;
class QAbstractItemVew;

/**
* \class QmitkAbstractNodeSelectionWidget
* \brief Abstract base class for the selection of data from a data storage.
*/
class MITKQTWIDGETS_EXPORT QmitkAbstractNodeSelectionWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkAbstractNodeSelectionWidget(QWidget* parent = nullptr);
  virtual ~QmitkAbstractNodeSelectionWidget() override;

  /**
  * @brief Sets the data storage that will be used / monitored by widget.
  *
  * @par dataStorage      A pointer to the data storage to set.
  */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
  * Sets the node predicate and updates the widget, according to the node predicate.
  * Implement OnNodePredicateChange() for custom actualization of a derived widget class.
  *
  * @par nodePredicate    A pointer to node predicate.
  */
  void SetNodePredicate(const mitk::NodePredicateBase* nodePredicate);

  const mitk::NodePredicateBase* GetNodePredicate() const;

  QString GetInvalidInfo() const;
  QString GetEmptyInfo() const;
  QString GetPopUpTitel() const;
  QString GetPopUpHint() const;

  bool GetSelectionIsOptional() const;

  bool GetSelectOnlyVisibleNodes() const;

  using NodeList = QList<mitk::DataNode::Pointer>;
  /** Other node container type often used in the code base.*/
  using ConstNodeStdVector = std::vector<mitk::DataNode::ConstPointer>;

  /** Returns the selected nodes, as emitted with CurrentSelectionChanged*/
  NodeList GetSelectedNodes() const;
  /** Convinience method that returns the selected nodes as ConstNodeStdVector.
   This is a type also often used in the  mitk code base.*/
  ConstNodeStdVector GetSelectedNodesStdVector() const;

Q_SIGNALS:
  /**
  * @brief A signal that will be emitted if the selected node has changed.
  *
  * @par	nodes		A list of data nodes that are newly selected.
  */
  void CurrentSelectionChanged(NodeList nodes);

public Q_SLOTS:
  /**
  * @brief Change the selection modus of the item view's selection model.
  *
  *   If true, an incoming selection will be filtered (reduced) to only those nodes that are visible by the current view.
  *   An outgoing selection can then at most contain the filtered nodes.
  *   If false, the incoming non-visible selection will be stored and later added to the outgoing selection,
  *   to include the original selection that could not be modified.
  *   The part of the original selection, that is non-visible are the nodes, that do not fullfill the predicate.
  *
  * @par selectOnlyVisibleNodes   The bool value to define the selection modus.
  */
  void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes);

  /**
  * @brief Transform a list of data nodes (a selection) into a model selection and set this as a new selection of the
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

  /** Set the info text that should be displayed if no (valid) node is selected,
  *   but a selection is mandatory.
  *   The string can contain HTML code, if desired.
  */
  void SetInvalidInfo(QString info);

  /** Set the info text that should be displayed if no (valid) node is selected,
  *   but a selection is optional.
  *   The string can contain HTML code, if desired.
  */
  void SetEmptyInfo(QString info);

  /** Set the caption of the popup that is displayed to alter the selection.
  *   The string can contain HTML code, if desired.
  */
  void SetPopUpTitel(QString info);

  /** Set the hint text of the popup that is displayed to alter the selection.
  *   The string can contain HTML code, if desired.
  */
  void SetPopUpHint(QString info);

  /** Set the widget into an optional mode. Optional means that the selection of no valid
  *   node does not mean an invalid state. Thus no node is a valid "node" selection too.
  */
  void SetSelectionIsOptional(bool isOptional);

protected Q_SLOTS:
  /** Call to remove a node from the current selection. If the node is part of the current selection,
  *   this will trigger ReviseSelectionChanged(), AllowEmissionOfSelection() and if there is really a change,
  *   will also emit CurrentSelectionChanged.
  */
  void RemoveNodeFromSelection(const mitk::DataNode* node);

protected:
  /** Method is called if the display of the selected nodes should be updated (e.g. because the selection changed). */
  virtual void UpdateInfo() = 0;

  /** Method is called if the predicate has changed, before the selection will be updated according to the new predicate.
  *   The default implementation does nothing.
  *   @remark If you are only interested to know when the selection has changed, overwrite OnInternalSelectionChange().
  */
  virtual void OnNodePredicateChanged();

  /** Method is called if the data storage has changed. The selection will be automatically be reseted afterwards.
  *   The default implementation does nothing.
  */
  virtual void OnDataStorageChanged();

  /** This member function will called when ever a new internal selection has been determined. This can be
  *   used to update the state of internal widgets. The default implementation does nothing.
  */
  virtual void OnInternalSelectionChanged();

  /** Method is called when a node is added to the storage. Default implementation does nothing.
  *   Derived widgets can override the method if they want to react on new nodes in the storage.
  */
  virtual void OnNodeAddedToStorage(const mitk::DataNode* node);

  /** Method is called when a node is removed from the storage. The removed node is passed as
  *   variable. This member is called directly before the node will be removed from the current selection if
  *   he was a part. Default implementation does nothing.
  */
  virtual void OnNodeRemovedFromStorage(const mitk::DataNode* node);

  /** Method is called if the internal selection has changed. It will call following methods, that can be overriden to change
  *   behavior in derived classes:
  *   - pre internal selection change: ReviseSelectionChanged()
  *   - post internal selection change: OnInternalSelectionChanged(), UpdateInfo() and AllowEmissionOfSelection() (via EmitSelection()).
  *   If the emission is needed and allowed it will also trigger the emission via EmitSelection().
  */
  void HandleChangeOfInternalSelection(NodeList newInternalSelection);

  /** Compiles the list of node that would be emitted. It always contains the internal selection.
  *   Depending on SelectOnlyVisibleNodes it also adds all external select nodes that weren't visible (failed the predicate).
  */
  NodeList CompileEmitSelection() const;

  /** This member function is called if the internal selection is about to be changed by the base implementation.
  *   This is the slot where derived classes can revise and change the internal selection before widget updates,
  *   signal emissions and other things are triggered. Default implementation does nothing, thus it keeps the
  *   passed internal selection as compiled by the base implementation.
  */
  virtual void ReviseSelectionChanged(const NodeList& oldInternalSelection, NodeList& newInternalSelection);

  /** This function will be called before the CurrentSelectionChanged signal is emitted. The return value indicates
  *   if the signal should be emitted (true = emission; false = no emission). The default implementation always
  *   returns true.
  *   @param emissionCandidates The nodes that will be emitted if the function returns true.
  */
  virtual bool AllowEmissionOfSelection(const NodeList& emissionCandidates) const;

  /** Checks if the new emission differs from the last emission. If this is the case and AllowEmissionOfSelection()
  *   returns true the new selection will be emited.
  */
  void EmitSelection(const NodeList& emissionCandidates);

  void SetCurrentInternalSelection(NodeList selectedNodes);
  const NodeList& GetCurrentInternalSelection() const;
  const NodeList& GetCurrentExternalSelection() const;

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::ConstPointer m_NodePredicate;

  QString m_InvalidInfo;
  QString m_EmptyInfo;
  QString m_PopUpTitel;
  QString m_PopUpHint;

  /** See documentation of SetSelectOnlyVisibleNodes for details*/
  bool m_IsOptional;
  /** See documentation of SetSelectionIsOptional for details*/
  bool m_SelectOnlyVisibleNodes;

private:
  /** Helper triggered on the storage delete event */
  void SetDataStorageDeleted();

  /**Member is called when a node is added to the storage.
  Derived widgets can override the method OnNodeAddedToStorage if they want to react on new nodes in the storage.*/
  void NodeAddedToStorage(const mitk::DataNode* node);

  /**Member is called when a node is removed from the storage. It calls OnNodeRemovedFromStorage() and afterwards
  it removes the removed node form the selection (if it is part of the current selection).
  Derived classes can override OnNodeRemovedFromStorage() to react on the fact that a node might be removed and
  their selection might change, because the removed node is part of there selection.*/
  void NodeRemovedFromStorage(const mitk::DataNode* node);

  void OnNodeModified(const itk::Object * /*caller*/, const itk::EventObject &);

  void AddNodeObserver(mitk::DataNode* node);
  void RemoveNodeObserver(mitk::DataNode* node);

  unsigned long m_DataStorageDeletedTag;

  NodeList m_CurrentInternalSelection;
  NodeList m_CurrentExternalSelection;

  NodeList m_LastEmission;
  bool m_LastEmissionAllowance;

  using NodeObserverTagMapType = std::map<const mitk::DataNode*, unsigned long>;
  NodeObserverTagMapType m_NodeObserverTags;

  /** Help to prevent recursions due to signal loops when emitting selections.*/
  bool m_RecursionGuard;
};

#endif // QMITK_ABSTRACT_NODE_SELECTION_WIDGET_H
