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
#include "QmitkDataNodeAddToSemanticRelationsAction.h"
#include "QmitkDataNodeRemoveFromSemanticRelationsAction.h"

// semantic relations module
#include <mitkDICOMHelper.h>
#include <mitkNodePredicates.h>
#include <mitkSemanticRelationsInference.h>
#include <mitkRelationStorage.h>

// mitk qt widgets module
#include <QmitkDnDDataNodeWidget.h>
#include <QmitkRenderWindow.h>

// mitk multi label module
#include <mitkLabelSetImage.h>

// berry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// qt
#include <QMenu>
#include <QTreeView>

const std::string QmitkSemanticRelationsView::VIEW_ID = "org.mitk.views.semanticrelations";

void QmitkSemanticRelationsView::SetFocus()
{
  // nothing here
}

void QmitkSemanticRelationsView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);

  m_LesionInfoWidget = new QmitkLesionInfoWidget(GetDataStorage(), parent);
  m_Controls.gridLayout->addWidget(m_LesionInfoWidget);

  m_PatientTableInspector = new QmitkPatientTableInspector(parent);
  m_PatientTableInspector->SetDataStorage(GetDataStorage());
  m_Controls.gridLayout->addWidget(m_PatientTableInspector);

  QGridLayout* dndDataNodeWidgetLayout = new QGridLayout;
  dndDataNodeWidgetLayout->addWidget(m_PatientTableInspector, 0, 0);
  dndDataNodeWidgetLayout->setContentsMargins(0, 0, 0, 0);

  m_DnDDataNodeWidget = new QmitkDnDDataNodeWidget(parent);
  m_DnDDataNodeWidget->setLayout(dndDataNodeWidgetLayout);

  m_Controls.gridLayout->addWidget(m_DnDDataNodeWidget);

  m_ContextMenu = new QmitkSemanticRelationsContextMenu(GetSite(), m_PatientTableInspector);
  m_ContextMenu->SetDataStorage(GetDataStorage());

  mitk::IRenderWindowPart* renderWindowPart = GetRenderWindowPart();
  if (nullptr != renderWindowPart)
  {
    RenderWindowPartActivated(renderWindowPart);
  }

  SetUpConnections();

  const auto& allCaseIDs = mitk::RelationStorage::GetAllCaseIDs();
  for (const auto& caseID : allCaseIDs)
  {
    AddToComboBox(caseID);
  }
}

void QmitkSemanticRelationsView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  // connect QmitkRenderWindows - underlying vtkRenderWindow is the same as "mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows()"
  QHash<QString, QmitkRenderWindow*> windowMap = renderWindowPart->GetQmitkRenderWindows();
  QHash<QString, QmitkRenderWindow*>::Iterator it;

  mitk::BaseRenderer* baseRenderer = nullptr;
  mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer;
  for (it = windowMap.begin(); it != windowMap.end(); ++it)
  {
    baseRenderer = mitk::BaseRenderer::GetInstance(it.value()->GetVtkRenderWindow());
    if (nullptr != baseRenderer)
    {
      controlledRenderer.push_back(baseRenderer);
    }
  }

  m_ContextMenu->SetControlledRenderer(controlledRenderer);
}

void QmitkSemanticRelationsView::SetUpConnections()
{
  connect(m_Controls.caseIDComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &QmitkSemanticRelationsView::OnCaseIDSelectionChanged);
  connect(m_LesionInfoWidget, &QmitkLesionInfoWidget::LesionSelectionChanged, this, &QmitkSemanticRelationsView::OnLesionSelectionChanged);
  connect(m_PatientTableInspector, &QmitkPatientTableInspector::CurrentSelectionChanged, this, &QmitkSemanticRelationsView::OnDataNodeSelectionChanged);
  connect(m_PatientTableInspector, &QmitkPatientTableInspector::DataNodeDoubleClicked, this, &QmitkSemanticRelationsView::OnDataNodeDoubleClicked);
  connect(m_DnDDataNodeWidget, &QmitkDnDDataNodeWidget::NodesDropped, this, &QmitkSemanticRelationsView::OnNodesAdded);

  connect(m_PatientTableInspector, &QmitkPatientTableInspector::OnContextMenuRequested, m_ContextMenu, &QmitkSemanticRelationsContextMenu::OnContextMenuRequested);
  connect(m_PatientTableInspector, &QmitkPatientTableInspector::OnNodeRemoved, this, &QmitkSemanticRelationsView::NodeRemoved);
}

QItemSelectionModel* QmitkSemanticRelationsView::GetDataNodeSelectionModel() const
{
  return m_PatientTableInspector->GetSelectionModel();
}

