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

#include "QmitkSegmentationView.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkToolReferenceDataSelectionBox.h"
#include "QmitkToolSelectionBox.h"
#include "QmitkNewSegmentationDialog.h"
#include "QmitkCommonFunctionality.h"
#include "QmitkSlicesInterpolator.h"
#include "QmitkNodeDescriptorManager.h"

#include "mitkToolManager.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkOrganTypeProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkSegTool2D.h"
#include "mitkShowSegmentationAsSurface.h"
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"

#include <QPushButton>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QAbstractItemView>
#include <QMessageBox>

#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageReference.h>

#include <mitkNodePredicateDataType.h>

#include <QmitkStdMultiWidget.h>
#include <QmitkDataStorageListModel.h>

#include <QmitkStdMultiWidgetEditor.h>

#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>
#include "mitkIDataStorageService.h"
#include "mitkDataTreeNodeSelection.h"
#include "mitkDataTreeNodeObject.h"

#include "itkTreeChangeEvent.h"

QmitkSegmentationView::QmitkSegmentationView()
:m_MultiWidget(NULL)
{
}


void QmitkSegmentationView::CreateQtPartControl(QWidget* parent)
{
  // setup the basic GUI of this view

  m_Parent = parent;

  m_Controls = new Ui::QmitkSegmentationControls;
  m_Controls->setupUi(parent);

  m_DataStorage = this->GetDefaultDataStorage();

  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  toolManager->SetDataStorage( *m_DataStorage );
  assert ( toolManager );

  m_Controls->m_ManualToolSelectionBox->SetGenerateAccelerators(true);
  m_Controls->m_ManualToolSelectionBox->SetToolGUIArea( m_Controls->m_ManualToolGUIContainer );
  m_Controls->m_ManualToolSelectionBox->SetDisplayedToolGroups("Add Subtract Paint Wipe 'Region Growing' Correction Fill Erase");
  m_Controls->m_ManualToolSelectionBox->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingData );

  m_Controls->m_OrganToolSelectionBox->SetToolManager( *toolManager );
  m_Controls->m_OrganToolSelectionBox->SetToolGUIArea( m_Controls->m_OrganToolGUIContainer );
  m_Controls->m_OrganToolSelectionBox->SetDisplayedToolGroups("organ_segmentation");
  m_Controls->m_OrganToolSelectionBox->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceData );

  m_Controls->m_LesionToolSelectionBox->SetToolManager( *toolManager );
  m_Controls->m_LesionToolSelectionBox->SetToolGUIArea( m_Controls->m_LesionToolGUIContainer );
  m_Controls->m_LesionToolSelectionBox->SetDisplayedToolGroups("'Lymph Node' 'Lymph Node Correction' 'Homogeneous Tumor' 'Tumor Correction'");
  m_Controls->m_LesionToolSelectionBox->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceData );

  if(this->GetActiveStdMultiWidget())
  {
    m_Controls->m_SlicesInterpolator->Initialize( toolManager, this->GetActiveStdMultiWidget() );
    m_Controls->m_SlicesInterpolator->SetDataStorage( *m_DataStorage );
  }

  // create signal/slot connections
  connect( m_Controls->btnNewSegmentation, SIGNAL(clicked()), this, SLOT(CreateNewSegmentation()) );
  connect( m_Controls->m_ManualToolSelectionBox, SIGNAL(ToolSelected(int)), this, SLOT(OnToolSelected(int)) );

  // register as listener for openCherry selection events (mainly from DataManager)
  m_SelectionListener = cherry::ISelectionListener::Pointer(new cherry::SelectionChangedAdapter<QmitkSegmentationView>(this, &QmitkSegmentationView::SelectionChanged));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelectionListener);
  cherry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection());
  m_CurrentSelection = selection.Cast<const cherry::IStructuredSelection>();
  this->SelectionChanged(cherry::SmartPointer<IWorkbenchPart>(NULL), selection);

  // register a couple of additional actions for DataManager's context menu
  QmitkNodeDescriptor* binaryImageDataTreeNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("ImageMask");

  if (binaryImageDataTreeNodeDescriptor)
  {
    m_CreateSurfaceAction = new QAction("Create polygon model", parent);
    binaryImageDataTreeNodeDescriptor->AddAction(m_CreateSurfaceAction);
    connect( m_CreateSurfaceAction, SIGNAL( triggered(bool) ) , this, SLOT( CreateSurface(bool) ) );
  }
  else
  {
    LOG_WARN << "Could not get datamanager's node descriptor for 'ImageMask'";
  }
}

