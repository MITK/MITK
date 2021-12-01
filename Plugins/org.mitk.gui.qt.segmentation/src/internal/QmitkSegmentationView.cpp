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
#include <mitkCameraController.h>
#include <mitkImageTimeSelector.h>
#include <mitkLabelSetImage.h>
#include <mitkNodePredicateSubGeometry.h>
#include <mitkSegmentationObjectFactory.h>
#include <mitkSegTool2D.h>
#include <mitkStatusBar.h>
#include <mitkToolManagerProvider.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkWorkbenchUtil.h>

// Qmitk
#include <QmitkNewSegmentationDialog.h>
#include <QmitkRenderWindow.h>
#include <QmitkSegmentationOrganNamesHandling.cpp>

// us
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

// Qt
#include <QMessageBox>

#include <regex>

const std::string QmitkSegmentationView::VIEW_ID = "org.mitk.views.segmentation";

QmitkSegmentationView::QmitkSegmentationView()
  : m_Parent(nullptr)
  , m_Controls(nullptr)
  , m_RenderWindowPart(nullptr)
  , m_ToolManager(nullptr)
  , m_ReferenceNode(nullptr)
  , m_WorkingNode(nullptr)
  , m_AutoSelectionEnabled(false)
  , m_MouseCursorSet(false)
{
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  auto isDti = mitk::NodePredicateDataType::New("TensorImage");
  auto isOdf = mitk::NodePredicateDataType::New("OdfImage");
  auto isSegment = mitk::NodePredicateDataType::New("Segment");

  mitk::NodePredicateOr::Pointer validImages = mitk::NodePredicateOr::New();
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

QmitkSegmentationView::~QmitkSegmentationView()
{
  if (m_Controls)
  {
    // deactivate all tools
    m_ToolManager->ActivateTool(-1);

    // removing all observers
    for (NodeTagMapType::iterator dataIter = m_WorkingDataObserverTags.begin(); dataIter != m_WorkingDataObserverTags.end(); ++dataIter)
    {
      (*dataIter).first->GetProperty("visible")->RemoveObserver((*dataIter).second);
    }
    m_WorkingDataObserverTags.clear();

    mitk::RenderingManager::GetInstance()->RemoveObserver(m_RenderingManagerObserverTag);

    ctkPluginContext* context = mitk::PluginActivator::getContext();
    ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
    mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);
    service->RemoveAllPlanePositions();
    context->ungetService(ppmRef);

    m_ToolManager->SetReferenceData(nullptr);
    m_ToolManager->SetWorkingData(nullptr);
  }

  delete m_Controls;
}

/**********************************************************************/
/* private Q_SLOTS                                                    */
/**********************************************************************/
void QmitkSegmentationView::OnReferenceSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  m_ToolManager->ActivateTool(-1);

  if (nodes.empty())
  {
    m_Controls->segImageSelector->SetNodePredicate(m_SegmentationPredicate);
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

    m_Controls->segImageSelector->SetNodePredicate(segPredicate);

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

void QmitkSegmentationView::OnSegmentationSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
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
    this->InitializeRenderWindows(referenceImage->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false);
  }

  this->UpdateGUI();
}

