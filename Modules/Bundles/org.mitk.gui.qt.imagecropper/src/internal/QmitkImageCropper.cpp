/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 14134 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <iostream>

#include "QmitkImageCropper.h"
#include <QAction>

#include <QInputDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <vtkRenderWindow.h>


#include <mitkEllipsoid.h>
#include <mitkCylinder.h>
#include <mitkCone.h>
#include <mitkRenderingManager.h>
#include <mitkProperties.h>
#include <mitkGlobalInteraction.h>
#include "mitkUndoController.h"
#include "mitkBoundingObjectCutter.h"
#include "mitkImageAccessByItk.h"
#include "mitkITKImageImport.h"
#include "mitkIDataStorageService.h"
#include "mitkNodePredicateDataType.h"

#include <itkCommand.h>

//to be moved to mitkInteractionConst.h by StateMachineEditor
const mitk::OperationType QmitkImageCropper::OP_EXCHANGE = 717;

// constructors for operation classes
QmitkImageCropper::opExchangeNodes::opExchangeNodes( mitk::OperationType type, mitk::DataNode* node, mitk::BaseData* oldData, mitk::BaseData* newData )
:mitk::Operation(type),m_Node(node),m_OldData(oldData),m_NewData(newData), m_NodeDeletedObserverTag(0), m_OldDataDeletedObserverTag(0),
m_NewDataDeletedObserverTag(0)
{
  // listen to the node the image is hold
  itk::MemberCommand<opExchangeNodes>::Pointer nodeDeletedCommand = itk::MemberCommand<opExchangeNodes>::New();
  nodeDeletedCommand->SetCallbackFunction(this, &opExchangeNodes::NodeDeleted);

  m_NodeDeletedObserverTag = m_Node->AddObserver(itk::DeleteEvent(), nodeDeletedCommand);
  m_OldDataDeletedObserverTag = m_OldData->AddObserver(itk::DeleteEvent(), nodeDeletedCommand);
  m_NewDataDeletedObserverTag = m_NewData->AddObserver(itk::DeleteEvent(), nodeDeletedCommand);
}

// destructor for operation class
QmitkImageCropper::opExchangeNodes::~opExchangeNodes()
{
  if (m_Node != NULL)
  {
    m_Node->RemoveObserver(m_NodeDeletedObserverTag);
    m_Node=NULL;
  }

  if (m_OldData.IsNotNull())
  {
    m_OldData->RemoveObserver(m_OldDataDeletedObserverTag);
    m_OldData=NULL;
  }

  if (m_NewData.IsNotNull())
  {
    m_NewData->RemoveObserver(m_NewDataDeletedObserverTag);
    m_NewData=NULL;
  }
}

void QmitkImageCropper::opExchangeNodes::NodeDeleted(const itk::Object *caller, const itk::EventObject &event)
{
  m_Node = NULL;
  m_OldData = NULL;
  m_NewData = NULL;
}

QmitkImageCropper::QmitkImageCropper(QObject *parent)
: QObject(parent),
m_Controls(NULL)
{
}


QmitkImageCropper::~QmitkImageCropper()
{
  //delete smart pointer objects
  m_CroppingObjectNode = NULL;
  m_CroppingObject = NULL;
}

void QmitkImageCropper::CreateQtPartControl(QWidget* parent)
{
  if (!m_Controls)
  {
    // build ui elements
    m_Controls = new Ui::QmitkImageCropperControls;
    m_Controls->setupUi(parent);

    // setup ui elements
    m_Controls->groupInfo->hide();
    m_Controls->m_SurroundingSlider->hide();
    m_Controls->m_SurroundingSpin->hide();
    m_Controls->m_TLGrayvalue->hide();

    // create ui element connections
    this->CreateConnections();

    m_Controls->cmbImage->SetDataStorage(this->GetDefaultDataStorage());
    m_Controls->cmbImage->SetPredicate(mitk::NodePredicateDataType::New("Image"));
  }

}

