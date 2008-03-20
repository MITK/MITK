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
#include "QmitkCopyToClipBoardDialog.h"

#include "mitkToolManager.h"
#include "mitkProgressBar.h"
#include "mitkAutoCropImageFilter.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkOrganTypeProperty.h"
#include "mitkImageCast.h"
#include "mitkShowSegmentationAsSurface.h"
#include "mitkCalculateSegmentationVolume.h"
#include "mitkVtkResliceInterpolationProperty.h"

#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkMapContainer.h>

#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qmessagebox.h>
#include <qslider.h>
#include <qcheckbox.h>

#include <limits>

#define SEGMENTATION_DATATYPE unsigned char

QmitkSliceBasedSegmentation::QmitkSliceBasedSegmentation(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it), 
  m_MultiWidget(mitkStdMultiWidget), 
  m_Controls(NULL),
  m_NumberOfVolumeCalculationThreads(0)
{
  SetAvailability(true);

  m_ThresholdFeedbackNode = mitk::DataTreeNode::New();
  mitk::DataTreeNodeFactory::SetDefaultImageProperties ( m_ThresholdFeedbackNode );
  m_ThresholdFeedbackNode->SetProperty( "color", new mitk::ColorProperty(0.2, 1.0, 0.2) );
  m_ThresholdFeedbackNode->SetProperty( "texture interpolation", new mitk::BoolProperty(false) );
  m_ThresholdFeedbackNode->SetProperty( "layer", new mitk::IntProperty( 20 ) );
  m_ThresholdFeedbackNode->SetProperty( "levelwindow", new mitk::LevelWindowProperty( mitk::LevelWindow(100, 1) ) );
  m_ThresholdFeedbackNode->SetProperty( "name", new mitk::StringProperty("Thresholding feedback") );
  m_ThresholdFeedbackNode->SetProperty( "opacity", new mitk::FloatProperty(0.2) );
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
    m_Controls->btnAcceptThreshold->hide(); // hide initially
    m_Controls->sliderThreshold->hide();

    mitk::ToolManager* toolManager = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager();

    assert ( toolManager );

    m_Controls->m_ToolReferenceDataSelectionBox->Initialize( m_DataTreeIterator->GetTree() );

    m_Controls->m_ToolWorkingDataListBox->SetToolManager( *toolManager );
    m_Controls->m_ToolWorkingDataListBox->SetAdditionalColumns("volume:Vol. [ml]");

    m_Controls->m_ToolSelectionBox->SetToolManager( *toolManager );

    m_Controls->m_SlicesInterpolator->Initialize( toolManager, m_MultiWidget );
  }
  return m_Controls;
}

void QmitkSliceBasedSegmentation::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->btnReinit, SIGNAL(clicked()), this, SLOT(ReinitializeToImage()) );
    connect( m_Controls->btnNewSegmentation, SIGNAL(clicked()), this, SLOT(CreateNewSegmentation()) );
    connect( m_Controls->btnLoadSegmentation, SIGNAL(clicked()), this, SLOT(LoadSegmentation()) );
    connect( m_Controls->btnDeleteSegmentation, SIGNAL(clicked()), this, SLOT(DeleteSegmentation()) );
    connect( m_Controls->btnAutoCropSegmentation, SIGNAL(clicked()), this, SLOT(AutoCropSegmentation()) );
    connect( m_Controls->btnCreateSurface, SIGNAL(clicked()), this, SLOT(CreateSurfaceFromSegmentation()) );
    connect( m_Controls->btnVolumetry, SIGNAL(clicked()), this, SLOT(CalculateVolumeForSegmentation()) );
    connect( m_Controls->btnGrayValueStatistics, SIGNAL(clicked()), this, SLOT(CalculateStatisticsVolumeForSegmentation()) );
    connect( m_Controls->btnNewFromThreshold, SIGNAL(toggled(bool)), this, SLOT(InitiateCreateNewSegmentationFromThreshold(bool)) );
    connect( m_Controls->sliderThreshold, SIGNAL(valueChanged(int)), this, SLOT(CreateNewSegmentationFromThresholdSliderChanged(int)) );
    connect( m_Controls->btnAcceptThreshold, SIGNAL(clicked()), this, SLOT(CreateNewSegmentationFromThreshold()) );
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
    
  m_Controls->m_ToolWorkingDataListBox->InstallKeyFilterOn( qApp );
}

