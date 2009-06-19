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
//!#include "QmitkImageCropperControls.h"
#include <QAction>

#include <QMessageBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <vtkRenderWindow.h>


#include <mitkRenderingManager.h>
#include <mitkCuboid.h>
#include <mitkProperties.h>
#include <mitkGlobalInteraction.h>
#include "mitkUndoController.h"
#include "mitkBoundingObjectCutter.h"
#include "mitkDataTreeHelper.h"
#include "mitkImageAccessByItk.h"
#include "mitkITKImageImport.h"
#include "mitkIDataStorageService.h"
#include "mitkNodePredicateDataType.h"

//to be moved to mitkInteractionConst.h by StateMachineEditor
const mitk::OperationType QmitkImageCropper::OP_EXCHANGE = 717;

// constructors for operation classes
QmitkImageCropper::opExchangeNodes::opExchangeNodes( mitk::OperationType type, mitk::DataTreeNode* node, mitk::BaseData* oldData, mitk::BaseData* newData )
:mitk::Operation(type),m_Node(node),m_OldData(oldData),m_NewData(newData)
{
}

//!QmitkImageCropper::QmitkImageCropper(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
//!: QmitkFunctionality(parent, name, it),
//!m_MultiWidget(mitkStdMultiWidget),
QmitkImageCropper::QmitkImageCropper(QObject *parent)
: QObject(parent),
m_Controls(NULL)
{
  this->CreateBoundingObject(); // Create cuboid. This should only happen once.

  m_AffineInteractor = mitk::AffineInteractor::New("AffineInteractions ctrl-drag", m_CroppingObjectNode);
}


QmitkImageCropper::~QmitkImageCropper()
{
  // delete smart pointer objects
  m_CroppingObjectNode = NULL;
  m_CroppingObject = NULL;
}

/*!
QWidget * QmitkImageCropper::CreateMainWidget(QWidget *)
{
  // We use the standard multi-widget. If there isn't one, we probably crash :-(
  return NULL;
}
*/

/*!
QWidget * QmitkImageCropper::CreateControlWidget(QWidget *parent)
{
  if (!m_Controls)
  {
    m_Controls = new Ui_QmitkImageCropperControls(parent);
    if (!m_Controls) return NULL;

    // tell the node selector (inside the controls), which data tree to use
    m_Controls->cmbImage->SetDataTree(m_DataTreeIterator.GetPointer());
  }
  return m_Controls;
}
*/


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

/*!QWidget* QmitkImageCropper::GetControls()
{
  return m_Controls;
}*/

void QmitkImageCropper::CreateConnections()
{
  if ( m_Controls )
  {
    //!connect( (QObject*) m_Controls, SIGNAL(cropImage()), this, SLOT(CropImage()) );   // click on the crop button
    connect( m_Controls->btnCrop, SIGNAL(clicked()), this, SLOT(CropImage()));   // click on the crop button
    connect( m_Controls->m_NewBoxButton, SIGNAL(clicked()), this, SLOT(ImageSelectionChanged()) );
    connect( m_Controls->cmbImage, SIGNAL(OnSelectionChanged(const mitk::DataTreeNode*)), this, SLOT(ImageNodeChanged(const mitk::DataTreeNode*)) );
    connect( m_Controls->m_EnableSurroundingCheckBox, SIGNAL(toggled(bool)), this, SLOT(SurroundingCheck(bool)) );
    connect( m_Controls->chkInformation, SIGNAL(toggled(bool)), this, SLOT(ChkInformationToggled(bool)) );
    //! from QmitkImageCropperControls::init()
  }
}


/*!
QAction * QmitkImageCropper::CreateAction(QActionGroup *parent)
{
  QAction* action = new QAction(
    tr( "Image Cropper" ),
    QPixmap((const char**)icon_xpm),
    tr( "Image Cropper" ),
    0,
    parent,
    "BoundingObjectImageCropper" );
  return action;
}
*/


void QmitkImageCropper::Activated()
{
  if (m_CroppingObjectNode)
    ImageSelectionChanged();

  QmitkFunctionality::Activated();  // just call the inherited function
}


