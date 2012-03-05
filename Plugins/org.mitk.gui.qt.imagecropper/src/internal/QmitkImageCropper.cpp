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

void QmitkImageCropper::opExchangeNodes::NodeDeleted(const itk::Object * /*caller*/, const itk::EventObject &/*event*/)
{
  m_Node = NULL;
  m_OldData = NULL;
  m_NewData = NULL;
}

QmitkImageCropper::QmitkImageCropper(QObject *parent)
: m_Controls(NULL), m_ParentWidget(0)
{
   m_Interface = new mitk::ImageCropperEventInterface;
   m_Interface->SetImageCropper( this );
}


QmitkImageCropper::~QmitkImageCropper()
{
  //delete smart pointer objects
  m_CroppingObjectNode = NULL;
  m_CroppingObject = NULL;

  m_Interface->Delete();

}

void QmitkImageCropper::CreateQtPartControl(QWidget* parent)
{
  if (!m_Controls)
  {
    m_ParentWidget = parent;

    // build ui elements
    m_Controls = new Ui::QmitkImageCropperControls;
    m_Controls->setupUi(parent);

    // setup ui elements
    m_Controls->groupInfo->hide();
    m_Controls->m_SurroundingSlider->hide();
    m_Controls->m_SurroundingSpin->hide();
    m_Controls->m_NewBoxButton->setEnabled(true);

    // create ui element connections
    this->CreateConnections();
  }

}

void QmitkImageCropper::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->btnCrop, SIGNAL(clicked()), this, SLOT(CropImage()));   // click on the crop button
    connect( m_Controls->m_NewBoxButton, SIGNAL(clicked()), this, SLOT(CreateNewBoundingObject()) );
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

void QmitkImageCropper::CreateNewBoundingObject()
{
  // attach the cuboid to the image and update the views
  if (this->IsVisible())
  {
    if (m_ImageNode.IsNotNull())
    {
      m_ImageToCrop = dynamic_cast<mitk::Image*>(m_ImageNode->GetData());
      if(m_ImageToCrop.IsNotNull())
      {
        if (this->GetDefaultDataStorage()->GetNamedDerivedNode("CroppingObject", m_ImageNode))
        {
          // We are in "Reset bounding box!" mode
          m_CroppingObject->FitGeometry(m_ImageToCrop->GetTimeSlicedGeometry());
          mitk::RenderingManager::GetInstance()->RequestUpdateAll();
          return;
        }

        bool fitCroppingObject = false;
        if(m_CroppingObject.IsNull())
        {
          CreateBoundingObject();
          fitCroppingObject = true;
        }

        if (m_CroppingObject.IsNull())
          return;

        AddBoundingObjectToNode( m_ImageNode, fitCroppingObject );

        m_ImageNode->SetVisibility(true);
        mitk::RenderingManager::GetInstance()->InitializeViews();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        m_Controls->m_NewBoxButton->setText("Reset bounding box!");
        m_Controls->btnCrop->setEnabled(true);
      }
    }
    else 
      QMessageBox::information(NULL, "Image cropping functionality", "Load an image first!");
  }
}


void QmitkImageCropper::SurroundingCheck(bool value)
{
  if(value)
  {
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
    }
    else
      m_Controls->m_EnableSurroundingCheckBox->setChecked(false);
  }
  else
  {
    m_Controls->m_SurroundingSlider->hide();
    m_Controls->m_SurroundingSpin->hide();
  }
}

