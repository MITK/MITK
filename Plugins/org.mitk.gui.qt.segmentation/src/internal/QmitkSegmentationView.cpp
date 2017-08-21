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

#include <QmitkRenderWindow.h>

#include "usModuleResource.h"
#include "usModuleResourceStream.h"

//micro service to get the ToolManager instance
#include "mitkToolManagerProvider.h"

const std::string QmitkSegmentationView::VIEW_ID = "org.mitk.views.segmentation";

QmitkSegmentationView::QmitkSegmentationView()
  : m_Parent(nullptr)
  , m_Controls(nullptr)
  , m_RenderWindowPart(nullptr)
  , m_MouseCursorSet(false)
  , m_DataSelectionChanged(false)
  , m_AutoSelectionEnabled(false)
{
  mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
  mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");
  mitk::NodePredicateOr::Pointer isDiffusionImage = mitk::NodePredicateOr::New(isDwi, isDti);
  isDiffusionImage = mitk::NodePredicateOr::New(isDiffusionImage, isQbi);
  m_IsOfTypeImagePredicate = mitk::NodePredicateOr::New(isDiffusionImage, mitk::TNodePredicateDataType<mitk::Image>::New());

  m_IsBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  m_IsNotBinaryPredicate = mitk::NodePredicateNot::New(m_IsBinaryPredicate);

  m_IsNotABinaryImagePredicate = mitk::NodePredicateAnd::New(m_IsOfTypeImagePredicate, m_IsNotBinaryPredicate);
  m_IsABinaryImagePredicate = mitk::NodePredicateAnd::New(m_IsOfTypeImagePredicate, m_IsBinaryPredicate);

  m_IsASegmentationImagePredicate = mitk::NodePredicateOr::New(m_IsABinaryImagePredicate, mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  m_IsAPatientImagePredicate = mitk::NodePredicateAnd::New(m_IsNotABinaryImagePredicate, mitk::NodePredicateNot::New(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New()));
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

    for (NodeTagMapType::iterator dataIter = m_BinaryPropertyObserverTags.begin(); dataIter != m_BinaryPropertyObserverTags.end(); ++dataIter)
    {
      (*dataIter).first->GetProperty("binary")->RemoveObserver((*dataIter).second);
    }
    m_BinaryPropertyObserverTags.clear();

    mitk::RenderingManager::GetInstance()->RemoveObserver(m_RenderingManagerObserverTag);

    ctkPluginContext* context = mitk::PluginActivator::getContext();
    ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
    mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);
    service->RemoveAllPlanePositions();
    context->ungetService(ppmRef);
    SetToolManagerSelection(0, 0);
  }

  delete m_Controls;
}

void QmitkSegmentationView::NewNodesGenerated()
{
  MITK_WARN << "Use of deprecated function: NewNodesGenerated!! This function is empty and will be removed in the next time!";
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

void QmitkSegmentationView::Visible()
{
}

void QmitkSegmentationView::Hidden()
{
}

void QmitkSegmentationView::Activated()
{
}

void QmitkSegmentationView::Deactivated()
{
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
   }

   m_AutoSelectionEnabled = prefs->GetBool("auto selection", false);
   this->ForceDisplayPreferencesUponAllImages();
}

