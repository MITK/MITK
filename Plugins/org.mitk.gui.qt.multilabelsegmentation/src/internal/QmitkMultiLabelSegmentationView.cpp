/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiLabelSegmentationView.h"

// blueberry
#include <berryConstants.h>
#include <berryIWorkbenchPage.h>

// mitk
#include "mitkApplicationCursor.h"
#include "mitkLabelSetImage.h"
#include "mitkStatusBar.h"
#include "mitkToolManagerProvider.h"
#include "mitkInteractionEventObserver.h"
#include "mitkPlanePositionManager.h"
#include "mitkPluginActivator.h"
#include "mitkSegTool2D.h"
#include "mitkImageTimeSelector.h"
#include "mitkNodePredicateSubGeometry.h"

// Qmitk
#include <QmitkStyleManager.h>
#include "QmitkNewSegmentationDialog.h"
#include "QmitkRenderWindow.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"
#include "QmitkCreateMultiLabelPresetAction.h"
#include "QmitkLoadMultiLabelPresetAction.h"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

// Qt
#include <QDateTime>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>

#include <itksys/SystemTools.hxx>

#include <regex>

const std::string QmitkMultiLabelSegmentationView::VIEW_ID = "org.mitk.views.multilabelsegmentation";

QmitkMultiLabelSegmentationView::QmitkMultiLabelSegmentationView()
  : m_Parent(nullptr),
    m_IRenderWindowPart(nullptr),
    m_ToolManager(nullptr),
    m_ReferenceNode(nullptr),
    m_WorkingNode(nullptr),
    m_AutoSelectionEnabled(false),
    m_MouseCursorSet(false)
{
  m_SegmentationPredicate = mitk::NodePredicateAnd::New();
  m_SegmentationPredicate->AddPredicate(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::NodePredicateProperty::Pointer isBinary =
    mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer isMask = mitk::NodePredicateAnd::New(isBinary, isImage);

  mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
  mitk::NodePredicateDataType::Pointer isOdf = mitk::NodePredicateDataType::New("OdfImage");
  auto isSegment = mitk::NodePredicateDataType::New("Segment");

  mitk::NodePredicateOr::Pointer validImages = mitk::NodePredicateOr::New();
  validImages->AddPredicate(mitk::NodePredicateAnd::New(isImage, mitk::NodePredicateNot::New(isSegment)));
  validImages->AddPredicate(isDwi);
  validImages->AddPredicate(isDti);
  validImages->AddPredicate(isOdf);

  m_ReferencePredicate = mitk::NodePredicateAnd::New();
  m_ReferencePredicate->AddPredicate(validImages);
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(m_SegmentationPredicate));
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(isMask));
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));
}

QmitkMultiLabelSegmentationView::~QmitkMultiLabelSegmentationView()
{
  // Loose LabelSetConnections
  OnLooseLabelSetConnection();
}