void QmitkSegmentationView::CreateNewSegmentation()
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

  m_ToolManager->ActivateTool(-1);

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

  // ask about the name and organ type of the new segmentation
  auto dialog = new QmitkNewSegmentationDialog(m_Parent);
  QStringList organColors = mitk::OrganNamesHandling::GetDefaultOrganColorString();
  dialog->SetSuggestionList(organColors);
  dialog->SetSegmentationName(newName);

  int dialogReturnValue = dialog->exec();
  if (dialogReturnValue == QDialog::Rejected)
  {
    return;
  }

  std::string newNodeName = dialog->GetSegmentationName().toStdString();
  if (newNodeName.empty())
  {
    newNodeName = "Unnamed";
  }

  // create a new image of the same dimensions and smallest possible pixel type
  auto firstTool = m_ToolManager->GetToolById(0);
  if (nullptr == firstTool)
  {
    return;
  }

  mitk::DataNode::Pointer emptySegmentation = nullptr;
  try
  {
    emptySegmentation = firstTool->CreateEmptySegmentationNode(segTemplateImage, newNodeName, dialog->GetColor());
  }
  catch (const std::bad_alloc &)
  {
    QMessageBox::warning(m_Parent, tr("New segmentation"), tr("Could not allocate memory for new segmentation"));
  }

  if (nullptr == emptySegmentation)
  {
    return; // could have been aborted by user
  }

  // initialize "showVolume"-property to false to prevent recalculating the volume while working on the segmentation
  emptySegmentation->SetProperty("showVolume", mitk::BoolProperty::New(false));

  mitk::OrganNamesHandling::UpdateOrganList(organColors, dialog->GetSegmentationName(), dialog->GetColor());

  // escape ';' here (replace by '\;')
  QString stringForStorage = organColors.replaceInStrings(";", "\\;").join(";");
  MITK_DEBUG << "Will store: " << stringForStorage;
  this->GetPreferences()->Put("Organ-Color-List", stringForStorage);
  this->GetPreferences()->Flush();

  this->GetDataStorage()->Add(emptySegmentation, referenceNode);

  if (m_ToolManager->GetWorkingData(0))
  {
    m_ToolManager->GetWorkingData(0)->SetSelected(false);
  }
  emptySegmentation->SetSelected(true);
  m_Controls->segImageSelector->SetCurrentSelectedNode(emptySegmentation);

  this->InitializeRenderWindows(referenceImage->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false);
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

    if (manualSegmentationTool)
    {
      if (state == true)
      {
        manualSegmentationTool->SetShowMarkerNodes(true);
      }
      else
      {
        manualSegmentationTool->SetShowMarkerNodes(false);
      }
    }
  }
}

/**********************************************************************/
/* private                                                            */
/**********************************************************************/
void QmitkSegmentationView::CreateQtPartControl(QWidget* parent)
{
   m_Parent = parent;

   m_Controls = new Ui::QmitkSegmentationControls;
   m_Controls->setupUi(parent);

   m_Controls->patImageSelector->SetDataStorage(GetDataStorage());
   m_Controls->patImageSelector->SetNodePredicate(m_ReferencePredicate);
   m_Controls->patImageSelector->SetInvalidInfo("Select an image");
   m_Controls->patImageSelector->SetPopUpTitel("Select an image");
   m_Controls->patImageSelector->SetPopUpHint("Select an image that should be used to define the geometry and bounds of the segmentation.");

   m_Controls->segImageSelector->SetDataStorage(GetDataStorage());
   m_Controls->segImageSelector->SetNodePredicate(m_SegmentationPredicate);
   m_Controls->segImageSelector->SetInvalidInfo("Select a segmentation");
   m_Controls->segImageSelector->SetPopUpTitel("Select a segmentation");
   m_Controls->segImageSelector->SetPopUpHint("Select a segmentation that should be modified. Only segmentation with the same geometry and within the bounds of the reference image are selected.");

   connect(m_Controls->patImageSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
           this, &QmitkSegmentationView::OnReferenceSelectionChanged);
   connect(m_Controls->segImageSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
           this, &QmitkSegmentationView::OnSegmentationSelectionChanged);

   m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
   m_ToolManager->SetDataStorage(*(this->GetDataStorage()));
   m_ToolManager->InitializeTools();

   QString segTools2D = tr("Add Subtract Fill Erase Paint Wipe 'Region Growing' 'Live Wire' '2D Fast Marching'");
   QString segTools3D = tr("Threshold 'UL Threshold' Otsu 'Fast Marching 3D' 'Region Growing 3D' Watershed Picking");
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

   // all part of open source MITK
   m_Controls->m_ManualToolSelectionBox2D->SetToolManager(*m_ToolManager);
   m_Controls->m_ManualToolSelectionBox2D->SetGenerateAccelerators(true);
   m_Controls->m_ManualToolSelectionBox2D->SetToolGUIArea(m_Controls->m_ManualToolGUIContainer2D);
   m_Controls->m_ManualToolSelectionBox2D->SetDisplayedToolGroups(segTools2D.toStdString());
   m_Controls->m_ManualToolSelectionBox2D->SetLayoutColumns(3);
   m_Controls->m_ManualToolSelectionBox2D->SetEnabledMode(
     QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);
   connect(m_Controls->m_ManualToolSelectionBox2D, &QmitkToolSelectionBox::ToolSelected,
           this, &QmitkSegmentationView::OnManualTool2DSelected);

   //setup 3D Tools
   m_Controls->m_ManualToolSelectionBox3D->SetToolManager(*m_ToolManager);
   m_Controls->m_ManualToolSelectionBox3D->SetGenerateAccelerators(true);
   m_Controls->m_ManualToolSelectionBox3D->SetToolGUIArea(m_Controls->m_ManualToolGUIContainer3D);
   m_Controls->m_ManualToolSelectionBox3D->SetDisplayedToolGroups(segTools3D.toStdString());
   m_Controls->m_ManualToolSelectionBox3D->SetLayoutColumns(3);
   m_Controls->m_ManualToolSelectionBox3D->SetEnabledMode(
     QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);

   // create signal/slot connections
   connect(m_Controls->btnNewSegmentation, &QToolButton::clicked, this, &QmitkSegmentationView::CreateNewSegmentation);
   connect(m_Controls->m_SlicesInterpolator, &QmitkSlicesInterpolator::SignalShowMarkerNodes, this, &QmitkSegmentationView::OnShowMarkerNodes);

   // set callback function for already existing segmentation nodes
   mitk::DataStorage::SetOfObjects::ConstPointer allSegmentations = GetDataStorage()->GetSubset(m_SegmentationPredicate);
   for (mitk::DataStorage::SetOfObjects::const_iterator iter = allSegmentations->begin(); iter != allSegmentations->end(); ++iter)
   {
     mitk::DataNode* node = *iter;
     auto command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
     command->SetCallbackFunction(this, &QmitkSegmentationView::ValidateSelectionInput);
     m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode *, unsigned long>(
       node, node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));
   }

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
   m_Controls->patImageSelector->SetAutoSelectNewNodes(true);
   m_Controls->segImageSelector->SetAutoSelectNewNodes(true);

   this->UpdateGUI();
}

