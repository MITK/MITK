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


#ifndef QMITK_ABSTRACT_NODE_SELECTION_WIDGET_H
#define QMITK_ABSTRACT_NODE_SELECTION_WIDGET_H

#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkNodePredicateBase.h>

#include "org_mitk_gui_qt_common_Export.h"

#include <QWidget>

class QmitkAbstractDataStorageModel;
class QAbstractItemVew;

/**
* \class QmitkAbstractNodeSelectionWidget
* \brief Abstract base class for the selection of data from a data storage.
*/
class MITK_QT_COMMON QmitkAbstractNodeSelectionWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkAbstractNodeSelectionWidget(QWidget* parent = nullptr);
  virtual ~QmitkAbstractNodeSelectionWidget();

  /**
  * @brief Sets the data storage that will be used /monitored by widget.
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
  void SetNodePredicate(mitk::NodePredicateBase* nodePredicate);

  mitk::NodePredicateBase* GetNodePredicate() const;

  QString GetInvalidInfo() const;
  QString GetEmptyInfo() const;
  QString GetPopUpTitel() const;
  QString GetPopUpHint() const;

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
  virtual void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes);

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
  virtual void SetCurrentSelection(NodeList selectedNodes) = 0;

  /** Set the info text that should be displayed if no (valid) node is selected,
   * but a selection is mandatory.
   * The string can contain HTML code. if wanted*/
  void SetInvalidInfo(QString info);

  /** Set the info text that should be displayed if no (valid) node is selected,
  * but a selection is optional.
  * The string can contain HTML code. if wanted*/
  void SetEmptyInfo(QString info);

  /** Set the caption of the popup that is displayed to alter the selection.
  * The string can contain HTML code. if wanted*/
  void SetPopUpTitel(QString info);

  /** Set the hint text of the popup that is displayed to alter the selection.
  * The string can contain HTML code. if wanted*/
  void SetPopUpHint(QString info);

  /** Set the widget into an optional mode. Optional means that the selection of no valid
   node does not mean an invalid state. Thus no node is a valid "node" selection too.*/
  void SetSelectionIsOptional(bool isOptional);

protected:
  /**Member is called if the display of the selected nodes should be updated.*/
  virtual void UpdateInfo() = 0;

  /**Member is called if the predicate has changed. Thus the selection might change to. The new (changed) predicate
  is passed with the function call. It is the same like this->GetNodePredicate() called in the function call.*/
  virtual void OnNodePredicateChanged(mitk::NodePredicateBase* newPredicate) = 0;

  /**Member is called if the data storage has changed. Thus the selection might change to.*/
  virtual void OnDataStorageChanged() = 0;

  virtual void NodeRemovedFromStorage(const mitk::DataNode* node) = 0;

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::Pointer m_NodePredicate;

  QString m_InvalidInfo;
  QString m_EmptyInfo;
  QString m_PopUpTitel;
  QString m_PopUpHint;

  bool m_IsOptional;
  bool m_SelectOnlyVisibleNodes;

private:
  /** Helper triggered on the storage delete event */
  void SetDataStorageDeleted();

  unsigned long m_DataStorageDeletedTag;

};
#endif // QmitkAbstractNodeSelectionWidget_H