void QmitkMultiLabelSegmentationView::CreateQtPartControl(QWidget *parent)
{
  // setup the basic GUI of this view
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

  m_Controls.m_ReferenceNodeSelector->SetNodePredicate(m_ReferencePredicate);
  m_Controls.m_ReferenceNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_ReferenceNodeSelector->SetInvalidInfo("Select an image");
  m_Controls.m_ReferenceNodeSelector->SetPopUpTitel("Select an image");
  m_Controls.m_ReferenceNodeSelector->SetPopUpHint("Select an image that should be used to define the geometry and bounds of the segmentation.");


  m_Controls.m_WorkingNodeSelector->SetNodePredicate(m_SegmentationPredicate);
  m_Controls.m_WorkingNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_WorkingNodeSelector->SetInvalidInfo("Select a segmentation");
  m_Controls.m_WorkingNodeSelector->SetPopUpTitel("Select a segmentation");
  m_Controls.m_WorkingNodeSelector->SetPopUpHint("Select a segmentation that should be modified. Only segmentation with the same geometry and within the bounds of the reference image are selected.");

  connect(m_Controls.m_ReferenceNodeSelector,
          &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
          this,&QmitkMultiLabelSegmentationView::OnReferenceSelectionChanged);
  connect(m_Controls.m_WorkingNodeSelector,
          &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
          this,&QmitkMultiLabelSegmentationView::OnSegmentationSelectionChanged);

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
  connect(m_Controls.m_cbInterpolation, SIGNAL(activated(int)), this, SLOT(OnInterpolationSelectionChanged(int)));

  m_Controls.m_cbInterpolation->setCurrentIndex(0);
  m_Controls.m_swInterpolation->hide();

  m_Controls.m_gbInterpolation->hide(); // See T27436

  QString segTools2D = tr("Add Subtract Fill Erase Paint Wipe 'Region Growing' FastMarching2D 'Live Wire'");
  QString segTools3D = tr("Threshold 'Two Thresholds' 'Auto Threshold' 'Multiple Otsu'");

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
  m_Controls.m_ManualToolSelectionBox2D->SetDisplayedToolGroups(segTools2D.toStdString()); // todo: "Correction
  // 'Live Wire'"
  m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode(
    QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);
  connect(m_Controls.m_ManualToolSelectionBox2D, SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)));

  // *------------------------
  // * ToolSelection 3D
  // *------------------------

  m_Controls.m_ManualToolSelectionBox3D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox3D->SetToolGUIArea(m_Controls.m_ManualToolGUIContainer3D);
  m_Controls.m_ManualToolSelectionBox3D->SetDisplayedToolGroups(segTools3D.toStdString()); // todo add : FastMarching3D RegionGrowing Watershed
  m_Controls.m_ManualToolSelectionBox3D->SetLayoutColumns(2);
  m_Controls.m_ManualToolSelectionBox3D->SetEnabledMode(
    QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);

  // *------------------------*
  // * Connect PushButtons (pb)
  // *------------------------*

  connect(m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()));
  connect(m_Controls.m_tbSavePreset, SIGNAL(clicked()), this, SLOT(OnSavePreset()));
  connect(m_Controls.m_tbLoadPreset, SIGNAL(clicked()), this, SLOT(OnLoadPreset()));
  connect(m_Controls.m_pbNewSegmentationSession, SIGNAL(clicked()), this, SLOT(OnNewSegmentationSession()));
  connect(m_Controls.m_pbShowLabelTable, SIGNAL(toggled(bool)), this, SLOT(OnShowLabelTable(bool)));

  // *------------------------*
  // * Connect LabelSetWidget
  // *------------------------*

  connect(m_Controls.m_LabelSetWidget,
          SIGNAL(goToLabel(const mitk::Point3D &)),
          this,
          SLOT(OnGoToLabel(const mitk::Point3D &)));
  connect(m_Controls.m_LabelSetWidget, SIGNAL(resetView()), this, SLOT(OnResetView()));

  // *------------------------*
  // * DATA SLECTION WIDGET
  // *------------------------*
  m_IRenderWindowPart = this->GetRenderWindowPart();
  if (m_IRenderWindowPart)
  {
    QList<mitk::SliceNavigationController *> controllers;
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls.m_SliceBasedInterpolatorWidget->SetSliceNavigationControllers(controllers);
  }

  //  this->InitializeListeners();

  connect(m_Controls.m_btAddLayer, SIGNAL(clicked()), this, SLOT(OnAddLayer()));
  connect(m_Controls.m_btDeleteLayer, SIGNAL(clicked()), this, SLOT(OnDeleteLayer()));
  connect(m_Controls.m_btPreviousLayer, SIGNAL(clicked()), this, SLOT(OnPreviousLayer()));
  connect(m_Controls.m_btNextLayer, SIGNAL(clicked()), this, SLOT(OnNextLayer()));
  connect(m_Controls.m_btLockExterior, SIGNAL(toggled(bool)), this, SLOT(OnLockExteriorToggled(bool)));
  connect(m_Controls.m_cbActiveLayer, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChangeLayer(int)));

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

  // Make sure the GUI notices if appropriate data is already present on creation
  m_Controls.m_ReferenceNodeSelector->SetAutoSelectNewNodes(true);
  m_Controls.m_WorkingNodeSelector->SetAutoSelectNewNodes(true);
}