void QmitkSegmentationView::SetFocus()
{
}

QmitkSegmentationView::~QmitkSegmentationView()
{
  // unregister a couple of additional actions for DataManager's context menu
  QmitkNodeDescriptor* binaryImageDataTreeNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("ImageMask");

  if (binaryImageDataTreeNodeDescriptor)
  {
    binaryImageDataTreeNodeDescriptor->RemoveAction( m_CreateSurfaceAction );
  }
  else
  {
    LOG_WARN << "Could not get datamanager's node descriptor for 'ImageMask'";
  }

  this->Deactivated();
  delete m_Controls;
}

void QmitkSegmentationView::CreateConnections()
{
}

void QmitkSegmentationView::CreateNewSegmentation()
{
  mitk::DataTreeNode::Pointer node = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
    if (image.IsNotNull())
    {

      // ask about the name and organ type of the new segmentation
      QmitkNewSegmentationDialog dialog( m_Parent ); // needs a QWidget as parent, "this" is not QWidget
      int dialogReturnValue = dialog.exec();

      if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

      // ask the user about an organ type and name, add this information to the image's (!) propertylist
      // create a new image of the same dimensions and smallest possible pixel type
      mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
      mitk::Tool* firstTool = toolManager->GetToolById(0);
      if (firstTool)
      {
        mitk::DataTreeNode::Pointer emptySegmentation =
          firstTool->CreateEmptySegmentationNode( image, dialog.GetOrganType(), dialog.GetSegmentationName() );

        if (!emptySegmentation) return; // could be aborted by user
        //emptySegmentation->SetProperty("volumerendering", mitk::BoolProperty::New(true) );

        this->GetDefaultDataStorage()->Add( emptySegmentation, node ); // add as a child, because the segmentation "derives" from the original

        // TODO select this new segmentation in data manager
        m_Controls->m_ManualToolSelectionBox->GetToolManager()->SetWorkingData( emptySegmentation );
      }
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::OnToolSelected(int id)
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

void QmitkSegmentationView::CheckImageAlignment()
{
  LOG_INFO << "Updating alignment warning";

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

void QmitkSegmentationView::OnReferenceNodeSelected(const mitk::DataTreeNode* node)
{
  LOG_DEBUG << "OnReferenceNodeSelected(" << (void*)node << ")";
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
            mitk::NodePredicateProperty::New("binary", mitk::GenericProperty<bool>::New(false))));
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

  // check, wheter image is aligned like render windows. Otherwise display a visible warning (because 2D tools will probably not work)
  CheckImageAlignment();
}

void QmitkSegmentationView::OnWorkingDataSelectionChanged(const mitk::DataTreeNode* node)
{
  LOG_DEBUG << "OnWorkingDataSelectionChanged(" << (void*)node << ")";
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
            mitk::NodePredicateProperty::New("binary", mitk::GenericProperty<bool>::New(true))));
    const unsigned int size = segmentationSet->Size();

    for (unsigned int i = 0u; i < size; ++i)
    {
      mitk::DataTreeNode* currentNode = segmentationSet->GetElement(i);

      currentNode->SetVisibility(currentNode == node);
    }

    m_Controls->lblWorkingImageSelectionWarning->hide();
    //m_Controls->grpReferenceData->setTitle( QString("Patient image '%1'").arg( node->GetName().c_str() ) );
    // TODO show this image, hide all other images. example code maybe in QmitkToolReferenceDataSelectionBox
  }
  else
  {
    m_Controls->lblWorkingImageSelectionWarning->show();
  }

  m_Controls->m_SlicesInterpolator->EnableInterpolation( m_Controls->widgetStack->currentWidget() == m_Controls->pageManual );
}

void QmitkSegmentationView::StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget )
{
  if(m_MultiWidget == 0)
  {
    m_Parent->setEnabled(true);
    // save the actual multiwidget as the working widget
    m_MultiWidget = &stdMultiWidget;

    mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();

    // tell the interpolation about toolmanager and multiwidget
    m_Controls->m_SlicesInterpolator->SetDataStorage( *m_DataStorage );
    m_Controls->m_SlicesInterpolator->Initialize( toolManager, m_MultiWidget );
    this->Activated();
  }
}

void QmitkSegmentationView::StdMultiWidgetNotAvailable()
{
  m_Parent->setEnabled(false);
  this->Deactivated();
}

