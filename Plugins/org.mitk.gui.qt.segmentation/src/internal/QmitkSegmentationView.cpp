/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationView.h"
#include "mitkPluginActivator.h"

// blueberry
#include <berryIWorkbenchPage.h>

// mitk
#include <mitkApplicationCursor.h>
#include <mitkBaseApplication.h>
#include <mitkBaseRendererHelper.h>
#include <mitkCameraController.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkMultiLabelIOHelper.h>
#include <mitkManualPlacementAnnotationRenderer.h>
#include <mitkNodePredicateSubGeometry.h>
#include <mitkSegmentationObjectFactory.h>
#include <mitkSegTool2D.h>
#include <mitkStatusBar.h>
#include <mitkToolManagerProvider.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkWorkbenchUtil.h>
#include <mitkIPreferences.h>

// Qmitk
#include <QmitkRenderWindow.h>
#include <QmitkStaticDynamicSegmentationDialog.h>
#include <QmitkNewSegmentationDialog.h>
#include <QmitkMultiLabelManager.h>

// us
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

// Qt
#include <QMessageBox>
#include <QShortcut>
#include <QDir>

// vtk
#include <vtkQImageToImageSource.h>

#include <regex>

namespace
{
  QList<QmitkRenderWindow*> Get2DWindows(const QList<QmitkRenderWindow*> allWindows)
  {
    QList<QmitkRenderWindow*> all2DWindows;
    for (auto* window : allWindows)
    {
      if (window->GetRenderer()->GetMapperID() == mitk::BaseRenderer::Standard2D)
      {
        all2DWindows.append(window);
      }
    }
    return all2DWindows;
  }
}

const std::string QmitkSegmentationView::VIEW_ID = "org.mitk.views.segmentation";

QmitkSegmentationView::QmitkSegmentationView()
  : m_Parent(nullptr)
  , m_Controls(nullptr)
  , m_RenderWindowPart(nullptr)
  , m_ToolManager(nullptr)
  , m_ReferenceNode(nullptr)
  , m_WorkingNode(nullptr)
  , m_DrawOutline(true)
  , m_SelectionMode(false)
  , m_MouseCursorSet(false)
  , m_DefaultLabelNaming(true)
  , m_SelectionChangeIsAlreadyBeingHandled(false)
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

  m_SegmentationPredicate = mitk::NodePredicateAnd::New();
  m_SegmentationPredicate->AddPredicate(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

  m_ReferencePredicate = mitk::NodePredicateAnd::New();
  m_ReferencePredicate->AddPredicate(validImages);
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(m_SegmentationPredicate));
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));
}

QmitkSegmentationView::~QmitkSegmentationView()
{
  if (nullptr != m_Controls)
  {
    // deactivate all tools
    m_ToolManager->ActivateTool(-1);

    // removing all observers from working data
    for (NodeTagMapType::iterator dataIter = m_WorkingDataObserverTags.begin(); dataIter != m_WorkingDataObserverTags.end(); ++dataIter)
    {
      (*dataIter).first->GetProperty("visible")->RemoveObserver((*dataIter).second);
    }
    m_WorkingDataObserverTags.clear();

    this->RemoveObserversFromWorkingImage();

    // removing all observers from reference data
    for (NodeTagMapType::iterator dataIter = m_ReferenceDataObserverTags.begin(); dataIter != m_ReferenceDataObserverTags.end(); ++dataIter)
    {
      (*dataIter).first->GetProperty("visible")->RemoveObserver((*dataIter).second);
    }
    m_ReferenceDataObserverTags.clear();

    mitk::RenderingManager::GetInstance()->RemoveObserver(m_RenderingManagerObserverTag);

    ctkPluginContext* context = mitk::PluginActivator::getContext();
    ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
    mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);
    service->RemoveAllPlanePositions();
    context->ungetService(ppmRef);

    m_ToolManager->SetReferenceData(nullptr);
    m_ToolManager->SetWorkingData(nullptr);
  }

  m_ToolManager->ActiveToolChanged -=
    mitk::MessageDelegate<Self>(this, &Self::ActiveToolChanged);

  delete m_Controls;
}

/**********************************************************************/
/* private Q_SLOTS                                                    */
/**********************************************************************/
void QmitkSegmentationView::OnReferenceSelectionChanged(QList<mitk::DataNode::Pointer>)
{
  this->OnAnySelectionChanged();
}

void QmitkSegmentationView::OnSegmentationSelectionChanged(QList<mitk::DataNode::Pointer>)
{
  this->OnAnySelectionChanged();
}