void QmitkSemanticRelationsView::NodeRemoved(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (mitk::SemanticRelationsInference::InstanceExists(dataNode))
  {
    // no observer needed for the integration; simply use a temporary instance for removing
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;
    RemoveFromSemanticRelationsAction::Run(&semanticRelationsIntegration, dataNode);
    mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(dataNode);
    RemoveFromComboBox(caseID);
  }
}

void QmitkSemanticRelationsView::OnLesionSelectionChanged(const mitk::SemanticTypes::Lesion& lesion)
{
  m_PatientTableInspector->SetLesion(lesion);
}

void QmitkSemanticRelationsView::OnDataNodeSelectionChanged(const QList<mitk::DataNode::Pointer>& dataNodeSelection)
{
  m_LesionInfoWidget->SetDataNodeSelection(dataNodeSelection);
}

void QmitkSemanticRelationsView::OnDataNodeDoubleClicked(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (mitk::NodePredicates::GetImagePredicate()->CheckNode(dataNode))
  {
    OpenInEditor(dataNode);
  }
  else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
  {
    JumpToPosition(dataNode);
  }
}

void QmitkSemanticRelationsView::OnCaseIDSelectionChanged(const QString& caseID)
{
  m_LesionInfoWidget->SetCaseID(caseID.toStdString());
  m_PatientTableInspector->SetCaseID(caseID.toStdString());
}

void QmitkSemanticRelationsView::OnNodesAdded(std::vector<mitk::DataNode*> nodes)
{
  mitk::SemanticTypes::CaseID caseID = "";
  for (mitk::DataNode* dataNode : nodes)
  {
    // no observer needed for the integration; simply use a temporary instance for adding
    mitk::SemanticRelationsIntegration semanticRelationsIntegration;
    AddToSemanticRelationsAction::Run(&semanticRelationsIntegration, GetDataStorage(), dataNode);
    caseID = mitk::GetCaseIDFromDataNode(dataNode);
    AddToComboBox(caseID);
  }
}

void QmitkSemanticRelationsView::OnNodeRemoved(const mitk::DataNode* dataNode)
{
  NodeRemoved(dataNode);
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
  std::vector<mitk::SemanticTypes::ControlPoint> allControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
  int foundIndex = m_Controls.caseIDComboBox->findText(QString::fromStdString(caseID));
  if (allControlPoints.empty() && -1 != foundIndex)
  {
    // caseID does not contain any control points and therefore no data
    // remove the caseID, if it is still contained
    m_Controls.caseIDComboBox->removeItem(foundIndex);
  }
}

void QmitkSemanticRelationsView::OpenInEditor(const mitk::DataNode* dataNode)
{
  auto renderWindowPart = GetRenderWindowPart();
  if (nullptr == renderWindowPart)
  {
    renderWindowPart = GetRenderWindowPart(mitk::WorkbenchUtil::IRenderWindowPartStrategy::BRING_TO_FRONT | mitk::WorkbenchUtil::IRenderWindowPartStrategy::OPEN);
    if (nullptr == renderWindowPart)
    {
      // no render window available
      return;
    }
  }

  auto image = dynamic_cast<mitk::Image*>(dataNode->GetData());
  if (nullptr != image)
  {
    mitk::RenderingManager::GetInstance()->InitializeViews(image->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
  }
}

void QmitkSemanticRelationsView::JumpToPosition(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    return;
  }

  mitk::LabelSetImage* labelSetImage = dynamic_cast<mitk::LabelSetImage*>(dataNode->GetData());
  if (nullptr == labelSetImage)
  {
    return;
  }

  unsigned int activeLayer = labelSetImage->GetActiveLayer();
  mitk::Label* activeLabel = labelSetImage->GetActiveLabel(activeLayer);
  labelSetImage->UpdateCenterOfMass(activeLabel->GetValue(), activeLayer);
  const mitk::Point3D& centerPosition = activeLabel->GetCenterOfMassCoordinates();
  if (centerPosition.GetVnlVector().max_value() > 0.0)
  {
    auto renderWindowPart = GetRenderWindowPart();
    if (nullptr == renderWindowPart)
    {
      renderWindowPart = GetRenderWindowPart(mitk::WorkbenchUtil::IRenderWindowPartStrategy::BRING_TO_FRONT | mitk::WorkbenchUtil::IRenderWindowPartStrategy::OPEN);
      if (nullptr == renderWindowPart)
      {
        // no render window available
        return;
      }
    }

    auto segmentation = dynamic_cast<mitk::LabelSetImage*>(dataNode->GetData());
    if (nullptr != segmentation)
    {
      renderWindowPart->SetSelectedPosition(centerPosition);
      mitk::RenderingManager::GetInstance()->InitializeViews(segmentation->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
    }
  }
}
