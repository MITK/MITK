/*========================================================================= 

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkDataTreeNodeObject.h"
#include "mitkProperties.h"
#include "mitkSegTool2D.h"
#include "mitkGlobalInteraction.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkNewSegmentationDialog.h"

#include <QMessageBox> 

#include <berryIWorkbenchPage.h>

#include "QmitkSegmentationView.h"
#include "QmitkSegmentationPostProcessing.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"

// public methods

QmitkSegmentationView::QmitkSegmentationView()
:m_Parent(NULL)
,m_Controls(NULL)
,m_MultiWidget(NULL)
,m_JustSentASelection(false)
,m_PostProcessing(NULL)
,m_RenderingManagerObserverTag(0)
{
}

QmitkSegmentationView::~QmitkSegmentationView()
{
  mitk::RenderingManager::GetInstance()->RemoveObserver( m_RenderingManagerObserverTag );

  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
  {
    s->RemovePostSelectionListener(m_SelectionListener);
  }

  if(m_SegmentationPreferencesNode.IsNotNull())
  {
    m_SegmentationPreferencesNode->OnChanged
      .RemoveListener(berry::MessageDelegate1<QmitkSegmentationView, const berry::IBerryPreferences*>(this, &QmitkSegmentationView::PreferencesChanged));
  }

  delete m_PostProcessing;
  delete m_Controls;
}

void QmitkSegmentationView::NewNodesGenerated()
{
  ForceDisplayPreferencesUponAllImages();
}

void QmitkSegmentationView::NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType* nodes)
{
  if (!nodes) return;

  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  if (!toolManager) return;
  for (mitk::ToolManager::DataVectorType::iterator iter = nodes->begin(); iter != nodes->end(); ++iter)
  {
    SendSelectedEvent( toolManager->GetReferenceData(0), *iter );
    // only last iteration meaningful, multiple generated objects are not taken into account here
  }
}

void QmitkSegmentationView::Visible()
{
  // should be moved to ::BecomesVisible() or similar
  if( m_Controls )
  {
    m_Controls->m_ManualToolSelectionBox->setEnabled( true );
    m_Controls->m_OrganToolSelectionBox->setEnabled( true );
    m_Controls->m_LesionToolSelectionBox->setEnabled( true );
  
    m_Controls->m_SlicesInterpolator->EnableInterpolation( m_Controls->widgetStack->currentWidget() == m_Controls->pageManual );
  }
}

void QmitkSegmentationView::Hidden()
{
  if( m_Controls )
  {
    m_Controls->m_ManualToolSelectionBox->setEnabled( false );
    m_Controls->m_OrganToolSelectionBox->setEnabled( false );
    m_Controls->m_LesionToolSelectionBox->setEnabled( false );
  
    m_Controls->m_SlicesInterpolator->EnableInterpolation( false );
  }
}

void QmitkSegmentationView::StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget )
{
  SetMultiWidget(&stdMultiWidget);
}

void QmitkSegmentationView::StdMultiWidgetNotAvailable()
{
  SetMultiWidget(NULL);
}

void QmitkSegmentationView::StdMultiWidgetClosed( QmitkStdMultiWidget& /*stdMultiWidget*/ )
{
  SetMultiWidget(NULL);
}
  