void QmitkSegmentationView::OnAnySelectionChanged()
{
  // When only a segmentation has been selected and the method is then called by a reference image selection,
  // the already selected segmentation may not match the geometry predicate of the new reference image anymore.
  // This will trigger a recursive call of this method further below. While it would be resolved gracefully, we
  // can spare the extra call with an early-out. The original call of this method will handle the segmentation
  // selection change afterwards anyway.

  if (m_SelectionChangeIsAlreadyBeingHandled)
    return;

  auto selectedReferenceNode = m_Controls->referenceNodeSelector->GetSelectedNode();
  bool referenceNodeChanged = false;

  m_ToolManager->ActivateTool(-1);

  if (m_ReferenceNode != selectedReferenceNode)
  {
    referenceNodeChanged = true;

    // Remove visibility observer for the current reference node
    if (m_ReferenceDataObserverTags.find(m_ReferenceNode) != m_ReferenceDataObserverTags.end())
    {
      m_ReferenceNode->GetProperty("visible")->RemoveObserver(m_ReferenceDataObserverTags[m_ReferenceNode]);
      m_ReferenceDataObserverTags.erase(m_ReferenceNode);
    }

    // Set new reference node
    m_ReferenceNode = selectedReferenceNode;
    m_ToolManager->SetReferenceData(m_ReferenceNode);

    // Prepare for a potential recursive call when changing node predicates of the working node selector
    m_SelectionChangeIsAlreadyBeingHandled = true;

    if (m_ReferenceNode.IsNull())
    {
      // Without a reference image, allow all segmentations to be selected
      m_Controls->workingNodeSelector->SetNodePredicate(m_SegmentationPredicate);
      m_SelectionChangeIsAlreadyBeingHandled = false;
    }
    else
    {
      // With a reference image, only allow segmentations that fit the geometry of the reference image to be selected.
      m_Controls->workingNodeSelector->SetNodePredicate(mitk::NodePredicateAnd::New(
        mitk::NodePredicateSubGeometry::New(m_ReferenceNode->GetData()->GetGeometry()),
        m_SegmentationPredicate.GetPointer()));

      m_SelectionChangeIsAlreadyBeingHandled = false;

      this->ApplySelectionModeOnReferenceNode();

      // Add visibility observer for the new reference node
      auto command = itk::SimpleMemberCommand<Self>::New();
      command->SetCallbackFunction(this, &Self::ValidateSelectionInput);

      m_ReferenceDataObserverTags[m_ReferenceNode] =
        m_ReferenceNode->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command);
    }
  }

  auto selectedWorkingNode = m_Controls->workingNodeSelector->GetSelectedNode();
  bool workingNodeChanged = false;

  if (m_WorkingNode != selectedWorkingNode)
  {
    workingNodeChanged = true;

    this->RemoveObserversFromWorkingImage();

    // Remove visibility observer for the current working node
    if (m_WorkingDataObserverTags.find(m_WorkingNode) != m_WorkingDataObserverTags.end())
    {
      m_WorkingNode->GetProperty("visible")->RemoveObserver(m_WorkingDataObserverTags[m_WorkingNode]);
      m_WorkingDataObserverTags.erase(m_WorkingNode);
    }

    // Set new working node
    m_WorkingNode = selectedWorkingNode;
    m_ToolManager->SetWorkingData(m_WorkingNode);

    if (m_WorkingNode.IsNotNull())
    {
      this->ApplySelectionModeOnWorkingNode();

      // Add visibility observer for the new segmentation node
      auto command = itk::SimpleMemberCommand<Self>::New();
      command->SetCallbackFunction(this, &Self::ValidateSelectionInput);

      m_WorkingDataObserverTags[m_WorkingNode] =
        m_WorkingNode->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command);

      this->AddObserversToWorkingImage();
    }
  }

  // Reset camera if any selection changed but only if both reference node and working node are set
  if ((referenceNodeChanged || workingNodeChanged) && (m_ReferenceNode.IsNotNull() && m_WorkingNode.IsNotNull()))
  {
    if (nullptr != m_RenderWindowPart)
    {
      m_RenderWindowPart->InitializeViews(m_ReferenceNode->GetData()->GetTimeGeometry(), false);
    }
  }

  this->UpdateGUI();
}

void QmitkSegmentationView::OnLabelAdded(mitk::LabelSetImage::LabelValueType)
{
  this->ValidateSelectionInput();
}

void QmitkSegmentationView::OnLabelRemoved(mitk::LabelSetImage::LabelValueType)
{
  this->ValidateSelectionInput();
}

void QmitkSegmentationView::OnGroupRemoved(mitk::LabelSetImage::GroupIndexType)
{
  this->ValidateSelectionInput();
}

mitk::LabelSetImage* QmitkSegmentationView::GetWorkingImage()
{
  if (m_WorkingNode.IsNull())
    return nullptr;

  return dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
}

