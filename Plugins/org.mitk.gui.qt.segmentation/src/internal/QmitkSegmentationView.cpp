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
#include <mitkLabelSetIOHelper.h>
#include <mitkManualPlacementAnnotationRenderer.h>
#include <mitkNodePredicateSubGeometry.h>
#include <mitkSegmentationInteractionEvents.h>
#include <mitkSegmentationObjectFactory.h>
#include <mitkSegTool2D.h>
#include <mitkStatusBar.h>
#include <mitkToolManagerProvider.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkWorkbenchUtil.h>

// Qmitk
#include <QmitkRenderWindow.h>
#include <QmitkStaticDynamicSegmentationDialog.h>
#include <QmitkNewSegmentationDialog.h>

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

const std::string QmitkSegmentationView::VIEW_ID = "org.mitk.views.segmentation";

QmitkSegmentationView::QmitkSegmentationView()
  : m_Parent(nullptr)
  , m_Controls(nullptr)
  , m_RenderWindowPart(nullptr)
  , m_ToolManager(nullptr)
  , m_ReferenceNode(nullptr)
  , m_WorkingNode(nullptr)
  , m_RendererAnnotation(mitk::LogoAnnotation::New())
  , m_DrawOutline(true)
  , m_SelectionMode(false)
  , m_MouseCursorSet(false)
  , m_DefaultLabelNaming(true)
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

  // setup annotation for an overlay image to indicated a blocked renderer
  m_RendererAnnotation->SetOpacity(0.5);
  m_RendererAnnotation->SetRelativeSize(0.25);
  m_RendererAnnotation->SetCornerPosition(4);

  QImage* qimage = new QImage(qPrintable(":/Qmitk/Cancel_48x48.png"));
  vtkSmartPointer<vtkQImageToImageSource> qImageToVtk;
  qImageToVtk = vtkSmartPointer<vtkQImageToImageSource>::New();

  qImageToVtk->SetQImage(qimage);
  qImageToVtk->Update();
  vtkSmartPointer<vtkImageData> vtkLogo = qImageToVtk->GetOutput();
  m_RendererAnnotation->SetLogoImage(vtkLogo);

  m_SegmentationInteractor = mitk::SegmentationInteractor::New();
  // add observer for the 'SegmentationInteractionEvent'
  itk::ReceptorMemberCommand<QmitkSegmentationView>::Pointer geometryNotAlignedCommand =
    itk::ReceptorMemberCommand<QmitkSegmentationView>::New();
  geometryNotAlignedCommand->SetCallbackFunction(this, &QmitkSegmentationView::ValidateRendererGeometry);
  m_SegmentationInteractor->AddObserver(mitk::SegmentationInteractionEvent(nullptr, true), geometryNotAlignedCommand);
  m_SegmentationInteractor->Disable();
}

QmitkSegmentationView::~QmitkSegmentationView()
{
  if (nullptr != m_Controls)
  {
    OnLooseLabelSetConnection();

    // deactivate all tools
    m_ToolManager->ActivateTool(-1);

    // removing all observers from working data
    for (NodeTagMapType::iterator dataIter = m_WorkingDataObserverTags.begin(); dataIter != m_WorkingDataObserverTags.end(); ++dataIter)
    {
      (*dataIter).first->GetProperty("visible")->RemoveObserver((*dataIter).second);
    }
    m_WorkingDataObserverTags.clear();

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
    mitk::MessageDelegate<QmitkSegmentationView>(this, &QmitkSegmentationView::ActiveToolChanged);

  delete m_Controls;
}

/**********************************************************************/
/* private Q_SLOTS                                                    */
/**********************************************************************/
void QmitkSegmentationView::OnReferenceSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  m_ToolManager->ActivateTool(-1);

  // Remove observer if one was registered to the reference node
  auto finding = m_ReferenceDataObserverTags.find(m_ReferenceNode);
  if (finding != m_ReferenceDataObserverTags.end())
  {
    m_ReferenceNode->GetProperty("visible")->RemoveObserver(m_ReferenceDataObserverTags[m_ReferenceNode]);
    m_ReferenceDataObserverTags.erase(m_ReferenceNode);
  }

  if (nodes.empty())
  {
    m_Controls->workingNodeSelector->SetNodePredicate(m_SegmentationPredicate);
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

    m_Controls->workingNodeSelector->SetNodePredicate(segPredicate);

    if (m_SelectionMode)
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

    auto command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
    command->SetCallbackFunction(this, &QmitkSegmentationView::ValidateSelectionInput);
    m_ReferenceDataObserverTags[m_ReferenceNode] =
      m_ReferenceNode->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command);
  }

  this->UpdateGUI();
}

