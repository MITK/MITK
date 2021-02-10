/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QObject>

#include "mitkProperties.h"
#include "mitkSegTool2D.h"
#include "mitkStatusBar.h"

#include "QmitkNewSegmentationDialog.h"
#include <QmitkSegmentationOrganNamesHandling.cpp>

#include <QMessageBox>

#include <berryIWorkbenchPage.h>

#include "QmitkSegmentationView.h"

#include <mitkSurfaceToImageFilter.h>

#include "mitkVtkResliceInterpolationProperty.h"

#include "mitkApplicationCursor.h"
#include "mitkSegmentationObjectFactory.h"
#include "mitkPluginActivator.h"
#include "mitkCameraController.h"
#include "mitkLabelSetImage.h"
#include "mitkImageTimeSelector.h"
#include "mitkNodePredicateSubGeometry.h"

#include <QmitkRenderWindow.h>

#include "usModuleResource.h"
#include "usModuleResourceStream.h"

//micro service to get the ToolManager instance
#include "mitkToolManagerProvider.h"

#include <mitkWorkbenchUtil.h>
#include <regex>

const std::string QmitkSegmentationView::VIEW_ID = "org.mitk.views.segmentation";

QmitkSegmentationView::QmitkSegmentationView()
  : m_Parent(nullptr)
  , m_Controls(nullptr)
  , m_RenderWindowPart(nullptr)
  , m_MouseCursorSet(false)
  , m_DataSelectionChanged(false)
{
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
  mitk::NodePredicateDataType::Pointer isOdf = mitk::NodePredicateDataType::New("OdfImage");
  auto isSegment = mitk::NodePredicateDataType::New("Segment");

  mitk::NodePredicateOr::Pointer validImages = mitk::NodePredicateOr::New();
  validImages->AddPredicate(mitk::NodePredicateAnd::New(isImage, mitk::NodePredicateNot::New(isSegment)));
  validImages->AddPredicate(isDwi);
  validImages->AddPredicate(isDti);
  validImages->AddPredicate(isOdf);

  m_IsNotAHelperObject = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));

  m_IsOfTypeImagePredicate = mitk::NodePredicateAnd::New(validImages, m_IsNotAHelperObject);

  mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateNot::Pointer isNotBinaryPredicate = mitk::NodePredicateNot::New(isBinaryPredicate);

  mitk::NodePredicateAnd::Pointer isABinaryImagePredicate = mitk::NodePredicateAnd::New(m_IsOfTypeImagePredicate, isBinaryPredicate);
  mitk::NodePredicateAnd::Pointer isNotABinaryImagePredicate = mitk::NodePredicateAnd::New(m_IsOfTypeImagePredicate, isNotBinaryPredicate);

  auto isMLImageType = mitk::TNodePredicateDataType<mitk::LabelSetImage>::New();
  mitk::NodePredicateAnd::Pointer isAMLImagePredicate = mitk::NodePredicateAnd::New(isMLImageType, m_IsNotAHelperObject);
  mitk::NodePredicateAnd::Pointer isNotAMLImagePredicate = mitk::NodePredicateAnd::New(mitk::NodePredicateNot::New(isMLImageType), m_IsNotAHelperObject);

  m_IsASegmentationImagePredicate = mitk::NodePredicateOr::New(isABinaryImagePredicate, isAMLImagePredicate);
  m_IsAPatientImagePredicate = mitk::NodePredicateAnd::New(isNotABinaryImagePredicate, isNotAMLImagePredicate);
}

QmitkSegmentationView::~QmitkSegmentationView()
{
  if (m_Controls)
  {
    SetToolSelectionBoxesEnabled(false);
    // deactivate all tools
    mitk::ToolManagerProvider::GetInstance()->GetToolManager()->ActivateTool(-1);

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
    SetToolManagerSelection(nullptr, nullptr);
  }

  delete m_Controls;
}

