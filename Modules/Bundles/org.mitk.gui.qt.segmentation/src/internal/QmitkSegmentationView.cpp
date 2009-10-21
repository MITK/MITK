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

#include "mitkToolManager.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkOrganTypeProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkSegTool2D.h"

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

  this->CreateConnections();

  m_SelectionListener = cherry::ISelectionListener::Pointer(new cherry::SelectionChangedAdapter<QmitkSegmentationView>(this, &QmitkSegmentationView::SelectionChanged));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelectionListener);
  cherry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection());
  m_CurrentSelection = selection.Cast<const cherry::IStructuredSelection>();
  this->SelectionChanged(cherry::SmartPointer<IWorkbenchPart>(NULL), selection);
}

void QmitkSegmentationView::SetFocus()
{
}

QmitkSegmentationView::~QmitkSegmentationView()
{
  this->Deactivated();
  delete m_Controls;
}

void QmitkSegmentationView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->btnNewSegmentation, SIGNAL(clicked()), this, SLOT(CreateNewSegmentation()) );
    connect( m_Controls->m_ManualToolSelectionBox, SIGNAL(ToolSelected(int)), this, SLOT(OnToolSelected(int)) );
  }
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

        this->GetDefaultDataStorage()->Add( emptySegmentation, node ); // add as a child, because the segmentation "derives" from the original

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
  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  toolManager->SetReferenceData(const_cast<mitk::DataTreeNode*>(node));

  QString patientImageText("Patient image");

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
  		mitk::DataTreeNode* act_node = image_set->GetElement(i);

  		if (act_node != node)
  		{
  			act_node->SetVisibility(false);
  		}
  		else
  		{
  			act_node->SetVisibility(true);
  		}
  	}

    m_Controls->lblReferenceImageSelectionWarning->hide();
    patientImageText = QString("Patient image '%1'").arg( node->GetName().c_str() );
    // TODO show this image, hide all other images. example code maybe in QmitkToolReferenceDataSelectionBox
  }
  else
  {
    m_Controls->lblReferenceImageSelectionWarning->show();
  }

  //m_Controls->grpReferenceData->setTitle( patientImageText );

  // check, wheter image is aligned like render windows. Otherwise display a visible warning (because 2D tools will probably not work)
  CheckImageAlignment();
}

