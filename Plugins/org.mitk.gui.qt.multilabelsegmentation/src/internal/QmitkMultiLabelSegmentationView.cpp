/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiLabelSegmentationView.h"
#include "mitkPluginActivator.h"

// blueberry
#include <berryIWorkbenchPage.h>

// mitk
#include <mitkApplicationCursor.h>
#include <mitkInteractionEventObserver.h>
#include <mitkImageTimeSelector.h>
#include <mitkLabelSetImage.h>
#include <mitkNodePredicateSubGeometry.h>
#include <mitkPlanePositionManager.h>
#include <mitkStatusBar.h>
#include <mitkToolManagerProvider.h>

// Qmitk
#include "QmitkCreateMultiLabelPresetAction.h"
#include "QmitkLoadMultiLabelPresetAction.h"
#include <QmitkNewSegmentationDialog.h>
#include <QmitkRenderWindow.h>
#include <QmitkSegmentationOrganNamesHandling.cpp>
#include <QmitkStyleManager.h>

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

// Qt
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>

#include <itksys/SystemTools.hxx>

#include <regex>

const std::string QmitkMultiLabelSegmentationView::VIEW_ID = "org.mitk.views.multilabelsegmentation";

QmitkMultiLabelSegmentationView::QmitkMultiLabelSegmentationView()
  : m_Parent(nullptr),
    m_RenderWindowPart(nullptr),
    m_ToolManager(nullptr),
    m_ReferenceNode(nullptr),
    m_WorkingNode(nullptr),
    m_AutoSelectionEnabled(false),
    m_MouseCursorSet(false)
{

  auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  auto isDti = mitk::NodePredicateDataType::New("TensorImage");
  auto isOdf = mitk::NodePredicateDataType::New("OdfImage");
  auto isSegment = mitk::NodePredicateDataType::New("Segment");

  auto validImages = mitk::NodePredicateOr::New();
  validImages->AddPredicate(mitk::NodePredicateAnd::New(isImage, mitk::NodePredicateNot::New(isSegment)));
  validImages->AddPredicate(isDwi);
  validImages->AddPredicate(isDti);
  validImages->AddPredicate(isOdf);

  auto isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  auto isMask = mitk::NodePredicateAnd::New(isBinary, isImage);

  auto validSegmentations = mitk::NodePredicateOr::New();
  validSegmentations->AddPredicate(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  validSegmentations->AddPredicate(isMask);

  m_SegmentationPredicate = mitk::NodePredicateAnd::New();
  m_SegmentationPredicate->AddPredicate(validSegmentations);
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

  m_ReferencePredicate = mitk::NodePredicateAnd::New();
  m_ReferencePredicate->AddPredicate(validImages);
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(m_SegmentationPredicate));
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));
}

QmitkMultiLabelSegmentationView::~QmitkMultiLabelSegmentationView()
{
  OnLooseLabelSetConnection();

  // removing all observers
  for (NodeTagMapType::iterator dataIter = m_WorkingDataObserverTags.begin(); dataIter != m_WorkingDataObserverTags.end(); ++dataIter)
  {
    (*dataIter).first->GetProperty("visible")->RemoveObserver((*dataIter).second);
  }
  m_WorkingDataObserverTags.clear();

  mitk::RenderingManager::GetInstance()->RemoveObserver(m_RenderingManagerObserverTag);

  m_ToolManager->SetReferenceData(nullptr);
  m_ToolManager->SetWorkingData(nullptr);
}

/**********************************************************************/
/* private Q_SLOTS                                                    */
/**********************************************************************/
void QmitkMultiLabelSegmentationView::OnReferenceSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  m_ToolManager->ActivateTool(-1);

  if (nodes.empty())
  {
    m_Controls.m_WorkingNodeSelector->SetNodePredicate(m_SegmentationPredicate);
    m_ReferenceNode = nullptr;
    m_ToolManager->SetReferenceData(m_ReferenceNode);
    this->UpdateGUI();
    return;
  }

  m_ReferenceNode = nodes.first();
  m_ToolManager->SetReferenceData(m_ReferenceNode);

  if (m_ReferenceNode.IsNotNull())
  {
    // set a predicate such that a segmentation fits the selected reference image geometry
    auto segPredicate = mitk::NodePredicateAnd::New(m_SegmentationPredicate.GetPointer(),
      mitk::NodePredicateSubGeometry::New(m_ReferenceNode->GetData()->GetGeometry()));

    m_Controls.m_WorkingNodeSelector->SetNodePredicate(segPredicate);

    if (m_AutoSelectionEnabled)
    {
      // hide all image nodes to later show only the automatically selected ones
      mitk::DataStorage::SetOfObjects::ConstPointer imageNodes =
        this->GetDataStorage()->GetSubset(m_ReferencePredicate);
      for (mitk::DataStorage::SetOfObjects::const_iterator iter = imageNodes->begin(); iter != imageNodes->end(); ++iter)
      {
        (*iter)->SetVisibility(false);
      }
    }
    m_ReferenceNode->SetVisibility(true);
  }

  this->UpdateGUI();
}