void QmitkSegmentationView::NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType* nodes)
{
   if (!nodes) return;

   mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
   if (!toolManager) return;
   for (mitk::ToolManager::DataVectorType::iterator iter = nodes->begin(); iter != nodes->end(); ++iter)
   {
      this->FireNodeSelected( *iter );
      // only last iteration meaningful, multiple generated objects are not taken into account here
   }
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
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    m_Controls->m_SlicesInterpolator->SetDataStorage(this->GetDataStorage());
    QList<mitk::SliceNavigationController*> controllers;
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls->m_SlicesInterpolator->Initialize(toolManager, controllers);
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
      m_Controls->btnNewSegmentation->setToolButtonStyle(slimView
        ? Qt::ToolButtonIconOnly
        : Qt::ToolButtonTextOnly);
   }

   auto autoSelectionEnabled = prefs->GetBool("auto selection", true);
   m_Controls->patImageSelector->SetAutoSelectNewNodes(autoSelectionEnabled);
   m_Controls->segImageSelector->SetAutoSelectNewNodes(autoSelectionEnabled);
   this->ForceDisplayPreferencesUponAllImages();
}

void QmitkSegmentationView::CreateNewSegmentation()
{
   mitk::DataNode::Pointer node = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetReferenceData(0);
   if (node.IsNotNull())
   {
     mitk::Image::ConstPointer referenceImage = dynamic_cast<mitk::Image*>(node->GetData());
     if (referenceImage.IsNotNull())
     {
       if (referenceImage->GetDimension() > 1)
       {
         // ask about the name and organ type of the new segmentation
         QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog(m_Parent); // needs a QWidget as parent, "this" is not QWidget
         QStringList organColors = mitk::OrganNamesHandling::GetDefaultOrganColorString();;

         dialog->SetSuggestionList(organColors);

         int dialogReturnValue = dialog->exec();
         if (dialogReturnValue == QDialog::Rejected)
         {
           // user clicked cancel or pressed Esc or something similar
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
           auto result = QMessageBox::question(m_Parent, tr("Create a static or dynamic segmentation?"), tr("The patient image has multiple time steps.\n\nDo you want to create a static segmentation that is identical for all time steps or do you want to create a dynamic segmentation to segment individual time steps?"), tr("Create static segmentation"), tr("Create dynamic segmentation"), QString(), 0,0);
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

         // ask the user about an organ type and name, add this information to the image's (!) propertylist
         // create a new image of the same dimensions and smallest possible pixel type
         mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
         mitk::Tool* firstTool = toolManager->GetToolById(0);
         if (firstTool)
         {
           try
           {
             std::string newNodeName = dialog->GetSegmentationName().toStdString();
             if (newNodeName.empty())
             {
               newNodeName = "no_name";
             }

             mitk::DataNode::Pointer emptySegmentation = firstTool->CreateEmptySegmentationNode(segTemplateImage, newNodeName, dialog->GetColor());
             // initialize showVolume to false to prevent recalculating the volume while working on the segmentation
             emptySegmentation->SetProperty("showVolume", mitk::BoolProperty::New(false));
             if (!emptySegmentation)
             {
               return; // could be aborted by user
             }

             mitk::OrganNamesHandling::UpdateOrganList(organColors, dialog->GetSegmentationName(), dialog->GetColor());

             // escape ';' here (replace by '\;'), see longer comment above
             QString stringForStorage = organColors.replaceInStrings(";", "\\;").join(";");
             MITK_DEBUG << "Will store: " << stringForStorage;
             this->GetPreferences()->Put("Organ-Color-List", stringForStorage);
             this->GetPreferences()->Flush();

             if (mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0))
             {
               mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0)->SetSelected(false);
             }
             emptySegmentation->SetSelected(true);
             this->GetDataStorage()->Add(emptySegmentation, node); // add as a child, because the segmentation "derives" from the original

             m_Controls->segImageSelector->SetCurrentSelectedNode(emptySegmentation);
             mitk::RenderingManager::GetInstance()->InitializeViews(referenceImage->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
             mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->SetPos(imageTimeStep);
           }
           catch (const std::bad_alloc&)
           {
             QMessageBox::warning(nullptr, tr("Create new segmentation"), tr("Could not allocate memory for new segmentation"));
           }
         }
       }
       else
       {
         QMessageBox::information(nullptr, tr("Segmentation"), tr("Segmentation is currently not supported for 2D images"));
       }
     }
   }
   else
   {
     MITK_ERROR << "'Create new segmentation' button should never be clickable unless a patient image is selected...";
   }
}