void QmitkSegmentationView::AddObserversToWorkingImage()
{
  auto* workingImage = this->GetWorkingImage();

  if (workingImage != nullptr)
  {
    workingImage->AddLabelAddedListener(mitk::MessageDelegate1<Self, mitk::LabelSetImage::LabelValueType>(this, &Self::OnLabelAdded));
    workingImage->AddLabelRemovedListener(mitk::MessageDelegate1<Self, mitk::LabelSetImage::LabelValueType>(this, &Self::OnLabelRemoved));
    workingImage->AddGroupRemovedListener(mitk::MessageDelegate1<Self, mitk::LabelSetImage::GroupIndexType>(this, &Self::OnGroupRemoved));
  }
}

void QmitkSegmentationView::RemoveObserversFromWorkingImage()
{
  auto* workingImage = this->GetWorkingImage();

  if (workingImage != nullptr)
  {
    workingImage->RemoveLabelAddedListener(mitk::MessageDelegate1<Self, mitk::LabelSetImage::LabelValueType>(this, &Self::OnLabelAdded));
    workingImage->RemoveLabelRemovedListener(mitk::MessageDelegate1<Self, mitk::LabelSetImage::LabelValueType>(this, &Self::OnLabelRemoved));
    workingImage->RemoveGroupRemovedListener(mitk::MessageDelegate1<Self, mitk::LabelSetImage::GroupIndexType>(this, &Self::OnGroupRemoved));
  }
}