void QmitkMultiLabelSegmentationView::Activated()
{
  m_ToolManager->SetReferenceData(m_Controls.m_ReferenceNodeSelector->GetSelectedNode());
  m_ToolManager->SetWorkingData(m_Controls.m_WorkingNodeSelector->GetSelectedNode());
}

void QmitkMultiLabelSegmentationView::Deactivated()
{
  // Not yet implemented
}

void QmitkMultiLabelSegmentationView::Visible()
{
  // Not yet implemented
}

void QmitkMultiLabelSegmentationView::Hidden()
{
  // Not yet implemented
}

int QmitkMultiLabelSegmentationView::GetSizeFlags(bool width)
{
  if (!width)
  {
    return berry::Constants::MIN | berry::Constants::MAX | berry::Constants::FILL;
  }
  else
  {
    return 0;
  }
}

int QmitkMultiLabelSegmentationView::ComputePreferredSize(bool width,
                                                          int /*availableParallel*/,
                                                          int /*availablePerpendicular*/,
                                                          int preferredResult)
{
  if (width == false)
  {
    return 100;
  }
  else
  {
    return preferredResult;
  }
}

/************************************************************************/
/* protected slots                                                      */
/************************************************************************/
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

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

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
      m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  if (nullptr == m_ReferenceNode->GetData())
  {
    QMessageBox::information(
      m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode)
  {
    QMessageBox::information(
      m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (!workingImage)
  {
    QMessageBox::information(
      m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog(m_Parent);
  dialog->SetSuggestionList(mitk::OrganNamesHandling::GetDefaultOrganColorString());
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

  UpdateControls();
  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();

  this->ReinitializeViews();
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
  mitk::DataNode *referenceNode = m_Controls.m_ReferenceNodeSelector->GetSelectedNode();

  if (!referenceNode)
  {
    QMessageBox::information(
      m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  m_ToolManager->ActivateTool(-1);

  mitk::Image::ConstPointer referenceImage = dynamic_cast<mitk::Image*>(referenceNode->GetData());
  assert(referenceImage);

  const auto currentTimePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  unsigned int imageTimeStep = 0;
  if (referenceImage->GetTimeGeometry()->IsValidTimePoint(currentTimePoint))
  {
    imageTimeStep = referenceImage->GetTimeGeometry()->TimePointToTimeStep(currentTimePoint);
  }

  auto segTemplateImage = referenceImage;
  if (referenceImage->GetDimension() > 3)
  {
    auto result = QMessageBox::question(m_Parent, tr("Create a static or dynamic segmentation?"), tr("The patient image has multiple time steps.\n\nDo you want to create a static segmentation that is identical for all time steps or do you want to create a dynamic segmentation to segment individual time steps?"), tr("Create static segmentation"), tr("Create dynamic segmentation"), QString(), 0, 0);
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
  newName = QInputDialog::getText(m_Parent, "New Segmentation Session", "New name:", QLineEdit::Normal, newName, &ok);

  if (!ok)
  {
    return;
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
    QMessageBox::information(m_Parent, "New Segmentation Session", "Could not create a new segmentation session.\n");
    return;
  }

  this->WaitCursorOff();

  mitk::DataNode::Pointer workingNode = mitk::DataNode::New();
  workingNode->SetData(workingImage);
  workingNode->SetName(newName.toStdString());

  workingImage->GetExteriorLabel()->SetProperty("name.parent", mitk::StringProperty::New(referenceNode->GetName().c_str()));
  workingImage->GetExteriorLabel()->SetProperty("name.image", mitk::StringProperty::New(newName.toStdString().c_str()));

  if (!GetDataStorage()->Exists(workingNode))
  {
    GetDataStorage()->Add(workingNode, referenceNode);
  }

  m_Controls.m_WorkingNodeSelector->SetCurrentSelectedNode(workingNode);

  OnNewLabel();
}

void QmitkMultiLabelSegmentationView::OnGoToLabel(const mitk::Point3D& pos)
{
  if (m_IRenderWindowPart)
    m_IRenderWindowPart->SetSelectedPosition(pos);
}

void QmitkMultiLabelSegmentationView::OnResetView()
{
  if (m_IRenderWindowPart)
    m_IRenderWindowPart->ForceImmediateUpdate();
}

void QmitkMultiLabelSegmentationView::OnAddLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

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

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

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

  UpdateControls();
  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
}

void QmitkMultiLabelSegmentationView::OnPreviousLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);
  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  assert(workingImage);

  OnChangeLayer(workingImage->GetActiveLayer() - 1);
}

void QmitkMultiLabelSegmentationView::OnNextLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);
  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  assert(workingImage);

  OnChangeLayer(workingImage->GetActiveLayer() + 1);
}

void QmitkMultiLabelSegmentationView::OnChangeLayer(int layer)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  assert(workingImage);

  this->WaitCursorOn();
  workingImage->SetActiveLayer(layer);
  this->WaitCursorOff();

  UpdateControls();
  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
}

void QmitkMultiLabelSegmentationView::OnDeactivateActiveTool()
{
  m_ToolManager->ActivateTool(-1);
}

void QmitkMultiLabelSegmentationView::OnLockExteriorToggled(bool checked)
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  workingImage->GetLabel(0)->SetLocked(checked);
}