void QmitkImageCropper::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->btnCrop, SIGNAL(clicked()), this, SLOT(CropImage()));   // click on the crop button
    connect( m_Controls->m_NewBoxButton, SIGNAL(clicked()), this, SLOT(ImageSelectionChanged()) );
    connect( m_Controls->cmbImage, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(ImageNodeChanged(const mitk::DataNode*)) );
    connect( m_Controls->m_EnableSurroundingCheckBox, SIGNAL(toggled(bool)), this, SLOT(SurroundingCheck(bool)) );
    connect( m_Controls->chkInformation, SIGNAL(toggled(bool)), this, SLOT(ChkInformationToggled(bool)) );
  }
}

void QmitkImageCropper::Activated()
{
  QmitkFunctionality::Activated();  // just call the inherited function
}


void QmitkImageCropper::Deactivated()
{
  RemoveBoundingObjectFromNode();

  QmitkFunctionality::Deactivated(); // just call the inherited function

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

/*! When called with an opExchangeNodes, it changes the content of a node from one data set to another
*/
void QmitkImageCropper::ExecuteOperation (mitk::Operation *operation)
{
  if (!operation) return;

  switch (operation->GetOperationType())
  {
  case OP_EXCHANGE:
    {
      //RemoveBoundingObjectFromNode();
      opExchangeNodes* op = static_cast<opExchangeNodes*>(operation);
      op->GetNode()->SetData(op->GetNewData());
      mitk::RenderingManager::GetInstance()->InitializeViews();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }
  default:;
  }

}
void QmitkImageCropper::ImageSelectionChanged()
{
  // 1. Get the selected image
  // 2. If any image is selected,
  //    attach the cuboid to it, and update the views
  if (this->IsVisible())
  {
    m_ImageNode = selectedImage();
    if (m_ImageNode.IsNotNull())
    {
      m_ImageToCrop = dynamic_cast<mitk::Image*>(m_ImageNode->GetData());
      if(m_ImageToCrop.IsNotNull())
      {
        if(m_CroppingObject.IsNull())
          CreateBoundingObject();

        AddBoundingObjectToNode( m_ImageNode );

        m_ImageNode->SetVisibility(true);
        mitk::RenderingManager::GetInstance()->InitializeViews();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        m_Controls->m_NewBoxButton->setEnabled(false);
        m_Controls->btnCrop->setEnabled(true);
      }
    }
  }
}


void QmitkImageCropper::SurroundingCheck(bool value)
{
  if(value)
  {
    m_ImageNode = selectedImage();

    if(m_ImageNode.IsNotNull())
    {
      mitk::DataNode *imageNode = m_ImageNode.GetPointer();
      if (imageNode)
      {
        mitk::BaseData* data = imageNode->GetData();
        if (data)
        {
          // test if this data item is an image or not (could also be a surface or something totally different)
          mitk::Image* image = dynamic_cast<mitk::Image*>( data );
          if (image)
          {
            float min = 10000.0;
            float max = -10000.0;

            min = image->GetScalarValueMin();
            max = image->GetScalarValueMax();

            m_Controls->m_SurroundingSlider->setRange((int)min,(int)max);
            m_Controls->m_SurroundingSpin->setRange((int)min,(int)max);
          }
        }
      }
      m_Controls->m_SurroundingSlider->show();
      m_Controls->m_SurroundingSpin->show();
      m_Controls->m_TLGrayvalue->show();
    }
    else
      m_Controls->m_EnableSurroundingCheckBox->setChecked(false);
  }
  else
  {
    m_Controls->m_SurroundingSlider->hide();
    m_Controls->m_SurroundingSpin->hide();
    m_Controls->m_TLGrayvalue->hide();
  }
}

void QmitkImageCropper::CropImage()
{
  // test, if image is selected
  if (m_ImageToCrop.IsNull()) return;

  // test, if bounding box is visible
  if (!m_CroppingObjectNode->IsVisible(NULL))
  {
    QMessageBox::information(NULL, "Image cropping functionality", "Generate a new bounding object first!");
    return;
  }

  // image and bounding object ok
  mitk::BoundingObjectCutter::Pointer cutter = mitk::BoundingObjectCutter::New();
  cutter->SetBoundingObject( m_CroppingObject );
  cutter->SetInput( m_ImageToCrop );
  cutter->AutoOutsideValueOff();

  // do the actual cutting
  try
  {
    cutter->Update();
    //cutter->UpdateLargestPossibleRegion();
  }
  catch(itk::ExceptionObject&)
  {
    QMessageBox::warning ( NULL,
      tr("Cropping not possible"),
      tr("Sorry, the bounding box has to be completely inside the image.\n\n"
      "The possibility to drag it larger than the image a bug and has to be fixed."),
      QMessageBox::Ok,  QMessageBox::NoButton,  QMessageBox::NoButton );
    return;
  }

  // cutting successful
  mitk::Image::Pointer resultImage = cutter->GetOutput();
  resultImage->DisconnectPipeline();

  if(m_Controls->m_EnableSurroundingCheckBox->isChecked())
  {
    AccessByItk_1( resultImage, AddSurrounding, resultImage);
    resultImage = m_surrImage;
  }

  RemoveBoundingObjectFromNode();

  {
    opExchangeNodes*  doOp   = new opExchangeNodes(OP_EXCHANGE, m_ImageNode.GetPointer(),
      m_ImageNode->GetData(),
      resultImage);
    opExchangeNodes* undoOp  = new opExchangeNodes(OP_EXCHANGE, m_ImageNode.GetPointer(),
      resultImage,
      m_ImageNode->GetData());

    // TODO: MITK doesn't recognize that a new event happens in the next line,
    //       because nothing happens in the render window.
    //       As a result the undo action will happen together with the last action
    //       recognized by MITK.
    mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent(
      new mitk::OperationEvent(this, doOp, undoOp, "Crop image") ); // tell the undo controller about the action
    ExecuteOperation(doOp); // execute action
  }

  m_Controls->m_NewBoxButton->setEnabled(true);
  m_Controls->btnCrop->setEnabled(false);

}

template < typename TPixel, unsigned int VImageDimension >
void QmitkImageCropper::AddSurrounding( itk::Image< TPixel, VImageDimension >* itkImage, mitk::Image::Pointer image)
{
  typedef itk::Image< TPixel, VImageDimension > InputImageType;

  typename InputImageType::Pointer extended = InputImageType::New();
  typename InputImageType::IndexType start;
  start[0]=0;
  start[1]=0;
  start[2]=0;

  unsigned int *dims = image->GetDimensions();
  typename InputImageType::SizeType size;

  size[0]=dims[0];
  size[1]=dims[1];
  size[2]=dims[2];

  typename InputImageType::RegionType region;
  region.SetSize(size);
  region.SetIndex(start);

  extended->SetRegions(region);
  extended->SetDirection(itkImage->GetDirection());
  extended->SetOrigin(itkImage->GetOrigin());
  extended->Allocate();

  extended->SetSpacing(itkImage->GetSpacing());

  typename InputImageType::IndexType idx;

  progress = new QProgressDialog( "Adding surrounding...", "Abort", 0, (size[0]-1), m_Parent);
  progress->setLabelText("Image cropper");
  progress->show();

  for (unsigned int i=0;i<size[0];i++)
  {
    for (unsigned int j=0;j<size[1];j++)
    {
      for (unsigned int k=0;k<size[2];k++)
      {
        idx[0]=i;
        idx[1]=j;
        idx[2]=k;

        if(i==0 || j==0 || k==0 || i==size[0]-1 || j==size[1]-1 || k==size[2]-1)
        {
          extended->SetPixel(idx, m_Controls->m_SurroundingSpin->value());
        }
        else
        {
          extended->SetPixel(idx, itkImage->GetPixel(idx));
        }
      }
    }
    progress->setValue(i);
    if ( progress->wasCanceled() )
      break;
  }
  m_surrImage = mitk::Image::New();
  m_surrImage = mitk::ImportItkImage(extended);

}

void QmitkImageCropper::CreateBoundingObject()
{
  QStringList items;
  items << tr("Cuboid") << tr("Ellipsoid") << tr("Cylinder") << tr("Cone");

  bool ok;
  QString item = QInputDialog::getItem(m_Parent, tr("Select Bounding Object"),
    tr("Type of Bounding Object:")
    , items, 0, false, &ok);

  if (item == "Ellipsoid")
    m_CroppingObject = mitk::Ellipsoid::New();
  else if(item == "Cylinder")
    m_CroppingObject = mitk::Cylinder::New();
  else if (item == "Cone")
    m_CroppingObject = mitk::Cone::New();
  else
    m_CroppingObject = mitk::Cuboid::New();

  m_CroppingObjectNode = mitk::DataNode::New();
  m_CroppingObjectNode->SetData( m_CroppingObject );
  m_CroppingObjectNode->SetProperty( "name", mitk::StringProperty::New( "CroppingObject" ) );
  m_CroppingObjectNode->SetProperty( "color", mitk::ColorProperty::New(1.0, 1.0, 0.0) );
  m_CroppingObjectNode->SetProperty( "opacity", mitk::FloatProperty::New(0.4) );
  m_CroppingObjectNode->SetProperty( "layer", mitk::IntProperty::New(99) ); // arbitrary, copied from segmentation functionality
  m_CroppingObjectNode->SetProperty( "selected",  mitk::BoolProperty::New(true) );

  m_AffineInteractor = mitk::AffineInteractor::New("AffineInteractions ctrl-drag", m_CroppingObjectNode);
}


void QmitkImageCropper::AddBoundingObjectToNode(mitk::DataNode* node)
{
  m_ImageToCrop = dynamic_cast<mitk::Image*>(node->GetData());

  if(!this->GetDefaultDataStorage()->Exists(m_CroppingObjectNode))
  {
    this->GetDefaultDataStorage()->Add(m_CroppingObjectNode, node);
    m_CroppingObject->FitGeometry(m_ImageToCrop->GetTimeSlicedGeometry());

    mitk::GlobalInteraction::GetInstance()->AddInteractor( m_AffineInteractor );
  }
  m_CroppingObjectNode->SetVisibility(true);
}

void QmitkImageCropper::RemoveBoundingObjectFromNode()
{
  if (m_CroppingObjectNode.IsNotNull())
  {
    if(this->GetDefaultDataStorage()->Exists(m_CroppingObjectNode))
    {
      this->GetDefaultDataStorage()->Remove(m_CroppingObjectNode);
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_AffineInteractor);
    }
    m_CroppingObjectNode = NULL;
    m_CroppingObject = NULL;
    m_Controls->btnCrop->setEnabled(false);
    m_Controls->m_NewBoxButton->setEnabled(true);
  }
}

void QmitkImageCropper::ImageNodeChanged( const mitk::DataNode* item )
{
  if(m_Controls != 0)
  {
    // called when the selection of the image selector changes
    RemoveBoundingObjectFromNode();
    ImageSelectionChanged();
  }
}


const mitk::DataNode::Pointer QmitkImageCropper::selectedImage()
{
  return m_Controls->cmbImage->GetSelectedNode();
}


void QmitkImageCropper::ChkInformationToggled( bool on )
{
  if (on)
    m_Controls->groupInfo->show();
  else
    m_Controls->groupInfo->hide();
}

void QmitkImageCropper::StdMultiWidgetAvailable( QmitkStdMultiWidget&  stdMultiWidget )
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkImageCropper::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}