void QmitkSliceBasedSegmentation::Deactivated()
{
  QmitkFunctionality::Deactivated();
  
  m_Controls->m_ToolSelectionBox->setEnabled( false );
  
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

void QmitkSliceBasedSegmentation::AutoCropSegmentation()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  QString problemBaeren;

  mitk::ToolManager* toolManager = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager();
  if (!toolManager) 
  {
    QApplication::restoreOverrideCursor();
    return;
  }

  mitk::ToolManager::DataVectorType nodes = toolManager->GetWorkingData();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(nodes.size());

  // for all selected nodes: try to crop the image
  for ( mitk::ToolManager::DataVectorType::iterator nodeiter = nodes.begin();
        nodeiter != nodes.end();
        ++nodeiter )
  {
    mitk::DataTreeNode::Pointer node = *nodeiter;

    if (node.IsNotNull())
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNotNull())
      {
        mitk::AutoCropImageFilter::Pointer cropFilter = mitk::AutoCropImageFilter::New();
        cropFilter->SetInput( image );
        cropFilter->SetBackgroundValue( 0 );
        try
        {
          cropFilter->Update();

          image = cropFilter->GetOutput();
          if (image.IsNotNull())
          {
            node->SetData( image );
          }
          else
          {
            std::string nodeName;
            problemBaeren += " ";
            if ( node->GetName( nodeName ) )
            {
              problemBaeren += nodeName.c_str();
            }
            else
            {
              problemBaeren += "(no name)";
            }
          }
        }
        catch(...)
        {
          std::string nodeName;
          problemBaeren += " ";
          if ( node->GetName( nodeName ) )
          {
            problemBaeren += nodeName.c_str();
          }
          else
          {
            problemBaeren += "(no name)";
          }
        }
      }
    }
    
    mitk::ProgressBar::GetInstance()->Progress();
  }

  // report possible errors
  if (!problemBaeren.isEmpty())
  {
#ifndef BUILD_TESTING
    QMessageBox::information(NULL, "MITK", QString("Could not crop these images:\n") + problemBaeren, QMessageBox::Ok);
#endif
  }

  QApplication::restoreOverrideCursor();
}
    
void QmitkSliceBasedSegmentation::CreateSurfaceFromSegmentation()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  mitk::ToolManager* toolManager = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager();
  if (!toolManager) 
  {
    QApplication::restoreOverrideCursor();
    return;
  }

  mitk::ToolManager::DataVectorType nodes = toolManager->GetWorkingData();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(nodes.size());

  // for all selected nodes: try to crop the image
  for ( mitk::ToolManager::DataVectorType::iterator nodeiter = nodes.begin();
        nodeiter != nodes.end();
        ++nodeiter )
  {
    mitk::DataTreeNode::Pointer node = *nodeiter;

    if (node.IsNotNull())
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNotNull())
      {
        // create threaded surface algorithm object
        mitk::ShowSegmentationAsSurface::Pointer surfaceFilter = mitk::ShowSegmentationAsSurface::New();

        // attach observer to get notified about result
        itk::SimpleMemberCommand<QmitkSliceBasedSegmentation>::Pointer goodCommand = itk::SimpleMemberCommand<QmitkSliceBasedSegmentation>::New();
        goodCommand->SetCallbackFunction(this, &QmitkSliceBasedSegmentation::OnSurfaceCalculationDone);
        /* tag = */ surfaceFilter->AddObserver(mitk::ResultAvailable(), goodCommand);

        surfaceFilter->SetPointerParameter("Input", image);
        surfaceFilter->SetPointerParameter("Group node", node);
        surfaceFilter->SetParameter("Show result", true );
        surfaceFilter->SetParameter("Sync visibility", false );
        
        surfaceFilter->StartAlgorithm(); // starts thread
      }
    }
    
    mitk::ProgressBar::GetInstance()->Progress();
  }
   
  if ( !nodes.empty() )
  {
    QMessageBox::information(NULL, "MITK", QString("Surface creation is started in a background task, it will take some time..."), QMessageBox::Ok);
  }

  QApplication::restoreOverrideCursor();
}