void QmitkSegmentationView::CreateNewSegmentation()
{
   mitk::DataNode::Pointer node = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetReferenceData(0);
   if (node.IsNotNull())
   {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNotNull())
      {
         if (image->GetDimension()>1)
         {
            // ask about the name and organ type of the new segmentation
            QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( m_Parent ); // needs a QWidget as parent, "this" is not QWidget

            QString storedList = this->GetPreferences()->Get("Organ-Color-List","");
            QStringList organColors;
            if (storedList.isEmpty())
            {
              organColors = mitk::OrganNamesHandling::GetDefaultOrganColorString();
            }
            else
            {
               /*
               a couple of examples of how organ names are stored:

               a simple item is built up like 'name#AABBCC' where #AABBCC is the hexadecimal notation of a color as known from HTML

               items are stored separated by ';'
               this makes it necessary to escape occurrences of ';' in name.
               otherwise the string "hugo;ypsilon#AABBCC;eugen#AABBCC" could not be parsed as two organs
               but we would get "hugo" and "ypsilon#AABBCC" and "eugen#AABBCC"

               so the organ name "hugo;ypsilon" is stored as "hugo\;ypsilon"
               and must be unescaped after loading

               the following lines could be one split with Perl's negative lookbehind
               */

               // recover string list from BlueBerry view's preferences
               QString storedString = this->GetPreferences()->Get("Organ-Color-List","");
               MITK_DEBUG << "storedString: " << storedString.toStdString();
               // match a string consisting of any number of repetitions of either "anything but ;" or "\;". This matches everything until the next unescaped ';'
               QRegExp onePart("(?:[^;]|\\\\;)*");
               MITK_DEBUG << "matching " << onePart.pattern().toStdString();
               int count = 0;
               int pos = 0;
               while( (pos = onePart.indexIn( storedString, pos )) != -1 )
               {
                  ++count;
                  int length = onePart.matchedLength();
                  if (length == 0) break;
                  QString matchedString = storedString.mid(pos, length);
                  MITK_DEBUG << "   Captured length " << length << ": " << matchedString.toStdString();
                  pos += length + 1; // skip separating ';'

                  // unescape possible occurrences of '\;' in the string
                  matchedString.replace("\\;", ";");

                  // add matched string part to output list
                  organColors << matchedString;
               }
               MITK_DEBUG << "Captured " << count << " organ name/colors";
            }

            dialog->SetSuggestionList( organColors );

            int dialogReturnValue = dialog->exec();

            if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

            // ask the user about an organ type and name, add this information to the image's (!) propertylist
            // create a new image of the same dimensions and smallest possible pixel type
            mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
            mitk::Tool* firstTool = toolManager->GetToolById(0);
            if (firstTool)
            {
               try
               {
                  std::string newNodeName = dialog->GetSegmentationName().toStdString();
                  if(newNodeName.empty())
                     newNodeName = "no_name";

                  mitk::DataNode::Pointer emptySegmentation =
                     firstTool->CreateEmptySegmentationNode( image, newNodeName, dialog->GetColor() );

                  // initialize showVolume to false to prevent recalculating the volume while working on the segmentation
                  emptySegmentation->SetProperty( "showVolume", mitk::BoolProperty::New( false ) );

                  if (!emptySegmentation) return; // could be aborted by user

                  mitk::OrganNamesHandling::UpdateOrganList(organColors, dialog->GetSegmentationName(), dialog->GetColor());

                  /*
                  escape ';' here (replace by '\;'), see longer comment above
                  */
                  QString stringForStorage = organColors.replaceInStrings(";","\\;").join(";");
                  MITK_DEBUG << "Will store: " << stringForStorage;
                  this->GetPreferences()->Put("Organ-Color-List", stringForStorage);
                  this->GetPreferences()->Flush();

                  if(mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0))
                  {
                     mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0)->SetSelected(false);
                  }
                  emptySegmentation->SetSelected(true);
                  this->GetDataStorage()->Add( emptySegmentation, node ); // add as a child, because the segmentation "derives" from the original

                  this->ApplyDisplayOptions( emptySegmentation );
                  this->FireNodeSelected( emptySegmentation );
                  this->OnSelectionChanged( emptySegmentation );

                  m_Controls->segImageSelector->SetSelectedNode(emptySegmentation);
                  mitk::RenderingManager::GetInstance()->InitializeViews(emptySegmentation->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
               }
               catch (std::bad_alloc)
               {
                  QMessageBox::warning(nullptr,tr("Create new segmentation"),tr("Could not allocate memory for new segmentation"));
               }
            }
         }
         else
         {
            QMessageBox::information(nullptr,tr("Segmentation"),tr("Segmentation is currently not supported for 2D images"));
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
   mitk::DataNode* selectedNode = m_Controls->segImageSelector->GetSelectedNode();
   if ( !selectedNode )
   {
     this->SetToolSelectionBoxesEnabled(false);
     return;
   }

   mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
   bool selectedNodeIsVisible = renderWindowPart && selectedNode->IsVisible(renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer());

   if (!selectedNodeIsVisible)
   {
      this->SetToolSelectionBoxesEnabled(false);
      this->UpdateWarningLabel(tr("The selected segmentation is currently not visible!"));
   }
   else
   {
      this->SetToolSelectionBoxesEnabled(true);
      this->UpdateWarningLabel("");
   }
}

void QmitkSegmentationView::OnBinaryPropertyChanged()
{
   mitk::DataStorage::SetOfObjects::ConstPointer patImages = m_Controls->patImageSelector->GetNodes();

   bool isBinary(false);

   for (mitk::DataStorage::SetOfObjects::ConstIterator it = patImages->Begin(); it != patImages->End(); ++it)
   {
      const mitk::DataNode* node = it->Value();
      node->GetBoolProperty("binary", isBinary);
      mitk::LabelSetImage::Pointer labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
      isBinary = isBinary || labelSetImage.IsNotNull();

      if(isBinary)
      {
         m_Controls->patImageSelector->RemoveNode(node);
         m_Controls->segImageSelector->AddNode(node);
         this->SetToolManagerSelection(nullptr,nullptr);
         return;
      }
   }

   mitk::DataStorage::SetOfObjects::ConstPointer segImages = m_Controls->segImageSelector->GetNodes();

   isBinary = true;

   for (mitk::DataStorage::SetOfObjects::ConstIterator it = segImages->Begin(); it != segImages->End(); ++it)
   {
      const mitk::DataNode* node = it->Value();
      node->GetBoolProperty("binary", isBinary);
      mitk::LabelSetImage::Pointer labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
      isBinary = isBinary || labelSetImage.IsNotNull();

      if(!isBinary)
      {
         m_Controls->segImageSelector->RemoveNode(node);
         m_Controls->patImageSelector->AddNode(node);
         if (mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0) == node)
            mitk::ToolManagerProvider::GetInstance()->GetToolManager()->SetWorkingData(nullptr);
         return;
      }
   }
}