void QmitkSegmentationView::OnSegmentationSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  m_ToolManager->ActivateTool(-1);

  // Remove observer if one was registered
  auto finding = m_WorkingDataObserverTags.find(m_WorkingNode);
  if (finding != m_WorkingDataObserverTags.end())
  {
    m_WorkingNode->GetProperty("visible")->RemoveObserver(m_WorkingDataObserverTags[m_WorkingNode]);
    m_WorkingDataObserverTags.erase(m_WorkingNode);
  }

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

  mitk::Image::ConstPointer referenceImage = dynamic_cast<mitk::Image*>(m_ReferenceNode->GetData());
  if (referenceImage.IsNull())
  {
    this->UpdateGUI();
    return;
  }

  m_WorkingNode = nodes.first();
  m_ToolManager->SetWorkingData(m_WorkingNode);
  if (m_WorkingNode.IsNotNull())
  {
    if (m_SelectionMode)
    {
      // hide all segmentation nodes to later show only the selected ones
      mitk::DataStorage::SetOfObjects::ConstPointer segmentationNodes =
        this->GetDataStorage()->GetSubset(m_SegmentationPredicate);
      for (mitk::DataStorage::SetOfObjects::const_iterator iter = segmentationNodes->begin(); iter != segmentationNodes->end(); ++iter)
      {
        (*iter)->SetVisibility(false);
      }
    }
    m_WorkingNode->SetVisibility(true);

    this->OnEstablishLabelSetConnection();
    m_Controls->labelSetWidget->ResetAllTableWidgetItems();

    auto command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
    command->SetCallbackFunction(this, &QmitkSegmentationView::ValidateSelectionInput);
    m_WorkingDataObserverTags[m_WorkingNode] =
      m_WorkingNode->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command);

    this->InitializeRenderWindows(referenceImage->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false);
  }

  this->UpdateGUI();
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

  if (labelSetPreset.empty() || !mitk::LabelSetIOHelper::LoadLabelSetImagePreset(labelSetPreset, newLabelSetImage))
  {
    auto newLabel = mitk::LabelSetImageHelper::CreateNewLabel(newLabelSetImage);

    if (!m_DefaultLabelNaming)
    {
      QmitkNewSegmentationDialog dialog(m_Parent);
      dialog.SetName(QString::fromStdString(newLabel->GetName()));
      dialog.SetColor(newLabel->GetColor());

      if (QDialog::Rejected == dialog.exec())
        return;

      auto name = dialog.GetName();

      if (!name.isEmpty())
        newLabel->SetName(name.toStdString());

      newLabel->SetColor(dialog.GetColor());
    }

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

void QmitkSegmentationView::OnLayersChanged()
{
  this->OnEstablishLabelSetConnection();
  m_Controls->labelSetWidget->ResetAllTableWidgetItems();
}

void QmitkSegmentationView::OnShowLabelTable(bool value)
{
  m_Controls->labelSetWidget->setVisible(value);
}

void QmitkSegmentationView::OnGoToLabel(const mitk::Point3D& pos)
{
  if (m_RenderWindowPart)
  {
    m_RenderWindowPart->SetSelectedPosition(pos);
  }
}

void QmitkSegmentationView::OnLabelSetWidgetReset()
{
  this->ValidateSelectionInput();
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
   connect(visibilityShortcut, &QShortcut::activated, this, &QmitkSegmentationView::OnVisibilityShortcutActivated);
   QShortcut* labelToggleShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key::Key_L, Qt::CTRL | Qt::Key::Key_I), parent);
   connect(labelToggleShortcut, &QShortcut::activated, this, &QmitkSegmentationView::OnLabelToggleShortcutActivated);

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
           this, &QmitkSegmentationView::OnReferenceSelectionChanged);
   connect(m_Controls->workingNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
           this, &QmitkSegmentationView::OnSegmentationSelectionChanged);

   // *------------------------
   // * TOOLMANAGER
   // *------------------------
   m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
   m_ToolManager->SetDataStorage(*(this->GetDataStorage()));
   m_ToolManager->InitializeTools();

   // react if the active tool changed
   m_ToolManager->ActiveToolChanged +=
     mitk::MessageDelegate<QmitkSegmentationView>(this, &QmitkSegmentationView::ActiveToolChanged);

   QString segTools2D = tr("Add Subtract Lasso Fill Erase Close Paint Wipe 'Region Growing' 'Live Wire'");
   QString segTools3D = tr("Threshold 'UL Threshold' Otsu 'Region Growing 3D' Picking GrowCut");

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
   m_Controls->toolSelectionBox2D->SetLayoutColumns(3);
   m_Controls->toolSelectionBox2D->SetEnabledMode(
     QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);
   connect(m_Controls->toolSelectionBox2D, &QmitkToolSelectionBox::ToolSelected,
           this, &QmitkSegmentationView::OnManualTool2DSelected);

   // setup 3D Tools
   m_Controls->toolSelectionBox3D->SetToolManager(*m_ToolManager);
   m_Controls->toolSelectionBox3D->SetGenerateAccelerators(true);
   m_Controls->toolSelectionBox3D->SetToolGUIArea(m_Controls->toolGUIArea3D);
   m_Controls->toolSelectionBox3D->SetDisplayedToolGroups(segTools3D.toStdString());
   m_Controls->toolSelectionBox3D->SetLayoutColumns(3);
   m_Controls->toolSelectionBox3D->SetEnabledMode(
     QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);

   m_Controls->slicesInterpolator->SetDataStorage(this->GetDataStorage());

   // create general signal / slot connections
   connect(m_Controls->newSegmentationButton, &QToolButton::clicked, this, &QmitkSegmentationView::OnNewSegmentation);
   connect(m_Controls->slicesInterpolator, &QmitkSlicesInterpolator::SignalShowMarkerNodes, this, &QmitkSegmentationView::OnShowMarkerNodes);

   connect(m_Controls->layersWidget, &QmitkLayersWidget::LayersChanged, this, &QmitkSegmentationView::OnLayersChanged);
   connect(m_Controls->labelsWidget, &QmitkLabelsWidget::ShowLabelTable, this, &QmitkSegmentationView::OnShowLabelTable);

   // *------------------------
   // * LABELSETWIDGET
   // *------------------------
   connect(m_Controls->labelSetWidget, &QmitkLabelSetWidget::goToLabel, this, &QmitkSegmentationView::OnGoToLabel);
   connect(m_Controls->labelSetWidget, &QmitkLabelSetWidget::LabelSetWidgetReset, this, &QmitkSegmentationView::OnLabelSetWidgetReset);

   m_Controls->labelSetWidget->SetDataStorage(this->GetDataStorage());
   m_Controls->labelSetWidget->hide();

   auto command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
   command->SetCallbackFunction(this, &QmitkSegmentationView::ValidateSelectionInput);
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
  auto activeTool = m_ToolManager->GetActiveTool();
  if (nullptr == activeTool)
  {
    // no tool activated, deactivate the segmentation interactor
    m_SegmentationInteractor->Disable();
    return;
  }

  // activate segmentation interactor to get informed about render window entered / left events
  m_SegmentationInteractor->Enable();
}