void QmitkSliceBasedSegmentation::CalculateVolumeForSegmentation()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  mitk::ToolManager* toolManager = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager();
  if (!toolManager) 
  {
    QApplication::restoreOverrideCursor();
    return;
  }

  mitk::ToolManager::DataVectorType nodes = toolManager->GetWorkingData();

  // for all selected nodes: try to crop the image
  for ( mitk::ToolManager::DataVectorType::iterator nodeiter = nodes.begin();
        nodeiter != nodes.end();
        ++nodeiter )
  {
    mitk::DataTreeNode::Pointer node = *nodeiter;

    if (node.IsNotNull())
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNotNull())
      {
        mitk::CalculateSegmentationVolume::Pointer volumeFilter = mitk::CalculateSegmentationVolume::New();

        // attach observer to get notified about result
        itk::SimpleMemberCommand<QmitkSliceBasedSegmentation>::Pointer goodCommand = itk::SimpleMemberCommand<QmitkSliceBasedSegmentation>::New();
        goodCommand->SetCallbackFunction(this, &QmitkSliceBasedSegmentation::OnVolumeCalculationDone);
        /* tag = */ volumeFilter->AddObserver(mitk::ResultAvailable(), goodCommand);

        volumeFilter->SetPointerParameter("Input", image);
        volumeFilter->SetPointerParameter("Group node", node);

        if ( m_Controls->btnVolumetry->isEnabled() )
        {
          m_Controls->btnVolumetry->setTextLabel("Calculating...");
          m_Controls->btnVolumetry->setEnabled(false);
        }


        ++m_NumberOfVolumeCalculationThreads;
        volumeFilter->StartAlgorithm();
      }
    }
  }
    
  QApplication::restoreOverrideCursor();
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

mitk::DataTreeNode::Pointer QmitkSliceBasedSegmentation::CreateEmptySegmentationNode( mitk::Image* image, const std::string& organType, const std::string name )
{
  // we NEED a reference image for size etc.
  if (!image) return NULL;

  // actually create a new empty segmentation
  mitk::PixelType pixelType( typeid(SEGMENTATION_DATATYPE) );
  mitk::Image::Pointer segmentation = mitk::Image::New();
  segmentation->SetProperty( "organ type", new mitk::OrganTypeProperty( organType ) );
  segmentation->Initialize( pixelType, image->GetDimension(), image->GetDimensions() );
  memset( segmentation->GetData(), 0, sizeof(SEGMENTATION_DATATYPE) * segmentation->GetDimension(0) * segmentation->GetDimension(1) * segmentation->GetDimension(2) );

  if (image->GetGeometry() )
  {
    mitk::AffineGeometryFrame3D::Pointer originalGeometryAGF = image->GetGeometry()->Clone();
    mitk::Geometry3D::Pointer originalGeometry = dynamic_cast<mitk::Geometry3D*>( originalGeometryAGF.GetPointer() );
    segmentation->SetGeometry( originalGeometry );
  }

  return CreateSegmentationNode( segmentation, name, organType );
}