void QmitkSegmentationView::NodeAdded(const mitk::DataNode *node)
{
  bool isBinary(false);
  bool isHelperObject(false);
  bool isImage(false);
  node->GetBoolProperty("binary", isBinary);
  mitk::LabelSetImage::Pointer labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  isBinary = isBinary || labelSetImage.IsNotNull();
  node->GetBoolProperty("helper object", isHelperObject);

  if (dynamic_cast<mitk::Image*>(node->GetData()))
  {
    isImage = true;
  }

  if (m_AutoSelectionEnabled)
  {
    if (!isBinary && isImage)
    {
      FireNodeSelected(const_cast<mitk::DataNode*>(node));
    }
  }

  if (isImage && !isHelperObject)
  {
    itk::SimpleMemberCommand<QmitkSegmentationView>::Pointer command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
    command->SetCallbackFunction(this, &QmitkSegmentationView::OnVisiblePropertyChanged);
    m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(const_cast<mitk::DataNode*>(node), node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));

    itk::SimpleMemberCommand<QmitkSegmentationView>::Pointer command2 = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
    command2->SetCallbackFunction(this, &QmitkSegmentationView::OnBinaryPropertyChanged);
    m_BinaryPropertyObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(const_cast<mitk::DataNode*>(node), node->GetProperty("binary")->AddObserver(itk::ModifiedEvent(), command2)));

    ApplyDisplayOptions(const_cast<mitk::DataNode*>(node));
    m_Controls->segImageSelector->setCurrentIndex(m_Controls->segImageSelector->Find(node));
  }
}

void QmitkSegmentationView::NodeRemoved(const mitk::DataNode* node)
{
   bool isSeg(false);
   bool isHelperObject(false);
   node->GetBoolProperty("helper object", isHelperObject);
   node->GetBoolProperty("binary", isSeg);
   mitk::LabelSetImage::Pointer labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
   isSeg = isSeg || labelSetImage.IsNotNull();

   mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
   if(isSeg && !isHelperObject && image)
   {
      //First of all remove all possible contour markers of the segmentation
      mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers = this->GetDataStorage()->GetDerivations(node, mitk::NodePredicateProperty::New("isContourMarker"
         , mitk::BoolProperty::New(true)));

      ctkPluginContext* context = mitk::PluginActivator::getContext();
      ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
      mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);

      for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End(); ++it)
      {
         std::string nodeName = node->GetName();
         unsigned int t = nodeName.find_last_of(" ");
         unsigned int id = atof(nodeName.substr(t+1).c_str())-1;

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

      mitk::SurfaceInterpolationController::GetInstance()->RemoveInterpolationSession(image);
   }
   mitk::DataNode* tempNode = const_cast<mitk::DataNode*>(node);
   //Since the binary property could be changed during runtime by the user
   if (image && !isHelperObject)
   {
      node->GetProperty("visible")->RemoveObserver( m_WorkingDataObserverTags[tempNode] );
      m_WorkingDataObserverTags.erase(tempNode);
      node->GetProperty("binary")->RemoveObserver( m_BinaryPropertyObserverTags[tempNode] );
      m_BinaryPropertyObserverTags.erase(tempNode);
   }

   if((mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetReferenceData(0) == node))
   {
      //as we don't know which node was actually removed e.g. our reference node, disable 'New Segmentation' button.
      //consider the case that there is no more image in the datastorage
      this->SetToolManagerSelection(nullptr, nullptr);
      this->SetToolSelectionBoxesEnabled( false );
   }
}

