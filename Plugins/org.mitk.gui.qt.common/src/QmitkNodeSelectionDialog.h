/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITK_NODE_SELECTION_DIALOG_H
#define QMITK_NODE_SELECTION_DIALOG_H

#include "org_mitk_gui_qt_common_Export.h"

#include "ui_QmitkNodeSelectionDialog.h"

#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkNodePredicateBase.h>
#include "mitkIDataStorageInspectorProvider.h"

#include <QmitkAbstractDataStorageInspector.h>

#include <QDialog>
#include <QPushButton>

/**
* @class QmitkNodeSelectionDialog
* @brief A customized QDialog that displays different data storage inspectors and allows to
*        set and get a current selection by selecting data nodes in the data storage inspectors.
*/
class MITK_QT_COMMON QmitkNodeSelectionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkNodeSelectionDialog(QWidget* parent = nullptr, QString caption = "", QString hint = "");

  /**
  * @brief Set the data storage that will be used.
  *        The function iterates over the dialog's panels and sets the data storage of each panel accordingly.
  *        Each panel is a specific data storage inspector.
  *
  * @param dataStorage      A pointer to the data storage to set.
  */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
  * @brief Set the node predicate that will be used.
  *        The function iterates over the dialog's panels and sets the node predicate of each panel accordingly.
  *        Each panel is a specific data storage inspector.
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
  /**
  * @brief Set the Qt selection mode (e.g. Single selection, multi selection).
  *        The function iterates over the dialog's panels and sets the Qt selection mode of each panel accordingly.
  *        Each panel is a concrete data storage inspector.
  *
  * @param mode   The QAbstractItemView::SelectionMode to define the selection mode.
  */
  void SetSelectionMode(SelectionMode mode);
  SelectionMode GetSelectionMode() const;

Q_SIGNALS:
  /**
  * @brief A signal that will be emitted if the selected node has changed.
  *
  * @param nodes		A list of data nodes that are newly selected.
  */
  void CurrentSelectionChanged(NodeList nodes);

  public Q_SLOTS:
  /**
  * @brief Set the selection modus to (not) include invisible nodes in the selection.
  *        The function iterates over the dialog's panels and sets the selection modus of each panel accordingly.
  *        Each panel is a concrete data storage inspector.
  *
  * @param selectOnlyVisibleNodes   The bool value to define the selection modus.
  */
  void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes);
  /**
  * @brief Set the currently selected nodes given a list of data nodes.
  *        The function iterates over the dialog's panels and sets the current selection of each panel accordingly.
  *        Each panel is a concrete data storage inspector.
  *
  * @param selectedNodes A list of data nodes that should be newly selected.
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
