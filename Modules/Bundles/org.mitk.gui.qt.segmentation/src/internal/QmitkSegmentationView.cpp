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

#include "QmitkStdMultiWidget.h"
#include "QmitkNewSegmentationDialog.h"

#include <QMessageBox> 

#include <berryIWorkbenchPage.h>

#include "QmitkSegmentationView.h"
#include "QmitkSegmentationPostProcessing.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"

QmitkSegmentationView::QmitkSegmentationView()
:m_MultiWidget(NULL)
{
}


void QmitkSegmentationView::CreateQtPartControl(QWidget* parent)
{
  // preferences
  berry::IPreferencesService::Pointer prefService 
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_SegmentationPreferencesNode = (prefService->GetSystemPreferences()->Node("/Segmentation")).Cast<berry::IBerryPreferences>();
  if(m_SegmentationPreferencesNode.IsNotNull())
  {
    m_SegmentationPreferencesNode->OnChanged
      .AddListener(berry::MessageDelegate1<QmitkSegmentationView, const berry::IBerryPreferences*>(this, &QmitkSegmentationView::PreferencesChanged));
  }

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
  m_SelectionListener = berry::ISelectionListener::Pointer(new berry::SelectionChangedAdapter<QmitkSegmentationView>(this, &QmitkSegmentationView::SelectionChanged));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelectionListener);
 
  // also make us a provider of selections 
  m_SelectionProvider = new mitk::SegmentationSelectionProvider();
  this->GetSite()->SetSelectionProvider(m_SelectionProvider);


  // create helper class to provide context menus for segmentations in data manager
  new QmitkSegmentationPostProcessing(this->GetDefaultDataStorage(), this, parent);
 
  // call preferences changed for initialization
  // changes GUI according to current data manager selection (because it might have changed before QmitkSegmentationView came into being)
  this->PreferencesChanged(m_SegmentationPreferencesNode.GetPointer());
}