mitk::DataTreeNode::Pointer QmitkSliceBasedSegmentation::CreateSegmentationNode( mitk::Image* image, const std::string& name, const std::string& organType )
{
  if (!image) return NULL;

  // decorate the datatreenode with some properties
  mitk::DataTreeNode::Pointer segmentationNode = mitk::DataTreeNode::New();
  segmentationNode->SetData( image );
  mitk::DataTreeNodeFactory::SetDefaultImageProperties ( segmentationNode );

  // name
  segmentationNode->SetProperty( "name", new mitk::StringProperty( name ) );

  // organ type
  mitk::OrganTypeProperty::Pointer organTypeProperty = new mitk::OrganTypeProperty( organType );
  if ( !organTypeProperty->IsValidEnumerationValue( organType ) )
  {
    organTypeProperty->AddEnum( organType, organTypeProperty->Size() ); // add a new organ type
    organTypeProperty->SetValue( organType );
  }

  // visualization properties
  segmentationNode->SetProperty( "binary", new mitk::BoolProperty(true) );
  segmentationNode->SetProperty( "color", mitk::DataTreeNodeFactory::DefaultColorForOrgan( organType ) );
  segmentationNode->SetProperty( "texture interpolation", new mitk::BoolProperty(false) );
  segmentationNode->SetProperty( "layer", new mitk::IntProperty(10) );
  segmentationNode->SetProperty( "levelwindow", new mitk::LevelWindowProperty( mitk::LevelWindow(0.5, 1) ) );
  segmentationNode->SetProperty( "opacity", new mitk::FloatProperty(0.3) );
  segmentationNode->SetProperty( "segmentation", new mitk::BoolProperty(true) );
  segmentationNode->SetProperty( "reslice interpolation", new mitk::VtkResliceInterpolationProperty() ); // otherwise -> segmentation appears in 2 slices sometimes (only visual effect, not different data)

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

void QmitkSliceBasedSegmentation::InitiateCreateNewSegmentationFromThreshold(bool toggled)
{
  m_Controls->btnAcceptThreshold->setShown(toggled);
  m_Controls->sliderThreshold->setShown(toggled);

  if (toggled) // initiate thresholding
  {
    bool everythingFine(false);
    mitk::DataTreeNode::Pointer node = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager()->GetReferenceData(0);
    if (node.IsNotNull())
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNotNull())
      {
        // initialize and a new node with the same image as our reference image
        // use the level window property of this image copy to display the result of a thresholding operation
        m_ThresholdFeedbackNode->SetData( image );
        int layer(0);
        node->GetIntProperty("layer", layer);
        m_ThresholdFeedbackNode->SetIntProperty("layer", layer+1);
         
        mitk::DataStorage::GetInstance()->Add( m_ThresholdFeedbackNode, node );
 
        m_Controls->sliderThreshold->setMinValue( static_cast<int>( image->GetScalarValueMin() ) );
        m_Controls->sliderThreshold->setMaxValue( static_cast<int>( image->GetScalarValueMax() ) );
  
        mitk::LevelWindowProperty::Pointer lwp = dynamic_cast<mitk::LevelWindowProperty*>( m_ThresholdFeedbackNode->GetProperty( "levelwindow" ));
        if (lwp)
        {
          m_Controls->sliderThreshold->setValue( static_cast<int>( lwp->GetLevelWindow().GetLevel() ) );
        }
        else
        {
          m_Controls->sliderThreshold->setValue( (m_Controls->sliderThreshold->maxValue() + m_Controls->sliderThreshold->minValue()) / 2 );
        }

        everythingFine = true;
      }
    }

    if (!everythingFine)
    {
      m_Controls->btnNewFromThreshold->setOn(false);
    }
  }
  else
  {
    mitk::DataStorage::GetInstance()->Remove( m_ThresholdFeedbackNode );
    m_ThresholdFeedbackNode->SetData( NULL );
  }
        
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSliceBasedSegmentation::CreateNewSegmentationFromThreshold()
{
  m_Controls->btnNewFromThreshold->setOn(false);

  // only proceed if we still have a reference data object (otherwise the user quite messed up)
  mitk::DataTreeNode::Pointer node = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
    if (image.IsNotNull())
    {
      // ask about the name and organ type of the new segmentation
      QmitkNewSegmentationDialog dialog( m_Controls ); // needs a QWidget as parent, "this" is not QWidget
      dialog.setPrompt("What did you just segment?");
      int dialogReturnValue = dialog.exec();

      if ( dialogReturnValue != QDialog::Rejected ) // user clicked cancel or pressed Esc or something similar
      {
        // ask the user about an organ type and name, add this information to the image's (!) propertylist
        // create a new image of the same dimensions and smallest possible pixel type
        mitk::DataTreeNode::Pointer emptySegmentation = CreateEmptySegmentationNode( image, dialog.GetOrganType(), dialog.GetSegmentationName() );

        if (emptySegmentation)
        {
          // actually perform a thresholding and ask for an organ type
          AccessFixedDimensionByItk_1( image, ITKThresholding, 3, dynamic_cast<mitk::Image*>(emptySegmentation->GetData()) );

          mitk::DataStorage::GetInstance()->Add( emptySegmentation, node ); // add as a child, because the segmentation "derives" from the original

          m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager()->SetWorkingData( emptySegmentation );
        }
      }
    }
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSliceBasedSegmentation::CreateNewSegmentationFromThresholdSliderChanged(int threshold)
{
  m_ThresholdFeedbackNode->SetProperty( "levelwindow", new mitk::LevelWindowProperty( mitk::LevelWindow(threshold, 1) ) );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

template <typename TPixel, unsigned int VImageDimension>
void QmitkSliceBasedSegmentation::ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image* segmentation )
{
  typedef itk::Image<SEGMENTATION_DATATYPE, 3> SegmentationType; // this is sure for new segmentations
  SegmentationType::Pointer itkSegmentation;
  mitk::CastToItkImage( segmentation, itkSegmentation );

  // iterate over original and segmentation
  typedef itk::ImageRegionConstIterator< itk::Image<TPixel, VImageDimension> >     InputIteratorType;
  typedef itk::ImageRegionIterator< SegmentationType >     SegmentationIteratorType;

  InputIteratorType inputIterator( originalImage, originalImage->GetLargestPossibleRegion() );
  SegmentationIteratorType outputIterator( itkSegmentation, itkSegmentation->GetLargestPossibleRegion() );

  inputIterator.GoToBegin();
  outputIterator.GoToBegin();

  int threshold = m_Controls->sliderThreshold->value();

  while (!outputIterator.IsAtEnd())
  {
    if ( (signed)inputIterator.Get() >= threshold )
    {
      outputIterator.Set( 1 );
    }
    else
    {
      outputIterator.Set( 0 );
    }

    ++inputIterator;
    ++outputIterator;
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
      m_MultiWidget->InitializeStandardViews( basedata->GetTimeSlicedGeometry() );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

  }
}

void QmitkSliceBasedSegmentation::SetReferenceImagePixelSmoothing(bool on)
{
  mitk::DataTreeNode::Pointer node = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    node->SetProperty("texture interpolation", new mitk::BoolProperty(on));
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

void QmitkSliceBasedSegmentation::OnSurfaceCalculationDone()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}        

void QmitkSliceBasedSegmentation::OnVolumeCalculationDone()
{
  m_Controls->m_ToolWorkingDataListBox->UpdateDataDisplay();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  --m_NumberOfVolumeCalculationThreads;

  if (m_NumberOfVolumeCalculationThreads == 0)
  {
    m_Controls->btnVolumetry->setTextLabel("Volumetry");
    m_Controls->btnVolumetry->setEnabled(true);
  }
}

void QmitkSliceBasedSegmentation::CalculateStatisticsVolumeForSegmentation()
{
  mitk::ToolManager* toolManager = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager();
  if (!toolManager) return;

  mitk::DataTreeNode* referencenode = toolManager->GetReferenceData(0);
  mitk::ToolManager::DataVectorType nodes = toolManager->GetWorkingData();

  QString completeReport;

  // for all selected nodes: try to crop the image
  for ( mitk::ToolManager::DataVectorType::iterator nodeiter = nodes.begin();
        nodeiter != nodes.end();
        ++nodeiter )
  {
    mitk::DataTreeNode::Pointer node = *nodeiter;
    if (!node) continue;

    std::string nodename("structure");
    node->GetName(nodename);

    if (node.IsNotNull())
    {
      mitk::Image::Pointer refImage = dynamic_cast<mitk::Image*>( referencenode->GetData() );
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNotNull() && refImage.IsNotNull() )
      {
        completeReport += QString("============= Gray value analysis of %1 ====\n").arg(nodename.c_str());
        AccessFixedDimensionByItk_2( refImage, ITKHistogramming, 3, image, completeReport );
        completeReport += QString("============= End of %1 ====================\n\n\n").arg(nodename.c_str());
      }
    }
  }

  if(completeReport.isEmpty() == false)
  {
    std::cout << completeReport.ascii() << std::endl;

    QmitkCopyToClipBoardDialog* dialog = new QmitkCopyToClipBoardDialog(completeReport, NULL);
    dialog->show();
  }

  // TODO (bug #1155): nice output in some graphical window (ready for copy to clipboard)
  // TODO (bug #874): remove these 5 buttons for segmentation operations. find some good interface for them and make them use less GUI space 
}

