/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkSliceBasedSegmentation.h"
#include "QmitkSliceBasedSegmentationControls.h"
#include "slicebasedsegmentation.h" // icon

#include "QmitkStdMultiWidget.h"
#include "QmitkToolReferenceDataSelectionBox.h"
#include "QmitkToolWorkingDataListBox.h"
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

#include <qaction.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qcheckbox.h>

QmitkSliceBasedSegmentation::QmitkSliceBasedSegmentation(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it), 
  m_MultiWidget(mitkStdMultiWidget), 
  m_Controls(NULL)
{
  SetAvailability(true);
}

QmitkSliceBasedSegmentation::~QmitkSliceBasedSegmentation()
{
}

QWidget * QmitkSliceBasedSegmentation::CreateMainWidget(QWidget *parent)
{
  if ( m_MultiWidget == NULL )
  {
    m_MultiWidget = new QmitkStdMultiWidget( parent );
  }
  return m_MultiWidget;
}

QWidget * QmitkSliceBasedSegmentation::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkSliceBasedSegmentationControls(parent); // creates a tool manager

    mitk::ToolManager* toolManager = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager();

    assert ( toolManager );

    m_Controls->m_ToolReferenceDataSelectionBox->Initialize( m_DataTreeIterator->GetTree() );

    m_Controls->m_ToolWorkingDataListBox->SetToolManager( *toolManager );
    m_Controls->m_ToolWorkingDataListBox->SetAdditionalColumns("volume:Vol. [ml]");
    m_Controls->m_ToolWorkingDataListBox->SetToolGroupsForFiltering("default segmentationProcessing");

    m_Controls->m_ToolSelectionBox->SetToolManager( *toolManager );
    m_Controls->m_ToolSelectionBox->SetGenerateAccelerators(true);

    m_Controls->m_AutoSegmentationToolSelectionBox->setTitle("");
    m_Controls->m_AutoSegmentationToolSelectionBox->setInsideMargin(0);
    m_Controls->m_AutoSegmentationToolSelectionBox->setFrameStyle( QFrame::NoFrame );
    m_Controls->m_AutoSegmentationToolSelectionBox->SetLayoutColumns(1);
    m_Controls->m_AutoSegmentationToolSelectionBox->SetToolManager( *toolManager );
    m_Controls->m_AutoSegmentationToolSelectionBox->SetDisplayedToolGroups("autoSegmentation");
    m_Controls->m_AutoSegmentationToolSelectionBox->SetToolGUIArea( m_Controls->m_AutoSegmentationToolGUIContainer );
    m_Controls->m_AutoSegmentationToolSelectionBox->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceData );

    m_Controls->m_PostProcessingToolSelectionBox->setTitle("");
    m_Controls->m_PostProcessingToolSelectionBox->setInsideMargin(0);
    m_Controls->m_PostProcessingToolSelectionBox->setFrameStyle( QFrame::NoFrame );
    m_Controls->m_PostProcessingToolSelectionBox->SetLayoutColumns(1);
    m_Controls->m_PostProcessingToolSelectionBox->SetToolManager( *toolManager );
    m_Controls->m_PostProcessingToolSelectionBox->SetDisplayedToolGroups("segmentationProcessing"); // show only tools which are marked with "segmentationProcessing"
    m_Controls->m_PostProcessingToolSelectionBox->SetToolGUIArea( m_Controls->m_PostProcessingToolGUIContainer );

    m_Controls->m_SlicesInterpolator->Initialize( toolManager, m_MultiWidget );

    toolManager->NodePropertiesChanged.AddListener( this, &QmitkSliceBasedSegmentation::OnNodePropertiesChanged );
    toolManager->NewNodesGenerated.AddListener( this, &QmitkSliceBasedSegmentation::OnNewNodesGenerated );
  }
  return m_Controls;
}
    
void QmitkSliceBasedSegmentation::OnNodePropertiesChanged()
{
  m_Controls->m_ToolWorkingDataListBox->UpdateDataDisplay();
}
  