void QmitkSegmentationView::OnPatientComboBoxSelectionChanged( const mitk::DataNode* node )
{
   //mitk::DataNode* selectedNode = const_cast<mitk::DataNode*>(node);
   if( node != nullptr )
   {
      this->UpdateWarningLabel("");
      mitk::DataNode* segNode = m_Controls->segImageSelector->GetSelectedNode();
      if (segNode)
      {
        mitk::DataStorage::SetOfObjects::ConstPointer possibleParents = this->GetDataStorage()->GetSources(segNode, m_IsAPatientImagePredicate);
         bool isSourceNode(false);

         for (mitk::DataStorage::SetOfObjects::ConstIterator it = possibleParents->Begin(); it != possibleParents->End(); it++)
         {
            if (it.Value() == node)
               isSourceNode = true;
         }

         if ( !isSourceNode && (!this->CheckForSameGeometry(segNode, node) || possibleParents->Size() > 0 ))
         {
            this->SetToolManagerSelection(node, nullptr);
            this->SetToolSelectionBoxesEnabled( false );
            this->UpdateWarningLabel(tr("The selected patient image does not match with the selected segmentation!"));
         }
         else if ((!isSourceNode && this->CheckForSameGeometry(segNode, node)) || isSourceNode )
         {
            this->SetToolManagerSelection(node, segNode);
            //Doing this we can assure that the segmenation is always visible if the segmentation and the patient image are
            //loaded separately
            int layer(10);
            node->GetIntProperty("layer", layer);
            layer++;
            segNode->SetProperty("layer", mitk::IntProperty::New(layer));
            //this->UpdateWarningLabel("");
            RenderingManagerReinitialized();
         }
      }
      else
      {
         this->SetToolManagerSelection(node, nullptr);
         this->SetToolSelectionBoxesEnabled( false );
         this->UpdateWarningLabel(tr("Select or create a segmentation"));
      }
   }
   else
   {
      this->UpdateWarningLabel(tr("Please load an image!"));
      this->SetToolSelectionBoxesEnabled( false );
   }
}