void QmitkSegmentationView::OnVisibilityShortcutActivated()
{
  if (m_WorkingNode.IsNull())
  {
    return;
  }

  bool isVisible = false;
  m_WorkingNode->GetBoolProperty("visible", isVisible);
  m_WorkingNode->SetVisibility(!isVisible);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::OnLabelToggleShortcutActivated()
{
  if (m_WorkingNode.IsNull())
  {
    return;
  }

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
  if (nullptr == workingImage)
  {
    return;
  }

  this->WaitCursorOn();
  workingImage->GetActiveLabelSet()->SetNextActiveLabel();
  workingImage->Modified();
  this->WaitCursorOff();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::OnNewSegmentation()
{
  m_ToolManager->ActivateTool(-1);

  if (m_ReferenceNode.IsNull())
  {
    MITK_ERROR << "'Create new segmentation' button should never be clickable unless a reference image is selected.";
    return;
  }

  mitk::Image::ConstPointer referenceImage = dynamic_cast<mitk::Image*>(m_ReferenceNode->GetData());
  if (referenceImage.IsNull())
  {
    QMessageBox::information(
      m_Parent, "New segmentation", "Please load and select an image before starting some action.");
    return;
  }

  if (referenceImage->GetDimension() <= 1)
  {
    QMessageBox::information(
      m_Parent, "New segmentation", "Segmentation is currently not supported for 2D images");
    return;
  }

  auto segTemplateImage = referenceImage;
  if (referenceImage->GetDimension() > 3)
  {
    QmitkStaticDynamicSegmentationDialog dialog(m_Parent);
    dialog.SetReferenceImage(referenceImage.GetPointer());
    dialog.exec();
    segTemplateImage = dialog.GetSegmentationTemplate();
  }

  mitk::DataNode::Pointer newSegmentationNode;
  try
  {
    this->WaitCursorOn();
    newSegmentationNode = mitk::LabelSetImageHelper::CreateNewSegmentationNode(m_ReferenceNode, segTemplateImage);
    this->WaitCursorOff();
  }
  catch (mitk::Exception& e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::warning(m_Parent, "New segmentation", "Could not create a new segmentation.");
    return;
  }

  auto newLabelSetImage = dynamic_cast<mitk::LabelSetImage*>(newSegmentationNode->GetData());
  if (nullptr == newLabelSetImage)
  {
    // something went wrong
    return;
  }

  const auto labelSetPreset = this->GetDefaultLabelSetPreset();

  if (labelSetPreset.empty() || !mitk::MultiLabelIOHelper::LoadLabelSetImagePreset(labelSetPreset, newLabelSetImage))
  {
    auto newLabel = mitk::LabelSetImageHelper::CreateNewLabel(newLabelSetImage);

    if (!m_DefaultLabelNaming)
      QmitkNewSegmentationDialog::DoRenameLabel(newLabel, nullptr, m_Parent);

    newLabelSetImage->GetActiveLabelSet()->AddLabel(newLabel);
  }

  if (!this->GetDataStorage()->Exists(newSegmentationNode))
  {
    this->GetDataStorage()->Add(newSegmentationNode, m_ReferenceNode);
  }

  if (m_ToolManager->GetWorkingData(0))
  {
    m_ToolManager->GetWorkingData(0)->SetSelected(false);
  }

  newSegmentationNode->SetSelected(true);
  m_Controls->workingNodeSelector->SetCurrentSelectedNode(newSegmentationNode);
}

std::string QmitkSegmentationView::GetDefaultLabelSetPreset() const
{
  auto labelSetPreset = mitk::BaseApplication::instance().config().getString(mitk::BaseApplication::ARG_SEGMENTATION_LABELSET_PRESET.toStdString(), "");

  if (labelSetPreset.empty())
    labelSetPreset = m_LabelSetPresetPreference.toStdString();

  return labelSetPreset;
}

void QmitkSegmentationView::OnManualTool2DSelected(int id)
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

void QmitkSegmentationView::OnShowMarkerNodes(bool state)
{
  mitk::SegTool2D::Pointer manualSegmentationTool;

  unsigned int numberOfExistingTools = m_ToolManager->GetTools().size();

  for (unsigned int i = 0; i < numberOfExistingTools; i++)
  {
    manualSegmentationTool = dynamic_cast<mitk::SegTool2D*>(m_ToolManager->GetToolById(i));
    if (nullptr == manualSegmentationTool)
    {
      continue;
    }

    manualSegmentationTool->SetShowMarkerNodes(state);
  }
}

void QmitkSegmentationView::OnCurrentLabelSelectionChanged(QmitkMultiLabelManager::LabelValueVectorType labels)
{
  auto segmentation = this->GetCurrentSegmentation();

  const auto labelValue = labels.front();
  const auto groupID = segmentation->GetGroupIndexOfLabel(labelValue);
  if (groupID != segmentation->GetActiveLayer()) segmentation->SetActiveLayer(groupID);
  if (labelValue != segmentation->GetActiveLabelSet()->GetActiveLabel()->GetValue()) segmentation->GetActiveLabelSet()->SetActiveLabel(labelValue);

  segmentation->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::OnGoToLabel(mitk::LabelSetImage::LabelValueType /*label*/, const mitk::Point3D& pos)
{
  if (m_RenderWindowPart)
  {
    m_RenderWindowPart->SetSelectedPosition(pos);
  }
}

void QmitkSegmentationView::OnLabelRenameRequested(mitk::Label* label, bool rename) const
{
  auto segmentation = this->GetCurrentSegmentation();

  if (rename)
  {
    QmitkNewSegmentationDialog::DoRenameLabel(label, segmentation, this->m_Parent, QmitkNewSegmentationDialog::Mode::RenameLabel);
    return;
  }

  QmitkNewSegmentationDialog::DoRenameLabel(label, nullptr, this->m_Parent, QmitkNewSegmentationDialog::Mode::NewLabel);
}

mitk::LabelSetImage* QmitkSegmentationView::GetCurrentSegmentation() const
{
  auto workingNode = m_Controls->workingNodeSelector->GetSelectedNode();
  if (workingNode.IsNull()) mitkThrow() << "Segmentation view is in an invalid state. Working node is null, but a label selection change has been triggered.";

  auto segmentation = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (nullptr == segmentation) mitkThrow() << "Segmentation view is in an invalid state. Working node contains no segmentation, but a label selection change has been triggered.";

  return segmentation;
}

/**********************************************************************/
/* private                                                            */
/**********************************************************************/
void QmitkSegmentationView::CreateQtPartControl(QWidget* parent)
{
   m_Parent = parent;

   m_Controls = new Ui::QmitkSegmentationViewControls;
   m_Controls->setupUi(parent);

   // *------------------------
   // * SHORTCUTS
   // *------------------------
   QShortcut* visibilityShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key::Key_H), parent);
   connect(visibilityShortcut, &QShortcut::activated, this, &Self::OnVisibilityShortcutActivated);
   QShortcut* labelToggleShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key::Key_L, Qt::CTRL | Qt::Key::Key_I), parent);
   connect(labelToggleShortcut, &QShortcut::activated, this, &Self::OnLabelToggleShortcutActivated);

   // *------------------------
   // * DATA SELECTION WIDGETS
   // *------------------------
   m_Controls->referenceNodeSelector->SetDataStorage(GetDataStorage());
   m_Controls->referenceNodeSelector->SetNodePredicate(m_ReferencePredicate);
   m_Controls->referenceNodeSelector->SetInvalidInfo("Select an image");
   m_Controls->referenceNodeSelector->SetPopUpTitel("Select an image");
   m_Controls->referenceNodeSelector->SetPopUpHint("Select an image that should be used to define the geometry and bounds of the segmentation.");

   m_Controls->workingNodeSelector->SetDataStorage(GetDataStorage());
   m_Controls->workingNodeSelector->SetNodePredicate(m_SegmentationPredicate);
   m_Controls->workingNodeSelector->SetInvalidInfo("Select a segmentation");
   m_Controls->workingNodeSelector->SetPopUpTitel("Select a segmentation");
   m_Controls->workingNodeSelector->SetPopUpHint("Select a segmentation that should be modified. Only segmentation with the same geometry and within the bounds of the reference image are selected.");

   connect(m_Controls->referenceNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
           this, &Self::OnReferenceSelectionChanged);
   connect(m_Controls->workingNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
           this, &Self::OnSegmentationSelectionChanged);

   // *------------------------
   // * TOOLMANAGER
   // *------------------------
   m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
   m_ToolManager->SetDataStorage(*(this->GetDataStorage()));
   m_ToolManager->InitializeTools();

   QString segTools2D = tr("Add Subtract Lasso Fill Erase Close Paint Wipe 'Region Growing' 'Live Wire'");
   QString segTools3D = tr("Threshold 'UL Threshold' Otsu 'Region Growing 3D' Picking GrowCut TotalSegmentator");

