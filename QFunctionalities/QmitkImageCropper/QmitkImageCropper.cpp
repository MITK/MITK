/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include <iostream>
#include "icon.xpm"

#include "QmitkImageCropper.h"
#include "QmitkImageCropperControls.h"

#include <qaction.h>
#include <qmessagebox.h>

#include <mitkRenderWindow.h>
#include <mitkRenderingManager.h>
#include <QmitkStdMultiWidget.h>
#include <QmitkTreeNodeSelector.h>

#include <mitkCuboid.h>
#include <mitkProperties.h>
#include <mitkGlobalInteraction.h>
#include <mitkBoundingObjectCutter.h>
#include <mitkDataTreeHelper.h>

const mitk::OperationType QmitkImageCropper::OP_EXCHANGE = 717;

// constructors for operation classes
QmitkImageCropper::opExchangeNodes::opExchangeNodes( mitk::OperationType type, mitk::DataTreeNode* node, 
                                                                                             mitk::BaseData* oldData, 
                                                                                             mitk::BaseData* newData )
:mitk::Operation(type),
 m_Node(node),
 m_OldData(oldData),
 m_NewData(newData)
{
}

QmitkImageCropper::QmitkImageCropper(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it) 
: QmitkFunctionality(parent, name, it),
  m_MultiWidget(mitkStdMultiWidget),
  m_Controls(NULL),
  m_AffineInteractor(NULL)
{
  CreateBoundingObject(); // Create cuboid. This should only happen once.
}


QmitkImageCropper::~QmitkImageCropper()
{
  // delete smart pointer objects
  m_CroppingObjectNode = NULL;
  m_CroppingObject = NULL;
}


QWidget * QmitkImageCropper::CreateMainWidget(QWidget *parent)
{
  // We use the standard multi-widget. If there isn't one, we probably crash :-(
  return NULL;
}


QWidget * QmitkImageCropper::CreateControlWidget(QWidget *parent)
{
  if (!m_Controls)
  {
    m_Controls = new QmitkImageCropperControls(parent);
    if (!m_Controls) return NULL;

    // tell the node selector (inside the controls), which data tree to use
    m_Controls->qmitkNodeSelector->SetDataTreeNodeIterator(m_DataTreeIterator);
    // install filter function, that accepts only images
    m_Controls->qmitkNodeSelector->m_FilterFunction = &QmitkImageCropper::DataTreeNodeFilter;
  }
  return m_Controls;
}


void QmitkImageCropper::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*) m_Controls, SIGNAL(cropImage()), this, SLOT(CropImage()) );   // click on the crop button
    connect( (QObject*) m_Controls, SIGNAL(ImageSelectionChanged()), this, SLOT(ImageSelectionChanged()) );  // selection of another image (or no image)
  }
}


QAction * QmitkImageCropper::CreateAction(QActionGroup *parent)
{
  QAction* action = new QAction( 
                                 tr( "Image Cropper" ), 
                                 QPixmap((const char**)icon_xpm), 
                                 tr( "Image Cropper" ), 
                                 Qt::CTRL + Qt::Key_R, 
                                 parent, 
                                 "BoundingObjectImageCropper" );
  return action;
}


void QmitkImageCropper::Activated()
{
  QmitkFunctionality::Activated();  // just call the inherited function
}


void QmitkImageCropper::Deactivated()
{
  RemoveBoundingObjectFromNode();

  QmitkFunctionality::Deactivated(); // just call the inherited function
}


void QmitkImageCropper::TreeChanged()
{
  // Update the image selection widget
  m_Controls->qmitkNodeSelector->SetDataTreeNodeIterator( m_DataTreeIterator );
}