void QmitkSegmentationView::OnVisiblePropertyChanged()
{
   this->CheckRenderingState();
}

void QmitkSegmentationView::NodeAdded(const mitk::DataNode *node)
{
  if (!m_IsASegmentationImagePredicate->CheckNode(node))
  {
    return;
  }

  itk::SimpleMemberCommand<QmitkSegmentationView>::Pointer command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
  command->SetCallbackFunction(this, &QmitkSegmentationView::OnVisiblePropertyChanged);
  m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(const_cast<mitk::DataNode*>(node), node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));

  ApplyDisplayOptions(const_cast<mitk::DataNode*>(node));
}

void QmitkSegmentationView::NodeRemoved(const mitk::DataNode* node)
{
  if (m_IsASegmentationImagePredicate->CheckNode(node))
  {
    //First of all remove all possible contour markers of the segmentation
    mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers = this->GetDataStorage()->GetDerivations(node, mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true)));

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

    if ((mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0) == node) && m_Controls->patImageSelector->GetSelectedNode().IsNotNull())
    {
      this->SetToolManagerSelection(mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetReferenceData(0), nullptr);
      this->UpdateWarningLabel(tr("Select or create a segmentation"));
    }

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

void QmitkSegmentationView::OnPatientSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
   if(! nodes.empty())
   {
      this->UpdateWarningLabel("");
      auto node = nodes.first();

      auto segPredicate = mitk::NodePredicateAnd::New(m_IsASegmentationImagePredicate.GetPointer(), mitk::NodePredicateSubGeometry::New(node->GetData()->GetGeometry()));

      m_Controls->segImageSelector->SetNodePredicate(segPredicate);

      mitk::DataNode* segNode = m_Controls->segImageSelector->GetSelectedNode();
      this->SetToolManagerSelection(node, segNode);
      if (segNode)
      {
        //Doing this we can assure that the segmentation is always visible if the segmentation and the patient image are
        //loaded separately
        int layer(10);
        node->GetIntProperty("layer", layer);
        layer++;
        segNode->SetProperty("layer", mitk::IntProperty::New(layer));
        this->CheckRenderingState();
      }
      else
      {
         this->SetToolSelectionBoxesEnabled( false );
         this->UpdateWarningLabel(tr("Select or create a segmentation"));
      }
   }
   else
   {
     m_Controls->segImageSelector->SetNodePredicate(m_IsASegmentationImagePredicate);
     this->UpdateWarningLabel(tr("Please select an image!"));
     this->SetToolSelectionBoxesEnabled( false );
   }
}

void QmitkSegmentationView::OnSegmentationSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
   if (nodes.empty())
   {
      this->UpdateWarningLabel(tr("Select or create a segmentation"));
      this->SetToolSelectionBoxesEnabled( false );
      return;
   }

   auto refNode = m_Controls->patImageSelector->GetSelectedNode();
   auto segNode = nodes.front();

   if (!refNode)
   {
     this->UpdateWarningLabel(tr("Please select the matching patient image!"));
     this->SetToolSelectionBoxesEnabled(false);
     this->SetToolManagerSelection(nullptr, segNode);
     return;
   }

   this->CheckRenderingState();
   if ( m_Controls->lblSegmentationWarnings->isVisible()) // "this->CheckRenderingState()" caused a warning. we do not need to go any further
      return;

   this->SetToolManagerSelection(refNode, segNode);

   if (segNode)
   {
     //Doing this we can assure that the segmenation is always visible if the segmentation and the patient image are
     //loaded separately
     int layer(10);
     refNode->GetIntProperty("layer", layer);
     layer++;
     segNode->SetProperty("layer", mitk::IntProperty::New(layer));
   }
   else
   {
     this->SetToolSelectionBoxesEnabled(false);
     this->UpdateWarningLabel(tr("Select or create a segmentation"));
   }

   mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
   if (!renderWindowPart || !segNode->IsVisible(renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer()))
   {
     this->UpdateWarningLabel(tr("The selected segmentation is currently not visible!"));
     this->SetToolSelectionBoxesEnabled( false );
   }
}