void QmitkSliceBasedSegmentation::OnNewNodesGenerated()
{
  m_Controls->m_ToolWorkingDataListBox->UpdateDataDisplay();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSliceBasedSegmentation::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->btnReinit, SIGNAL(clicked()), this, SLOT(ReinitializeToImage()) );
    connect( m_Controls->btnNewSegmentation, SIGNAL(clicked()), this, SLOT(CreateNewSegmentation()) );
    connect( m_Controls->btnLoadSegmentation, SIGNAL(clicked()), this, SLOT(LoadSegmentation()) );
    connect( m_Controls->btnDeleteSegmentation, SIGNAL(clicked()), this, SLOT(DeleteSegmentation()) );
    connect( m_Controls->btnSaveSegmentation, SIGNAL(clicked()), this, SLOT(SaveSegmentation()) );
    connect( m_Controls->m_ToolSelectionBox, SIGNAL(ToolSelected(int)), this, SLOT(OnToolSelected(int)) );
    connect( m_Controls->grpInterpolation, SIGNAL(toggled(bool)), m_Controls->m_SlicesInterpolator, SLOT(EnableInterpolation(bool)) );
    connect( m_Controls->chkPixelSmoothing, SIGNAL(toggled(bool)), this, SLOT(SetReferenceImagePixelSmoothing(bool)) );
    
    connect( m_Controls->m_ToolReferenceDataSelectionBox, SIGNAL(ReferenceNodeSelected (const mitk::DataTreeNode*)), this, SLOT(OnReferenceNodeSelected(const mitk::DataTreeNode*)) );
  }
}

QAction * QmitkSliceBasedSegmentation::CreateAction(QActionGroup *parent)
{
  QImage icon = qembed_findImage("slicebasedsegmentation");
  QPixmap pixmap(icon);
  QAction* action;
  action = new QAction( tr( "Manual segmentation" ), pixmap, tr( "QmitkSliceBasedSegmentation" ), 0, parent, "QmitkSliceBasedSegmentation" );
  return action;
}

void QmitkSliceBasedSegmentation::TreeChanged()
{
  m_Controls->m_ToolReferenceDataSelectionBox->UpdateDataDisplay();
  m_Controls->m_ToolWorkingDataListBox->UpdateDataDisplay();
}

void QmitkSliceBasedSegmentation::Activated()
{
  QmitkFunctionality::Activated();

  m_MultiWidget->SetWidgetPlanesVisibility(false);

  m_Controls->m_ToolSelectionBox->setEnabled( true );
  m_Controls->m_PostProcessingToolSelectionBox->setEnabled( true );
    
  m_Controls->m_ToolWorkingDataListBox->InstallKeyFilterOn( qApp );
}

void QmitkSliceBasedSegmentation::Deactivated()
{
  QmitkFunctionality::Deactivated();
  
  m_Controls->m_ToolSelectionBox->setEnabled( false );
  m_Controls->m_PostProcessingToolSelectionBox->setEnabled( false );
  
  m_Controls->m_ToolWorkingDataListBox->InstallKeyFilterOn( NULL );
}