void QmitkSegmentationView::ValidateRendererGeometry(const itk::EventObject& event)
{
  if (!mitk::SegmentationInteractionEvent().CheckEvent(&event))
  {
    return;
  }

  const auto* segmentationInteractionEvent =
    dynamic_cast<const mitk::SegmentationInteractionEvent*>(&event);

  const mitk::BaseRenderer::Pointer sendingRenderer = segmentationInteractionEvent->GetSender();
  if (nullptr == sendingRenderer)
  {
    return;
  }

  bool entered = segmentationInteractionEvent->HasEnteredRenderWindow();
  if (entered)
  {
    // mouse cursor of tool inside render window
    // check if tool can be used: reference geometry needs to be aligned with renderer geometry
    const auto* referenceDataNode = m_ToolManager->GetReferenceData(0);
    if (nullptr != referenceDataNode)
    {
      const auto workingImage = dynamic_cast<mitk::Image*>(referenceDataNode->GetData());
      if (nullptr != workingImage)
      {
        const mitk::TimeGeometry* workingImageGeometry = workingImage->GetTimeGeometry();
        if (nullptr != workingImageGeometry)
        {
          bool isGeometryAligned =
            mitk::BaseRendererHelper::IsRendererAlignedWithSegmentation(sendingRenderer, workingImageGeometry);

          if (!isGeometryAligned)
          {
            this->BlockRenderer(sendingRenderer, true);
            this->UpdateWarningLabel(
              tr("Please perform a reinit on the segmentation image inside the entered Render Window!"));
            return;
          }
        }
      }
    }
  }

  this->BlockRenderer(sendingRenderer, false);
  this->UpdateWarningLabel(tr(""));
}