void QmitkSegmentationView::OnWorkingDataSelectionChanged(const mitk::DataTreeNode* node)
{
  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  toolManager->SetWorkingData(const_cast<mitk::DataTreeNode*>(node));

  if (node)
  {
  	/*
  	 * Other segmentations are set invisible.
  	 */
  	mitk::DataStorage::SetOfObjects::ConstPointer segmentation_set = this->GetDefaultDataStorage()->GetSubset(
  			mitk::NodePredicateAND::New(
  					mitk::TNodePredicateDataType<mitk::Image>::New(),
  					mitk::NodePredicateProperty::New("segmentation", mitk::GenericProperty<bool>::New(true))));
  	const unsigned int size = segmentation_set->Size();

  	for (unsigned int i = 0u; i < size; ++i)
  	{
  		mitk::DataTreeNode* act_node = segmentation_set->GetElement(i);

  		if (act_node != node)
  		{
  			act_node->SetVisibility(false);
  		}
  		else
  		{
  			act_node->SetVisibility(true);
  		}
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
//  if ( sourcepart.IsNull() || sourcepart->GetPartName() != "Datamanager" ) return;
  if ( sourcepart == this ||  // prevents being notified by own selection events
       !selection.Cast<const cherry::IStructuredSelection>() ) // checks that the selection is a IStructuredSelection and not NULL
  {
    return; // otherwise we get "null selection" events each time the view is activated/focussed
  }

  std::cout << "selection changed" << std::endl;
  // save current selection in member variable
  m_CurrentSelection = selection.Cast<const cherry::IStructuredSelection>();

  bool newReferenceImageSelected(false);
  bool newWorkingImageSelected(false);
  mitk::DataTreeNode::Pointer working_data = 0u, reference_data = 0u;

  // TODO warning when two images are selected
  // do something with the selected items
  if(m_CurrentSelection && (m_CurrentSelection->Size() < 3u))
  {
  	bool error = false;

    // iterate selection
    for (cherry::IStructuredSelection::iterator i = m_CurrentSelection->Begin(); !error && (i != m_CurrentSelection->End()); ++i)
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
        //bool isVisible(true);
        node->GetBoolProperty("segmentation", isSegmentation);
        //node->GetBoolProperty("visible", isVisible);

				if (isImage)
				{
					if ( isSegmentation )
					{
						if (!newWorkingImageSelected)
						{
							LOG_INFO << "Working image '" << node->GetName() << "' selected";
							newWorkingImageSelected = true;
							working_data = node;
						}
						else
						{
							error = true;
						}
					}
					else
					{
						if (!newReferenceImageSelected)
						{
							LOG_INFO << "Reference image '" << node->GetName() << "' selected";
							newReferenceImageSelected = true;
							reference_data = node;
						}
						else
						{
							error = true;
						}
					}
				}
      }
    }

    if (error)
    {
    	newWorkingImageSelected = false;
    	newReferenceImageSelected = false;
    	LOG_INFO << "WARNING: No image or too many (>2) were selected.";
    }
    else
    {
    	OnWorkingDataSelectionChanged(working_data);
    	OnReferenceNodeSelected(reference_data);
    }
  }

  // if necessary try to find parent for segmentation
  if ( newWorkingImageSelected && !newReferenceImageSelected )
  {
    // try to find a "normal image" parent, select this as reference image
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateNOT::Pointer isNotBinary = mitk::NodePredicateNOT::New( isBinary );
    mitk::NodePredicateAND::Pointer isNormalImage = mitk::NodePredicateAND::New( isImage, isNotBinary );

    mitk::DataStorage::SetOfObjects::ConstPointer possibleParents =
      m_DataStorage->GetSources( m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetWorkingData(0), isNormalImage );

    if (possibleParents->size() > 0)
    {
      if (possibleParents->size() > 1)
      {
        LOG_INFO << "Selected binary image has multiple parents. Using arbitrary first one for segmentation.";
      }

      mitk::DataTreeNode::Pointer referenceNode = (*possibleParents)[0];
      LOG_INFO << "Reference image '" << referenceNode->GetName() << "' selected";
      newReferenceImageSelected = true;
      OnReferenceNodeSelected(referenceNode);
    }
  }

  if (!newReferenceImageSelected)
  {
    LOG_INFO << "NO reference image selected";
    OnReferenceNodeSelected(NULL);
  }

  if (!newWorkingImageSelected)
  {
    // if an normal image is chosen, all child segmentations are visible
  	// if nothing is chosen all segmentations are visible
  	mitk::NodePredicateAND::Pointer and_predicate = mitk::NodePredicateAND::New(
				mitk::TNodePredicateDataType<mitk::Image>::New(),
				mitk::NodePredicateProperty::New("segmentation", mitk::GenericProperty<bool>::New(true)));
  	mitk::DataStorage::SetOfObjects::ConstPointer segmentation_set = this->GetDefaultDataStorage()->GetSubset(and_predicate);
  	const unsigned int size = segmentation_set->Size();

  	if (newReferenceImageSelected)
  	{
    	mitk::DataStorage::SetOfObjects::ConstPointer child_segmentation_set = this->GetDefaultDataStorage()->GetDerivations(reference_data, and_predicate);
    	const unsigned int child_size = child_segmentation_set->Size();

			for (unsigned int i = 0u; i < size; ++i)
			{
				segmentation_set->GetElement(i)->SetVisibility(false);
			}

			for (unsigned int i = 0u; i < child_size; ++i)
			{
				child_segmentation_set->GetElement(i)->SetVisibility(true);
			}
  	}
  	else
  	{
			for (unsigned int i = 0u; i < size; ++i)
			{
				segmentation_set->GetElement(i)->SetVisibility(true);
			}
  	}

    LOG_INFO << "NO working image selected";
    OnWorkingDataSelectionChanged(NULL);
  }
}

void QmitkSegmentationView::PartHidden(cherry::IWorkbenchPartReference::Pointer)
{
  m_Controls->m_ManualToolSelectionBox->setEnabled( false );
}

void QmitkSegmentationView::PartVisible(cherry::IWorkbenchPartReference::Pointer)
{
  if(m_MultiWidget)
  {
    m_MultiWidget->SetWidgetPlanesVisibility(false);

    m_Controls->m_ManualToolSelectionBox->setEnabled( true );
  }
}

void QmitkSegmentationView::Activated()
{
}

void QmitkSegmentationView::Deactivated()
{
}