void QmitkSegmentationView::OnShowMarkerNodes (bool state)
{
   mitk::SegTool2D::Pointer manualSegmentationTool;

   unsigned int numberOfExistingTools = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetTools().size();

   for(unsigned int i = 0; i < numberOfExistingTools; i++)
   {
      manualSegmentationTool = dynamic_cast<mitk::SegTool2D*>(mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetToolById(i));

      if (manualSegmentationTool)
      {
         if(state == true)
         {
            manualSegmentationTool->SetShowMarkerNodes( true );
         }
         else
         {
            manualSegmentationTool->SetShowMarkerNodes( false );
         }
      }
   }
}

void QmitkSegmentationView::OnContourMarkerSelected(const mitk::DataNode *node)
{
   QmitkRenderWindow* selectedRenderWindow = nullptr;
   QmitkRenderWindow* axialRenderWindow = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("axial");
   QmitkRenderWindow* sagittalRenderWindow = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("sagittal");
   QmitkRenderWindow* coronalRenderWindow = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("coronal");
   QmitkRenderWindow* _3DRenderWindow = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("3d");
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
      unsigned int id = atof(nodeName.substr(t+1).c_str())-1;

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

void QmitkSegmentationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer> &nodes)
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

void QmitkSegmentationView::OnTabWidgetChanged(int id)
{
   //always disable tools on tab changed
   mitk::ToolManagerProvider::GetInstance()->GetToolManager()->ActivateTool(-1);

   //2D Tab ID = 0
   //3D Tab ID = 1
   if (id == 0)
   {
      //Hide 3D selection box, show 2D selection box
      m_Controls->m_ManualToolSelectionBox3D->hide();
      m_Controls->m_ManualToolSelectionBox2D->show();
      //Deactivate possible active tool

      //TODO Remove possible visible interpolations -> Maybe changes in SlicesInterpolator
   }
   else
   {
      //Hide 3D selection box, show 2D selection box
      m_Controls->m_ManualToolSelectionBox2D->hide();
      m_Controls->m_ManualToolSelectionBox3D->show();
      //Deactivate possible active tool
   }
}

void QmitkSegmentationView::SetToolManagerSelection(mitk::DataNode* referenceData, mitk::DataNode* workingData)
{
  // called as a result of new BlueBerry selections
  //   tells the ToolManager for manual segmentation about new selections
  //   updates GUI information about what the user should select
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->SetReferenceData(const_cast<mitk::DataNode*>(referenceData));
  toolManager->SetWorkingData(const_cast<mitk::DataNode*>(workingData));

  m_Controls->btnNewSegmentation->setEnabled(referenceData != nullptr);
}