void QmitkSegmentationView::BlockRenderer(mitk::BaseRenderer* baseRenderer, bool blocked)
{
  if (blocked)
  {
    mitk::ManualPlacementAnnotationRenderer::AddAnnotation(m_RendererAnnotation.GetPointer(), baseRenderer);
    m_RendererAnnotation->Update(baseRenderer);
    baseRenderer->RequestUpdate();
  }
  else
  {
    m_RendererAnnotation->RemoveFromBaseRenderer(baseRenderer);
  }
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

  // tell the interpolation about tool manager, data storage and render window part
  if (nullptr != m_RenderWindowPart)
  {
    QList<mitk::SliceNavigationController*> controllers;
    controllers.push_back(m_RenderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(m_RenderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(m_RenderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls->slicesInterpolator->Initialize(m_ToolManager, controllers);
  }
}

void QmitkSegmentationView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  m_RenderWindowPart = nullptr;
  if (nullptr != m_Parent)
  {
    m_Parent->setEnabled(false);
  }
}

void QmitkSegmentationView::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  auto labelSuggestions = mitk::BaseApplication::instance().config().getString(mitk::BaseApplication::ARG_SEGMENTATION_LABEL_SUGGESTIONS.toStdString(), "");

  m_DefaultLabelNaming = labelSuggestions.empty()
    ? prefs->GetBool("default label naming", true)
    : false; // No default label naming when label suggestions are enforced via command-line argument

  if (nullptr != m_Controls)
  {
    m_Controls->labelsWidget->SetDefaultLabelNaming(m_DefaultLabelNaming);

    bool slimView = prefs->GetBool("slim view", false);
    m_Controls->toolSelectionBox2D->SetShowNames(!slimView);
    m_Controls->toolSelectionBox3D->SetShowNames(!slimView);
  }

  m_DrawOutline = prefs->GetBool("draw outline", true);
  m_SelectionMode = prefs->GetBool("selection mode", false);

  m_LabelSetPresetPreference = prefs->Get("label set preset", "");

  this->ApplyDisplayOptions();
}

void QmitkSegmentationView::NodeAdded(const mitk::DataNode* node)
{
  if (m_SegmentationPredicate->CheckNode(node))
  {
    this->ApplyDisplayOptions(const_cast<mitk::DataNode*>(node));
  }
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

void QmitkSegmentationView::OnEstablishLabelSetConnection()
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

  workingImage->GetActiveLabelSet()->AddLabelEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls->labelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->RemoveLabelEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls->labelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ModifyLabelEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls->labelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->AllLabelsModifiedEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls->labelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ActiveLabelEvent += mitk::MessageDelegate1<QmitkLabelSetWidget,
    mitk::Label::PixelType>(m_Controls->labelSetWidget, &QmitkLabelSetWidget::SelectLabelByPixelValue);

  // Removed in T27851 to have a chance to react to AfterChangeLayerEvent. Did it brake something?
  // workingImage->BeforeChangeLayerEvent += mitk::MessageDelegate<QmitkMultiLabelSegmentationView>(
  //   this, &QmitkMultiLabelSegmentationView::OnLooseLabelSetConnection);

  workingImage->AfterChangeLayerEvent += mitk::MessageDelegate<QmitkSegmentationView>(
    this, &QmitkSegmentationView::UpdateGUI);
}

void QmitkSegmentationView::OnLooseLabelSetConnection()
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

  // Reset LabelSetWidget Events
  workingImage->GetActiveLabelSet()->AddLabelEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls->labelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->RemoveLabelEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls->labelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ModifyLabelEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls->labelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->AllLabelsModifiedEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls->labelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ActiveLabelEvent -= mitk::MessageDelegate1<QmitkLabelSetWidget,
    mitk::Label::PixelType>(m_Controls->labelSetWidget, &QmitkLabelSetWidget::SelectLabelByPixelValue);

  // Removed in T27851 to have a chance to react to AfterChangeLayerEvent. Did it brake something?
  // workingImage->BeforeChangeLayerEvent -= mitk::MessageDelegate<QmitkMultiLabelSegmentationView>(
  //   this, &QmitkMultiLabelSegmentationView::OnLooseLabelSetConnection);

  workingImage->AfterChangeLayerEvent -= mitk::MessageDelegate<QmitkSegmentationView>(
    this, &QmitkSegmentationView::UpdateGUI);
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
  if (nullptr != labelSetImage)
  {
    // node is a multi label segmentation
    // its outline property can be set in the segmentation preference page
    node->SetProperty("labelset.contour.active", mitk::BoolProperty::New(m_DrawOutline));

    // force render window update to show outline
    node->GetData()->Modified();
  }
  else if (nullptr != node->GetData())
  {
    // node is a legacy binary segmentation
    bool isBinary = false;
    node->GetBoolProperty("binary", isBinary);
    if (isBinary)
    {
      node->SetProperty("outline binary", mitk::BoolProperty::New(m_DrawOutline));
      node->SetProperty("outline width", mitk::FloatProperty::New(2.0));
      // force render window update to show outline
      node->GetData()->Modified();
    }
  }
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

  m_Controls->layersWidget->UpdateGUI();
  m_Controls->labelsWidget->UpdateGUI();

  this->ValidateSelectionInput();
}