void QmitkSegmentationView::OnSegmentationComboBoxSelectionChanged(const mitk::DataNode *node)
{
   if (node == nullptr)
   {
      this->UpdateWarningLabel(tr("Select or create a segmentation"));
      this->SetToolSelectionBoxesEnabled( false );
      return;
   }

   mitk::DataNode* refNode = m_Controls->patImageSelector->GetSelectedNode();

   RenderingManagerReinitialized();
   if ( m_Controls->lblSegmentationWarnings->isVisible()) // "RenderingManagerReinitialized()" caused a warning. we do not need to go any further
      return;

   if (m_AutoSelectionEnabled)
   {
      this->OnSelectionChanged(const_cast<mitk::DataNode*>(node));
   }
   else
   {
     mitk::DataStorage::SetOfObjects::ConstPointer possibleParents = this->GetDataStorage()->GetSources(node, m_IsAPatientImagePredicate);

      if ( possibleParents->Size() == 1 )
      {
         mitk::DataNode* parentNode = possibleParents->ElementAt(0);

         if (parentNode != refNode)
         {
            this->UpdateWarningLabel(tr("The selected segmentation does not match with the selected patient image!"));
            this->SetToolSelectionBoxesEnabled( false );
            this->SetToolManagerSelection(nullptr, node);
         }
         else
         {
            this->UpdateWarningLabel("");
            this->SetToolManagerSelection(refNode, node);
         }
      }
      else if (refNode && this->CheckForSameGeometry(node, refNode))
      {
         this->UpdateWarningLabel("");
         this->SetToolManagerSelection(refNode, node);
      }
      else if (!refNode || !this->CheckForSameGeometry(node, refNode))
      {
         this->UpdateWarningLabel(tr("Please select or load the according patient image!"));
      }
   }

   mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
   if (!renderWindowPart || !node->IsVisible(renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer()))
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

void QmitkSegmentationView::OnSelectionChanged(mitk::DataNode* node)
{
   berry::IWorkbenchPart::Pointer nullPart;
   QList<mitk::DataNode::Pointer> nodes;
   nodes.push_back(node);
   this->OnSelectionChanged(nullPart, nodes);
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
      OnContourMarkerSelected(nodes.at(0));
      return;
    }
  }

  if (m_AutoSelectionEnabled)
  {
    if (nodes.size() == 0 && m_Controls->patImageSelector->GetSelectedNode().IsNull())
    {
      SetToolManagerSelection(nullptr, nullptr);
    }
    else if (nodes.size() == 1)
    {
      mitk::DataNode::Pointer selectedNode = nodes.at(0);
      if (selectedNode.IsNull())
      {
        return;
      }

      mitk::Image::Pointer selectedImage = dynamic_cast<mitk::Image*>(selectedNode->GetData());
      if (selectedImage.IsNull())
      {
        SetToolManagerSelection(nullptr, nullptr);
        return;
      }

      bool isASegmentation(false);
      selectedNode->GetBoolProperty("binary", isASegmentation);
      mitk::LabelSetImage::Pointer labelSetImage = dynamic_cast<mitk::LabelSetImage*>(selectedNode->GetData());
      isASegmentation = isASegmentation || labelSetImage.IsNotNull();

      if (isASegmentation)
      {
        // set all nodes to invisible
        mitk::DataStorage::SetOfObjects::ConstPointer allImages = GetDataStorage()->GetSubset(m_IsOfTypeImagePredicate);
        for (mitk::DataStorage::SetOfObjects::const_iterator iter = allImages->begin(); iter != allImages->end(); ++iter)
        {
          (*iter)->SetVisibility(false);
        }

        // if a segmentation is selected find a possible reference image:
        mitk::DataStorage::SetOfObjects::ConstPointer sources = GetDataStorage()->GetSources(selectedNode, m_IsAPatientImagePredicate);
        mitk::DataNode::Pointer refNode;
        if (sources->Size() != 0)
        {
          // found one or more sources - use the first one
          refNode = sources->ElementAt(0);
          refNode->SetVisibility(true);
          selectedNode->SetVisibility(true);
          SetToolManagerSelection(refNode, selectedNode);
        }
        else
        {
          // did not find a source, check all images an compare geometry
          mitk::DataStorage::SetOfObjects::ConstPointer possiblePatientImages = GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);
          for (mitk::DataStorage::SetOfObjects::ConstIterator iter = possiblePatientImages->Begin(); iter != possiblePatientImages->End(); ++iter)
          {
            refNode = iter->Value();

            if (CheckForSameGeometry(selectedNode, iter->Value()))
            {
              refNode->SetVisibility(true);
              selectedNode->SetVisibility(true);
              SetToolManagerSelection(refNode, selectedNode);

              // doing this we can assure that the segmentation is always visible if the segmentation and the patient image are at the
              // same level in the data manager
              int layer(10);
              refNode->GetIntProperty("layer", layer);
              layer++;
              selectedNode->SetProperty("layer", mitk::IntProperty::New(layer));
              return;
            }
          }
          // did not find a source / patient image with the same geometry
          SetToolManagerSelection(nullptr, selectedNode);
        }
        mitk::RenderingManager::GetInstance()->InitializeViews(selectedNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
      }
      else
      {
        if (mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetReferenceData(0) != selectedNode)
        {
          SetToolManagerSelection(selectedNode, nullptr);
          // may be a bug in the selection services. A node which is deselected will be passed as selected node to the OnSelectionChanged function
          mitk::IRenderWindowPart* renderWindowPart = GetRenderWindowPart();
          if (renderWindowPart && !selectedNode->IsVisible(renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer()))
          {
            selectedNode->SetVisibility(true);
          }
          UpdateWarningLabel(tr("The selected patient image does not match with the selected segmentation!"));
          SetToolSelectionBoxesEnabled(false);
        }
      }
    }

    if (m_Controls->lblSegmentationWarnings->isVisible()) // "RenderingManagerReinitialized()" caused a warning. we do not need to go any further
    {
      return;
    }
    RenderingManagerReinitialized();
  }
}