void QmitkMultiLabelSegmentationView::OnReferenceSelectionChanged(QList<mitk::DataNode::Pointer> /*nodes*/)
{
  m_ToolManager->ActivateTool(-1);

  auto refNode = m_Controls.m_ReferenceNodeSelector->GetSelectedNode();
  m_ReferenceNode = refNode;
  m_ToolManager->SetReferenceData(m_ReferenceNode);

  if (m_ReferenceNode.IsNotNull())
  {
    auto segPredicate = mitk::NodePredicateAnd::New(m_SegmentationPredicate.GetPointer(), mitk::NodePredicateSubGeometry::New(refNode->GetData()->GetGeometry()));

    m_Controls.m_WorkingNodeSelector->SetNodePredicate(segPredicate);

    if (m_AutoSelectionEnabled)
    {
      // hide all image nodes to later show only the automatically selected ones
      mitk::DataStorage::SetOfObjects::ConstPointer patientNodes = GetDataStorage()->GetSubset(m_ReferencePredicate);
      for (mitk::DataStorage::SetOfObjects::const_iterator iter = patientNodes->begin(); iter != patientNodes->end(); ++iter)
      {
        (*iter)->SetVisibility(false);
      }
    }
    m_ReferenceNode->SetVisibility(true);
  }

  UpdateControls();
}

void QmitkMultiLabelSegmentationView::OnSegmentationSelectionChanged(QList<mitk::DataNode::Pointer> /*nodes*/)
{
  m_ToolManager->ActivateTool(-1);

  if (m_WorkingNode.IsNotNull())
    OnLooseLabelSetConnection();

  m_WorkingNode = m_Controls.m_WorkingNodeSelector->GetSelectedNode();
  m_ToolManager->SetWorkingData(m_WorkingNode);
  if (m_WorkingNode.IsNotNull())
  {
    OnEstablishLabelSetConnection();

    if (m_AutoSelectionEnabled)
    {
      // hide all segmentation nodes to later show only the automatically selected ones
      mitk::DataStorage::SetOfObjects::ConstPointer segmentationNodes = GetDataStorage()->GetSubset(m_SegmentationPredicate);
      for (mitk::DataStorage::SetOfObjects::const_iterator iter = segmentationNodes->begin(); iter != segmentationNodes->end(); ++iter)
      {
        (*iter)->SetVisibility(false);
      }
    }
    m_WorkingNode->SetVisibility(true);
  }

  UpdateControls();
  if (m_WorkingNode.IsNotNull())
  {
    m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
    this->ReinitializeViews();
  }
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

/************************************************************************/
/* protected                                                            */
/************************************************************************/
void QmitkMultiLabelSegmentationView::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  m_AutoSelectionEnabled = prefs->GetBool("auto selection", false);

  mitk::BoolProperty::Pointer drawOutline = mitk::BoolProperty::New(prefs->GetBool("draw outline", true));
  mitk::LabelSetImage* labelSetImage;
  mitk::DataNode* segmentation;

  // iterate all segmentations (binary (single label) and LabelSetImages)
  mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateOr::Pointer allSegmentationsPredicate = mitk::NodePredicateOr::New(isBinaryPredicate, m_SegmentationPredicate);
  mitk::DataStorage::SetOfObjects::ConstPointer allSegmentations = GetDataStorage()->GetSubset(allSegmentationsPredicate);

  for (mitk::DataStorage::SetOfObjects::const_iterator it = allSegmentations->begin(); it != allSegmentations->end(); ++it)
  {
    segmentation = *it;
    labelSetImage = dynamic_cast<mitk::LabelSetImage*>(segmentation->GetData());
    if (nullptr != labelSetImage)
    {
      // segmentation node is a multi label segmentation
      segmentation->SetProperty("labelset.contour.active", drawOutline);
      //segmentation->SetProperty("opacity", mitk::FloatProperty::New(drawOutline->GetValue() ? 1.0f : 0.3f));
      // force render window update to show outline
      segmentation->GetData()->Modified();
    }
    else if (nullptr != segmentation->GetData())
    {
      // node is actually a 'single label' segmentation,
      // but its outline property can be set in the 'multi label' segmentation preference page as well
      bool isBinary = false;
      segmentation->GetBoolProperty("binary", isBinary);
      if (isBinary)
      {
        segmentation->SetProperty("outline binary", drawOutline);
        segmentation->SetProperty("outline width", mitk::FloatProperty::New(2.0));
        //segmentation->SetProperty("opacity", mitk::FloatProperty::New(drawOutline->GetValue() ? 1.0f : 0.3f));
        // force render window update to show outline
        segmentation->GetData()->Modified();
      }
    }
  }
}