void QmitkMultiLabelSegmentationView::OnSegmentationSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  m_ToolManager->ActivateTool(-1);

  if (nodes.empty())
  {
    m_WorkingNode = nullptr;
    m_ToolManager->SetWorkingData(m_WorkingNode);
    this->UpdateGUI();
    return;
  }

  if (m_ReferenceNode.IsNull())
  {
    this->UpdateGUI();
    return;
  }

  mitk::Image::ConstPointer referenceImage = dynamic_cast<mitk::Image *>(m_ReferenceNode->GetData());
  if (referenceImage.IsNull())
  {
    this->UpdateGUI();
    return;
  }

  if (m_WorkingNode.IsNotNull())
  {
    this->OnLooseLabelSetConnection();
  }

  m_WorkingNode = nodes.first();
  m_ToolManager->SetWorkingData(m_WorkingNode);

  if (m_WorkingNode.IsNotNull())
  {
    if (m_AutoSelectionEnabled)
    {
      // hide all segmentation nodes to later show only the automatically selected ones
      mitk::DataStorage::SetOfObjects::ConstPointer segmentationNodes =
        this->GetDataStorage()->GetSubset(m_SegmentationPredicate);
      for (mitk::DataStorage::SetOfObjects::const_iterator iter = segmentationNodes->begin(); iter != segmentationNodes->end(); ++iter)
      {
        (*iter)->SetVisibility(false);
      }
    }
    m_WorkingNode->SetVisibility(true);

    this->OnEstablishLabelSetConnection();
    m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();

    this->InitializeRenderWindows(referenceImage->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false);
  }

  this->UpdateGUI();
}

void QmitkMultiLabelSegmentationView::OnVisibilityShortcutActivated()
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  bool isVisible = false;
  workingNode->GetBoolProperty("visible", isVisible);
  workingNode->SetVisibility(!isVisible);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMultiLabelSegmentationView::OnLabelToggleShortcutActivated()
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (nullptr == workingImage)
  {
    return;
  }

  WaitCursorOn();
  workingImage->GetActiveLabelSet()->SetNextActiveLabel();
  workingImage->Modified();
  WaitCursorOff();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMultiLabelSegmentationView::OnManualTool2DSelected(int id)
{
  this->ResetMouseCursor();
  mitk::StatusBar::GetInstance()->DisplayText("");

  if (id >= 0)
  {
    std::string text = "Active Tool: \"";
    text += m_ToolManager->GetToolById(id)->GetName();
    text += "\"";
    mitk::StatusBar::GetInstance()->DisplayText(text.c_str());

    us::ModuleResource resource = m_ToolManager->GetToolById(id)->GetCursorIconResource();
    this->SetMouseCursor(resource, 0, 0);
  }
}

void QmitkMultiLabelSegmentationView::OnNewLabel()
{
  m_ToolManager->ActivateTool(-1);

  if (m_ReferenceNode.IsNull())
  {
    QMessageBox::information(
      m_Parent, "New Segmentation", "Please load and select an image before starting some action.");
    return;
  }

  mitk::Image::ConstPointer referenceImage = dynamic_cast<mitk::Image *>(m_ReferenceNode->GetData());
  if (referenceImage.IsNull())
  {
    QMessageBox::information(
      m_Parent, "New segmentation", "Reference data needs to be an image in order to create a new segmentation.");
    return;
  }

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode)
  {
    QMessageBox::information(
      m_Parent, "New segmentation", "Please load and select a multilabel segmentation before starting some action.");
    return;
  }

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (!workingImage)
  {
    QMessageBox::information(
      m_Parent, "New segmentation", "Please load and select a multilabel segmentation before starting some action.");
    return;
  }

  // ask about the name and organ type of the new segmentation
  auto dialog = new QmitkNewSegmentationDialog(m_Parent);
  QStringList organColors = mitk::OrganNamesHandling::GetDefaultOrganColorString();
  dialog->SetSuggestionList(organColors);
  dialog->setWindowTitle("New Label");

  int dialogReturnValue = dialog->exec();
  if (dialogReturnValue == QDialog::Rejected)
  {
    return;
  }

  QString segName = dialog->GetSegmentationName();
  if (segName.isEmpty())
  {
    segName = "Unnamed";
  }
  workingImage->GetActiveLabelSet()->AddLabel(segName.toStdString(), dialog->GetColor());

  UpdateGUI();
  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();

  this->InitializeRenderWindows(referenceImage->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false);
}

void QmitkMultiLabelSegmentationView::OnSavePreset()
{
  QmitkAbstractNodeSelectionWidget::NodeList nodes;
  nodes.append(m_WorkingNode);

  QmitkCreateMultiLabelPresetAction action;
  action.Run(nodes);
}

void QmitkMultiLabelSegmentationView::OnLoadPreset()
{
  QmitkAbstractNodeSelectionWidget::NodeList nodes;
  nodes.append(m_WorkingNode);

  QmitkLoadMultiLabelPresetAction action;
  action.Run(nodes);
}

void QmitkMultiLabelSegmentationView::OnShowLabelTable(bool value)
{
  if (value)
    m_Controls.m_LabelSetWidget->show();
  else
    m_Controls.m_LabelSetWidget->hide();
}

