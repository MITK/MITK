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

#ifndef QMITK_NODE_SELECTION_DIALOG_H
#define QMITK_NODE_SELECTION_DIALOG_H

#include "org_mitk_gui_qt_common_Export.h"

#include "ui_QmitkNodeSelectionDialog.h"

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkNodePredicateBase.h>

// mitk qt widgets module
#include <QmitkAbstractDataStorageInspector.h>

// qt
#include <QDialog>

/**
* @class QmitkNodeSelectionDialog
* @brief A customized QDialog that displays different concrete data storage inspectors and allows to
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
  *        Each panel is a concrete data storage inspector.
  *
  * @param dataStorage      A pointer to the data storage to set.
  */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
  * @brief Set the node predicate that will be used.
  *        The function iterates over the dialog's panels and sets the node predicate of each panel accordingly.
  *        Each panel is a concrete data storage inspector.
  *
  * @param nodePredicate    A pointer to node predicate.
  */
  virtual void SetNodePredicate(mitk::NodePredicateBase* nodePredicate);

  mitk::NodePredicateBase* GetNodePredicate() const;

  using NodeList = QList<mitk::DataNode::Pointer>;

  NodeList GetSelectedNodes() const;

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
  * @param nodes		A list of data nodes that should be newly selected.
  */
  void SetCurrentSelection(NodeList selectedNodes);

protected Q_SLOTS:
  void OnSelectionChanged(NodeList selectedNodes);
  void OnOK();
  void OnCancel();

protected:
  void AddPanel(QmitkAbstractDataStorageInspector* view, QString name, QString desc);

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::Pointer m_NodePredicate;
  bool m_SelectOnlyVisibleNodes;
  NodeList m_SelectedNodes;

  SelectionMode m_SelectionMode;

  using PanelVectorType = std::vector<QmitkAbstractDataStorageInspector*>;
  PanelVectorType m_Panels;

  Ui_QmitkNodeSelectionDialog m_Controls;
};

#endif // QMITK_NODE_SELECTION_DIALOG_H