void QmitkSegmentationView::SetMultiWidget(QmitkStdMultiWidget* multiWidget)
{
  if (m_MultiWidget)
  {
  mitk::SlicesCoordinator* coordinator = m_MultiWidget->GetSlicesRotator();
  if (coordinator)
  {
    coordinator->RemoveObserver( m_SlicesRotationObserverTag1 );
  }
  coordinator = m_MultiWidget->GetSlicesSwiveller();
  if (coordinator)
  {
    coordinator->RemoveObserver( m_SlicesRotationObserverTag2 );
  }
  }

  // save the current multiwidget as the working widget
  m_MultiWidget = multiWidget;

  if (m_MultiWidget)
  {
    mitk::SlicesCoordinator* coordinator = m_MultiWidget->GetSlicesRotator();
    if (coordinator)
    {
      itk::ReceptorMemberCommand<QmitkSegmentationView>::Pointer command2 = itk::ReceptorMemberCommand<QmitkSegmentationView>::New();
      command2->SetCallbackFunction( this, &QmitkSegmentationView::SliceRotation );
      m_SlicesRotationObserverTag1 = coordinator->AddObserver( mitk::SliceRotationEvent(), command2 );
    }

    coordinator = m_MultiWidget->GetSlicesSwiveller();
    if (coordinator)
    {
      itk::ReceptorMemberCommand<QmitkSegmentationView>::Pointer command2 = itk::ReceptorMemberCommand<QmitkSegmentationView>::New();
      command2->SetCallbackFunction( this, &QmitkSegmentationView::SliceRotation );
      m_SlicesRotationObserverTag2 = coordinator->AddObserver( mitk::SliceRotationEvent(), command2 );
    }
  }

  if (m_Parent)
  {
    m_Parent->setEnabled(m_MultiWidget);
  }

  // tell the interpolation about toolmanager and multiwidget (and data storage)
  if (m_Controls && m_MultiWidget)
  {
    mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
    m_Controls->m_SlicesInterpolator->SetDataStorage( *(this->GetDefaultDataStorage()));
    m_Controls->m_SlicesInterpolator->Initialize( toolManager, m_MultiWidget );
  }
}

void QmitkSegmentationView::PreferencesChanged(const berry::IBerryPreferences* )
{
  ForceDisplayPreferencesUponAllImages();
}

void QmitkSegmentationView::RenderingManagerReinitialized(const itk::EventObject&)
{
  CheckImageAlignment();
}

void QmitkSegmentationView::SliceRotation(const itk::EventObject&)
{
  CheckImageAlignment();
}
 

// protected slots

void QmitkSegmentationView::CreateNewSegmentation()
{
  mitk::DataTreeNode::Pointer node = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
    if (image.IsNotNull())
    {
      if (image->GetDimension()>2)
      {
        // ask about the name and organ type of the new segmentation
        QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( m_Parent ); // needs a QWidget as parent, "this" is not QWidget

        QString storedList = QString::fromStdString( m_SegmentationPreferencesNode->GetByteArray("Organ-Color-List","") );
        QStringList organColors;
        if (storedList.isEmpty())
        {
          organColors = GetDefaultOrganColorString();
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
          QString storedString = QString::fromStdString( m_SegmentationPreferencesNode->GetByteArray("Organ-Color-List","") );
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
        mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
        mitk::Tool* firstTool = toolManager->GetToolById(0);
        if (firstTool)
        {
          try
          {
            mitk::DataTreeNode::Pointer emptySegmentation =
              firstTool->CreateEmptySegmentationNode( image, dialog->GetSegmentationName().toStdString(), dialog->GetColor() );

            if (!emptySegmentation) return; // could be aborted by user

            UpdateOrganList( organColors, dialog->GetSegmentationName(), dialog->GetColor() );

            /*
               escape ';' here (replace by '\;'), see longer comment above
            */
            std::string stringForStorage = organColors.replaceInStrings(";","\\;").join(";").toStdString();
            MITK_DEBUG << "Will store: " << stringForStorage;
            m_SegmentationPreferencesNode->PutByteArray("Organ-Color-List", stringForStorage );
            m_SegmentationPreferencesNode->Flush();

            this->GetDefaultDataStorage()->Add( emptySegmentation, node ); // add as a child, because the segmentation "derives" from the original

            SendSelectedEvent( node, emptySegmentation );
          }
          catch (std::bad_alloc)
          {
            QMessageBox::warning(NULL,"Create new segmentation","Could not allocate memory for new segmentation");
          }
        }
      }
      else
      {
        QMessageBox::information(NULL,"Segmentation","Segmentation is currently not supported for 2D images");
      }
    }
  }
  else
  {
    MITK_ERROR << "'Create new segmentation' button should never be clickable unless a patient image is selected...";
  }
}

