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

// semantic relations module
#include <mitkSemanticRelations.h>
#include <mitkSemanticTypes.h>

// semantic relations UI module
#include <QmitkPatientTableWidget.h>
#include <QmitkSimpleDatamanagerWidget.h>
#include <QmitkSelectNodeDialog.h>

// blueberry
#include <berryISelectionListener.h>

// mitk qt
#include <QmitkAbstractView.h>

/*
* @brief The QmitkSemanticRelationsView is a MITK view to combine and show the widgets of the 'SemanticRelationsUI'-module.
*
*         It allows the MITK user to see and modify the content of the SemanticRelations-session.
*         It provides a dialog to add images from the data storage to the semantic relations model and a dialog to select
*         data nodes that were added to the semantic relations model before.
*         If provides functionality to create new lesions and link them with segmentation nodes.
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
  void OnSelectPatientNodeButtonClicked();
  /*
  * @brief Generates a new, empty lesion to add to the semantic relations model for the current case ID.
  */
  void OnAddLesionButtonClicked();
  void OnRemoveLesionButtonClicked();
  void OnLinkLesionButtonClicked();
  void OnUnlinkLesionButtonClicked();
  void OnAddImageButtonClicked();
  void OnRemoveImageButtonClicked();

private:

  virtual void NodeRemoved(const mitk::DataNode* node) override;
  void AddToComboBox(const mitk::SemanticTypes::CaseID& caseID);
  void RemoveFromComboBox(const mitk::SemanticTypes::CaseID& caseID);

  // the Qt parent of our GUI
  QWidget* m_Parent;
  Ui::QmitkSemanticRelationsControls m_Controls;
  QmitkPatientTableWidget* m_PatientTableWidget;
  QmitkSimpleDatamanagerWidget* m_SimpleDatamanagerWidget;
  QmitkSelectNodeDialog* m_SelectPatientNodeDialog;
  QmitkSelectNodeDialog* m_SimpleDatamanagerNodeDialog;

  mitk::SemanticTypes::CaseID m_CaseID;
  std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;
};

#endif // QMITKSEMANTICRELATIONSVIEW_H