void QmitkImageCropper::CropImage()
{
  // test, if image is selected
  if (m_ImageToCrop.IsNull()) return;

  // test, if bounding box is visible
  if (m_CroppingObjectNode.IsNull())
  {
    QMessageBox::information(NULL, "Image cropping functionality", "Generate a new bounding object first!");
    return;
  }

  // image and bounding object ok
  mitk::BoundingObjectCutter::Pointer cutter = mitk::BoundingObjectCutter::New();
  cutter->SetBoundingObject( m_CroppingObject );
  cutter->SetInput( m_ImageToCrop );
  cutter->AutoOutsideValueOff();

  if (m_Controls->m_EnableSurroundingCheckBox->isChecked())
  {
    cutter->SetOutsideValue(m_Controls->m_SurroundingSpin->value());
  }

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
      "The possibility to drag it larger than the image is a bug and has to be fixed."),
      QMessageBox::Ok,  QMessageBox::NoButton,  QMessageBox::NoButton );
    return;
  }

  // cutting successful
  mitk::Image::Pointer resultImage = cutter->GetOutput();
  resultImage->DisconnectPipeline();

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
    mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doOp, undoOp, "Crop image");
    mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent ); // tell the undo controller about the action
    
    ExecuteOperation(doOp); // execute action
  }

  m_Controls->m_NewBoxButton->setEnabled(true);
  m_Controls->btnCrop->setEnabled(false);
}

void QmitkImageCropper::CreateBoundingObject()
{
  QStringList items;
  items << tr("Cuboid") << tr("Ellipsoid") << tr("Cylinder") << tr("Cone");

  bool ok;
  QString item = QInputDialog::getItem(m_Parent, tr("Select Bounding Object"), tr("Type of Bounding Object:"), items, 0, false, &ok);

  if (!ok)
    return;

  if (item == "Ellipsoid")
    m_CroppingObject = mitk::Ellipsoid::New();
  else if(item == "Cylinder")
    m_CroppingObject = mitk::Cylinder::New();
  else if (item == "Cone")
    m_CroppingObject = mitk::Cone::New();
  else if (item == "Cuboid")
    m_CroppingObject = mitk::Cuboid::New();
  else
    return;

  m_CroppingObjectNode = mitk::DataNode::New();
  m_CroppingObjectNode->SetData( m_CroppingObject );
  m_CroppingObjectNode->SetProperty( "name", mitk::StringProperty::New( "CroppingObject" ) );
  m_CroppingObjectNode->SetProperty( "color", mitk::ColorProperty::New(1.0, 1.0, 0.0) );
  m_CroppingObjectNode->SetProperty( "opacity", mitk::FloatProperty::New(0.4) );
  m_CroppingObjectNode->SetProperty( "layer", mitk::IntProperty::New(99) ); // arbitrary, copied from segmentation functionality
  m_CroppingObjectNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );

  m_AffineInteractor = mitk::AffineInteractor::New("AffineInteractions ctrl-drag", m_CroppingObjectNode);
}

void QmitkImageCropper::OnSelectionChanged(std::vector<mitk::DataNode*> nodes)
{
  this->RemoveBoundingObjectFromNode();

  if (nodes.size() != 1 || dynamic_cast<mitk::Image*>(nodes[0]->GetData()) == 0)
  {
    m_ParentWidget->setEnabled(false);
    return;
  }

  m_ImageNode = nodes[0];
  m_ParentWidget->setEnabled(true);
}

void QmitkImageCropper::AddBoundingObjectToNode(mitk::DataNode* node, bool fit)
{
  m_ImageToCrop = dynamic_cast<mitk::Image*>(node->GetData());

  if(!this->GetDefaultDataStorage()->Exists(m_CroppingObjectNode))
  {
    this->GetDefaultDataStorage()->Add(m_CroppingObjectNode, node);
    if (fit)
    {
      m_CroppingObject->FitGeometry(m_ImageToCrop->GetTimeSlicedGeometry());
    }

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
    m_Controls->m_NewBoxButton->setText("New bounding box!");
  }
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

void QmitkImageCropper::NodeRemoved(const mitk::DataNode *node)
{
  std::string name = node->GetName();

  if (strcmp(name.c_str(), "CroppingObject")==0)
  {
    m_Controls->btnCrop->setEnabled(false);
    m_Controls->m_NewBoxButton->setEnabled(true);
  }
}