#define ROUND_P(x) ((int)((x)+0.5))
    
template <typename TPixel, unsigned int VImageDimension>
void QmitkSliceBasedSegmentation::ITKHistogramming( itk::Image<TPixel, VImageDimension>* referenceImage, mitk::Image* segmentation, QString& report )
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<SEGMENTATION_DATATYPE, VImageDimension> SegmentationType;

  typename SegmentationType::Pointer segmentationItk;
  mitk::CastToItkImage( segmentation, segmentationItk );
  
  // generate histogram

  typename SegmentationType::RegionType segmentationRegion = segmentationItk->GetLargestPossibleRegion();

  segmentationRegion.Crop( referenceImage->GetLargestPossibleRegion() );
    
  itk::ImageRegionConstIteratorWithIndex< SegmentationType > segmentationIterator( segmentationItk, segmentationRegion);
  itk::ImageRegionConstIteratorWithIndex< ImageType >        referenceIterator( referenceImage, segmentationRegion);

  segmentationIterator.GoToBegin();
  referenceIterator.GoToBegin();
    
  typedef itk::MapContainer<TPixel, long int> HistogramType;
  TPixel minimum = std::numeric_limits<TPixel>::max();
  TPixel maximum = std::numeric_limits<TPixel>::min();
  HistogramType histogram;
    
  double mean(0.0);
  double sd(0.0);
  double voxelCount(0.0);

  // first pass for mean, min, max, histogram values
  while ( !segmentationIterator.IsAtEnd() )
  {
    itk::Point< float, 3 > pt;
    segmentationItk->TransformIndexToPhysicalPoint( segmentationIterator.GetIndex(), pt );

    typename ImageType::IndexType ind;
    itk::ContinuousIndex<float, 3> contInd;
    if (referenceImage->TransformPhysicalPointToContinuousIndex(pt, contInd))
    {
      for (unsigned int i = 0; i < 3; ++i) ind[i] = ROUND_P(contInd[i]);
      
      referenceIterator.SetIndex( ind );

      if ( segmentationIterator.Get() > 0 )
      {
        ++histogram[referenceIterator.Get()];
        if (referenceIterator.Get() < minimum) minimum = referenceIterator.Get();
        if (referenceIterator.Get() > maximum) maximum = referenceIterator.Get();
      
        mean =   (mean * ( static_cast<double>(voxelCount) / static_cast<double>(voxelCount+1) ) )  // 3 points:   old center * 2/3 + currentPoint * 1/3;
           + static_cast<double>(referenceIterator.Get() ) / static_cast<double>(voxelCount+1);

        voxelCount += 1.0;
      }
    }

    ++segmentationIterator;
  }

  // second pass for SD
  segmentationIterator.GoToBegin();
  referenceIterator.GoToBegin();
  while ( !segmentationIterator.IsAtEnd() )
  {
    itk::Point< float, 3 > pt;
    segmentationItk->TransformIndexToPhysicalPoint( segmentationIterator.GetIndex(), pt );

    typename ImageType::IndexType ind;
    itk::ContinuousIndex<float, 3> contInd;
    if (referenceImage->TransformPhysicalPointToContinuousIndex(pt, contInd))
    {
      for (unsigned int i = 0; i < 3; ++i) ind[i] = ROUND_P(contInd[i]);
      
      referenceIterator.SetIndex( ind );

      if ( segmentationIterator.Get() > 0 )
      {
        sd += ((static_cast<double>(referenceIterator.Get() ) - mean)*(static_cast<double>(referenceIterator.Get() ) - mean));
      }
    }

    ++segmentationIterator;
  }

  sd /= static_cast<double>(voxelCount - 1);
  sd = sqrt( sd );

  // evaluate histogram, generate quantiles

  long int totalCount(0);

  for ( typename HistogramType::iterator iter = histogram.begin();
        iter != histogram.end();
        ++iter )
  {
    totalCount += iter->second;
  }

  TPixel histogramQuantileValues[102];

  double quantiles[102];
  for (unsigned int i = 0; i < 102; ++i) quantiles[i] = static_cast<double>(i) / 100.0; quantiles[102-1] = 2.0;

  for (unsigned int i = 0; i < 102; ++i) histogramQuantileValues[i] = 0;

  int currentQuantile(0);

  double relativeCurrentCount(0.0);

  for ( typename HistogramType::iterator iter = histogram.begin();
        iter != histogram.end();
        ++iter )
  {
    TPixel grayvalue = iter->first;
    long int count = iter->second;

    double relativeCount = static_cast<double>(count) / static_cast<double>(totalCount);

    relativeCurrentCount += relativeCount;

    while ( relativeCurrentCount >= quantiles[currentQuantile] )
    {
      histogramQuantileValues[currentQuantile] = grayvalue;
      ++currentQuantile;
    }
  }

  // report histogram values
  
  report += QString("      Minimum: %1\n  5% quantile: %2\n 25% quantile: %3\n 50% quantile: %4\n 75% quantile: %5\n 95% quantile: %6\n      Maximum: %7\n          Mean:%8\n            SD:%9\n")
               .arg(minimum)
               .arg(histogramQuantileValues[5])
               .arg(histogramQuantileValues[25])
               .arg(histogramQuantileValues[50])
               .arg(histogramQuantileValues[75])
               .arg(histogramQuantileValues[95])
               .arg(maximum)
               .arg(mean)
               .arg(sd);
}


