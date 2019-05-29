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


#ifndef QMITK_MULTI_NODE_SELECTION_WIDGET_H
#define QMITK_MULTI_NODE_SELECTION_WIDGET_H

#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkNodePredicateBase.h>

#include "QmitkSimpleTextOverlayWidget.h"

#include "org_mitk_gui_qt_common_Export.h"

#include "ui_QmitkMultiNodeSelectionWidget.h"

#include <QmitkAbstractNodeSelectionWidget.h>

class QmitkAbstractDataStorageModel;
class QAbstractItemVew;

/**
* \class QmitkMultiNodeSelectionWidget
* \brief Widget that allows to perform and represents a multiple node selection.
*/
class MITK_QT_COMMON QmitkMultiNodeSelectionWidget : public QmitkAbstractNodeSelectionWidget
{
  Q_OBJECT

public:
  explicit QmitkMultiNodeSelectionWidget(QWidget* parent = nullptr);

  using NodeList = QmitkAbstractNodeSelectionWidget::NodeList;

  NodeList GetSelectedNodes() const;

  /**Helper function that is used to check the given selection for consistency.
   Returning an empty string assumes that everything is alright and the selection
   is valid. If the string is not empty, the content of the string will be used
   as error message in the overlay to indicate the problem.*/
  using SelectionCheckFunctionType = std::function<std::string(const NodeList &)>;
  /**A selection check function can be set. If set the widget uses this function to
   check the made/set selection. If the selection is valid, everything is fine.
   If selection is indicated as invalid, it will not be communicated by the widget
   (no signal emission.*/
  void SetSelectionCheckFunction(const SelectionCheckFunctionType &checkFunction);

public Q_SLOTS:
  virtual void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes) override;
  virtual void SetCurrentSelection(NodeList selectedNodes) override;
  void OnEditSelection();

protected Q_SLOTS:
  void OnClearSelection(const mitk::DataNode* node);

protected:
  NodeList CompileEmitSelection() const;

  void UpdateInfo() override;
  virtual void UpdateList();

  void OnNodePredicateChanged(mitk::NodePredicateBase* newPredicate) override;
  void OnDataStorageChanged() override;
  void NodeRemovedFromStorage(const mitk::DataNode* node) override;

  NodeList m_CurrentSelection;

  QmitkSimpleTextOverlayWidget* m_Overlay;

  SelectionCheckFunctionType m_CheckFunction;
  std::string m_CheckResponse;

  Ui_QmitkMultiNodeSelectionWidget m_Controls;
};
#endif // QmitkMultiNodeSelectionWidget_H