void QmitkSegmentationView::ForceDisplayPreferencesUponAllImages()
{
   if (!m_Parent)
   {
     return;
   }

   // check all images and segmentations in DataStorage:
   // (items in brackets are implicitly done by previous steps)
   // 1.
   //   if  a reference image is selected,
   //     show the reference image
   //     and hide all other images (orignal and segmentation),
   //     (and hide all segmentations of the other original images)
   //     and show all the reference's segmentations
   //   if no reference image is selected, do do nothing
   //
   // 2.
   //   if  a segmentation is selected,
   //     show it
   //     (and hide all all its siblings (childs of the same parent, incl, nullptr parent))
   //   if no segmentation is selected, do nothing

   if (!m_Controls)
   {
     return; // might happen on initialization (preferences loaded)
   }

   mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
   mitk::DataNode::Pointer referenceData = toolManager->GetReferenceData(0);
   mitk::DataNode::Pointer workingData =   toolManager->GetWorkingData(0);

   // 1.
   if (referenceData.IsNotNull())
   {
      // iterate all images
     mitk::DataStorage::SetOfObjects::ConstPointer allImages = this->GetDataStorage()->GetSubset(m_IsASegmentationImagePredicate);

      for ( mitk::DataStorage::SetOfObjects::const_iterator iter = allImages->begin(); iter != allImages->end(); ++iter)

      {
         mitk::DataNode* node = *iter;
         // apply display preferences
         ApplyDisplayOptions(node);

         // set visibility
         node->SetVisibility(node == referenceData);
      }
   }

   // 2.
   if (workingData.IsNotNull())
      workingData->SetVisibility(true);

   mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::ApplyDisplayOptions(mitk::DataNode* node)
{
  if (!node)
  {
    return;
  }

  mitk::BoolProperty::Pointer drawOutline = mitk::BoolProperty::New(GetPreferences()->GetBool("draw outline", true));
  mitk::LabelSetImage* labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  if (nullptr != labelSetImage)
  {
    // node is actually a multi label segmentation,
    // but its outline property can be set in the 'single label' segmentation preference page as well
    node->SetProperty("labelset.contour.active", drawOutline);
    //node->SetProperty("opacity", mitk::FloatProperty::New(drawOutline->GetValue() ? 1.0f : 0.3f));
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
      //node->SetProperty("opacity", mitk::FloatProperty::New(drawOutline->GetValue() ? 1.0f : 0.3f));
      // force render window update to show outline
      node->GetData()->Modified();
    }
  }
}

void QmitkSegmentationView::CheckRenderingState()
{
  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
  mitk::DataNode* workingNode = m_Controls->segImageSelector->GetSelectedNode();

  if (!workingNode)
  {
    this->SetToolSelectionBoxesEnabled(false);
    this->UpdateWarningLabel(tr("Select or create a segmentation"));
    return;
  }

  bool selectedNodeIsVisible = renderWindowPart && workingNode->IsVisible(renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer());

  if (!selectedNodeIsVisible)
  {
    this->SetToolSelectionBoxesEnabled(false);
    this->UpdateWarningLabel(tr("The selected segmentation is currently not visible!"));
    return;
  }

   /*
   * Here we check whether the geometry of the selected segmentation image if aligned with the worldgeometry
   * At the moment it is not supported to use a geometry different from the selected image for reslicing.
   * For further information see Bug 16063
   */

   const mitk::BaseGeometry* worldGeo = this->GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetSliceNavigationController()->GetCurrentGeometry3D();

   if (workingNode && worldGeo)
   {

      const mitk::BaseGeometry* workingNodeGeo = workingNode->GetData()->GetGeometry();
      const mitk::BaseGeometry* worldGeo = this->GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetSliceNavigationController()->GetCurrentGeometry3D();

      if (mitk::Equal(*workingNodeGeo->GetBoundingBox(), *worldGeo->GetBoundingBox(), mitk::eps, true))
      {
         this->SetToolManagerSelection(m_Controls->patImageSelector->GetSelectedNode(), workingNode);
         this->SetToolSelectionBoxesEnabled(true);
         this->UpdateWarningLabel("");
         return;
      }
   }

   this->SetToolManagerSelection(m_Controls->patImageSelector->GetSelectedNode(), nullptr);
   this->SetToolSelectionBoxesEnabled(false);
   this->UpdateWarningLabel(tr("Please perform a reinit on the segmentation image!"));
}

void QmitkSegmentationView::UpdateWarningLabel(QString text)
{
   if (text.size() == 0)
      m_Controls->lblSegmentationWarnings->hide();
   else
      m_Controls->lblSegmentationWarnings->show();
   m_Controls->lblSegmentationWarnings->setText("<font color=\"red\">" + text + "</font>");
}