#ifdef __linux__
   segTools3D.append(" nnUNet"); // plugin not enabled for MacOS / Windows
#endif
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

   // setup 2D tools
   m_Controls->toolSelectionBox2D->SetToolManager(*m_ToolManager);
   m_Controls->toolSelectionBox2D->SetGenerateAccelerators(true);
   m_Controls->toolSelectionBox2D->SetToolGUIArea(m_Controls->toolGUIArea2D);
   m_Controls->toolSelectionBox2D->SetDisplayedToolGroups(segTools2D.toStdString());
   connect(m_Controls->toolSelectionBox2D, &QmitkToolSelectionBox::ToolSelected,
           this, &Self::OnManualTool2DSelected);

   // setup 3D Tools
   m_Controls->toolSelectionBox3D->SetToolManager(*m_ToolManager);
   m_Controls->toolSelectionBox3D->SetGenerateAccelerators(true);
   m_Controls->toolSelectionBox3D->SetToolGUIArea(m_Controls->toolGUIArea3D);
   m_Controls->toolSelectionBox3D->SetDisplayedToolGroups(segTools3D.toStdString());

   m_Controls->slicesInterpolator->SetDataStorage(this->GetDataStorage());

   // create general signal / slot connections
   connect(m_Controls->newSegmentationButton, &QToolButton::clicked, this, &Self::OnNewSegmentation);

   connect(m_Controls->slicesInterpolator, &QmitkSlicesInterpolator::SignalShowMarkerNodes, this, &Self::OnShowMarkerNodes);

   connect(m_Controls->multiLabelWidget, &QmitkMultiLabelManager::CurrentSelectionChanged, this, &Self::OnCurrentLabelSelectionChanged);
   connect(m_Controls->multiLabelWidget, &QmitkMultiLabelManager::GoToLabel, this, &Self::OnGoToLabel);
   connect(m_Controls->multiLabelWidget, &QmitkMultiLabelManager::LabelRenameRequested, this, &Self::OnLabelRenameRequested);

   auto command = itk::SimpleMemberCommand<Self>::New();
   command->SetCallbackFunction(this, &Self::ValidateSelectionInput);
   m_RenderingManagerObserverTag =
     mitk::RenderingManager::GetInstance()->AddObserver(mitk::RenderingManagerViewsInitializedEvent(), command);

   m_RenderWindowPart = this->GetRenderWindowPart();
   if (nullptr != m_RenderWindowPart)
   {
     this->RenderWindowPartActivated(m_RenderWindowPart);
   }

   // Make sure the GUI notices if appropriate data is already present on creation.
   // Should be done last, if everything else is configured because it triggers the autoselection of data.
   m_Controls->referenceNodeSelector->SetAutoSelectNewNodes(true);
   m_Controls->workingNodeSelector->SetAutoSelectNewNodes(true);

   this->UpdateGUI();
}

void QmitkSegmentationView::ActiveToolChanged()
{
  if (nullptr == m_RenderWindowPart)
  {
    return;
  }

  mitk::TimeGeometry* interactionReferenceGeometry = nullptr;
  auto activeTool = m_ToolManager->GetActiveTool();
  if (nullptr != activeTool && m_ReferenceNode.IsNotNull())
  {
    mitk::Image::ConstPointer referenceImage = dynamic_cast<mitk::Image *>(m_ReferenceNode->GetData());
    if (referenceImage.IsNotNull())
    {
      // tool activated, reference image available: set reference geometry
      interactionReferenceGeometry = m_ReferenceNode->GetData()->GetTimeGeometry();
    }
  }

  // set the interaction reference geometry for the render window part (might be nullptr)
  m_RenderWindowPart->SetInteractionReferenceGeometry(interactionReferenceGeometry);
}