void QmitkSegmentationView::StdMultiWidgetClosed( QmitkStdMultiWidget& stdMultiWidget )
{
  if(&stdMultiWidget == m_MultiWidget)
    m_MultiWidget = 0;
  // set new multi widget to the one that is available
  m_MultiWidget = this->GetActiveStdMultiWidget();

  // tell the interpolation about toolmanager and multiwidget
  if(m_MultiWidget)
    m_Controls->m_SlicesInterpolator->Initialize( m_Controls->m_ManualToolSelectionBox->GetToolManager(), m_MultiWidget );
}

void QmitkSegmentationView::SelectionChanged(cherry::IWorkbenchPart::Pointer sourcepart, cherry::ISelection::ConstPointer selection)
{
  if ( sourcepart == this ||  // prevents being notified by own selection events
       !selection.Cast<const cherry::IStructuredSelection>() ) // checks that the selection is a IStructuredSelection and not NULL
  {
    return; // otherwise we get "null selection" events each time the view is activated/focussed
  }

  std::cout << "selection changed" << std::endl;
  // save current selection in member variable
  m_CurrentSelection = selection.Cast<const cherry::IStructuredSelection>();

  mitk::DataTreeNode::Pointer workingData;
  mitk::DataTreeNode::Pointer referenceData;
  bool tooManySelection( false );

  // do something with the selected items
  if (m_CurrentSelection && (m_CurrentSelection->Size() < 3))
  {

    // iterate selection
    for (cherry::IStructuredSelection::iterator i = m_CurrentSelection->Begin(); !tooManySelection && (i != m_CurrentSelection->End()); ++i)
    {
      // extract datatree node
      if (mitk::DataTreeNodeObject::Pointer nodeObj = i->Cast<mitk::DataTreeNodeObject>())
      {
        mitk::DataTreeNode::Pointer node = nodeObj->GetDataTreeNode();
        LOG_INFO << "Node '" << node->GetName() << "' selected";

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
              LOG_INFO << "Working image '" << node->GetName() << "' selected";
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
              LOG_INFO << "Reference image '" << node->GetName() << "' selected";
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
    LOG_WARN << "WARNING: No image or too many (>2) were selected.";
    referenceData = NULL;
    workingData = NULL;
  }

  // if only a segmentation is selected, try to find its parent and use it as reference node
  if ( workingData.IsNotNull() && referenceData.IsNull() )
  {
    LOG_DEBUG << "Finding segmentation's parent";
    // try to find a "normal image" parent, select this as reference image
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateNOT::Pointer isNotBinary = mitk::NodePredicateNOT::New( isBinary );
    mitk::NodePredicateAND::Pointer isNormalImage = mitk::NodePredicateAND::New( isImage, isNotBinary );

    mitk::DataStorage::SetOfObjects::ConstPointer possibleParents = m_DataStorage->GetSources( workingData, isNormalImage );

    if (possibleParents->size() > 0)
    {
      if (possibleParents->size() > 1)
      {
        // TODO visible warning for this rare case
        LOG_INFO << "Selected binary image has multiple parents. Using arbitrary first one for segmentation.";
      }

      mitk::DataTreeNode::Pointer referenceNode = (*possibleParents)[0];
      LOG_INFO << "Reference image '" << referenceNode->GetName() << "' selected";
      referenceData = referenceNode;
    }
  }

  LOG_INFO << "Reference " << (void*)referenceData.GetPointer() << " Working " << (void*)workingData.GetPointer();
  // update image selections for our toolmanagers
  OnReferenceNodeSelected(referenceData);
  OnWorkingDataSelectionChanged(workingData);

  if ( referenceData.IsNull() && workingData.IsNull() )
  {
    LOG_INFO << "Nothing selected, re-show all segmentations";
    // if nothing is chosen all segmentations are visible
    mitk::NodePredicateAND::Pointer and_predicate = mitk::NodePredicateAND::New(
        mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true)));
    mitk::DataStorage::SetOfObjects::ConstPointer segmentationSet = this->GetDefaultDataStorage()->GetSubset(and_predicate);
    const unsigned int size = segmentationSet->Size();

    for (unsigned int i = 0u; i < size; ++i)
    {
      segmentationSet->GetElement(i)->SetVisibility(true);
    }
  }

  // if an normal image is chosen without a segmentation, all child segmentations are visible
  if (referenceData.IsNotNull() && workingData.IsNull())
  {
    LOG_INFO << "Only reference selected, re-show all its child segmentations";
    mitk::NodePredicateAND::Pointer and_predicate = mitk::NodePredicateAND::New(
        mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true)));
    mitk::DataStorage::SetOfObjects::ConstPointer segmentationSet = this->GetDefaultDataStorage()->GetSubset(and_predicate);
    const unsigned int size = segmentationSet->Size();
    for (unsigned int i = 0u; i < size; ++i)
    {
      segmentationSet->GetElement(i)->SetVisibility(false);
    }

    mitk::DataStorage::SetOfObjects::ConstPointer childSegmentationSet = this->GetDefaultDataStorage()->GetDerivations(referenceData, and_predicate);
    const unsigned int child_size = childSegmentationSet->Size();
    for (unsigned int i = 0u; i < child_size; ++i)
    {
      childSegmentationSet->GetElement(i)->SetVisibility(true);
    }
  }
}