void QmitkSegmentationView::ManualToolSelected(int id)
{
  // disable crosshair movement when a manual drawing tool is active (otherwise too much visual noise)
  if (m_MultiWidget)
  {
    if (id >= 0)
    {
      m_MultiWidget->DisableNavigationControllerEventListening();
    }
    else
    {
      m_MultiWidget->EnableNavigationControllerEventListening();
    }
  }
}

void QmitkSegmentationView::ToolboxStackPageChanged(int id)
{
  // interpolation only with manual tools visible
  m_Controls->m_SlicesInterpolator->EnableInterpolation( id == 0 );

  // this is just a workaround, should be removed when all tools support 3D+t
  if (id==2) // lesions
  {
    mitk::DataTreeNode::Pointer node = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
    if (node.IsNotNull())
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNotNull())
      {
        if (image->GetDimension()>3)
        {
          m_Controls->widgetStack->setCurrentIndex(0);
          QMessageBox::information(NULL,"Segmentation","Lesion segmentation is currently not supported for 4D images");
        }
      }
    }
  }
}

// protected 

void QmitkSegmentationView::PullCurrentDataManagerSelection()
{
  MITK_INFO << "Update selection from DataManager";
  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
    // buffer for the data manager selection
  mitk::DataTreeNodeSelection::ConstPointer currentSelection = selection.Cast<const mitk::DataTreeNodeSelection>();
  this->BlueBerrySelectionChanged(berry::SmartPointer<IWorkbenchPart>(NULL), currentSelection);
}

void QmitkSegmentationView::BlueBerrySelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection)
{
  if (!m_Parent || !m_Parent->isVisible()) return;

  if ( selection.IsNull() | (sourcepart == this && !m_JustSentASelection) )  // prevents being notified by own selection events (except when wanted ;-)
  {
    MITK_INFO << "Ignore this empty selection event.";
    return; // otherwise we get "null selection" events each time the view is activated/focussed
  }
  
  // reaction to BlueBerry selection events
  //   this method will try to figure out if a relevant segmentation and its corresponding original image were selected
  //   a warning is issued if the selection is invalid
  //   appropriate reactions are triggered otherwise

  mitk::DataTreeNodeSelection::ConstPointer blueberrySelection = selection.Cast<const mitk::DataTreeNodeSelection>();

  mitk::DataTreeNode::Pointer referenceData = FindFirstRegularImage( blueberrySelection );
  mitk::DataTreeNode::Pointer workingData =   FindFirstSegmentation( blueberrySelection );

  bool invalidSelection( blueberrySelection && 
                         (
                           blueberrySelection->Size() > 2 ||    // maximum 2 selected nodes
                           (blueberrySelection->Size() == 2 && (workingData.IsNull() || referenceData.IsNull()) ) // with two nodes, one must be the original image, one the segmentation
                           // one item is always ok (might be working or reference or nothing
                         )  
                       );

  if (invalidSelection)
  {
    // TODO visible warning when two images are selected
    MITK_ERROR << "WARNING: No image or too many (>2) were selected.";
    referenceData = NULL;
    workingData = NULL;
  }

  if ( workingData.IsNotNull() && referenceData.IsNull() )
  {
    // find the DataStorage parent of workingData
    // try to find a "normal image" parent, select this as reference image
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateNOT::Pointer isNotBinary = mitk::NodePredicateNOT::New( isBinary );
    mitk::NodePredicateAND::Pointer isNormalImage = mitk::NodePredicateAND::New( isImage, isNotBinary );

    mitk::DataStorage::SetOfObjects::ConstPointer possibleParents = this->GetDefaultDataStorage()->GetSources( workingData, isNormalImage );

    if (possibleParents->size() > 0)
    {
      if (possibleParents->size() > 1)
      {
        // TODO visible warning for this rare case
        MITK_ERROR << "Selected binary image has multiple parents. Using arbitrary first one for segmentation.";
      }

      referenceData = (*possibleParents)[0];
    }
  }

  SetToolManagerSelection(referenceData, workingData);
  ForceDisplayPreferencesUponAllImages();
}