void QmitkSegmentationView::CreateQtPartControl(QWidget* parent)
{
   // setup the basic GUI of this view
   m_Parent = parent;

   m_Controls = new Ui::QmitkSegmentationControls;
   m_Controls->setupUi(parent);

   m_Controls->patImageSelector->SetDataStorage(GetDataStorage());
   m_Controls->patImageSelector->SetNodePredicate(m_IsAPatientImagePredicate);
   m_Controls->patImageSelector->SetSelectionIsOptional(false);
   m_Controls->patImageSelector->SetInvalidInfo("Select an image.");
   m_Controls->patImageSelector->SetPopUpTitel("Select an image.");
   m_Controls->patImageSelector->SetPopUpHint("Select an image that should be used to define the geometry and bounds of the segmentation.");

   UpdateWarningLabel(tr("Please select an image"));

   if (m_Controls->patImageSelector->GetSelectedNode().IsNotNull())
   {
     UpdateWarningLabel(tr("Select or create a new segmentation"));
   }

   m_Controls->segImageSelector->SetDataStorage(GetDataStorage());
   m_Controls->segImageSelector->SetNodePredicate(m_IsASegmentationImagePredicate);
   m_Controls->segImageSelector->SetSelectionIsOptional(false);
   m_Controls->segImageSelector->SetInvalidInfo("Select a segmentation.");
   m_Controls->segImageSelector->SetPopUpTitel("Select a segmentation.");
   m_Controls->segImageSelector->SetPopUpHint("Select a segmentation that should be modified. Only segmentation with the same geometry and within the bounds of the reference image are selected.");

   if (m_Controls->segImageSelector->GetSelectedNode().IsNotNull())
   {
     UpdateWarningLabel("");
   }

   mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
   assert(toolManager);

   toolManager->SetDataStorage(*(GetDataStorage()));
   toolManager->InitializeTools();

   QString segTools2D = tr("Add Subtract Paint Wipe 'Region Growing' Fill Erase 'Live Wire' '2D Fast Marching'");
   QString segTools3D = tr("Threshold 'UL Threshold' Otsu 'Fast Marching 3D' 'Region Growing 3D' Watershed Picking");

   std::regex extSegTool2DRegEx("SegTool2D$");
   std::regex extSegTool3DRegEx("SegTool3D$");

   auto tools = toolManager->GetTools();

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
   m_Controls->m_ManualToolSelectionBox2D->setEnabled(true);
   m_Controls->m_ManualToolSelectionBox2D->SetGenerateAccelerators(true);
   m_Controls->m_ManualToolSelectionBox2D->SetToolGUIArea(m_Controls->m_ManualToolGUIContainer2D);

   m_Controls->m_ManualToolSelectionBox2D->SetDisplayedToolGroups(segTools2D.toStdString());
   m_Controls->m_ManualToolSelectionBox2D->SetLayoutColumns(3);
   m_Controls->m_ManualToolSelectionBox2D->SetEnabledMode(QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);
   connect(m_Controls->m_ManualToolSelectionBox2D, &QmitkToolSelectionBox::ToolSelected, this, &QmitkSegmentationView::OnManualTool2DSelected);

   //setup 3D Tools
   m_Controls->m_ManualToolSelectionBox3D->setEnabled(true);
   m_Controls->m_ManualToolSelectionBox3D->SetGenerateAccelerators(true);
   m_Controls->m_ManualToolSelectionBox3D->SetToolGUIArea(m_Controls->m_ManualToolGUIContainer3D);
   //specify tools to be added to 3D Tool area
   m_Controls->m_ManualToolSelectionBox3D->SetDisplayedToolGroups(segTools3D.toStdString());
   m_Controls->m_ManualToolSelectionBox3D->SetLayoutColumns(3);
   m_Controls->m_ManualToolSelectionBox3D->SetEnabledMode(QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);

   //Hide 3D selection box, show 2D selection box
   m_Controls->m_ManualToolSelectionBox3D->hide();
   m_Controls->m_ManualToolSelectionBox2D->show();

   // update the list of segmentations
   toolManager->NewNodeObjectsGenerated += mitk::MessageDelegate1<QmitkSegmentationView, mitk::ToolManager::DataVectorType*>(this, &QmitkSegmentationView::NewNodeObjectsGenerated);

   // create signal/slot connections
   connect(m_Controls->patImageSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkSegmentationView::OnPatientSelectionChanged);
   connect(m_Controls->segImageSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkSegmentationView::OnSegmentationSelectionChanged);

   connect(m_Controls->btnNewSegmentation, &QToolButton::clicked, this, &QmitkSegmentationView::CreateNewSegmentation);
   connect(m_Controls->tabWidgetSegmentationTools, &QTabWidget::currentChanged, this, &QmitkSegmentationView::OnTabWidgetChanged);
   connect(m_Controls->m_SlicesInterpolator, &QmitkSlicesInterpolator::SignalShowMarkerNodes, this, &QmitkSegmentationView::OnShowMarkerNodes);

   // set callback function for already existing nodes (images & segmentations)
   mitk::DataStorage::SetOfObjects::ConstPointer allImages = GetDataStorage()->GetSubset(m_IsOfTypeImagePredicate);
   for (mitk::DataStorage::SetOfObjects::const_iterator iter = allImages->begin(); iter != allImages->end(); ++iter)
   {
     mitk::DataNode* node = *iter;
     itk::SimpleMemberCommand<QmitkSegmentationView>::Pointer command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
     command->SetCallbackFunction(this, &QmitkSegmentationView::OnVisiblePropertyChanged);
     m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(node, node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));
   }

   itk::SimpleMemberCommand<QmitkSegmentationView>::Pointer command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
   command->SetCallbackFunction(this, &QmitkSegmentationView::CheckRenderingState);
   m_RenderingManagerObserverTag = mitk::RenderingManager::GetInstance()->AddObserver(mitk::RenderingManagerViewsInitializedEvent(), command);

   SetToolManagerSelection(m_Controls->patImageSelector->GetSelectedNode(), m_Controls->segImageSelector->GetSelectedNode());

   m_RenderWindowPart = GetRenderWindowPart();
   if (m_RenderWindowPart)
   {
     RenderWindowPartActivated(m_RenderWindowPart);
   }

   //Should be done last, if everything else is configured because it triggers the autoselection of data.
   m_Controls->patImageSelector->SetAutoSelectNewNodes(true);
   m_Controls->segImageSelector->SetAutoSelectNewNodes(true);
}