void QmitkSegmentationView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_RenderWindowPart != renderWindowPart)
  {
    m_RenderWindowPart = renderWindowPart;
  }

  if (nullptr != m_Parent)
  {
    m_Parent->setEnabled(true);
  }

  if (nullptr == m_Controls)
  {
    return;
  }

  if (nullptr != m_RenderWindowPart)
  {
    auto all2DWindows = Get2DWindows(m_RenderWindowPart->GetQmitkRenderWindows().values());
    m_Controls->slicesInterpolator->Initialize(m_ToolManager, all2DWindows);

    if (!m_RenderWindowPart->HasCoupledRenderWindows())
    {
      // react if the active tool changed, only if a render window part with decoupled render windows is used
      m_ToolManager->ActiveToolChanged +=
        mitk::MessageDelegate<Self>(this, &Self::ActiveToolChanged);
    }
  }
}

void QmitkSegmentationView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  m_RenderWindowPart = nullptr;
  if (nullptr != m_Parent)
  {
    m_Parent->setEnabled(false);
  }

  // remove message-connection to make sure no message is processed if no render window part is available
  m_ToolManager->ActiveToolChanged -=
    mitk::MessageDelegate<Self>(this, &Self::ActiveToolChanged);

  m_Controls->slicesInterpolator->Uninitialize();
}

void QmitkSegmentationView::RenderWindowPartInputChanged(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  if (nullptr == m_RenderWindowPart)
  {
    return;
  }

  m_Controls->slicesInterpolator->Uninitialize();
  auto all2DWindows = Get2DWindows(m_RenderWindowPart->GetQmitkRenderWindows().values());
  m_Controls->slicesInterpolator->Initialize(m_ToolManager, all2DWindows);
}

void QmitkSegmentationView::OnPreferencesChanged(const mitk::IPreferences* prefs)
{
  auto labelSuggestions = mitk::BaseApplication::instance().config().getString(mitk::BaseApplication::ARG_SEGMENTATION_LABEL_SUGGESTIONS.toStdString(), "");

  m_DefaultLabelNaming = labelSuggestions.empty()
    ? prefs->GetBool("default label naming", true)
    : false; // No default label naming when label suggestions are enforced via command-line argument

  if (nullptr != m_Controls)
  {
    m_Controls->multiLabelWidget->SetDefaultLabelNaming(m_DefaultLabelNaming);

    bool compactView = prefs->GetBool("compact view", false);
    int numberOfColumns = compactView ? 6 : 4;

    m_Controls->toolSelectionBox2D->SetLayoutColumns(numberOfColumns);
    m_Controls->toolSelectionBox2D->SetShowNames(!compactView);

    m_Controls->toolSelectionBox3D->SetLayoutColumns(numberOfColumns);
    m_Controls->toolSelectionBox3D->SetShowNames(!compactView);
  }

  m_DrawOutline = prefs->GetBool("draw outline", true);
  m_SelectionMode = prefs->GetBool("selection mode", false);

  m_LabelSetPresetPreference = QString::fromStdString(prefs->Get("label set preset", ""));

  this->ApplyDisplayOptions();
  this->ApplySelectionMode();
}

void QmitkSegmentationView::NodeAdded(const mitk::DataNode* node)
{
  if (m_SegmentationPredicate->CheckNode(node))
    this->ApplyDisplayOptions(const_cast<mitk::DataNode*>(node));

  this->ApplySelectionMode();
}

void QmitkSegmentationView::NodeRemoved(const mitk::DataNode* node)
{
  if (!m_SegmentationPredicate->CheckNode(node))
  {
    return;
  }

  // remove all possible contour markers of the segmentation
  mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers = this->GetDataStorage()->GetDerivations(
    node, mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true)));

  ctkPluginContext* context = mitk::PluginActivator::getContext();
  ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
  mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);

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

  mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
  mitk::SurfaceInterpolationController::GetInstance()->RemoveInterpolationSession(image);
}