void QmitkMultiLabelSegmentationView::OnNewSegmentationSession()
{
  mitk::DataNode::Pointer referenceNode = m_ToolManager->GetReferenceData(0);
  if (referenceNode.IsNull())
  {
    MITK_ERROR << "'Create new segmentation' button should never be clickable unless a reference image is selected.";
    return;
  }

  mitk::Image::ConstPointer referenceImage = dynamic_cast<mitk::Image *>(referenceNode->GetData());
  if (referenceImage.IsNull())
  {
    QMessageBox::information(
      m_Parent, "New segmentation", "Please load and select an image before starting some action.");
    return;
  }

  if (referenceImage->GetDimension() <= 1)
  {
    QMessageBox::information(m_Parent, "New segmentation", "Segmentation is currently not supported for 2D images");
    return;
  }

  const auto currentTimePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  unsigned int imageTimeStep = 0;
  if (referenceImage->GetTimeGeometry()->IsValidTimePoint(currentTimePoint))
  {
    imageTimeStep = referenceImage->GetTimeGeometry()->TimePointToTimeStep(currentTimePoint);
  }

  auto segTemplateImage = referenceImage;
  if (referenceImage->GetDimension() > 3)
  {
    auto result = QMessageBox::question(m_Parent,
      tr("Create a static or dynamic segmentation?"),
      tr("The selected image has multiple time steps.\n\nDo you want to create a static "
        "segmentation that is identical for all time steps or do you want to create a "
        "dynamic segmentation to segment individual time steps?"),
      tr("Create static segmentation"), tr("Create dynamic segmentation"),
      QString(), 0, 0);
    if (result == 0)
    {
      auto selector = mitk::ImageTimeSelector::New();
      selector->SetInput(referenceImage);
      selector->SetTimeNr(0);
      selector->Update();

      const auto refTimeGeometry = referenceImage->GetTimeGeometry();
      auto newTimeGeometry = mitk::ProportionalTimeGeometry::New();
      newTimeGeometry->SetFirstTimePoint(refTimeGeometry->GetMinimumTimePoint());
      newTimeGeometry->SetStepDuration(refTimeGeometry->GetMaximumTimePoint() - refTimeGeometry->GetMinimumTimePoint());

      mitk::Image::Pointer newImage = selector->GetOutput();
      newTimeGeometry->SetTimeStepGeometry(referenceImage->GetGeometry(imageTimeStep), 0);
      newImage->SetTimeGeometry(newTimeGeometry);
      segTemplateImage = newImage;
    }
  }

  QString newName = QString::fromStdString(referenceNode->GetName());
  newName.append("-labels");

  bool ok = false;
  newName = QInputDialog::getText(m_Parent, "New segmentation", "New name:", QLineEdit::Normal, newName, &ok);

  if (!ok)
  {
    return;
  }
  if (newName.isEmpty())
  {
    newName = "Unnamed";
  }

  this->WaitCursorOn();

  mitk::LabelSetImage::Pointer workingImage = mitk::LabelSetImage::New();
  try
  {
    workingImage->Initialize(segTemplateImage);
  }
  catch (mitk::Exception& e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(m_Parent, tr("New segmentation"), tr("Could not create a new segmentation.\n"));
    return;
  }

  this->WaitCursorOff();

  mitk::DataNode::Pointer workingNode = mitk::DataNode::New();
  workingNode->SetData(workingImage);
  workingNode->SetName(newName.toStdString());

  workingImage->GetExteriorLabel()->SetProperty("name.parent", mitk::StringProperty::New(referenceNode->GetName().c_str()));
  workingImage->GetExteriorLabel()->SetProperty("name.image", mitk::StringProperty::New(newName.toStdString().c_str()));

  this->GetDataStorage()->Add(workingNode, referenceNode);

  m_Controls.m_WorkingNodeSelector->SetCurrentSelectedNode(workingNode);

  OnNewLabel();
}

void QmitkMultiLabelSegmentationView::OnGoToLabel(const mitk::Point3D& pos)
{
  if (m_RenderWindowPart)
    m_RenderWindowPart->SetSelectedPosition(pos);
}

void QmitkMultiLabelSegmentationView::OnResetView()
{
  if (m_RenderWindowPart)
    m_RenderWindowPart->ForceImmediateUpdate();
}

void QmitkMultiLabelSegmentationView::OnAddLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (nullptr == workingImage)
  {
    return;
  }

  QString question = "Do you really want to add a layer to the current segmentation session?";
  QMessageBox::StandardButton answerButton = QMessageBox::question(
    m_Controls.m_LabelSetWidget, "Add layer", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton != QMessageBox::Yes) return;

  try
  {
    WaitCursorOn();
    workingImage->AddLayer();
    WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(
      m_Controls.m_LabelSetWidget, "Add Layer", "Could not add a new layer. See error log for details.\n");
    return;
  }

  OnNewLabel();
}

void QmitkMultiLabelSegmentationView::OnDeleteLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (nullptr == workingImage)
  {
    return;
  }

  if (workingImage->GetNumberOfLayers() < 2)
    return;

  QString question = "Do you really want to delete the current layer?";

  QMessageBox::StandardButton answerButton = QMessageBox::question(
    m_Controls.m_LabelSetWidget, "Delete layer", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton != QMessageBox::Yes)
  {
    return;
  }

  try
  {
    this->WaitCursorOn();
    workingImage->RemoveLayer();
    this->WaitCursorOff();
  }
  catch (mitk::Exception& e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(m_Controls.m_LabelSetWidget, "Delete Layer",
                             "Could not delete the currently active layer. See error log for details.\n");
    return;
  }

  UpdateGUI();
  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
}