void QmitkMultiLabelSegmentationView::NodeRemoved(const mitk::DataNode *node)
{
  bool isHelperObject(false);
  node->GetBoolProperty("helper object", isHelperObject);
  if (isHelperObject)
  {
    return;
  }

  if (m_ReferenceNode.IsNotNull() && dynamic_cast<mitk::LabelSetImage *>(node->GetData()))
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
}

void QmitkMultiLabelSegmentationView::OnEstablishLabelSetConnection()
{
  if (m_WorkingNode.IsNull())
  {
    return;
  }
  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(m_WorkingNode->GetData());
  assert(workingImage);

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
    this, &QmitkMultiLabelSegmentationView::UpdateControls);
}

void QmitkMultiLabelSegmentationView::OnLooseLabelSetConnection()
{
  if (m_WorkingNode.IsNull())
    return;

  auto* workingImage = dynamic_cast<mitk::LabelSetImage *>(m_WorkingNode->GetData());

  if (nullptr == workingImage)
    return; // data (type) was changed in-place, e.g. LabelSetImage -> Image

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
    this, &QmitkMultiLabelSegmentationView::UpdateControls);
}

void QmitkMultiLabelSegmentationView::SetFocus()
{
}

void QmitkMultiLabelSegmentationView::UpdateControls()
{
  mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
  bool hasReferenceNode = referenceNode != nullptr;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  bool hasValidWorkingNode = workingNode != nullptr;

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

  m_Controls.m_ManualToolSelectionBox3D->SetEnabledMode(QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);
  m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode(QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);

  if (hasValidWorkingNode)
  {
    // TODO adapt tool manager so that this check is done there, e.g. convenience function
    mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
    hasValidWorkingNode = workingImage != nullptr;
    if (hasValidWorkingNode)
    {
      m_Controls.m_pbNewLabel->setEnabled(true);
      m_Controls.m_btLockExterior->setEnabled(true);
      m_Controls.m_tbSavePreset->setEnabled(true);
      m_Controls.m_tbLoadPreset->setEnabled(true);
      m_Controls.m_pbShowLabelTable->setEnabled(true);
      m_Controls.m_gbInterpolation->setEnabled(true);
      m_Controls.m_SliceBasedInterpolatorWidget->setEnabled(true);
      m_Controls.m_SurfaceBasedInterpolatorWidget->setEnabled(true);
      m_Controls.m_LabelSetWidget->setEnabled(true);
      m_Controls.m_btAddLayer->setEnabled(true);

      int activeLayer = workingImage->GetActiveLayer();
      int numberOfLayers = workingImage->GetNumberOfLayers();

      m_Controls.m_cbActiveLayer->blockSignals(true);
      m_Controls.m_cbActiveLayer->clear();
      for (unsigned int lidx = 0; lidx < workingImage->GetNumberOfLayers(); ++lidx)
      {
        m_Controls.m_cbActiveLayer->addItem(QString::number(lidx));
      }
      m_Controls.m_cbActiveLayer->setCurrentIndex(activeLayer);
      m_Controls.m_cbActiveLayer->blockSignals(false);

      m_Controls.m_cbActiveLayer->setEnabled(numberOfLayers > 1);
      m_Controls.m_btDeleteLayer->setEnabled(numberOfLayers > 1);
      m_Controls.m_btPreviousLayer->setEnabled(activeLayer > 0);
      m_Controls.m_btNextLayer->setEnabled(activeLayer != numberOfLayers - 1);

      m_Controls.m_btLockExterior->setChecked(workingImage->GetLabel(0, activeLayer)->GetLocked());
      m_Controls.m_pbShowLabelTable->setChecked(workingImage->GetNumberOfLabels() > 1 /*1st is exterior*/);

      //MLI TODO
      //m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode(QmitkToolSelectionBox::EnabledWithWorkingDataVisible);
    }
  }

  if (hasValidWorkingNode && hasReferenceNode)
  {
    int layer = -1;
    referenceNode->GetIntProperty("layer", layer);
    workingNode->SetIntProperty("layer", layer + 1);
  }

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}