void QmitkSegmentationView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
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
  if (m_Controls)
  {
    m_Controls->m_SlicesInterpolator->SetDataStorage(this->GetDataStorage());
    QList<mitk::SliceNavigationController*> controllers;
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls->m_SlicesInterpolator->Initialize(m_ToolManager, controllers);
  }
}

void QmitkSegmentationView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  m_RenderWindowPart = nullptr;
  if (m_Parent)
  {
    m_Parent->setEnabled(false);
  }
}

void QmitkSegmentationView::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  if (m_Controls != nullptr)
  {
    bool slimView = prefs->GetBool("slim view", false);
    m_Controls->m_ManualToolSelectionBox2D->SetShowNames(!slimView);
    m_Controls->m_ManualToolSelectionBox3D->SetShowNames(!slimView);
  }

  m_AutoSelectionEnabled = prefs->GetBool("auto selection", false);

  this->ApplyDisplayOptions();
}

void QmitkSegmentationView::NodeAdded(const mitk::DataNode* node)
{
  if (!m_SegmentationPredicate->CheckNode(node))
  {
    return;
  }

  auto command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
  command->SetCallbackFunction(this, &QmitkSegmentationView::ValidateSelectionInput);
  m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode *, unsigned long>(
    const_cast<mitk::DataNode *>(node), node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));

  this->ApplyDisplayOptions(const_cast<mitk::DataNode*>(node));
}