void QmitkSegmentationView::SetFocus()
{
  m_Controls->btnNewSegmentation->setFocus();
}

void QmitkSegmentationView::OnManualTool2DSelected(int id)
{
   if (id >= 0)
   {
      std::string text = "Active Tool: \"";
      mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
      text += toolManager->GetToolById(id)->GetName();
      text += "\"";
      mitk::StatusBar::GetInstance()->DisplayText(text.c_str());

      us::ModuleResource resource = toolManager->GetToolById(id)->GetCursorIconResource();
      this->SetMouseCursor(resource, 0, 0);
   }
   else
   {
      this->ResetMouseCursor();
      mitk::StatusBar::GetInstance()->DisplayText("");
   }
}

void QmitkSegmentationView::ResetMouseCursor()
{
   if ( m_MouseCursorSet )
   {
      mitk::ApplicationCursor::GetInstance()->PopCursor();
      m_MouseCursorSet = false;
   }
}

void QmitkSegmentationView::SetMouseCursor( const us::ModuleResource& resource, int hotspotX, int hotspotY )
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

void QmitkSegmentationView::SetToolSelectionBoxesEnabled(bool status)
{
  if (status)
  {
    m_Controls->m_ManualToolSelectionBox2D->RecreateButtons();
    m_Controls->m_ManualToolSelectionBox3D->RecreateButtons();
  }

  m_Controls->m_ManualToolSelectionBox2D->setEnabled(status);
  m_Controls->m_ManualToolSelectionBox3D->setEnabled(status);
  m_Controls->m_SlicesInterpolator->setEnabled(status);
}