void QmitkMultiLabelSegmentationView::OnPreviousLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (nullptr == workingImage)
  {
    return;
  }

  OnChangeLayer(workingImage->GetActiveLayer() - 1);
}

void QmitkMultiLabelSegmentationView::OnNextLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (nullptr == workingImage)
  {
    return;
  }

  OnChangeLayer(workingImage->GetActiveLayer() + 1);
}

void QmitkMultiLabelSegmentationView::OnChangeLayer(int layer)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (nullptr == workingImage)
  {
    return;
  }

  this->WaitCursorOn();
  workingImage->SetActiveLayer(layer);
  this->WaitCursorOff();

  UpdateGUI();
  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
}

void QmitkMultiLabelSegmentationView::OnLockExteriorToggled(bool checked)
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (nullptr == workingImage)
  {
    return;
  }

  workingImage->GetLabel(0)->SetLocked(checked);
}

void QmitkMultiLabelSegmentationView::OnInterpolationSelectionChanged(int index)
{
  if (index == 1)
  {
    m_Controls.m_SurfaceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);//OnToggleWidgetActivation(false);
    m_Controls.m_swInterpolation->setCurrentIndex(0);
    m_Controls.m_swInterpolation->show();
  }
  else if (index == 2)
  {
    m_Controls.m_SliceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);
    m_Controls.m_swInterpolation->setCurrentIndex(1);
    m_Controls.m_swInterpolation->show();
  }
  else
  {
    m_Controls.m_SurfaceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);
    m_Controls.m_SliceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);
    m_Controls.m_swInterpolation->setCurrentIndex(2);
    m_Controls.m_swInterpolation->hide();
  }
}