bool QmitkImageCropper::DataTreeNodeFilter( mitk::DataTreeNode* node )
{
  return (node && node->GetData() && dynamic_cast<mitk::Image*>( node->GetData() )); // accept only images
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
        m_MultiWidget->InitializeStandardViews(m_DataTreeIterator.GetPointer());
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
  m_IteratorToImageToCrop = m_Controls->selectedImage();
  m_ImageToCrop = dynamic_cast<mitk::Image*>(m_IteratorToImageToCrop->Get()->GetData());
  if (m_IteratorToImageToCrop->Get().IsNotNull())
  {
    AddBoundingObjectToNode( m_IteratorToImageToCrop );
  
    m_IteratorToImageToCrop->Get()->SetVisibility(true);
    m_MultiWidget->InitializeStandardViews(m_DataTreeIterator.GetPointer());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void QmitkImageCropper::CropImage()
{
  // test, if image is selected
  if (m_ImageToCrop.IsNull()) return;

  // test, if bounding box is visible
  if (!m_CroppingObjectNode->IsVisible(NULL)) return;

  // image and bounding object ok
  mitk::BoundingObjectCutter::Pointer cutter = mitk::BoundingObjectCutter::New();
  cutter->SetBoundingObject( m_CroppingObject );
  cutter->SetInput( m_ImageToCrop );

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
  
  RemoveBoundingObjectFromNode();

  {
  opExchangeNodes*  doOp   = new opExchangeNodes(OP_EXCHANGE, m_IteratorToImageToCrop->Get().GetPointer(), 
                                                              m_IteratorToImageToCrop->Get()->GetData(), 
                                                              resultImage); 
  opExchangeNodes* undoOp  = new opExchangeNodes(OP_EXCHANGE, m_IteratorToImageToCrop->Get().GetPointer(), 
                                                              resultImage, 
                                                              m_IteratorToImageToCrop->Get()->GetData());

  // TODO: MITK doesn't recognize that a new event happens in the next line,
  //       because nothing happens in the render window.
  //       As a result the undo action will happen together with the last action
  //       recognized by MITK.
  mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( 
      new mitk::OperationEvent(this, doOp, undoOp, "Crop image") ); // tell the undo controller about the action
  ExecuteOperation(doOp); // execute action
  }

}


void QmitkImageCropper::CreateBoundingObject()
{
  m_CroppingObject = mitk::Cuboid::New();

  m_CroppingObjectNode = mitk::DataTreeNode::New(); 
  mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties( m_CroppingObjectNode );
  m_CroppingObjectNode->SetData( m_CroppingObject );
  m_CroppingObjectNode->SetProperty( "name", new mitk::StringProperty( "CroppingObject" ) );
  m_CroppingObjectNode->SetProperty( "color", new mitk::ColorProperty(1.0, 1.0, 0.0) );
  m_CroppingObjectNode->SetProperty( "opacity", new mitk::FloatProperty(0.4) );
  m_CroppingObjectNode->SetProperty( "layer", new mitk::IntProperty(99) ); // arbitrary, copied from segmentation functionality
  m_CroppingObjectNode->SetProperty( "selected",  new mitk::BoolProperty(true) );  
}


void QmitkImageCropper::AddBoundingObjectToNode(mitk::DataTreeIteratorClone& iterToNode)
{
  m_ImageToCrop = dynamic_cast<mitk::Image*>(iterToNode->Get()->GetData());
  m_CroppingObjectNode->SetVisibility(true);
    
  mitk::DataTreeIteratorClone iteratorToCroppingObject = mitk::DataTreeHelper::FindIteratorToNode(iterToNode.GetPointer(),  m_CroppingObjectNode);
  if (iteratorToCroppingObject->IsAtEnd())
  {
    iterToNode->Add(m_CroppingObjectNode);
    m_CroppingObject->FitGeometry(m_ImageToCrop->GetTimeSlicedGeometry());
  
    if (!m_AffineInteractor)
    {
      m_AffineInteractor = new mitk::AffineInteractor("AffineInteractions ctrl-drag", m_CroppingObjectNode);
      mitk::GlobalInteraction::GetInstance()->AddInteractor( m_AffineInteractor );
    }
  }
        
}


void QmitkImageCropper::RemoveBoundingObjectFromNode()
{
  if (m_CroppingObjectNode.IsNotNull())
  {
    mitk::DataTreeIteratorClone iteratorToCroppingObject = mitk::DataTreeHelper::FindIteratorToNode(m_DataTreeIterator.GetPointer(),  m_CroppingObjectNode);
    if (iteratorToCroppingObject.IsNotNull())
      iteratorToCroppingObject->Remove();
    m_CroppingObjectNode->SetVisibility(false);
  }
}