void QmitkMultiLabelSegmentationView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_IRenderWindowPart != renderWindowPart)
  {
    m_IRenderWindowPart = renderWindowPart;
    m_Parent->setEnabled(true);

    QList<mitk::SliceNavigationController*> controllers;
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls.m_SliceBasedInterpolatorWidget->SetSliceNavigationControllers(controllers);
  }
}

void QmitkMultiLabelSegmentationView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  m_ToolManager->ActivateTool(-1);
  m_IRenderWindowPart = nullptr;
  m_Parent->setEnabled(false);
}

void QmitkMultiLabelSegmentationView::ResetMouseCursor()
{
  if (m_MouseCursorSet)
  {
    mitk::ApplicationCursor::GetInstance()->PopCursor();
    m_MouseCursorSet = false;
  }
}

void QmitkMultiLabelSegmentationView::SetMouseCursor(const us::ModuleResource resource, int hotspotX, int hotspotY)
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

void QmitkMultiLabelSegmentationView::InitializeListeners()
{
  if (m_Interactor.IsNull())
  {
    us::Module* module = us::GetModuleContext()->GetModule();
    std::vector<us::ModuleResource> resources = module->FindResources("/", "*", true);
    for (std::vector<us::ModuleResource>::iterator iter = resources.begin(); iter != resources.end(); ++iter)
    {
      MITK_INFO << iter->GetResourcePath();
    }

    m_Interactor = mitk::SegmentationInteractor::New();
    if (!m_Interactor->LoadStateMachine("SegmentationInteraction.xml", module))
    {
      MITK_WARN << "Error loading state machine";
    }

    if (!m_Interactor->SetEventConfig("ConfigSegmentation.xml", module))
    {
      MITK_WARN << "Error loading state machine configuration";
    }

    // Register as listener via micro services
    us::ServiceProperties props;
    props["name"] = std::string("SegmentationInteraction");
    m_ServiceRegistration =
      us::GetModuleContext()->RegisterService<mitk::InteractionEventObserver>(m_Interactor.GetPointer(), props);
  }
}

void QmitkMultiLabelSegmentationView::ReinitializeViews() const
{
  if (m_ReferenceNode.IsNotNull() && nullptr != m_ReferenceNode->GetData())
  {
    const auto currentTimePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
    unsigned int imageTimeStep = 0;
    if (m_ReferenceNode->GetData()->GetTimeGeometry()->IsValidTimePoint(currentTimePoint))
    {
      imageTimeStep = m_ReferenceNode->GetData()->GetTimeGeometry()->TimePointToTimeStep(currentTimePoint);
    }

    mitk::RenderingManager::GetInstance()->InitializeViews(m_ReferenceNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
    mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->SetPos(imageTimeStep);
  }
}