mitk::DataTreeNode::Pointer QmitkSegmentationView::FindFirstRegularImage( mitk::DataTreeNodeSelection::ConstPointer selection )
{
  if (!selection) return NULL;

  for (mitk::DataTreeNodeSelection::iterator i = selection->Begin(); i != selection->End(); ++i)
  {
    if (mitk::DataTreeNodeObject::Pointer nodeObject = i->Cast<mitk::DataTreeNodeObject>())
    {
      mitk::DataTreeNode::Pointer node = nodeObject->GetDataTreeNode();

      bool isImage(false);
      if (node->GetData())
      {
        isImage = dynamic_cast<mitk::Image*>(node->GetData()) != NULL;
      }

      // make sure this is not a binary image
      bool isSegmentation(false);
      node->GetBoolProperty("binary", isSegmentation);

      // return first proper mitk::Image
      if (isImage && !isSegmentation) return node;
    }
  }
  
  return NULL;
}


mitk::DataTreeNode::Pointer QmitkSegmentationView::FindFirstSegmentation( mitk::DataTreeNodeSelection::ConstPointer selection )
{
  if (!selection) return NULL;

  for (mitk::DataTreeNodeSelection::iterator i = selection->Begin(); i != selection->End(); ++i)
  {
    if (mitk::DataTreeNodeObject::Pointer nodeObject = i->Cast<mitk::DataTreeNodeObject>())
    {
      mitk::DataTreeNode::Pointer node = nodeObject->GetDataTreeNode();

      bool isImage(false);
      if (node->GetData())
      {
        isImage = dynamic_cast<mitk::Image*>(node->GetData()) != NULL;
      }

      bool isSegmentation(false);
      node->GetBoolProperty("binary", isSegmentation);

      // return first proper binary mitk::Image 
      if (isImage && isSegmentation) return node;
    }
  }
  
  return NULL;
}

void QmitkSegmentationView::SetToolManagerSelection(const mitk::DataTreeNode* referenceData, const mitk::DataTreeNode* workingData)
{
  // called as a result of new BlueBerry selections
  //   tells the ToolManager for manual segmentation about new selections
  //   updates GUI information about what the user should select
  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  toolManager->SetReferenceData(const_cast<mitk::DataTreeNode*>(referenceData));
  toolManager->SetWorkingData(  const_cast<mitk::DataTreeNode*>(workingData));

  // check original image
  m_Controls->btnNewSegmentation->setEnabled(referenceData != NULL);
  if (referenceData)
  {
    m_Controls->lblReferenceImageSelectionWarning->hide();
  }
  else
  {
    m_Controls->lblReferenceImageSelectionWarning->show();
  }
  
  // check, wheter reference image is aligned like render windows. Otherwise display a visible warning (because 2D tools will probably not work)
  CheckImageAlignment();

  // check segmentation
  if (referenceData && !workingData)
  {
    m_Controls->lblWorkingImageSelectionWarning->show();
  }
  else
  {
    m_Controls->lblWorkingImageSelectionWarning->hide();
  }

}

void QmitkSegmentationView::SendSelectedEvent( mitk::DataTreeNode* /*referenceNode*/, mitk::DataTreeNode* workingNode )
{
  // send a BlueBerry selection event for workingNode (done here for newly created segmentations)
  m_JustSentASelection = true;
  m_SelectionProvider->FireSelectionChanged(workingNode);
  m_JustSentASelection = false;
}

void QmitkSegmentationView::CheckImageAlignment()
{
  bool wrongAlignment(false);

  mitk::DataTreeNode::Pointer node = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );

    if (image.IsNotNull() && m_MultiWidget)
    {
    
      wrongAlignment = !(    IsRenderWindowAligned(m_MultiWidget->GetRenderWindow1(), image )
                          && IsRenderWindowAligned(m_MultiWidget->GetRenderWindow2(), image )
                          && IsRenderWindowAligned(m_MultiWidget->GetRenderWindow3(), image )
                        );
    }
  }

  if (wrongAlignment)
  {
    m_Controls->lblAlignmentWarning->show();
  }
  else
  {
    m_Controls->lblAlignmentWarning->hide();
  }
}