void QmitkImageCropper::Deactivated()
{
  RemoveBoundingObjectFromNode();

  QmitkFunctionality::Deactivated(); // just call the inherited function

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


//!void QmitkImageCropper::TreeChanged()
void QmitkImageCropper::DataStorageChanged()
{
  // Update the image selection widget
  // m_Controls->cmbImage->Update();
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
      RemoveBoundingObjectFromNode();
      opExchangeNodes* op = static_cast<opExchangeNodes*>(operation);
      op->GetNode()->SetData(op->GetNewData());
      //!mitk::RenderingManager::GetInstance()->InitializeViews(m_DataTreeIterator.GetPointer());
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
  m_ImageNode = selectedImage();
  if (m_ImageNode.IsNotNull())
  {
    m_ImageToCrop = dynamic_cast<mitk::Image*>(m_ImageNode->GetData());
    if(m_ImageToCrop.IsNotNull())
    {
      AddBoundingObjectToNode( m_ImageNode );

      m_ImageNode->SetVisibility(true);
      //!mitk::RenderingManager::GetInstance()->InitializeViews(m_DataTreeIterator.GetPointer());
      mitk::RenderingManager::GetInstance()->InitializeViews();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      m_Controls->m_NewBoxButton->setEnabled(false);
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
      mitk::DataTreeNode *imageNode = m_ImageNode.GetPointer();
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
    cutter->UpdateLargestPossibleRegion();
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

  //Centering
  unsigned int *dims = resultImage->GetDimensions();
  mitk::Vector3D sp = resultImage->GetGeometry()->GetSpacing();
  mitk::Point3D origin = resultImage->GetGeometry()->GetOrigin();
  mitk::Point3D center = m_ImageToCrop->GetGeometry()->GetCenter();

  mitk::FillVector3D(origin,center[0]-((dims[0]/2)*sp[0]),center[1]-((dims[1]/2)*sp[1]),center[2]-((dims[2]/2)*sp[2]));

  resultImage->GetGeometry()->SetOrigin(origin);

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

  /*    CreateBoundingObject();
  m_ImageNode = selectedImage();
  AddBoundingObjectToNode(m_ImageNode);
  */
  m_Controls->m_NewBoxButton->setEnabled(true);

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
  /*
  size[0]=dims[0]+2;
  size[1]=dims[1]+2;
  size[2]=dims[2]+2;
  */
  size[0]=dims[0];
  size[1]=dims[1];
  size[2]=dims[2];

  typename InputImageType::RegionType region;
  region.SetSize(size);
  region.SetIndex(start);

  extended->SetRegions(region);
  extended->Allocate();

  extended->SetSpacing(itkImage->GetSpacing());

  typename InputImageType::IndexType idx;
  //typename InputImageType::IndexType idx_old;

  //!progress = new QProgressDialog( "Adding surrounding...", "Abort", (size[0]-1), NULL, "progress", true);
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
    //!progress->setProgress(i);
    progress->setValue(i);
    if ( progress->wasCanceled() )
      break;
  }
  m_surrImage = mitk::Image::New();
  m_surrImage = mitk::ImportItkImage(extended);

}

void QmitkImageCropper::CreateBoundingObject()
{
  m_CroppingObject = mitk::Cuboid::New();

  m_CroppingObjectNode = mitk::DataTreeNode::New();
  m_CroppingObjectNode->SetData( m_CroppingObject );
  m_CroppingObjectNode->SetProperty( "name", mitk::StringProperty::New( "CroppingObject" ) );
  m_CroppingObjectNode->SetProperty( "color", mitk::ColorProperty::New(1.0, 1.0, 0.0) );
  m_CroppingObjectNode->SetProperty( "opacity", mitk::FloatProperty::New(0.4) );
  m_CroppingObjectNode->SetProperty( "layer", mitk::IntProperty::New(99) ); // arbitrary, copied from segmentation functionality
  m_CroppingObjectNode->SetProperty( "selected",  mitk::BoolProperty::New(true) );
}


void QmitkImageCropper::AddBoundingObjectToNode(mitk::DataTreeNode* node)
{
  m_ImageToCrop = dynamic_cast<mitk::Image*>(node->GetData());

  //mitk::DataTreeIteratorClone iteratorToCroppingObject = mitk::DataTreeHelper::FindIteratorToNode(iterToNode.GetPointer(),  m_CroppingObjectNode);
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
    m_CroppingObjectNode->SetVisibility(false);
  }
}
void QmitkImageCropper::ImageNodeChanged( const mitk::DataTreeNode* item )
{
  if(m_Controls != 0)
  {
    // called when the selection of the image selector changes
    m_Controls->btnCrop->setEnabled( item != 0 );
    ImageSelectionChanged();
  }
}


const mitk::DataTreeNode::Pointer QmitkImageCropper::selectedImage()
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

void QmitkImageCropper::StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget )
{
  m_Parent->setEnabled(true);
}

void QmitkImageCropper::StdMultiWidgetNotAvailable()
{
  m_Parent->setEnabled(false);
}