void QmitkSegmentationView::OnContourMarkerSelected(const mitk::DataNode *node)
{
   QmitkRenderWindow* selectedRenderWindow = 0;
   QmitkRenderWindow* axialRenderWindow =
      this->GetRenderWindowPart(OPEN)->GetQmitkRenderWindow("axial");
   QmitkRenderWindow* sagittalRenderWindow =
      this->GetRenderWindowPart(OPEN)->GetQmitkRenderWindow("sagittal");
   QmitkRenderWindow* coronalRenderWindow =
      this->GetRenderWindowPart(OPEN)->GetQmitkRenderWindow("coronal");
   QmitkRenderWindow* _3DRenderWindow =
      this->GetRenderWindowPart(OPEN)->GetQmitkRenderWindow("3d");
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

void QmitkSegmentationView::InitToolManagerSelection(const mitk::DataNode* referenceData, const mitk::DataNode* workingData)
{
  // initial tool manager selection, called from 'CreateQtPartControl'
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->SetReferenceData(const_cast<mitk::DataNode*>(referenceData));
  toolManager->SetWorkingData(const_cast<mitk::DataNode*>(workingData));

  // check original image
  m_Controls->btnNewSegmentation->setEnabled(referenceData != nullptr);
  if (referenceData)
  {
    UpdateWarningLabel("");
  }
}

void QmitkSegmentationView::SetToolManagerSelection(const mitk::DataNode* referenceData, const mitk::DataNode* workingData)
{
  // called as a result of new BlueBerry selections
  //   tells the ToolManager for manual segmentation about new selections
  //   updates GUI information about what the user should select
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->SetReferenceData(const_cast<mitk::DataNode*>(referenceData));
  toolManager->SetWorkingData(const_cast<mitk::DataNode*>(workingData));

  // check original image
  m_Controls->btnNewSegmentation->setEnabled(referenceData != nullptr);
  if (referenceData)
  {
    UpdateWarningLabel("");
    disconnect(m_Controls->patImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnPatientComboBoxSelectionChanged(const mitk::DataNode*)));
    m_Controls->patImageSelector->setCurrentIndex(m_Controls->patImageSelector->Find(referenceData));
    connect(m_Controls->patImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnPatientComboBoxSelectionChanged(const mitk::DataNode*)));

    // check segmentation
    if (workingData)
    {
      //FireNodeSelected(const_cast<mitk::DataNode*>(workingData));

      disconnect(m_Controls->segImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnSegmentationComboBoxSelectionChanged(const mitk::DataNode*)));
      m_Controls->segImageSelector->setCurrentIndex(m_Controls->segImageSelector->Find(workingData));
      connect(m_Controls->segImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnSegmentationComboBoxSelectionChanged(const mitk::DataNode*)));
    }
  }
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
  mitk::BoolProperty::Pointer volumeRendering = mitk::BoolProperty::New(GetPreferences()->GetBool("volume rendering", false));
  mitk::LabelSetImage* labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  if (nullptr != labelSetImage)
  {
    // node is actually a multi label segmentation,
    // but its outline property can be set in the 'single label' segmentation preference page as well
    node->SetProperty("labelset.contour.active", drawOutline);
    node->SetProperty("opacity", mitk::FloatProperty::New(drawOutline->GetValue() ? 1.0f : 0.3f));
    node->SetProperty("volumerendering", volumeRendering);
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
      node->SetProperty("opacity", mitk::FloatProperty::New(drawOutline->GetValue() ? 1.0f : 0.3f));
      node->SetProperty("volumerendering", volumeRendering);
      // force render window update to show outline
      node->GetData()->Modified();
    }
  }
}

void QmitkSegmentationView::RenderingManagerReinitialized()
{
   if (!this->GetRenderWindowPart())
   {
     return;
   }

   /*
   * Here we check whether the geometry of the selected segmentation image if aligned with the worldgeometry
   * At the moment it is not supported to use a geometry different from the selected image for reslicing.
   * For further information see Bug 16063
   */
   mitk::DataNode* workingNode = m_Controls->segImageSelector->GetSelectedNode();
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
      }
      else
      {
         this->SetToolManagerSelection(m_Controls->patImageSelector->GetSelectedNode(), nullptr);
         this->SetToolSelectionBoxesEnabled(false);
         this->UpdateWarningLabel(tr("Please perform a reinit on the segmentation image!"));
      }
   }
}