bool QmitkSegmentationView::IsRenderWindowAligned(QmitkRenderWindow* renderWindow, mitk::Image* image)
{
  if (!renderWindow) return false;
    
  // for all 2D renderwindows of m_MultiWidget check alignment
  mitk::PlaneGeometry::ConstPointer displayPlane = dynamic_cast<const mitk::PlaneGeometry*>( renderWindow->GetRenderer()->GetCurrentWorldGeometry2D() );
  if (displayPlane.IsNull()) return false;
    
  int affectedDimension(-1);
  int affectedSlice(-1);
  return mitk::SegTool2D::DetermineAffectedImageSlice( image, displayPlane, affectedDimension, affectedSlice );
}

void QmitkSegmentationView::ForceDisplayPreferencesUponAllImages()
{
  if (!m_Parent || !m_Parent->isVisible()) return;

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
  //     (and hide all all its siblings (childs of the same parent, incl, NULL parent))
  //   if no segmentation is selected, do nothing

  if (!m_Controls) return; // might happen on initialization (preferences loaded)
  mitk::DataTreeNode::Pointer referenceData = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
  mitk::DataTreeNode::Pointer workingData =   m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetWorkingData(0);

  // 1.
  if (referenceData.IsNotNull())
  {
    // iterate all images
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();

    mitk::DataStorage::SetOfObjects::ConstPointer allImages = this->GetDefaultDataStorage()->GetSubset( isImage );
    for ( mitk::DataStorage::SetOfObjects::const_iterator iter = allImages->begin();
          iter != allImages->end();
          ++iter)

    {
      mitk::DataTreeNode* node = *iter;
      // apply display preferences
      ApplyDisplayOptions(node);
    
      // set visibility  
      node->SetVisibility(node == referenceData);
    }
  }

  // 2.
  if (workingData.IsNotNull())
  {
    workingData->SetVisibility(true);
  }
  
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::ApplyDisplayOptions(mitk::DataTreeNode* node)
{
  if (!node) return;

  node->SetProperty( "outline binary", mitk::BoolProperty::New( m_SegmentationPreferencesNode->GetBool("draw outline", true)) );
  node->SetProperty( "outline width", mitk::FloatProperty::New( 2.0 ) );
  node->SetProperty( "opacity", mitk::FloatProperty::New( m_SegmentationPreferencesNode->GetBool("draw outline", true) ? 1.0 : 0.3 ) );
  node->SetProperty( "volumerendering", mitk::BoolProperty::New( m_SegmentationPreferencesNode->GetBool("volume rendering", false) ) );
}

void QmitkSegmentationView::CreateQtPartControl(QWidget* parent)
{
  // setup the basic GUI of this view
  m_Parent = parent;

  m_Controls = new Ui::QmitkSegmentationControls;
  m_Controls->setupUi(parent);
  m_Controls->lblWorkingImageSelectionWarning->hide();
  m_Controls->lblAlignmentWarning->hide();

  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  toolManager->SetDataStorage( *(this->GetDefaultDataStorage()) );
  assert ( toolManager );

  // all part of open source MITK
  m_Controls->m_ManualToolSelectionBox->SetGenerateAccelerators(true);
  m_Controls->m_ManualToolSelectionBox->SetToolGUIArea( m_Controls->m_ManualToolGUIContainer );
  m_Controls->m_ManualToolSelectionBox->SetDisplayedToolGroups("Add Subtract Paint Wipe 'Region Growing' Correction Fill Erase");
  m_Controls->m_ManualToolSelectionBox->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingData );

  // available only in the 3M application
  if ( m_Controls->m_OrganToolSelectionBox->children().count() )
  {
    m_Controls->widgetStack->setItemEnabled( 1, false );
  }
  m_Controls->m_OrganToolSelectionBox->SetToolManager( *toolManager );
  m_Controls->m_OrganToolSelectionBox->SetToolGUIArea( m_Controls->m_OrganToolGUIContainer );
  m_Controls->m_OrganToolSelectionBox->SetDisplayedToolGroups("'Hippocampus left' 'Hippocampus right' 'Lung left' 'Lung right' 'Liver' 'Heart LV' 'Endocard LV' 'Epicard LV'");
  m_Controls->m_OrganToolSelectionBox->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceData );

  // available only in the 3M application
  if ( m_Controls->m_LesionToolSelectionBox->children().count() )
  {
    m_Controls->widgetStack->setItemEnabled( 2, false );
  }
  m_Controls->m_LesionToolSelectionBox->SetToolManager( *toolManager );
  m_Controls->m_LesionToolSelectionBox->SetToolGUIArea( m_Controls->m_LesionToolGUIContainer );
  m_Controls->m_LesionToolSelectionBox->SetDisplayedToolGroups("'Lymph Node' 'Lymph Node Correction'");
  m_Controls->m_LesionToolSelectionBox->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceData );
    
  toolManager->NewNodesGenerated += 
    mitk::MessageDelegate<QmitkSegmentationView>( this, &QmitkSegmentationView::NewNodesGenerated );          // update the list of segmentations
  toolManager->NewNodeObjectsGenerated += 
    mitk::MessageDelegate1<QmitkSegmentationView, mitk::ToolManager::DataVectorType*>( this, &QmitkSegmentationView::NewNodeObjectsGenerated );          // update the list of segmentations

  // create signal/slot connections
  connect( m_Controls->btnNewSegmentation, SIGNAL(clicked()), this, SLOT(CreateNewSegmentation()) );
  connect( m_Controls->m_ManualToolSelectionBox, SIGNAL(ToolSelected(int)), this, SLOT(ManualToolSelected(int)) );
  connect( m_Controls->widgetStack, SIGNAL(currentChanged(int)), this, SLOT(ToolboxStackPageChanged(int)) );

  // register as listener for BlueBerry selection events (mainly from DataManager)
  m_SelectionListener = berry::ISelectionListener::Pointer(new berry::SelectionChangedAdapter<QmitkSegmentationView>(this, &QmitkSegmentationView::BlueBerrySelectionChanged));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelectionListener);
 
  // also make us a provider of selections 
  m_SelectionProvider = new mitk::SegmentationSelectionProvider();
  this->GetSite()->SetSelectionProvider(m_SelectionProvider);

  // BlueBerry preferences
  berry::IPreferencesService::Pointer prefService 
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_SegmentationPreferencesNode = (prefService->GetSystemPreferences()->Node("/Segmentation")).Cast<berry::IBerryPreferences>();
  if(m_SegmentationPreferencesNode.IsNotNull())
  {
    m_SegmentationPreferencesNode->OnChanged
      .AddListener(berry::MessageDelegate1<QmitkSegmentationView, const berry::IBerryPreferences*>(this, &QmitkSegmentationView::PreferencesChanged));
  }

  // create helper class to provide context menus for segmentations in data manager
  m_PostProcessing = new QmitkSegmentationPostProcessing(this->GetDefaultDataStorage(), this, NULL);
 
  // call preferences changed for initialization
  // changes GUI according to current data manager selection (because it might have changed before QmitkSegmentationView came into being)
  this->PreferencesChanged(m_SegmentationPreferencesNode.GetPointer());

  itk::ReceptorMemberCommand<QmitkSegmentationView>::Pointer command1 = itk::ReceptorMemberCommand<QmitkSegmentationView>::New();
  command1->SetCallbackFunction( this, &QmitkSegmentationView::RenderingManagerReinitialized );
  m_RenderingManagerObserverTag = mitk::RenderingManager::GetInstance()->AddObserver( mitk::RenderingManagerViewsInitializedEvent(), command1 );
}

      
// ATTENTION some methods for handling the known list of (organ names, colors) are defined in QmitkSegmentationOrganNamesHandling.cpp