QmitkSegmentationView::~QmitkSegmentationView()
{
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
  {
    s->RemoveSelectionListener(m_SelectionListener);
  }

  this->Deactivated();
  delete m_Controls;
}

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
          organColors = QString::fromStdString( m_SegmentationPreferencesNode->GetByteArray("Organ-Color-List","") ).split(";");
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

            // TODO escape # and ; at all costs, this would mess up parsing of the stored list
            UpdateOrganList( organColors, dialog->GetSegmentationName(), dialog->GetColor() );

            m_SegmentationPreferencesNode->PutByteArray("Organ-Color-List", organColors.join(";").toStdString() );
            m_SegmentationPreferencesNode->Flush();

            if (!emptySegmentation) return; // could be aborted by user

            ApplyDisplayOptions(emptySegmentation);

            this->GetDefaultDataStorage()->Add( emptySegmentation, node ); // add as a child, because the segmentation "derives" from the original

            SendSelectedEvent( node, emptySegmentation );
            PullCurrentDataManagerSelection(); // needs to be done because this view ignores selection from self

            // TODO still working? m_Controls->m_ManualToolSelectionBox->GetToolManager()->SetWorkingData( emptySegmentation );
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

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::ManualToolSelected(int id)
{
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
  // this is just a workaround, should be removed when all tools support 3D+t
  if (id==0) //manual
  {

  }
  else if (id==1) // organs
  {

  }
  else if (id==2) // lesions
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

void QmitkSegmentationView::CheckImageAlignment()
{
  MITK_INFO << "Updating alignment warning";

  bool wrongAlignment(false);
  mitk::DataTreeNode::Pointer node = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );

    if (image.IsNotNull() && m_MultiWidget)
    {
      QmitkRenderWindow* renderWindow = m_MultiWidget->GetRenderWindow1();

      if (renderWindow)
      {
        // for all 2D renderwindows of m_MultiWidget check alignment
        mitk::PlaneGeometry::ConstPointer displayPlane
          = dynamic_cast<const mitk::PlaneGeometry*>( renderWindow->GetRenderer()->GetCurrentWorldGeometry2D() );
        if (displayPlane.IsNotNull())
        {
          int affectedDimension(-1);
          int affectedSlice(-1);
          if ( ! mitk::SegTool2D::DetermineAffectedImageSlice( image, displayPlane, affectedDimension, affectedSlice ) )
          {
            wrongAlignment = true;
          }
        }
      }

      renderWindow = m_MultiWidget->GetRenderWindow2();

      if (renderWindow)
      {
        // for all 2D renderwindows of m_MultiWidget check alignment
        mitk::PlaneGeometry::ConstPointer displayPlane
          = dynamic_cast<const mitk::PlaneGeometry*>( renderWindow->GetRenderer()->GetCurrentWorldGeometry2D() );
        if (displayPlane.IsNotNull())
        {
          int affectedDimension(-1);
          int affectedSlice(-1);
          if ( ! mitk::SegTool2D::DetermineAffectedImageSlice( image, displayPlane, affectedDimension, affectedSlice ) )
          {
            wrongAlignment = true;
          }
        }
      }

      renderWindow = m_MultiWidget->GetRenderWindow3();

      if (renderWindow)
      {
        // for all 2D renderwindows of m_MultiWidget check alignment
        mitk::PlaneGeometry::ConstPointer displayPlane
          = dynamic_cast<const mitk::PlaneGeometry*>( renderWindow->GetRenderer()->GetCurrentWorldGeometry2D() );
        if (displayPlane.IsNotNull())
        {
          int affectedDimension(-1);
          int affectedSlice(-1);
          if ( ! mitk::SegTool2D::DetermineAffectedImageSlice( image, displayPlane, affectedDimension, affectedSlice ) )
          {
            wrongAlignment = true;
          }
        }
      }
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

void QmitkSegmentationView::ReferenceNodeSelected(const mitk::DataTreeNode* node)
{
  MITK_DEBUG << "ReferenceNodeSelected(" << (void*)node << ")";
  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  toolManager->SetReferenceData(const_cast<mitk::DataTreeNode*>(node));

  if (node)
  {
    /*
     * Other images are set invisible.
     */
    mitk::DataStorage::SetOfObjects::ConstPointer image_set = this->GetDefaultDataStorage()->GetSubset(
        mitk::NodePredicateAND::New(
            mitk::TNodePredicateDataType<mitk::Image>::New(),
            mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(false))));
    const unsigned int size = image_set->Size();

    for (unsigned int i = 0u; i < size; ++i)
    {
      mitk::DataTreeNode* currentNode = image_set->GetElement(i);

      if (currentNode != node)
      {
        currentNode->SetVisibility(false);
      }
      else
      {
        currentNode->SetVisibility(true);
      }
    }

    m_Controls->lblReferenceImageSelectionWarning->hide();
    // TODO show this image, hide all other images. example code maybe in QmitkToolReferenceDataSelectionBox
  }
  else
  {
    m_Controls->lblReferenceImageSelectionWarning->show();
  }
    
  m_Controls->btnNewSegmentation->setEnabled(node != NULL);

  // check, wheter image is aligned like render windows. Otherwise display a visible warning (because 2D tools will probably not work)
  CheckImageAlignment();
}

void QmitkSegmentationView::WorkingDataSelectionChanged(const mitk::DataTreeNode* node)
{
  MITK_DEBUG << "WorkingDataSelectionChanged(" << (void*)node << ")";
  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  toolManager->SetWorkingData(const_cast<mitk::DataTreeNode*>(node));

  if (node)
  {
    /*
     * Other segmentations are set invisible.
     */
    mitk::DataStorage::SetOfObjects::ConstPointer segmentationSet = this->GetDefaultDataStorage()->GetSubset(
        mitk::NodePredicateAND::New(
            mitk::TNodePredicateDataType<mitk::Image>::New(),
            mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true))));
    const unsigned int size = segmentationSet->Size();

    for (unsigned int i = 0u; i < size; ++i)
    {
      mitk::DataTreeNode* currentNode = segmentationSet->GetElement(i);

      ApplyDisplayOptions(currentNode);
      currentNode->SetVisibility(currentNode == node);
    }

    m_Controls->lblWorkingImageSelectionWarning->hide();
    //m_Controls->grpReferenceData->setTitle( QString("Patient image '%1'").arg( node->GetName().c_str() ) );
    // TODO show this image, hide all other images. example code maybe in QmitkToolReferenceDataSelectionBox
  }
  else
  {
    if ( toolManager->GetReferenceData(0) )
    {
      m_Controls->lblWorkingImageSelectionWarning->show();
    }
    else
    {
      m_Controls->lblWorkingImageSelectionWarning->hide();
    }
  }

  m_Controls->m_SlicesInterpolator->EnableInterpolation( m_Controls->widgetStack->currentWidget() == m_Controls->pageManual );
}

void QmitkSegmentationView::StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget )
{
  if(!m_MultiWidget)
  {
    m_Parent->setEnabled(true);
    // save the actual multiwidget as the working widget
    m_MultiWidget = &stdMultiWidget;

    mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();

    // tell the interpolation about toolmanager and multiwidget
    m_Controls->m_SlicesInterpolator->SetDataStorage( *(this->GetDefaultDataStorage()));
    m_Controls->m_SlicesInterpolator->Initialize( toolManager, m_MultiWidget );
  }
}

void QmitkSegmentationView::StdMultiWidgetNotAvailable()
{
  m_Parent->setEnabled(false);
}

