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

// semantic relations plugin
#include "QmitkSemanticRelationsView.h"
#include "QmitkSemanticRelationsNodeSelectionDialog.h"

// semantic relations module
#include <mitkNodePredicates.h>
#include <mitkSemanticRelationException.h>
#include <mitkUIDGeneratorBoost.h>

// blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// qt
#include <QMessageBox>

const std::string QmitkSemanticRelationsView::VIEW_ID = "org.mitk.views.semanticrelations";

void QmitkSemanticRelationsView::SetFocus()
{
  // nothing here
}

void QmitkSemanticRelationsView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);

  // initialize the semantic relations
  m_SemanticRelations = std::make_unique<mitk::SemanticRelations>(GetDataStorage());

  m_LesionInfoWidget = new QmitkLesionInfoWidget(GetDataStorage(), parent);
  m_Controls.gridLayout->addWidget(m_LesionInfoWidget);

  m_PatientTableInspector = new QmitkPatientTableInspector(parent);
  m_PatientTableInspector->SetDataStorage(GetDataStorage());
  m_Controls.gridLayout->addWidget(m_PatientTableInspector);

  SetUpConnections();
}

void QmitkSemanticRelationsView::SetUpConnections()
{
  connect(m_Controls.caseIDComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &QmitkSemanticRelationsView::OnCaseIDSelectionChanged);

  connect(m_LesionInfoWidget, &QmitkLesionInfoWidget::LesionChanged, this, &QmitkSemanticRelationsView::OnLesionChanged);
}

void QmitkSemanticRelationsView::NodeRemoved(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (mitk::NodePredicates::GetImagePredicate()->CheckNode(dataNode))
  {
    RemoveImage(dataNode);
  }
  else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
  {
    RemoveSegmentation(dataNode);
  }
}
  }
}

void QmitkSemanticRelationsView::OnCaseIDSelectionChanged(const QString& caseID)
{
  m_LesionInfoWidget->SetCurrentCaseID(caseID.toStdString());
  m_PatientTableInspector->SetCaseID(caseID.toStdString());
}

void QmitkSemanticRelationsView::OnJumpToPosition(const mitk::Point3D& position)
{
  mitk::IRenderWindowPart* renderWindowPart = GetRenderWindowPart();
  if (nullptr != renderWindowPart)
  {
    renderWindowPart->SetSelectedPosition(position);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSemanticRelationsView::OnLesionChanged(const mitk::SemanticTypes::Lesion& lesion)
{
  m_PatientTableInspector->SetLesion(lesion);
}

void QmitkSemanticRelationsView::AddToComboBox(const mitk::SemanticTypes::CaseID& caseID)
{
  int foundIndex = m_Controls.caseIDComboBox->findText(QString::fromStdString(caseID));
  if (-1 == foundIndex)
  {
    // add the caseID to the combo box, as it is not already contained
    m_Controls.caseIDComboBox->addItem(QString::fromStdString(caseID));
  }
}

void QmitkSemanticRelationsView::RemoveFromComboBox(const mitk::SemanticTypes::CaseID& caseID)
{
  std::vector<mitk::SemanticTypes::ControlPoint> allControlPoints = m_SemanticRelations->GetAllControlPointsOfCase(caseID);
  int foundIndex = m_Controls.caseIDComboBox->findText(QString::fromStdString(caseID));
  if (allControlPoints.empty() && -1 != foundIndex)
  {
    // TODO: find new way to check for empty case id
    // caseID does not contain any control points and therefore no data
    // remove the caseID, if it is still contained
    m_Controls.caseIDComboBox->removeItem(foundIndex);
  }
}
void QmitkSemanticRelationsView::RemoveImage(const mitk::DataNode* image)
{
  try
  {
    m_SemanticRelations->RemoveImage(image);
    mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(image);
    RemoveFromComboBox(caseID);
  }
  catch (const mitk::SemanticRelationException& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not remove the selected image.");
    msgBox.setText("The program wasn't able to correctly remove the selected image from the semantic relations model.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
  }
}

void QmitkSemanticRelationsView::RemoveSegmentation(const mitk::DataNode* segmentation)
{
  try
  {
    m_SemanticRelations->RemoveSegmentation(segmentation);
  }
  catch (const mitk::SemanticRelationException& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not remove the selected segmentation.");
    msgBox.setText("The program wasn't able to correctly remove the selected segmentation from the semantic relations model.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
  }
}