bool QmitkSegmentationView::CheckForSameGeometry(const mitk::DataNode *node1, const mitk::DataNode *node2) const
{
   bool isSameGeometry(true);

   mitk::Image* image1 = dynamic_cast<mitk::Image*>(node1->GetData());
   mitk::Image* image2 = dynamic_cast<mitk::Image*>(node2->GetData());
   if (image1 && image2)
   {
      mitk::BaseGeometry* geo1 = image1->GetGeometry();
      mitk::BaseGeometry* geo2 = image2->GetGeometry();

      isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetOrigin(), geo2->GetOrigin());
      isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(0), geo2->GetExtent(0));
      isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(1), geo2->GetExtent(1));
      isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(2), geo2->GetExtent(2));
      isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetSpacing(), geo2->GetSpacing());
      isSameGeometry = isSameGeometry && mitk::MatrixEqualElementWise(geo1->GetIndexToWorldTransform()->GetMatrix(), geo2->GetIndexToWorldTransform()->GetMatrix());

      return isSameGeometry;
   }
   else
   {
      return false;
   }
}

void QmitkSegmentationView::UpdateWarningLabel(QString text)
{
   if (text.size() == 0)
      m_Controls->lblSegmentationWarnings->hide();
   else
      m_Controls->lblSegmentationWarnings->show();
   m_Controls->lblSegmentationWarnings->setText(text);
}