void QmitkSegmentationView::ValidateSelectionInput()
{
  this->UpdateWarningLabel("");

  m_Controls->layersWidget->setEnabled(false);
  m_Controls->labelsWidget->setEnabled(false);
  m_Controls->labelSetWidget->setEnabled(false);

  // the argument is actually not used
  // enable status depends on the tool manager selection
  m_Controls->toolSelectionBox2D->setEnabled(false);
  m_Controls->toolSelectionBox3D->setEnabled(false);
  m_Controls->slicesInterpolator->setEnabled(false);
  m_Controls->interpolatorWarningLabel->hide();

  mitk::DataNode* referenceNode = m_Controls->referenceNodeSelector->GetSelectedNode();
  mitk::DataNode* workingNode = m_Controls->workingNodeSelector->GetSelectedNode();

  if (nullptr == referenceNode)
  {
    return;
  }

  if (nullptr == workingNode)
  {
    return;
  }

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
  auto workingNodeIsVisible = renderWindowPart &&
    workingNode->IsVisible(renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer());
  if (!workingNodeIsVisible)
  {
    this->UpdateWarningLabel(tr("The selected segmentation is currently not visible!"));
    return;
  }

  m_ToolManager->SetReferenceData(referenceNode);
  m_ToolManager->SetWorkingData(workingNode);
  m_Controls->layersWidget->setEnabled(true);
  m_Controls->labelsWidget->setEnabled(true);
  m_Controls->labelSetWidget->setEnabled(true);
  m_Controls->toolSelectionBox2D->setEnabled(true);
  m_Controls->toolSelectionBox3D->setEnabled(true);

  auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  if (nullptr != labelSetImage)
  {
    int numberOfLabels = labelSetImage->GetNumberOfLabels(labelSetImage->GetActiveLayer());
    if (2 == numberOfLabels) // fix for T27319: exterior is label 0, first label is label 1
    {
      m_Controls->slicesInterpolator->setEnabled(true);
    }
    else
    {
      m_Controls->interpolatorWarningLabel->show();
      m_Controls->interpolatorWarningLabel->setText(
        "<font color=\"red\">Interpolation only works for single label segmentations.</font>");
    }
  }
}

void QmitkSegmentationView::UpdateWarningLabel(QString text)
{
  if (text.size() == 0)
  {
    m_Controls->selectionWarningLabel->hide();
  }
  else
  {
    m_Controls->selectionWarningLabel->show();
    m_Controls->selectionWarningLabel->setText("<font color=\"red\">" + text + "</font>");
  }
}