/**********************************************************************/
/* private                                                            */
/**********************************************************************/
void QmitkMultiLabelSegmentationView::CreateQtPartControl(QWidget *parent)
{
  m_Parent = parent;

  m_Controls.setupUi(parent);

  m_Controls.m_tbSavePreset->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-save.svg")));
  m_Controls.m_tbLoadPreset->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-open.svg")));

  // *------------------------
  // * Shortcuts
  // *------------------------
  QShortcut* visibilityShortcut = new QShortcut(QKeySequence("CTRL+H"), parent);
  connect(visibilityShortcut, &QShortcut::activated, this, &QmitkMultiLabelSegmentationView::OnVisibilityShortcutActivated);
  QShortcut* labelToggleShortcut = new QShortcut(QKeySequence("CTRL+L"), parent);
  connect(labelToggleShortcut, &QShortcut::activated, this, &QmitkMultiLabelSegmentationView::OnLabelToggleShortcutActivated);

  // *------------------------
  // * DATA SELECTION WIDGETS
  // *------------------------

  m_Controls.m_ReferenceNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_ReferenceNodeSelector->SetNodePredicate(m_ReferencePredicate);
  m_Controls.m_ReferenceNodeSelector->SetInvalidInfo("Select an image");
  m_Controls.m_ReferenceNodeSelector->SetPopUpTitel("Select an image");
  m_Controls.m_ReferenceNodeSelector->SetPopUpHint("Select an image that should be used to define the geometry and bounds of the segmentation.");

  m_Controls.m_WorkingNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_WorkingNodeSelector->SetNodePredicate(m_SegmentationPredicate);
  m_Controls.m_WorkingNodeSelector->SetInvalidInfo("Select a segmentation");
  m_Controls.m_WorkingNodeSelector->SetPopUpTitel("Select a segmentation");
  m_Controls.m_WorkingNodeSelector->SetPopUpHint("Select a segmentation that should be modified. Only segmentation with the same geometry and within the bounds of the reference image are selected.");

  connect(m_Controls.m_ReferenceNodeSelector,
    &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkMultiLabelSegmentationView::OnReferenceSelectionChanged);
  connect(m_Controls.m_WorkingNodeSelector,
    &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkMultiLabelSegmentationView::OnSegmentationSelectionChanged);

  // *------------------------
  // * ToolManager
  // *------------------------

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager(mitk::ToolManagerProvider::MULTILABEL_SEGMENTATION);
  m_ToolManager->SetDataStorage(*(this->GetDataStorage()));
  m_ToolManager->InitializeTools();
  m_Controls.m_ManualToolSelectionBox2D->SetToolManager(*m_ToolManager);
  m_Controls.m_ManualToolSelectionBox3D->SetToolManager(*m_ToolManager);

  // *------------------------
  // * LabelSetWidget
  // *------------------------

  m_Controls.m_LabelSetWidget->SetDataStorage(this->GetDataStorage());
  m_Controls.m_LabelSetWidget->SetOrganColors(mitk::OrganNamesHandling::GetDefaultOrganColorString());
  m_Controls.m_LabelSetWidget->hide();

  // *------------------------
  // * Interpolation
  // *------------------------

  m_Controls.m_SurfaceBasedInterpolatorWidget->SetDataStorage(*(this->GetDataStorage()));
  m_Controls.m_SliceBasedInterpolatorWidget->SetDataStorage(*(this->GetDataStorage()));
  connect(m_Controls.m_cbInterpolation, QOverload<int>::of(&QComboBox::activated), this, &QmitkMultiLabelSegmentationView::OnInterpolationSelectionChanged);

  m_Controls.m_cbInterpolation->setCurrentIndex(0);
  m_Controls.m_swInterpolation->hide();

  m_Controls.m_gbInterpolation->hide(); // See T27436

  QString segTools2D = tr("Add Subtract Fill Erase Paint Wipe 'Region Growing' 'Live Wire'");
  QString segTools3D = tr("Threshold");

  std::regex extSegTool2DRegEx("SegTool2D$");
  std::regex extSegTool3DRegEx("SegTool3D$");

  auto tools = m_ToolManager->GetTools();
  for (const auto &tool : tools)
  {
    if (std::regex_search(tool->GetNameOfClass(), extSegTool2DRegEx))
    {
      segTools2D.append(QString(" '%1'").arg(tool->GetName()));
    }
    else if (std::regex_search(tool->GetNameOfClass(), extSegTool3DRegEx))
    {
      segTools3D.append(QString(" '%1'").arg(tool->GetName()));
    }
  }

  // *------------------------
  // * ToolSelection 2D
  // *------------------------

  m_Controls.m_ManualToolSelectionBox2D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox2D->SetToolGUIArea(m_Controls.m_ManualToolGUIContainer2D);
  m_Controls.m_ManualToolSelectionBox2D->SetDisplayedToolGroups(segTools2D.toStdString());
  m_Controls.m_ManualToolSelectionBox2D->SetLayoutColumns(3);
  m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode(
    QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);
  connect(m_Controls.m_ManualToolSelectionBox2D, &QmitkToolSelectionBox::ToolSelected,
    this, &QmitkMultiLabelSegmentationView::OnManualTool2DSelected);

  // *------------------------
  // * ToolSelection 3D
  // *------------------------

  m_Controls.m_ManualToolSelectionBox3D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox3D->SetToolGUIArea(m_Controls.m_ManualToolGUIContainer3D);
  m_Controls.m_ManualToolSelectionBox3D->SetDisplayedToolGroups(segTools3D.toStdString()); // todo add : FastMarching3D RegionGrowing Watershed
  m_Controls.m_ManualToolSelectionBox3D->SetLayoutColumns(3);
  m_Controls.m_ManualToolSelectionBox3D->SetEnabledMode(
    QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);

  // *------------------------*
  // * Connect Buttons
  // *------------------------*

  connect(m_Controls.m_pbNewLabel, &QToolButton::clicked, this, &QmitkMultiLabelSegmentationView::OnNewLabel);
  connect(m_Controls.m_tbSavePreset, &QToolButton::clicked, this, &QmitkMultiLabelSegmentationView::OnSavePreset);
  connect(m_Controls.m_tbLoadPreset, &QToolButton::clicked, this, &QmitkMultiLabelSegmentationView::OnLoadPreset);
  connect(m_Controls.m_pbNewSegmentationSession, &QToolButton::clicked, this, &QmitkMultiLabelSegmentationView::OnNewSegmentationSession);
  connect(m_Controls.m_pbShowLabelTable, &QToolButton::toggled, this, &QmitkMultiLabelSegmentationView::OnShowLabelTable);

  // *------------------------*
  // * Connect LabelSetWidget
  // *------------------------*

  connect(m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::goToLabel,
    this, &QmitkMultiLabelSegmentationView::OnGoToLabel);
  connect(m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::resetView,
    this, &QmitkMultiLabelSegmentationView::OnResetView);

  // *------------------------*
  // * DATA SLECTION WIDGET
  // *------------------------*

  connect(m_Controls.m_btAddLayer, &QToolButton::clicked, this, &QmitkMultiLabelSegmentationView::OnAddLayer);
  connect(m_Controls.m_btDeleteLayer, &QToolButton::clicked, this, &QmitkMultiLabelSegmentationView::OnDeleteLayer);
  connect(m_Controls.m_btPreviousLayer, &QToolButton::clicked, this, &QmitkMultiLabelSegmentationView::OnPreviousLayer);
  connect(m_Controls.m_btNextLayer, &QToolButton::clicked, this, &QmitkMultiLabelSegmentationView::OnNextLayer);
  connect(m_Controls.m_btLockExterior, &QToolButton::toggled, this, &QmitkMultiLabelSegmentationView::OnLockExteriorToggled);
  connect(m_Controls.m_cbActiveLayer, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QmitkMultiLabelSegmentationView::OnChangeLayer);

  m_Controls.m_btAddLayer->setEnabled(false);
  m_Controls.m_btDeleteLayer->setEnabled(false);
  m_Controls.m_btNextLayer->setEnabled(false);
  m_Controls.m_btPreviousLayer->setEnabled(false);
  m_Controls.m_cbActiveLayer->setEnabled(false);

  m_Controls.m_pbNewLabel->setEnabled(false);
  m_Controls.m_btLockExterior->setEnabled(false);
  m_Controls.m_tbSavePreset->setEnabled(false);
  m_Controls.m_tbLoadPreset->setEnabled(false);
  m_Controls.m_pbShowLabelTable->setEnabled(false);

  // set callback function for already existing segmentation nodes
  mitk::DataStorage::SetOfObjects::ConstPointer allSegmentations = GetDataStorage()->GetSubset(m_SegmentationPredicate);
  for (mitk::DataStorage::SetOfObjects::const_iterator iter = allSegmentations->begin(); iter != allSegmentations->end(); ++iter)
  {
    mitk::DataNode* node = *iter;
    auto command = itk::SimpleMemberCommand<QmitkMultiLabelSegmentationView>::New();
    command->SetCallbackFunction(this, &QmitkMultiLabelSegmentationView::ValidateSelectionInput);
    m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode *, unsigned long>(
      node, node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));
  }

  auto command = itk::SimpleMemberCommand<QmitkMultiLabelSegmentationView>::New();
  command->SetCallbackFunction(this, &QmitkMultiLabelSegmentationView::ValidateSelectionInput);
  m_RenderingManagerObserverTag =
    mitk::RenderingManager::GetInstance()->AddObserver(mitk::RenderingManagerViewsInitializedEvent(), command);

  m_RenderWindowPart = this->GetRenderWindowPart();
  if (nullptr != m_RenderWindowPart)
  {
    this->RenderWindowPartActivated(m_RenderWindowPart);
  }

  // Make sure the GUI notices if appropriate data is already present on creation.
  // Should be done last, if everything else is configured because it triggers the autoselection of data.
  m_Controls.m_ReferenceNodeSelector->SetAutoSelectNewNodes(true);
  m_Controls.m_WorkingNodeSelector->SetAutoSelectNewNodes(true);

  this->UpdateGUI();
}