void QmitkSegmentationView::StdMultiWidgetClosed( QmitkStdMultiWidget& stdMultiWidget )
{
  m_MultiWidget = 0;
}

void QmitkSegmentationView::SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection)
{
  if (!m_Parent || !m_Parent->isVisible())
    return;

  if ( sourcepart == this || selection.IsNull() )  // prevents being notified by own selection events
  {
    MITK_INFO << "Ignore this selection event:"
             << " sourcepart == this " << (sourcepart == this)
             << " selection == NULL" << (selection == NULL);
    return; // otherwise we get "null selection" events each time the view is activated/focussed
  }

  // save current selection in member variable
  m_CurrentSelection = selection.Cast<const mitk::DataTreeNodeSelection>();

  mitk::DataTreeNode::Pointer workingData;
  mitk::DataTreeNode::Pointer referenceData;
  bool tooManySelection( false );

  // do something with the selected items
  if (m_CurrentSelection && (m_CurrentSelection->Size() < 3))
  {

    // iterate selection
    for (mitk::DataTreeNodeSelection::iterator i = m_CurrentSelection->Begin(); !tooManySelection && (i != m_CurrentSelection->End()); ++i)
    {
      // extract datatree node
      if (mitk::DataTreeNodeObject::Pointer nodeObj = i->Cast<mitk::DataTreeNodeObject>())
      {
        mitk::DataTreeNode::Pointer node = nodeObj->GetDataTreeNode();
        MITK_INFO << "Node '" << node->GetName() << "' selected";

        bool isImage(false);
        if (node->GetData())
        {
          isImage = dynamic_cast<mitk::Image*>(node->GetData()) != NULL;
        }

        bool isSegmentation(false);
        node->GetBoolProperty("binary", isSegmentation);

        if (isImage)
        {
          if ( isSegmentation )
          {
            if (workingData.IsNull())
            {
              // first selected binary image
              MITK_INFO << "Working image '" << node->GetName() << "' selected";
              workingData = node;
            }
            else
            {
              // second and other binary images
              tooManySelection = true;
            }
          }
          else // not a segmentation but an image
          {
            if ( referenceData.IsNull() )
            {
              // first selected image
              MITK_INFO << "Reference image '" << node->GetName() << "' selected";
              referenceData = node;
            }
            else
            {
              // second and other selected images
              tooManySelection = true;
            }
          }
        }
      }
    }
  }

  if (tooManySelection)
  {
    // TODO visible warning when two images are selected
    MITK_WARN << "WARNING: No image or too many (>2) were selected.";
    referenceData = NULL;
    workingData = NULL;
  }

  // if only a segmentation is selected, try to find its parent and use it as reference node
  if ( workingData.IsNotNull() && referenceData.IsNull() )
  {
    MITK_DEBUG << "Finding segmentation's parent";
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
        MITK_INFO << "Selected binary image has multiple parents. Using arbitrary first one for segmentation.";
      }

      mitk::DataTreeNode::Pointer referenceNode = (*possibleParents)[0];
      MITK_INFO << "Reference image '" << referenceNode->GetName() << "' selected";
      referenceData = referenceNode;
    }
  }

  MITK_INFO << "Reference " << (void*)referenceData.GetPointer() << " Working " << (void*)workingData.GetPointer();
  // update image selections for our toolmanagers
  ReferenceNodeSelected(referenceData);
  WorkingDataSelectionChanged(workingData);

  if ( referenceData.IsNull() && workingData.IsNull() )
  {
    MITK_INFO << "Nothing selected, re-show all segmentations";
    // if nothing is chosen all segmentations are visible
    mitk::NodePredicateAND::Pointer and_predicate = mitk::NodePredicateAND::New(
        mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true)));
    mitk::DataStorage::SetOfObjects::ConstPointer segmentationSet = this->GetDefaultDataStorage()->GetSubset(and_predicate);
    const unsigned int size = segmentationSet->Size();

    for (unsigned int i = 0u; i < size; ++i)
    {
      ApplyDisplayOptions(segmentationSet->GetElement(i));
      segmentationSet->GetElement(i)->SetVisibility(true);
    }
  }

  // if an normal image is chosen without a segmentation, all child segmentations are visible
  if (referenceData.IsNotNull() && workingData.IsNull())
  {
    MITK_INFO << "Only reference selected, re-show all its child segmentations";
    mitk::NodePredicateAND::Pointer and_predicate = mitk::NodePredicateAND::New(
        mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true)));
    mitk::DataStorage::SetOfObjects::ConstPointer segmentationSet = this->GetDefaultDataStorage()->GetSubset(and_predicate);
    const unsigned int size = segmentationSet->Size();
    for (unsigned int i = 0u; i < size; ++i)
    {
      ApplyDisplayOptions(segmentationSet->GetElement(i));
      segmentationSet->GetElement(i)->SetVisibility(false);
    }

    mitk::DataStorage::SetOfObjects::ConstPointer childSegmentationSet = this->GetDefaultDataStorage()->GetDerivations(referenceData, and_predicate);
    const unsigned int child_size = childSegmentationSet->Size();
    for (unsigned int i = 0u; i < child_size; ++i)
    {
      ApplyDisplayOptions(segmentationSet->GetElement(i));
      childSegmentationSet->GetElement(i)->SetVisibility(true);
    }
  }
}