void QmitkSegmentationView::CreateQtPartControl(QWidget* parent)
{
   // setup the basic GUI of this view
   m_Parent = parent;

   m_Controls = new Ui::QmitkSegmentationControls;
   m_Controls->setupUi(parent);

   m_Controls->patImageSelector->SetDataStorage(GetDataStorage());
   m_Controls->patImageSelector->SetPredicate(mitk::NodePredicateAnd::New(m_IsAPatientImagePredicate, mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer());

   UpdateWarningLabel(tr("Please load an image"));

   if (m_Controls->patImageSelector->GetSelectedNode().IsNotNull())
   {
     UpdateWarningLabel(tr("Select or create a new segmentation"));
   }

   m_Controls->segImageSelector->SetDataStorage(GetDataStorage());
   m_Controls->segImageSelector->SetPredicate(mitk::NodePredicateAnd::New(m_IsASegmentationImagePredicate, mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer());
   if (m_Controls->segImageSelector->GetSelectedNode().IsNotNull())
   {
     UpdateWarningLabel("");
   }

   mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
   assert(toolManager);

   toolManager->SetDataStorage(*(GetDataStorage()));
   toolManager->InitializeTools();

   // all part of open source MITK
   m_Controls->m_ManualToolSelectionBox2D->setEnabled(true);
   m_Controls->m_ManualToolSelectionBox2D->SetGenerateAccelerators(true);
   m_Controls->m_ManualToolSelectionBox2D->SetToolGUIArea( m_Controls->m_ManualToolGUIContainer2D );
   m_Controls->m_ManualToolSelectionBox2D->SetDisplayedToolGroups(tr("Add Subtract Correction Paint Wipe 'Region Growing' Fill Erase 'Live Wire' '2D Fast Marching'").toStdString());
   m_Controls->m_ManualToolSelectionBox2D->SetLayoutColumns(3);
   m_Controls->m_ManualToolSelectionBox2D->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible );
   connect( m_Controls->m_ManualToolSelectionBox2D, SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)) );

   //setup 3D Tools
   m_Controls->m_ManualToolSelectionBox3D->setEnabled(true);
   m_Controls->m_ManualToolSelectionBox3D->SetGenerateAccelerators(true);
   m_Controls->m_ManualToolSelectionBox3D->SetToolGUIArea( m_Controls->m_ManualToolGUIContainer3D );
   //specify tools to be added to 3D Tool area
   m_Controls->m_ManualToolSelectionBox3D->SetDisplayedToolGroups(tr("Threshold 'UL Threshold' Otsu 'Fast Marching 3D' 'Region Growing 3D' Watershed Picking").toStdString());
   m_Controls->m_ManualToolSelectionBox3D->SetLayoutColumns(3);
   m_Controls->m_ManualToolSelectionBox3D->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible );

   //Hide 3D selection box, show 2D selection box
   m_Controls->m_ManualToolSelectionBox3D->hide();
   m_Controls->m_ManualToolSelectionBox2D->show();

   // update the list of segmentations
   toolManager->NewNodesGenerated += mitk::MessageDelegate<QmitkSegmentationView>(this, &QmitkSegmentationView::NewNodesGenerated);
   // update the list of segmentations
   toolManager->NewNodeObjectsGenerated += mitk::MessageDelegate1<QmitkSegmentationView, mitk::ToolManager::DataVectorType*>(this, &QmitkSegmentationView::NewNodeObjectsGenerated);

   // create signal/slot connections
   connect(m_Controls->patImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnPatientComboBoxSelectionChanged(const mitk::DataNode*)));
   connect(m_Controls->segImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnSegmentationComboBoxSelectionChanged(const mitk::DataNode*)));
   connect( m_Controls->btnNewSegmentation, SIGNAL(clicked()), this, SLOT(CreateNewSegmentation()) );
   //  connect( m_Controls->CreateSegmentationFromSurface, SIGNAL(clicked()), this, SLOT(CreateSegmentationFromSurface()) );
   //  connect( m_Controls->widgetStack, SIGNAL(currentChanged(int)), this, SLOT(ToolboxStackPageChanged(int)) );

   connect( m_Controls->tabWidgetSegmentationTools, SIGNAL(currentChanged(int)), this, SLOT(OnTabWidgetChanged(int)));

   //  connect(m_Controls->MaskSurfaces,  SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
   //      this, SLOT( OnSurfaceSelectionChanged( ) ) );

   connect(m_Controls->m_SlicesInterpolator, SIGNAL(SignalShowMarkerNodes(bool)), this, SLOT(OnShowMarkerNodes(bool)));

   //  m_Controls->MaskSurfaces->SetDataStorage(this->GetDataStorage());
   //  m_Controls->MaskSurfaces->SetPredicate(mitk::NodePredicateDataType::New("Surface"));

   mitk::DataStorage::SetOfObjects::ConstPointer patientImages = GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);
   if (!patientImages->empty())
   {
     OnSelectionChanged(*patientImages->begin());
   }

   // set callback function for already existing nodes (images & segmentations)
   mitk::DataStorage::SetOfObjects::ConstPointer allImages = GetDataStorage()->GetSubset(m_IsOfTypeImagePredicate);
   for (mitk::DataStorage::SetOfObjects::const_iterator iter = allImages->begin(); iter != allImages->end(); ++iter)
   {
     mitk::DataNode* node = *iter;
     itk::SimpleMemberCommand<QmitkSegmentationView>::Pointer command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
     command->SetCallbackFunction(this, &QmitkSegmentationView::OnVisiblePropertyChanged);
     m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(node, node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));

     itk::SimpleMemberCommand<QmitkSegmentationView>::Pointer command2 = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
     command2->SetCallbackFunction(this, &QmitkSegmentationView::OnBinaryPropertyChanged);
     m_BinaryPropertyObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(node, node->GetProperty("binary")->AddObserver(itk::ModifiedEvent(), command2)));
   }

   itk::SimpleMemberCommand<QmitkSegmentationView>::Pointer command = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
   command->SetCallbackFunction(this, &QmitkSegmentationView::RenderingManagerReinitialized);
   m_RenderingManagerObserverTag = mitk::RenderingManager::GetInstance()->AddObserver(mitk::RenderingManagerViewsInitializedEvent(), command);

   InitToolManagerSelection(m_Controls->patImageSelector->GetSelectedNode(), m_Controls->segImageSelector->GetSelectedNode());

   m_RenderWindowPart = GetRenderWindowPart();
   if (m_RenderWindowPart)
   {
     RenderWindowPartActivated(m_RenderWindowPart);
   }
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
   if (!resource) return;

   // Remove previously set mouse cursor
   if ( m_MouseCursorSet )
   {
      mitk::ApplicationCursor::GetInstance()->PopCursor();
   }

   us::ModuleResourceStream cursor(resource, std::ios::binary);
   mitk::ApplicationCursor::GetInstance()->PushCursor( cursor, hotspotX, hotspotY );
   m_MouseCursorSet = true;
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