void QmitkSegmentationView::PartHidden(cherry::IWorkbenchPartReference::Pointer)
{
  m_Controls->m_ManualToolSelectionBox->setEnabled( false );
  m_Controls->m_OrganToolSelectionBox->setEnabled( false );
  m_Controls->m_LesionToolSelectionBox->setEnabled( false );
}

void QmitkSegmentationView::PartVisible(cherry::IWorkbenchPartReference::Pointer)
{
  if(m_MultiWidget)
  {
    m_MultiWidget->SetWidgetPlanesVisibility(false);

    m_Controls->m_ManualToolSelectionBox->setEnabled( true );
    m_Controls->m_OrganToolSelectionBox->setEnabled( true );
    m_Controls->m_LesionToolSelectionBox->setEnabled( true );
  }
}

void QmitkSegmentationView::Activated()
{
}

void QmitkSegmentationView::Deactivated()
{
}

void QmitkSegmentationView::CreateSurface(bool)
{
  LOG_INFO << "CreateSurface for:";

  NodeList selection = this->GetSelectedNodes();

  for ( NodeList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
  {
    mitk::DataTreeNode* node = *iter;

    if (node)
    {
      LOG_INFO << "   " << (*iter)->GetName();

      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNull()) return;
        
      try
      {
        mitk::ShowSegmentationAsSurface::Pointer surfaceFilter = mitk::ShowSegmentationAsSurface::New();

        // attach observer to get notified about result
        itk::SimpleMemberCommand<QmitkSegmentationView>::Pointer goodCommand = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
        goodCommand->SetCallbackFunction(this, &QmitkSegmentationView::OnSurfaceCalculationDone);
        surfaceFilter->AddObserver(mitk::ResultAvailable(), goodCommand);
        itk::SimpleMemberCommand<QmitkSegmentationView>::Pointer badCommand = itk::SimpleMemberCommand<QmitkSegmentationView>::New();
        badCommand->SetCallbackFunction(this, &QmitkSegmentationView::OnSurfaceCalculationDone);
        surfaceFilter->AddObserver(mitk::ProcessingError(), badCommand);

        mitk::DataTreeNode::Pointer nodepointer = node;
        surfaceFilter->SetPointerParameter("Input", image);
        surfaceFilter->SetPointerParameter("Group node", nodepointer);
        surfaceFilter->SetParameter("Smooth", true );
        surfaceFilter->SetParameter("Apply median", true );
        surfaceFilter->SetParameter("Median kernel size", 3u );
        surfaceFilter->SetParameter("Gaussian SD", 1.5f );
        surfaceFilter->SetParameter("Decimate mesh", true );
        surfaceFilter->SetParameter("Decimation rate", 0.8f );
        surfaceFilter->SetParameter("Show result", true );
        surfaceFilter->SetParameter("Sync visibility", false );
        surfaceFilter->SetDataStorage( *m_DataStorage );
        
        mitk::ProgressBar::GetInstance()->AddStepsToDo(1);
        mitk::StatusBar::GetInstance()->DisplayText("Surface creation started in background...");
        surfaceFilter->StartAlgorithm();
      }
      catch(...)
      {
        LOG_ERROR << "surface creation filter had an error";
      }
    }
    else
    {
      LOG_INFO << "   a NULL node selected";
    }
  }
 
  LOG_INFO << "That's all..";
}

QmitkSegmentationView::NodeList QmitkSegmentationView::GetSelectedNodes() const
{
  NodeList result;
  if (m_CurrentSelection)
  {
    // iterate selection
    for (cherry::IStructuredSelection::iterator i = m_CurrentSelection->Begin(); i != m_CurrentSelection->End(); ++i)
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

void QmitkSegmentationView::OnSurfaceCalculationDone()
{
  mitk::ProgressBar::GetInstance()->Progress();
}