void QmitkMultiLabelSegmentationView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_RenderWindowPart != renderWindowPart)
  {
    m_RenderWindowPart = renderWindowPart;
  }

  if (m_Parent)
  {
    m_Parent->setEnabled(true);
  }

  // tell the interpolation about tool manager, data storage and render window part
  QList<mitk::SliceNavigationController *> controllers;
  controllers.push_back(m_RenderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
  controllers.push_back(m_RenderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
  controllers.push_back(m_RenderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
  m_Controls.m_SliceBasedInterpolatorWidget->SetSliceNavigationControllers(controllers);
}

void QmitkMultiLabelSegmentationView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  m_RenderWindowPart = nullptr;
  if (m_Parent)
  {
    m_Parent->setEnabled(false);
  }
}

void QmitkMultiLabelSegmentationView::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  bool slimView = prefs->GetBool("slim view", false);
  m_Controls.m_ManualToolSelectionBox2D->SetShowNames(!slimView);
  m_Controls.m_ManualToolSelectionBox3D->SetShowNames(!slimView);

  m_AutoSelectionEnabled = prefs->GetBool("auto selection", false);

  this->ApplyDisplayOptions();
}

void QmitkMultiLabelSegmentationView::NodeAdded(const mitk::DataNode* node)
{
  if (!m_SegmentationPredicate->CheckNode(node))
  {
    return;
  }

  auto command = itk::SimpleMemberCommand<QmitkMultiLabelSegmentationView>::New();
  command->SetCallbackFunction(this, &QmitkMultiLabelSegmentationView::ValidateSelectionInput);
  m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode *, unsigned long>(
    const_cast<mitk::DataNode *>(node), node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));

  ApplyDisplayOptions(const_cast<mitk::DataNode*>(node));
}

void QmitkMultiLabelSegmentationView::NodeRemoved(const mitk::DataNode* node)
{
  if (m_SegmentationPredicate->CheckNode(node))
  {
    // remove all possible contour markers of the segmentation
    mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers = this->GetDataStorage()->GetDerivations(
      node, mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true)));

    ctkPluginContext *context = mitk::PluginActivator::getContext();
    ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
    mitk::PlanePositionManagerService *service = context->getService<mitk::PlanePositionManagerService>(ppmRef);

    for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End(); ++it)
    {
      std::string nodeName = node->GetName();
      unsigned int t = nodeName.find_last_of(" ");
      unsigned int id = atof(nodeName.substr(t + 1).c_str()) - 1;

      service->RemovePlanePosition(id);

      this->GetDataStorage()->Remove(it->Value());
    }

    context->ungetService(ppmRef);
    service = nullptr;
  }

  mitk::DataNode* tempNode = const_cast<mitk::DataNode*>(node);
  //Remove observer if one was registered
  auto finding = m_WorkingDataObserverTags.find(tempNode);
  if (finding != m_WorkingDataObserverTags.end())
  {
    node->GetProperty("visible")->RemoveObserver(m_WorkingDataObserverTags[tempNode]);
    m_WorkingDataObserverTags.erase(tempNode);
  }
}

