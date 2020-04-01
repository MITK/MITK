/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITK_NODE_SELECTION_DIALOG_H
#define QMITK_NODE_SELECTION_DIALOG_H

#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkNodePredicateBase.h>
#include "mitkIDataStorageInspectorProvider.h"

#include <QmitkAbstractDataStorageInspector.h>

#include "org_mitk_gui_qt_common_Export.h"

#include "ui_QmitkNodeSelectionDialog.h"

#include <QDialog>
#include <QPushButton>

/**
* \class QmitkNodeSelectionDialog
* \brief Widget that allows to show and edit the content of an mitk::IsoDoseLevel instance.
*/
class MITK_QT_COMMON QmitkNodeSelectionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkNodeSelectionDialog(QWidget* parent = nullptr, QString caption = "", QString hint = "");

  /**
  * @brief Sets the data storage that will be used /monitored by widget.
  *
  * @param dataStorage      A pointer to the data storage to set.
  */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
  * @brief Sets the node predicate and updates the widget, according to the node predicate.
  *
  * @param nodePredicate    A pointer to node predicate.
  */
  virtual void SetNodePredicate(const mitk::NodePredicateBase* nodePredicate);

  const mitk::NodePredicateBase* GetNodePredicate() const;

  using NodeList = QList<mitk::DataNode::Pointer>;
  NodeList GetSelectedNodes() const;

  /**
  * @brief Helper function that is used to check the given selection for consistency.
  *        Returning an empty string assumes that everything is alright and the selection is valid.
  *        If the string is not empty, the content of the string will be used as error message.
  */
  using SelectionCheckFunctionType = std::function<std::string(const NodeList &)>;
  /**
  * @brief A selection check function can be set. If set the dialog uses this function to check the made/set selection.
  *        If the selection is valid, everything is fine.
  *        If the selection is indicated as invalid, the dialog will display the selection check function error message.
  */
  void SetSelectionCheckFunction(const SelectionCheckFunctionType &checkFunction);

  bool GetSelectOnlyVisibleNodes() const;

  using SelectionMode = QAbstractItemView::SelectionMode;
  void SetSelectionMode(SelectionMode mode);
  SelectionMode GetSelectionMode() const;

Q_SIGNALS:
  /*
  * @brief A signal that will be emitted if the selected node has changed.
  *
  * @param nodes		A list of data nodes that are newly selected.
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
  * @param selectOnlyVisibleNodes   The bool value to define the selection modus.
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
  * @param nodes		A list of data nodes that should be newly selected.
  */
  void SetCurrentSelection(NodeList selectedNodes);

protected Q_SLOTS:

  void OnSelectionChanged(NodeList selectedNodes);
  void OnFavoriteNodesButtonClicked();
  void OnOK();
  void OnCancel();
  void OnDoubleClicked(const QModelIndex& index);

protected:
  void SetErrorText(const std::string& checkResponse);

  void AddPanel(const mitk::IDataStorageInspectorProvider* provider, const mitk::IDataStorageInspectorProvider::InspectorIDType &preferredID, bool &preferredFound, int &preferredIndex);

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::ConstPointer m_NodePredicate;
  bool m_SelectOnlyVisibleNodes;
  NodeList m_SelectedNodes;

  SelectionCheckFunctionType m_CheckFunction;

  SelectionMode m_SelectionMode;

  using PanelVectorType = std::vector<QmitkAbstractDataStorageInspector*>;
  PanelVectorType m_Panels;

  QPushButton* m_FavoriteNodesButton;
  Ui_QmitkNodeSelectionDialog m_Controls;
};
#endif // QMITK_NODE_SELECTION_DIALOG_H