void QmitkSliceBasedSegmentation::CreateNewSegmentation()
{
  mitk::DataTreeNode::Pointer node = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
    if (image.IsNotNull())
    {

      // ask about the name and organ type of the new segmentation
      QmitkNewSegmentationDialog dialog( m_Controls ); // needs a QWidget as parent, "this" is not QWidget
      int dialogReturnValue = dialog.exec();

      if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

      // ask the user about an organ type and name, add this information to the image's (!) propertylist
      // create a new image of the same dimensions and smallest possible pixel type
      mitk::DataTreeNode::Pointer emptySegmentation = CreateEmptySegmentationNode( image, dialog.GetOrganType(), dialog.GetSegmentationName() );

      if (!emptySegmentation) return; // could be aborted by user

      mitk::DataStorage::GetInstance()->Add( emptySegmentation, node ); // add as a child, because the segmentation "derives" from the original

      m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager()->SetWorkingData( emptySegmentation );
    }
  }
  
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSliceBasedSegmentation::DeleteSegmentation()
{
  mitk::ToolManager* toolManager = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager();
  if (!toolManager) return;

  mitk::ToolManager::DataVectorType nodes = toolManager->GetWorkingData();

  if (nodes.empty()) return;

  if ( QMessageBox::question( NULL, tr("MITK"), 
                                QString("Do you really want to delete the selected segmentations?"),
                                QMessageBox::Yes | QMessageBox::Default, 
                                QMessageBox::No  | QMessageBox::Escape
                              ) == QMessageBox::No )
  {
    return;
  }

  // for all selected nodes: try to remove the node from the scene
  bool failure(false);
  for ( mitk::ToolManager::DataVectorType::iterator nodeiter = nodes.begin();
        nodeiter != nodes.end();
        ++nodeiter )
  {
    mitk::DataTreeNode::Pointer node = *nodeiter;

    if (node.IsNotNull())
    {
      try
      {
        mitk::DataStorage::GetInstance()->Remove( node );
      }
      catch(...)
      {
        failure = true;
      }
    }
  }

  if (failure)
  {
    QMessageBox::information(NULL, "MITK", QString("Couldn't delete all of the segmentations."), QMessageBox::Ok);
  }

  toolManager->SetWorkingData(NULL); // unselect everything
      
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSliceBasedSegmentation::LoadSegmentation()
{
  try
  {
    mitk::DataTreeNode::Pointer automaticNode = CommonFunctionality::OpenVolumeOrSliceStack();

    if (automaticNode.IsNotNull())
    {
      // assign a nicer name (without the extension)
      std::string nodeName;
      automaticNode->GetName(nodeName);

      if (nodeName.find(".gz") == nodeName.size() -3 )
      {
        nodeName = nodeName.substr(0,nodeName.size() -3);
      }

      if (nodeName.find(".pic") == nodeName.size() -4 )
      {
        nodeName = nodeName.substr(0,nodeName.size() -4);
      }

      mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (automaticNode->GetData());
       
      if (    image.IsNull()
           || image->GetDimension() < 3
           || image->GetDimension() > 4
           || image->GetPixelType().GetNumberOfComponents() != 1 )
      {
        QMessageBox::information(NULL, "MITK", QString("Invalid segmentation. This program only supports monochrome 3D and 3D+t images."), QMessageBox::Ok);
        return;
      }

      QmitkNewSegmentationDialog dialog( m_Controls ); // needs a QWidget as parent, "this" is not QWidget
      dialog.setPrompt("What organ did you just load?");
      dialog.setSegmentationName( nodeName.c_str() );
      int dialogReturnValue = dialog.exec();

      if ( dialogReturnValue != QDialog::Rejected ) // user clicked cancel or pressed Esc or something similar
      {
        mitk::DataTreeNode::Pointer segmentationNode = CreateSegmentationNode( image, dialog.GetSegmentationName(), dialog.GetOrganType() );

        mitk::DataTreeNode::Pointer parentNode = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager()->GetReferenceData(0);
        mitk::DataStorage::GetInstance()->Add( segmentationNode, parentNode ); // add as a child of the currently active reference image

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        m_Controls->m_ToolWorkingDataListBox->UpdateDataDisplay();
      }
    }
    else 
    {
      QMessageBox::information(NULL, "MITK", QString("The selected file does not contain a segmentation, sorry."), QMessageBox::Ok);
      return;
    }

  }
  catch( std::exception& e )
  {
    HandleException( e.what(), m_Controls, true );
  }
  catch( ... )
  {
    HandleException( "An unknown error occurred!", m_Controls, true );
  }
}

void QmitkSliceBasedSegmentation::SaveSegmentation()
{
  mitk::ToolManager* toolManager = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager();
  if (!toolManager) return;

  mitk::DataTreeNode::Pointer node = toolManager->GetWorkingData(0); 
  if (node.IsNull()) return;
  
  mitk::DataTreeNode::Pointer node2 = toolManager->GetWorkingData(1); 
  if (node2.IsNotNull()) 
  {
    QMessageBox::information(NULL, "MITK", QString("You can only save one segmentation at a time. Please change your selection."), QMessageBox::Ok);
    return;
  }
  
  try
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );

    if (image.IsNotNull())
    {
      std::string nodeName;
      if ( node->GetName( nodeName ) )
      {
        nodeName += ".pic";
        CommonFunctionality::SaveImage(image, nodeName.c_str(), true); // use "name" property as initial filename suggestion
      }
      else
      {
        CommonFunctionality::SaveImage(image);
      }
    }
    else
    {
      QMessageBox::information(NULL, "MITK", QString("Your selection is technically not an image, unable to save it."), QMessageBox::Ok);
    }
  }
  catch( std::exception& e )
  {
    QMessageBox::information(NULL, "MITK", QString("Couldn't save your segmentation!"), QMessageBox::Ok);
    HandleException( e.what(), m_Controls, true );
  }
  catch( ... )
  {
    HandleException( "An unknown error occurred!", m_Controls, true );
  }
}

