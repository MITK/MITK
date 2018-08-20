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

  QGridLayout* dndDataNodeWidgetLayout = new QGridLayout;
  dndDataNodeWidgetLayout->addWidget(m_PatientTableInspector, 0, 0);
  dndDataNodeWidgetLayout->setContentsMargins(0, 0, 0, 0);

  m_DnDDataNodeWidget = new QmitkDnDDataNodeWidget(parent);
  m_DnDDataNodeWidget->setLayout(dndDataNodeWidgetLayout);

  m_Controls.gridLayout->addWidget(m_DnDDataNodeWidget);

  m_ContextMenu = new QMenu(m_PatientTableInspector);

  m_InformationTypeAction = new QmitkDataNodeInformationTypeAction(m_ContextMenu, GetSite());
  m_InformationTypeAction->SetDataStorage(GetDataStorage());
  m_ContextMenu->addAction(m_InformationTypeAction);

  m_OpenInAction = new QmitkDataNodeOpenInAction(m_ContextMenu, GetSite());
  m_ContextMenu->addAction(m_OpenInAction);

  mitk::IRenderWindowPart* renderWindowPart = GetRenderWindowPart();
  if (nullptr != renderWindowPart)
  {
    RenderWindowPartActivated(renderWindowPart);
  }

  SetUpConnections();
}

void QmitkSemanticRelationsView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  // connect QmitkRenderWindows - underlying vtkRenderWindow is the same as "mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows()"
  QHash<QString, QmitkRenderWindow*> windowMap = renderWindowPart->GetQmitkRenderWindows();
  QHash<QString, QmitkRenderWindow*>::Iterator it;

  mitk::BaseRenderer* baseRenderer = nullptr;
  RenderWindowLayerUtilities::RendererVector controlledRenderer;
  for (it = windowMap.begin(); it != windowMap.end(); ++it)
  {
    baseRenderer = mitk::BaseRenderer::GetInstance(it.value()->GetVtkRenderWindow());
    if (nullptr != baseRenderer)
    {
      controlledRenderer.push_back(baseRenderer);
    }
  }

  m_OpenInAction->SetControlledRenderer(controlledRenderer);
}

void QmitkSemanticRelationsView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  // nothing here
}

void QmitkSemanticRelationsView::SetUpConnections()
{
  connect(m_Controls.caseIDComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &QmitkSemanticRelationsView::OnCaseIDSelectionChanged);
  connect(m_LesionInfoWidget, &QmitkLesionInfoWidget::LesionChanged, this, &QmitkSemanticRelationsView::OnLesionChanged);
  connect(m_PatientTableInspector, &QmitkPatientTableInspector::DataNodeDoubleClicked, this, &QmitkSemanticRelationsView::OnDataNodeDoubleClicked);
  connect(m_DnDDataNodeWidget, &QmitkDnDDataNodeWidget::NodesDropped, this, &QmitkSemanticRelationsView::OnNodesAdded);

  connect(m_PatientTableInspector, &QmitkPatientTableInspector::OnContextMenuRequested, this, &QmitkSemanticRelationsView::OnContextMenuRequested);
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

  if (mitk::NodePredicates::GetImagePredicate()->CheckNode(dataNode))
  {
    RemoveImage(dataNode);
  }
  else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
  {
    RemoveSegmentation(dataNode);
  }
}