void QmitkSegmentationView::ApplyDisplayOptions()
{
  if (nullptr == m_Parent)
  {
    return;
  }

  if (nullptr == m_Controls)
  {
    return; // might happen on initialization (preferences loaded)
  }

  mitk::DataStorage::SetOfObjects::ConstPointer allImages = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);
  for (mitk::DataStorage::SetOfObjects::const_iterator iter = allImages->begin(); iter != allImages->end(); ++iter)
  {
    this->ApplyDisplayOptions(*iter);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::ApplyDisplayOptions(mitk::DataNode* node)
{
  if (nullptr == node)
  {
    return;
  }

  auto labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  if (nullptr == labelSetImage)
  {
    return;
  }

  // the outline property can be set in the segmentation preference page
  node->SetProperty("labelset.contour.active", mitk::BoolProperty::New(m_DrawOutline));

  // force render window update to show outline
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::ApplySelectionMode()
{
  if (!m_SelectionMode)
    return;

  this->ApplySelectionModeOnReferenceNode();
  this->ApplySelectionModeOnWorkingNode();
}

void QmitkSegmentationView::ApplySelectionModeOnReferenceNode()
{
  this->ApplySelectionMode(m_ReferenceNode, m_ReferencePredicate);
}

void QmitkSegmentationView::ApplySelectionModeOnWorkingNode()
{
  this->ApplySelectionMode(m_WorkingNode, m_SegmentationPredicate);
}

void QmitkSegmentationView::ApplySelectionMode(mitk::DataNode* node, mitk::NodePredicateBase* predicate)
{
  if (!m_SelectionMode || node == nullptr || predicate == nullptr)
    return;

  auto nodes = this->GetDataStorage()->GetSubset(predicate);

  for (auto iter = nodes->begin(); iter != nodes->end(); ++iter)
    (*iter)->SetVisibility(*iter == node);
}

void QmitkSegmentationView::OnContourMarkerSelected(const mitk::DataNode* node)
{
  QmitkRenderWindow* selectedRenderWindow = nullptr;
  auto* renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN);
  auto* axialRenderWindow = renderWindowPart->GetQmitkRenderWindow("axial");
  auto* sagittalRenderWindow = renderWindowPart->GetQmitkRenderWindow("sagittal");
  auto* coronalRenderWindow = renderWindowPart->GetQmitkRenderWindow("coronal");
  auto* threeDRenderWindow = renderWindowPart->GetQmitkRenderWindow("3d");
  bool PlanarFigureInitializedWindow = false;

  // find initialized renderwindow
  if (node->GetBoolProperty("PlanarFigureInitializedWindow",
    PlanarFigureInitializedWindow, axialRenderWindow->GetRenderer()))
  {
    selectedRenderWindow = axialRenderWindow;
  }
  if (!selectedRenderWindow && node->GetBoolProperty(
    "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
    sagittalRenderWindow->GetRenderer()))
  {
    selectedRenderWindow = sagittalRenderWindow;
  }
  if (!selectedRenderWindow && node->GetBoolProperty(
    "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
    coronalRenderWindow->GetRenderer()))
  {
    selectedRenderWindow = coronalRenderWindow;
  }
  if (!selectedRenderWindow && node->GetBoolProperty(
    "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
    threeDRenderWindow->GetRenderer()))
  {
    selectedRenderWindow = threeDRenderWindow;
  }

  // make node visible
  if (nullptr != selectedRenderWindow)
  {
    std::string nodeName = node->GetName();
    unsigned int t = nodeName.find_last_of(" ");
    unsigned int id = atof(nodeName.substr(t + 1).c_str()) - 1;

    ctkPluginContext* context = mitk::PluginActivator::getContext();
    ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
    mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);
    selectedRenderWindow->GetSliceNavigationController()->ExecuteOperation(service->GetPlanePosition(id));
    context->ungetService(ppmRef);

    selectedRenderWindow->GetRenderer()->GetCameraController()->Fit();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSegmentationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  if (0 == nodes.size())
  {
    return;
  }

  std::string markerName = "Position";
  unsigned int numberOfNodes = nodes.size();
  std::string nodeName = nodes.at(0)->GetName();
  if ((numberOfNodes == 1) && (nodeName.find(markerName) == 0))
  {
    this->OnContourMarkerSelected(nodes.at(0));
    return;
  }
}

void QmitkSegmentationView::ResetMouseCursor()
{
  if (m_MouseCursorSet)
  {
    mitk::ApplicationCursor::GetInstance()->PopCursor();
    m_MouseCursorSet = false;
  }
}

void QmitkSegmentationView::SetMouseCursor(const us::ModuleResource& resource, int hotspotX, int hotspotY)
{
  // Remove previously set mouse cursor
  if (m_MouseCursorSet)
  {
    this->ResetMouseCursor();
  }

  if (resource)
  {
    us::ModuleResourceStream cursor(resource, std::ios::binary);
    mitk::ApplicationCursor::GetInstance()->PushCursor(cursor, hotspotX, hotspotY);
    m_MouseCursorSet = true;
  }
}

void QmitkSegmentationView::UpdateGUI()
{
  mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
  bool hasReferenceNode = referenceNode != nullptr;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  bool hasWorkingNode = workingNode != nullptr;

  m_Controls->newSegmentationButton->setEnabled(false);

  if (hasReferenceNode)
  {
    m_Controls->newSegmentationButton->setEnabled(true);
  }

  if (hasWorkingNode && hasReferenceNode)
  {
    int layer = -1;
    referenceNode->GetIntProperty("layer", layer);
    workingNode->SetIntProperty("layer", layer + 1);
  }

  this->ValidateSelectionInput();
}

void QmitkSegmentationView::ValidateSelectionInput()
{
  auto referenceNode = m_Controls->referenceNodeSelector->GetSelectedNode();
  auto workingNode = m_Controls->workingNodeSelector->GetSelectedNode();

  bool hasReferenceNode = referenceNode.IsNotNull();
  bool hasWorkingNode = workingNode.IsNotNull();
  bool hasBothNodes = hasReferenceNode && hasWorkingNode;

  QString warning;
  bool toolSelectionBoxesEnabled = hasReferenceNode && hasWorkingNode;
  unsigned int numberOfLabels = 0;

  m_Controls->multiLabelWidget->setEnabled(hasWorkingNode);

  m_Controls->toolSelectionBox2D->setEnabled(hasBothNodes);
  m_Controls->toolSelectionBox3D->setEnabled(hasBothNodes);

  m_Controls->slicesInterpolator->setEnabled(false);
  m_Controls->interpolatorWarningLabel->hide();

  if (hasReferenceNode)
  {
    if (nullptr != m_RenderWindowPart && m_RenderWindowPart->HasCoupledRenderWindows() && !referenceNode->IsVisible(nullptr))
    {
      warning += tr("The selected reference image is currently not visible!");
      toolSelectionBoxesEnabled = false;
    }
  }

  if (hasWorkingNode)
  {
    if (nullptr != m_RenderWindowPart && m_RenderWindowPart->HasCoupledRenderWindows() && !workingNode->IsVisible(nullptr))
    {
      warning += (!warning.isEmpty() ? "<br>" : "") + tr("The selected segmentation is currently not visible!");
      toolSelectionBoxesEnabled = false;
    }

    m_ToolManager->SetReferenceData(referenceNode);
    m_ToolManager->SetWorkingData(workingNode);
    m_Controls->multiLabelWidget->setEnabled(true);
    m_Controls->toolSelectionBox2D->setEnabled(true);
    m_Controls->toolSelectionBox3D->setEnabled(true);

    auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
    numberOfLabels = labelSetImage->GetTotalNumberOfLabels();

    if (numberOfLabels > 0)
      m_Controls->slicesInterpolator->setEnabled(true);

    m_Controls->multiLabelWidget->SetMultiLabelSegmentation(dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData()));
  }
  else
  {
    m_Controls->multiLabelWidget->SetMultiLabelSegmentation(nullptr);
  }

  toolSelectionBoxesEnabled &= numberOfLabels > 0;

  // Here we need to check whether the geometry of the selected segmentation image (working image geometry)
  // is aligned with the geometry of the 3D render window.
  // It is not allowed to use a geometry different from the working image geometry for segmenting.
  // We only need to this if the tool selection box would be enabled without this check.
  // Additionally this check only has to be performed for render window parts with coupled render windows.
  // For different render window parts the user is given the option to reinitialize each render window individually
  // (see QmitkRenderWindow::ShowOverlayMessage).
  if (toolSelectionBoxesEnabled && nullptr != m_RenderWindowPart && m_RenderWindowPart->HasCoupledRenderWindows())
  {
    const mitk::BaseGeometry* workingNodeGeometry = workingNode->GetData()->GetGeometry();
    const mitk::BaseGeometry* renderWindowGeometry =
      m_RenderWindowPart->GetQmitkRenderWindow("3d")->GetSliceNavigationController()->GetCurrentGeometry3D();
    if (nullptr != workingNodeGeometry && nullptr != renderWindowGeometry)
    {
      if (!mitk::Equal(*workingNodeGeometry->GetBoundingBox(), *renderWindowGeometry->GetBoundingBox(), mitk::eps, true))
      {
        warning += (!warning.isEmpty() ? "<br>" : "") + tr("Please reinitialize the selected segmentation image!");
        toolSelectionBoxesEnabled = false;
      }
    }
  }

  m_Controls->toolSelectionBox2D->setEnabled(toolSelectionBoxesEnabled);
  m_Controls->toolSelectionBox3D->setEnabled(toolSelectionBoxesEnabled);

  this->UpdateWarningLabel(warning);

  m_ToolManager->SetReferenceData(referenceNode);
  m_ToolManager->SetWorkingData(workingNode);
}

void QmitkSegmentationView::UpdateWarningLabel(QString text)
{
  if (text.isEmpty())
  {
    m_Controls->selectionWarningLabel->hide();
  }
  else
  {
    m_Controls->selectionWarningLabel->setText("<font color=\"red\">" + text + "</font>");
    m_Controls->selectionWarningLabel->show();
  }
}