/** TODO bug #1342: should be more central, clearly not in functionality **/
mitk::DataTreeNode::Pointer QmitkSliceBasedSegmentation::CreateEmptySegmentationNode( mitk::Image* image, const std::string& organType, const std::string name )
{
  // we NEED a reference image for size etc.
  if (!image) return NULL;

  // actually create a new empty segmentation
  mitk::PixelType pixelType( typeid(mitk::Tool::DefaultSegmentationDataType) );
  mitk::Image::Pointer segmentation = mitk::Image::New();
  segmentation->SetProperty( "organ type", mitk::OrganTypeProperty::New( organType ) );
  segmentation->Initialize( pixelType, image->GetDimension(), image->GetDimensions() );

  unsigned int byteSize = sizeof(mitk::Tool::DefaultSegmentationDataType);
  for (unsigned int dim = 0; dim < segmentation->GetDimension(); ++dim) 
  {
    byteSize *= segmentation->GetDimension(dim);
  }
  memset( segmentation->GetData(), 0, byteSize );

  if (image->GetTimeSlicedGeometry() )
  {
    mitk::AffineGeometryFrame3D::Pointer originalGeometryAGF = image->GetTimeSlicedGeometry()->Clone();
    mitk::TimeSlicedGeometry::Pointer originalGeometry = dynamic_cast<mitk::TimeSlicedGeometry*>( originalGeometryAGF.GetPointer() );
    segmentation->SetGeometry( originalGeometry );
  }
  else
  {
    QMessageBox::information(NULL, "MITK", QString("Original image does not have a 'Time sliced geometry'! Cannot create a segmentation."), QMessageBox::Ok);
    return NULL;
  }

  return CreateSegmentationNode( segmentation, name, organType );
}

/** TODO bug #1342: should be more central, clearly not in functionality **/
mitk::DataTreeNode::Pointer QmitkSliceBasedSegmentation::CreateSegmentationNode( mitk::Image* image, const std::string& name, const std::string& organType )
{
  if (!image) return NULL;

  // decorate the datatreenode with some properties
  mitk::DataTreeNode::Pointer segmentationNode = mitk::DataTreeNode::New();
  segmentationNode->SetData( image );
  mitk::DataTreeNodeFactory::SetDefaultImageProperties ( segmentationNode );

  // name
  segmentationNode->SetProperty( "name", mitk::StringProperty::New( name ) );

  // organ type
  mitk::OrganTypeProperty::Pointer organTypeProperty = mitk::OrganTypeProperty::New( organType );
  if ( !organTypeProperty->IsValidEnumerationValue( organType ) )
  {
    organTypeProperty->AddEnum( organType, organTypeProperty->Size() ); // add a new organ type
    organTypeProperty->SetValue( organType );
  }

  // visualization properties
  segmentationNode->SetProperty( "binary", mitk::BoolProperty::New(true) );
  segmentationNode->SetProperty( "color", mitk::DataTreeNodeFactory::DefaultColorForOrgan( organType ) );
  segmentationNode->SetProperty( "texture interpolation", mitk::BoolProperty::New(false) );
  segmentationNode->SetProperty( "layer", mitk::IntProperty::New(10) );
  segmentationNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0.5, 1) ) );
  segmentationNode->SetProperty( "opacity", mitk::FloatProperty::New(0.3) );
  segmentationNode->SetProperty( "segmentation", mitk::BoolProperty::New(true) );
  segmentationNode->SetProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New() ); // otherwise -> segmentation appears in 2 slices sometimes (only visual effect, not different data)

  return segmentationNode;
}

void QmitkSliceBasedSegmentation::OnToolSelected(int id)
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

void QmitkSliceBasedSegmentation::ReinitializeToImage()
{
  mitk::DataTreeNode::Pointer node = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    mitk::BaseData::Pointer basedata = node->GetData();
    if (basedata.IsNotNull())
    {
      mitk::RenderingManager::GetInstance()->InitializeViews( basedata->GetTimeSlicedGeometry() );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

  }
}

void QmitkSliceBasedSegmentation::SetReferenceImagePixelSmoothing(bool on)
{
  mitk::DataTreeNode::Pointer node = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    node->SetProperty("texture interpolation", mitk::BoolProperty::New(on));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSliceBasedSegmentation::OnReferenceNodeSelected(const mitk::DataTreeNode* node)
{
 if (node)
 {
    bool on(false);
    if (node->GetBoolProperty("texture interpolation", on))
    {
      m_Controls->chkPixelSmoothing->setEnabled(true);
      m_Controls->chkPixelSmoothing->setChecked(on);
    }
    else
    {
      m_Controls->chkPixelSmoothing->setEnabled(false);
    }
 }
}