void QmitkSegmentationView::Activated()
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

void QmitkSegmentationView::Deactivated()
{
  if( m_Controls )
  {
    m_Controls->m_ManualToolSelectionBox->setEnabled( false );
    m_Controls->m_OrganToolSelectionBox->setEnabled( false );
    m_Controls->m_LesionToolSelectionBox->setEnabled( false );
  
    m_Controls->m_SlicesInterpolator->EnableInterpolation( false );
  }
}

QmitkSegmentationView::NodeList QmitkSegmentationView::GetSelectedNodes() const
{
  NodeList result;
  if (m_CurrentSelection)
  {

    // iterate selection
    for (mitk::DataTreeNodeSelection::iterator i = m_CurrentSelection->Begin(); i != m_CurrentSelection->End(); ++i)
    {
      // extract datatree node
      if (mitk::DataTreeNodeObject::Pointer nodeObj = i->Cast<mitk::DataTreeNodeObject>())
      {
        mitk::DataTreeNode::Pointer node = nodeObj->GetDataTreeNode();
        result.push_back( node );
      }
    }
  }

  return result;
}
        
void QmitkSegmentationView::SendSelectedEvent( mitk::DataTreeNode* referenceNode, mitk::DataTreeNode* workingNode )
{
  // should select both nodes and also make them visible (expand tree view if necessary)
  MITK_INFO << "Marking as selected: reference node '" << (referenceNode ? referenceNode->GetName() : "NULL") << " and working node " << (workingNode ? workingNode->GetName() : "NULL");

  //std::vector<mitk::DataTreeNode::Pointer > nodes;
  //if (referenceNode) nodes.push_back( referenceNode );
  //if (workingNode)   nodes.push_back( workingNode );

  //m_SelectionProvider->SetSelection( berry::ISelection::Pointer(new mitk::DataTreeNodeSelection(nodes)) );
  m_SelectionProvider->FireSelectionChanged(workingNode);
}

void QmitkSegmentationView::ApplyDisplayOptions(mitk::DataTreeNode* node)
{
  if (!node) return;

  node->SetProperty( "outline binary", mitk::BoolProperty::New( m_SegmentationPreferencesNode->GetBool("draw outline", true)) );
  node->SetProperty( "outline width", mitk::FloatProperty::New( 2.0 ) );
  node->SetProperty( "opacity", mitk::FloatProperty::New( m_SegmentationPreferencesNode->GetBool("draw outline", true) ? 1.0 : 0.3 ) );
  node->SetProperty( "volumerendering", mitk::BoolProperty::New( m_SegmentationPreferencesNode->GetBool("volume rendering", false) ) );
}

void QmitkSegmentationView::PreferencesChanged(const berry::IBerryPreferences* prefs )
{
  PullCurrentDataManagerSelection();
}

void QmitkSegmentationView::PullCurrentDataManagerSelection()
{
  MITK_INFO << "Update selection from DataManager";
  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = selection.Cast<const mitk::DataTreeNodeSelection>();
  this->SelectionChanged(berry::SmartPointer<IWorkbenchPart>(NULL), m_CurrentSelection);
}

void QmitkSegmentationView::NewNodesGenerated()
{
  PullCurrentDataManagerSelection();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType* nodes)
{
  PullCurrentDataManagerSelection(); // for display options
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  if (nodes)
  {
    mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
    for (mitk::ToolManager::DataVectorType::iterator iter = nodes->begin(); iter != nodes->end(); ++iter)
    {
      GetSite()->GetWorkbenchWindow()->GetActivePage()->ShowView("org.mitk.views.segmentation" ,"", berry::IWorkbenchPage::VIEW_ACTIVATE);
      SendSelectedEvent( toolManager->GetReferenceData(0), *iter );
      toolManager->SetWorkingData( *iter );
      WorkingDataSelectionChanged( *iter );
      break;
    }

    if (nodes->empty())
    {
      QMessageBox::warning(m_Parent, "Lymph node segmentation", "The algorithm could not find a lymph node. \nIf this is persistent, please report to http://bugs.mitk.org");
    }
  }
}
