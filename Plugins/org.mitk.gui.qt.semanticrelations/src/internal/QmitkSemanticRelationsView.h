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

#ifndef QMITKSEMANTICRELATIONSVIEW_H
#define QMITKSEMANTICRELATIONSVIEW_H

// semantic relations plugin
#include "ui_QmitkSemanticRelationsControls.h"
#include "QmitkLesionInfoWidget.h"

// semantic relations module
#include <mitkSemanticRelations.h>
#include <mitkSemanticTypes.h>

// semantic relations UI module
#include <QmitkPatientTableInspector.h>

// blueberry
#include <berryISelectionListener.h>

// mitk qt
#include <QmitkAbstractView.h>

class QmitkDnDDataNodeWidget;

/*
* @brief The QmitkSemanticRelationsView is an MITK view to combine and show the widgets of the 'SemanticRelationsUI'-module and this semantic relations plugin.
*
*         It allows the MITK user to see and modify the content of the SemanticRelations-session.
*         A combo box is used to select and show the current patient.
*
*         SIDE NOTE: Modifying the control points and information types of data in the semantic relations model is currently done
*                    by using the right-click context-menu of the "PatientTableWidget" in the "Select Patient Node"-Dialog.
*                    This is a leftover from when the widget was not used as a selection widget. Those functionality will be moved
*                    to this main GUI soon.
*/
class QmitkSemanticRelationsView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

protected:

  virtual void SetFocus() override;
  virtual void CreateQtPartControl(QWidget* parent) override;

private Q_SLOTS:

  void OnCaseIDSelectionChanged(const QString&);

  void AddToComboBox(const mitk::SemanticTypes::CaseID&);
  void OnLesionChanged(const mitk::SemanticTypes::Lesion&);
  void OnDataNodeDoubleClicked(const mitk::DataNode::Pointer);
  void OnNodesAdded(QmitkDnDDataNodeWidget*, std::vector<mitk::DataNode*>);

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
  virtual QItemSelectionModel* GetDataNodeSelectionModel() const override;

  virtual void NodeRemoved(const mitk::DataNode* dataNode) override;

  void RemoveFromComboBox(const mitk::SemanticTypes::CaseID& caseID);

  void OpenInEditor(const mitk::DataNode::Pointer dataNode);
  void JumpToPosition(const mitk::DataNode::Pointer dataNode);

  void AddImage(const mitk::DataNode* image);
  void AddSegmentation(const mitk::DataNode* segmentation);

  void RemoveImage(const mitk::DataNode* image);
  void RemoveSegmentation(const mitk::DataNode* segmentation);

  Ui::QmitkSemanticRelationsControls m_Controls;
  QmitkLesionInfoWidget* m_LesionInfoWidget;
  QmitkPatientTableInspector* m_PatientTableInspector;
  QmitkDnDDataNodeWidget* m_DnDDataNodeWidget;

  std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;
};

#endif // QMITKSEMANTICRELATIONSVIEW_H
