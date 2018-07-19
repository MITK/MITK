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

  connect(m_Controls.caseIDComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &QmitkSemanticRelationsView::OnCaseIDSelectionChanged);

  connect(m_LesionInfoWidget, &QmitkLesionInfoWidget::ImageAdded, this, &QmitkSemanticRelationsView::AddToComboBox);
  connect(m_LesionInfoWidget, &QmitkLesionInfoWidget::JumpToPosition, this, &QmitkSemanticRelationsView::OnJumpToPosition);
  connect(m_LesionInfoWidget, &QmitkLesionInfoWidget::ImageRemoved, this, &QmitkSemanticRelationsView::OnImageRemoved);
}

void QmitkSemanticRelationsView::NodeRemoved(const mitk::DataNode* node)
{
  if (mitk::NodePredicates::GetImagePredicate()->CheckNode(node))
  {
    m_LesionInfoWidget->OnRemoveImage(node);
  }
  else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(node))
  {
    m_LesionInfoWidget->OnRemoveSegmentation(node);
  }
}

void QmitkSemanticRelationsView::OnCaseIDSelectionChanged(const QString& caseID)
{
  m_LesionInfoWidget->SetCurrentCaseID(caseID.toStdString());
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

void QmitkSemanticRelationsView::OnImageRemoved(const mitk::DataNode* imageNode)
{
  mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(imageNode);
  RemoveFromComboBox(caseID);
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