void QmitkSegmentationView::NodeRemoved(const mitk::DataNode* node)
{
  if (m_SegmentationPredicate->CheckNode(node))
  {
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

  mitk::DataNode* tempNode = const_cast<mitk::DataNode*>(node);
  //Remove observer if one was registered
  auto finding = m_WorkingDataObserverTags.find(tempNode);
  if (finding != m_WorkingDataObserverTags.end())
  {
    node->GetProperty("visible")->RemoveObserver(m_WorkingDataObserverTags[tempNode]);
    m_WorkingDataObserverTags.erase(tempNode);
  }
}

void QmitkSegmentationView::ApplyDisplayOptions()
{
  if (!m_Parent)
  {
    return;
  }

  if (!m_Controls)
  {
    return; // might happen on initialization (preferences loaded)
  }

  mitk::DataNode::Pointer workingData = m_ToolManager->GetWorkingData(0);
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

  auto drawOutline = mitk::BoolProperty::New(GetPreferences()->GetBool("draw outline", true));
  auto labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  if (nullptr != labelSetImage)
  {
    // node is actually a multi label segmentation,
    // but its outline property can be set in the 'single label' segmentation preference page as well
    node->SetProperty("labelset.contour.active", drawOutline);
    // force render window update to show outline
    node->GetData()->Modified();
  }
  else
  {
    // node is a 'single label' segmentation
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

void QmitkSegmentationView::OnContourMarkerSelected(const mitk::DataNode* node)
{
  QmitkRenderWindow* selectedRenderWindow = nullptr;
  auto* renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN);
  auto* axialRenderWindow = renderWindowPart->GetQmitkRenderWindow("axial");
  auto* sagittalRenderWindow = renderWindowPart->GetQmitkRenderWindow("sagittal");
  auto* coronalRenderWindow = renderWindowPart->GetQmitkRenderWindow("coronal");
  auto* _3DRenderWindow = renderWindowPart->GetQmitkRenderWindow("3d");
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
    _3DRenderWindow->GetRenderer()))
  {
    selectedRenderWindow = _3DRenderWindow;
  }

  // make node visible
  if (selectedRenderWindow)
  {
    std::string nodeName = node->GetName();
    unsigned int t = nodeName.find_last_of(" ");
    unsigned int id = atof(nodeName.substr(t + 1).c_str()) - 1;

    {
      ctkPluginContext* context = mitk::PluginActivator::getContext();
      ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
      mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);
      selectedRenderWindow->GetSliceNavigationController()->ExecuteOperation(service->GetPlanePosition(id));
      context->ungetService(ppmRef);
    }

    selectedRenderWindow->GetRenderer()->GetCameraController()->Fit();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSegmentationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  if (nodes.size() != 0)
  {
    std::string markerName = "Position";
    unsigned int numberOfNodes = nodes.size();
    std::string nodeName = nodes.at(0)->GetName();
    if ((numberOfNodes == 1) && (nodeName.find(markerName) == 0))
    {
      this->OnContourMarkerSelected(nodes.at(0));
      return;
    }
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
    this->ResetMouseCursor();

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

  m_Controls->btnNewSegmentation->setEnabled(false);
  m_Controls->m_SlicesInterpolator->setEnabled(false);

  if (hasReferenceNode)
  {
    m_Controls->btnNewSegmentation->setEnabled(true);
  }

  if (hasWorkingNode && hasReferenceNode)
  {
    m_Controls->m_SlicesInterpolator->setEnabled(true);

    int layer = -1;
    referenceNode->GetIntProperty("layer", layer);
    workingNode->SetIntProperty("layer", layer + 1);
  }

  this->ValidateSelectionInput();
}

void QmitkSegmentationView::ValidateSelectionInput()
{
  this->UpdateWarningLabel("");

  // the argument is actually not used
  // enable status depends on the tool manager selection
  m_Controls->m_ManualToolSelectionBox2D->setEnabled(false);
  m_Controls->m_ManualToolSelectionBox3D->setEnabled(false);

  mitk::DataNode* referenceNode = m_Controls->patImageSelector->GetSelectedNode();
  mitk::DataNode* workingNode = m_Controls->segImageSelector->GetSelectedNode();

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
      m_Controls->m_ManualToolSelectionBox2D->setEnabled(true);
      m_Controls->m_ManualToolSelectionBox3D->setEnabled(true);
      return;
    }
  }

  m_ToolManager->SetReferenceData(referenceNode);
  m_ToolManager->SetWorkingData(nullptr);
  this->UpdateWarningLabel(tr("Please perform a reinit on the segmentation image!"));
}

void QmitkSegmentationView::UpdateWarningLabel(QString text)
{
  if (text.size() == 0)
  {
    m_Controls->lblSegmentationWarnings->hide();
  }
  else
  {
    m_Controls->lblSegmentationWarnings->show();
  }
  m_Controls->lblSegmentationWarnings->setText("<font color=\"red\">" + text + "</font>");
}
