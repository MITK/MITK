/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSEMANTICRELATIONSVIEW_H
#define QMITKSEMANTICRELATIONSVIEW_H

// semantic relations plugin
#include "ui_QmitkSemanticRelationsControls.h"
#include "QmitkLesionInfoWidget.h"
#include "QmitkSemanticRelationsContextMenu.h"

// semantic relations module
#include <mitkSemanticTypes.h>

// semantic relations UI module
#include <QmitkPatientTableInspector.h>

// mitk gui common plugin
#include <mitkIRenderWindowPartListener.h>

// berry
#include <berryISelectionListener.h>

// mitk qt gui common plugin
#include <QmitkAbstractView.h>

class QmitkDnDDataNodeWidget;
class QMenu;

/*
* @brief The QmitkSemanticRelationsView is an MITK view to combine and show the widgets of the 'SemanticRelationsUI'-module and this semantic relations plugin.
*
*         It allows the MITK user to see and modify the content of the SemanticRelations-session.
*         A combo box is used to select and show the current patient.
*/
class QmitkSemanticRelationsView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart) override;

protected:

  void SetFocus() override;
  void CreateQtPartControl(QWidget* parent) override;

private Q_SLOTS:

  void OnLesionSelectionChanged(const mitk::SemanticTypes::Lesion&);
  void OnDataNodeSelectionChanged(const QList<mitk::DataNode::Pointer>&);
  void OnDataNodeDoubleClicked(const mitk::DataNode*);
  void OnCaseIDSelectionChanged(const QString&);

  void OnNodesAdded(std::vector<mitk::DataNode*>);
  void OnNodeRemoved(const mitk::DataNode*);

private:

  void SetUpConnections();
  /**
  * @brief Provide a QItemSelectionModel, which supports the data role 'QmitkDataNodeRole' (\see QmitkRenderWindowDataModel).
  *
  * The provided QItemSelectionModel is used in the QmitkAbstractView-base class as the selection model of
  * the selection provider (\see QmitkAbstractView::SetSelectionProvider()).
  * The default selection provider is a QmitkDataNodeSelectionProvider. Each time a selection in the provided
  * QItemSeletionModel is changed, a selection changed event is fired. All plugins (views), that subclass the
  * QmitkAbstractView will be informed about the selection changed via the OnSelectionChanged-function.
  */
  QItemSelectionModel* GetDataNodeSelectionModel() const override;

  void NodeRemoved(const mitk::DataNode* dataNode) override;

  void AddToComboBox(const mitk::SemanticTypes::CaseID& caseID);
  void RemoveFromComboBox(const mitk::SemanticTypes::CaseID& caseID);

  void OpenInEditor(const mitk::DataNode* dataNode);

  void SetControlledRenderer();

  Ui::QmitkSemanticRelationsControls m_Controls;

  mitk::IRenderWindowPart* m_RenderWindowPart;

  QmitkLesionInfoWidget* m_LesionInfoWidget;
  QmitkPatientTableInspector* m_PatientTableInspector;
  QmitkDnDDataNodeWidget* m_DnDDataNodeWidget;

  QmitkSemanticRelationsContextMenu* m_ContextMenu;
};

#endif // QMITKSEMANTICRELATIONSVIEW_H