void QmitkSemanticRelationsView::OnCaseIDSelectionChanged(const QString& caseID)
{
  m_LesionInfoWidget->SetCurrentCaseID(caseID.toStdString());
  m_PatientTableInspector->SetCaseID(caseID.toStdString());
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

void QmitkSemanticRelationsView::OnLesionChanged(const mitk::SemanticTypes::Lesion& lesion)
{
  m_PatientTableInspector->SetLesion(lesion);
}

void QmitkSemanticRelationsView::OnDataNodeDoubleClicked(const mitk::DataNode::Pointer dataNode)
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

void QmitkSemanticRelationsView::OnNodesAdded(QmitkDnDDataNodeWidget* dnDDataNodeWidget, std::vector<mitk::DataNode*> nodes)
{
  for (mitk::DataNode* dataNode : nodes)
  {
    if (nullptr == dataNode)
    {
      continue;
    }

    if (mitk::NodePredicates::GetImagePredicate()->CheckNode(dataNode))
    {
      AddImage(dataNode);
    }
    else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
    {
      AddSegmentation(dataNode);
    }
  }
}

void QmitkSemanticRelationsView::OnContextMenuRequested(const QPoint& /*pos*/)
{
  m_ContextMenu->popup(QCursor::pos());
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

void QmitkSemanticRelationsView::OpenInEditor(const mitk::DataNode::Pointer dataNode)
{
  auto renderWindowPart = GetRenderWindowPart();
  if (nullptr == renderWindowPart)
  {
    renderWindowPart = GetRenderWindowPart(QmitkAbstractView::BRING_TO_FRONT | QmitkAbstractView::OPEN);
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

void QmitkSemanticRelationsView::JumpToPosition(const mitk::DataNode::Pointer dataNode)
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

  int activeLayer = labelSetImage->GetActiveLayer();
  mitk::Label* activeLabel = labelSetImage->GetActiveLabel(activeLayer);
  labelSetImage->UpdateCenterOfMass(activeLabel->GetValue(), activeLayer);
  const mitk::Point3D& centerPosition = activeLabel->GetCenterOfMassCoordinates();
  if (centerPosition.GetVnlVector().max_value() > 0.0)
  {
    auto renderWindowPart = GetRenderWindowPart();
    if (nullptr == renderWindowPart)
    {
      renderWindowPart = GetRenderWindowPart(QmitkAbstractView::BRING_TO_FRONT | QmitkAbstractView::OPEN);
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

void QmitkSemanticRelationsView::AddImage(const mitk::DataNode* image)
{
  if (nullptr == image)
  {
    return;
  }

  try
  {
    // add the image to the semantic relations storage
    m_SemanticRelations->AddImage(image);
    mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(image);
    AddToComboBox(caseID);
  }
  catch (const mitk::SemanticRelationException& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not add the selected image.");
    msgBox.setText("The program wasn't able to correctly add the selected images.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }
}

void QmitkSemanticRelationsView::AddSegmentation(const mitk::DataNode* segmentation)
{
  if (nullptr == segmentation)
  {
    return;
  }

  mitk::BaseData* baseData = segmentation->GetData();
  if (nullptr == baseData)
  {
    return;
  }

  // continue with valid segmentation data
  // get parent node of the current segmentation node with the node predicate
  mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = GetDataStorage()->GetSources(segmentation, mitk::NodePredicates::GetImagePredicate(), false);

  // check for already existing, identifying base properties
  mitk::BaseProperty* caseIDProperty = baseData->GetProperty("DICOM.0010.0010");
  mitk::BaseProperty* nodeIDProperty = baseData->GetProperty("DICOM.0020.000E");
  if (nullptr == caseIDProperty || nullptr == nodeIDProperty)
  {
    MITK_INFO << "No DICOM tags for case and node identification found. Transferring DICOM tags from the parent node to the selected segmentation node.";

    mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(parentNodes->front());
    mitk::SemanticTypes::ID nodeID = mitk::GetIDFromDataNode(parentNodes->front());
    // transfer DICOM tags to the segmentation node
    mitk::StringProperty::Pointer caseIDTag = mitk::StringProperty::New(caseID);
    baseData->SetProperty("DICOM.0010.0010", caseIDTag); // DICOM tag is "PatientName"

                                                         // add UID to distinguish between different segmentations of the same parent node
    mitk::StringProperty::Pointer nodeIDTag = mitk::StringProperty::New(nodeID + mitk::UIDGeneratorBoost::GenerateUID());
    baseData->SetProperty("DICOM.0020.000E", nodeIDTag); // DICOM tag is "SeriesInstanceUID"
  }

  try
  {
    m_SemanticRelations->AddSegmentation(segmentation, parentNodes->front());
  }
  catch (const mitk::SemanticRelationException& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not add the selected segmentation.");
    msgBox.setText("The program wasn't able to correctly add the selected segmentation.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
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
    MITK_WARN << "Could not correctly remove the selected image from the semantic relations model.\n"
      << "Reason: " + exceptionMessage.str();
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
    MITK_WARN << "Could not correctly remove the selected segmentation from the semantic relations model.\n"
              << "Reason: " + exceptionMessage.str();
  }
}