void QmitkMultiLabelSegmentationView::ApplyDisplayOptions()
{
  if (!m_Parent)
  {
    return;
  }

  mitk::DataNode::Pointer workingData = m_ToolManager->GetWorkingData(0);
  mitk::DataStorage::SetOfObjects::ConstPointer allImages = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);
  for (mitk::DataStorage::SetOfObjects::const_iterator iter = allImages->begin(); iter != allImages->end(); ++iter)
  {
    this->ApplyDisplayOptions(*iter);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMultiLabelSegmentationView::ApplyDisplayOptions(mitk::DataNode* node)
{
  if (nullptr == node)
  {
    return;
  }

  auto drawOutline = mitk::BoolProperty::New(GetPreferences()->GetBool("draw outline", true));
  auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(node->GetData());
  if (nullptr != labelSetImage)
  {
    // node is a multi label segmentation
    node->SetProperty("labelset.contour.active", drawOutline);
    // force render window update to show outline
    node->GetData()->Modified();
  }
  else if (nullptr != node->GetData())
  {
    // node is actually a 'single label' segmentation,
    // but its outline property can be set in the 'multi label' segmentation preference page as well
    bool isBinary = false;
    node->GetBoolProperty("binary", isBinary);
    if (isBinary)
    {
      node->SetProperty("outline binary", drawOutline);
      node->SetProperty("outline width", mitk::FloatProperty::New(2.0));
      // force render window update to show outline
      node->GetData()->Modified();
    }
  }
}

void QmitkMultiLabelSegmentationView::OnEstablishLabelSetConnection()
{
  if (m_WorkingNode.IsNull())
  {
    return;
  }

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
  if (nullptr == workingImage)
  {
    // node is a "single label" / "binary" image --> no label set
    return;
  }

  workingImage->GetActiveLabelSet()->AddLabelEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->RemoveLabelEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ModifyLabelEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->AllLabelsModifiedEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ActiveLabelEvent +=
    mitk::MessageDelegate1<QmitkLabelSetWidget, mitk::Label::PixelType>(m_Controls.m_LabelSetWidget,
                                                                        &QmitkLabelSetWidget::SelectLabelByPixelValue);

  // Removed in T27851 to have a chance to react to AfterChangeLayerEvent. Did it brake something?
  // workingImage->BeforeChangeLayerEvent += mitk::MessageDelegate<QmitkMultiLabelSegmentationView>(
  //   this, &QmitkMultiLabelSegmentationView::OnLooseLabelSetConnection);

  workingImage->AfterChangeLayerEvent += mitk::MessageDelegate<QmitkMultiLabelSegmentationView>(
    this, &QmitkMultiLabelSegmentationView::UpdateGUI);
}

void QmitkMultiLabelSegmentationView::OnLooseLabelSetConnection()
{
  if (m_WorkingNode.IsNull())
  {
    return;
  }

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
  if (nullptr == workingImage)
  {
    // data (type) was changed in-place, e.g. LabelSetImage -> (binary) image
    return;
  }

  // Reset LabelSetWidget Events
  workingImage->GetActiveLabelSet()->AddLabelEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->RemoveLabelEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ModifyLabelEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->AllLabelsModifiedEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ActiveLabelEvent -=
    mitk::MessageDelegate1<QmitkLabelSetWidget, mitk::Label::PixelType>(m_Controls.m_LabelSetWidget,
                                                                        &QmitkLabelSetWidget::SelectLabelByPixelValue);

  // Removed in T27851 to have a chance to react to AfterChangeLayerEvent. Did it brake something?
  // workingImage->BeforeChangeLayerEvent -= mitk::MessageDelegate<QmitkMultiLabelSegmentationView>(
  //   this, &QmitkMultiLabelSegmentationView::OnLooseLabelSetConnection);

  workingImage->AfterChangeLayerEvent -= mitk::MessageDelegate<QmitkMultiLabelSegmentationView>(
    this, &QmitkMultiLabelSegmentationView::UpdateGUI);
}

void QmitkMultiLabelSegmentationView::ResetMouseCursor()
{
  if (m_MouseCursorSet)
  {
    mitk::ApplicationCursor::GetInstance()->PopCursor();
    m_MouseCursorSet = false;
  }
}

void QmitkMultiLabelSegmentationView::SetMouseCursor(const us::ModuleResource& resource, int hotspotX, int hotspotY)
{
  // Remove previously set mouse cursor
  if (m_MouseCursorSet)
    this->ResetMouseCursor();

  if (resource)
  {
    us::ModuleResourceStream cursor(resource, std::ios::binary);
    mitk::ApplicationCursor::GetInstance()->PushCursor(cursor, hotspotX, hotspotY);
    m_MouseCursorSet = true;
  }
}

void QmitkMultiLabelSegmentationView::UpdateGUI()
{
  mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
  bool hasReferenceNode = referenceNode != nullptr;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  bool hasWorkingNode = workingNode != nullptr;

  m_Controls.m_pbNewSegmentationSession->setEnabled(false);
  m_Controls.m_pbNewLabel->setEnabled(false);
  m_Controls.m_gbInterpolation->setEnabled(false);
  m_Controls.m_SliceBasedInterpolatorWidget->setEnabled(false);
  m_Controls.m_SurfaceBasedInterpolatorWidget->setEnabled(false);
  m_Controls.m_LabelSetWidget->setEnabled(false);
  m_Controls.m_btAddLayer->setEnabled(false);
  m_Controls.m_btDeleteLayer->setEnabled(false);
  m_Controls.m_cbActiveLayer->setEnabled(false);
  m_Controls.m_btPreviousLayer->setEnabled(false);
  m_Controls.m_btNextLayer->setEnabled(false);
  m_Controls.m_btLockExterior->setChecked(false);
  m_Controls.m_btLockExterior->setEnabled(false);
  m_Controls.m_tbSavePreset->setEnabled(false);
  m_Controls.m_tbLoadPreset->setEnabled(false);
  m_Controls.m_pbShowLabelTable->setChecked(false);
  m_Controls.m_pbShowLabelTable->setEnabled(false);

  if (hasReferenceNode)
  {
    m_Controls.m_pbNewSegmentationSession->setEnabled(true);
  }

  if (hasWorkingNode)
  {
    m_Controls.m_pbNewLabel->setEnabled(true);
    m_Controls.m_btLockExterior->setEnabled(true);
    m_Controls.m_tbSavePreset->setEnabled(true);
    m_Controls.m_tbLoadPreset->setEnabled(true);
    m_Controls.m_pbShowLabelTable->setEnabled(true);
    m_Controls.m_LabelSetWidget->setEnabled(true);
    m_Controls.m_btAddLayer->setEnabled(true);

    m_Controls.m_cbActiveLayer->blockSignals(true);
    m_Controls.m_cbActiveLayer->clear();

    mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
    if (nullptr != workingImage)
    {
      int numberOfLayers = workingImage->GetNumberOfLayers();
      for (unsigned int lidx = 0; lidx < workingImage->GetNumberOfLayers(); ++lidx)
      {
        m_Controls.m_cbActiveLayer->addItem(QString::number(lidx));
      }

      int activeLayer = workingImage->GetActiveLayer();
      m_Controls.m_cbActiveLayer->setCurrentIndex(activeLayer);
      m_Controls.m_cbActiveLayer->blockSignals(false);

      m_Controls.m_cbActiveLayer->setEnabled(numberOfLayers > 1);
      m_Controls.m_btDeleteLayer->setEnabled(numberOfLayers > 1);
      m_Controls.m_btPreviousLayer->setEnabled(activeLayer > 0);
      m_Controls.m_btNextLayer->setEnabled(activeLayer != numberOfLayers - 1);

      m_Controls.m_btLockExterior->setChecked(workingImage->GetLabel(0, activeLayer)->GetLocked());
      m_Controls.m_pbShowLabelTable->setChecked(workingImage->GetNumberOfLabels() > 1 /*1st is exterior*/);
    }
  }

  if (hasWorkingNode && hasReferenceNode)
  {
    m_Controls.m_gbInterpolation->setEnabled(true);
    m_Controls.m_SliceBasedInterpolatorWidget->setEnabled(true);
    m_Controls.m_SurfaceBasedInterpolatorWidget->setEnabled(true);

    int layer = -1;
    referenceNode->GetIntProperty("layer", layer);
    workingNode->SetIntProperty("layer", layer + 1);
  }

  this->ValidateSelectionInput();
}

void QmitkMultiLabelSegmentationView::ValidateSelectionInput()
{
  this->UpdateWarningLabel("");

  // the argument is actually not used
  // enable status depends on the tool manager selection
  m_Controls.m_ManualToolSelectionBox2D->setEnabled(false);
  m_Controls.m_ManualToolSelectionBox3D->setEnabled(false);

  mitk::DataNode* referenceNode = m_Controls.m_ReferenceNodeSelector->GetSelectedNode();
  mitk::DataNode* workingNode = m_Controls.m_WorkingNodeSelector->GetSelectedNode();

  if (nullptr == referenceNode)
  {
    return;
  }

  if (nullptr == workingNode)
  {
    return;
  }

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
  auto referenceNodeIsVisible = renderWindowPart &&
    referenceNode->IsVisible(renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer());
  if (!referenceNodeIsVisible)
  {
    this->UpdateWarningLabel(tr("The selected reference image is currently not visible!"));
    return;
  }

  auto workingNodeIsVisible = renderWindowPart &&
    workingNode->IsVisible(renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer());
  if (!workingNodeIsVisible)
  {
    this->UpdateWarningLabel(tr("The selected segmentation is currently not visible!"));
    return;
  }

  /*
  * Here we check whether the geometry of the selected segmentation image is aligned with the worldgeometry.
  * At the moment it is not supported to use a geometry different from the selected image for reslicing.
  * For further information see Bug 16063
  */
  const mitk::BaseGeometry *workingNodeGeo = workingNode->GetData()->GetGeometry();
  const mitk::BaseGeometry *worldGeo =
    renderWindowPart->GetQmitkRenderWindow("3d")->GetSliceNavigationController()->GetCurrentGeometry3D();
  if (nullptr != workingNodeGeo && nullptr != worldGeo)
  {
    if (mitk::Equal(*workingNodeGeo->GetBoundingBox(), *worldGeo->GetBoundingBox(), mitk::eps, true))
    {
      m_ToolManager->SetReferenceData(referenceNode);
      m_ToolManager->SetWorkingData(workingNode);
      m_Controls.m_ManualToolSelectionBox2D->setEnabled(true);
      m_Controls.m_ManualToolSelectionBox3D->setEnabled(true);
      return;
    }
  }

  m_ToolManager->SetReferenceData(referenceNode);
  m_ToolManager->SetWorkingData(nullptr);
  this->UpdateWarningLabel(tr("Please perform a reinit on the segmentation image!"));
}

void QmitkMultiLabelSegmentationView::UpdateWarningLabel(QString text)
{
  if (text.size() == 0)
  {
    m_Controls.lblSegmentationWarnings->hide();
  }
  else
  {
    m_Controls.lblSegmentationWarnings->show();
  }
  m_Controls.lblSegmentationWarnings->setText("<font color=\"red\">" + text + "</font>");
}
