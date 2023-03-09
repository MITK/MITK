/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSingleNodeSelectionWidget_h
#define QmitkSingleNodeSelectionWidget_h

#include <MitkQtWidgetsExports.h>

#include <ui_QmitkSingleNodeSelectionWidget.h>

#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkNodePredicateBase.h>

#include <QmitkAbstractNodeSelectionWidget.h>
#include <QmitkNodeSelectionButton.h>

class QmitkAbstractDataStorageModel;

/**
* @class QmitkSingleNodeSelectionWidget
* @brief Widget that represents a node selection of (max) one node. It acts like a button. Clicking on it
*        allows to change the selection.
*
* @remark This class provides a public function 'SetAutoSelectNewNodes' that can be used to enable
*         the auto selection mode (default is false).
*         The user of this class calling this function has to make sure that the base-class Q_SIGNAL
*         'CurrentSelectionChanged', which will be emitted by this function, is already
*         connected to a receiving slot, if the initial valid auto selection should not get lost.
*/
class MITKQTWIDGETS_EXPORT QmitkSingleNodeSelectionWidget : public QmitkAbstractNodeSelectionWidget
{
  Q_OBJECT

public:
  explicit QmitkSingleNodeSelectionWidget(QWidget* parent = nullptr);

  mitk::DataNode::Pointer GetSelectedNode() const;
  bool GetAutoSelectNewNodes() const;

  using NodeList = QmitkAbstractNodeSelectionWidget::NodeList;

public Q_SLOTS:
  void SetCurrentSelectedNode(mitk::DataNode* selectedNode);

  /**
  * Sets the auto selection mode (default is false).
  * If auto select is true and the following conditions are fullfilled, the widget will
  * select a node automatically from the data storage:
  *  - a data storage is set
  *  - data storage contains at least one node that matches the given predicate
  *  - no selection is set
  *
  * @remark Enabling the auto selection mode by calling 'SetAutoSelectNewNodes(true)'
  *         will directly emit a 'QmitkSingleNodeSelectionWidget::CurrentSelectionChanged' Q_SIGNAL
  *         if a valid auto selection was made.
  *         If this initial emission should not get lost, auto selection mode needs to be enabled after this
  *         selection widget has been connected via the 'QmitkSingleNodeSelectionWidget::CurrentSelectionChanged'
  *         Q_SIGNAL to a receiving function.
  */
  void SetAutoSelectNewNodes(bool autoSelect);

protected Q_SLOTS:
  virtual void OnClearSelection();

protected:
  void ReviseSelectionChanged(const NodeList& oldInternalSelection, NodeList& newInternalSelection) override;

  bool eventFilter(QObject *obj, QEvent *ev) override;
  void EditSelection();
  void UpdateInfo() override;

  void OnDataStorageChanged() override;
  void OnNodeAddedToStorage(const mitk::DataNode* node) override;

  void AutoSelectNodes();

  /** Helper function that gets a suitable auto selected node from the datastorage that fits to the predicate settings.
   @param ignoreNodes You may pass a list of nodes that must not be choosen as auto selected node. */
  mitk::DataNode::Pointer DetermineAutoSelectNode(const NodeList& ignoreNodes = {});

  /** See documentation of SetAutoSelectNewNodes for details*/
  bool m_AutoSelectNodes;

  Ui_QmitkSingleNodeSelectionWidget m_Controls;
};

#endif
