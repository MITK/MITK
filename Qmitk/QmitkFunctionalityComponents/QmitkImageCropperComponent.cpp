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
#include "QmitkImageCropperComponent.h"
#include "QmitkImageCropperComponentGUI.h"


#include <QmitkDataTreeComboBox.h>

#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"
#include <QmitkStdMultiWidget.h>

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>


#include <iostream>

#include <qaction.h>
#include <qmessagebox.h>


#include <mitkCuboid.h>
#include <mitkCone.h>
#include <mitkEllipsoid.h>
#include <mitkCylinder.h>
#include <mitkGlobalInteraction.h>
//#include <mitkUndoController.h>
#include <mitkBoundingObjectCutter.h>
#include <mitkDataTreeHelper.h>

const mitk::OperationType QmitkImageCropperComponent::OP_EXCHANGE = 717;

/*************** constructors for operation classes  ***************/
QmitkImageCropperComponent::opExchangeNodes::opExchangeNodes( mitk::OperationType type, mitk::DataTreeNode* node, 
															 mitk::BaseData* oldData, 
															 mitk::BaseData* newData )
															 :mitk::Operation(type),
															 m_Node(node),
															 m_OldData(oldData),
															 m_NewData(newData)
{
}

/***************       CONSTRUCTOR      ***************/
QmitkImageCropperComponent::QmitkImageCropperComponent(QObject * parent, const char * parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget * mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, QmitkFunctionalityComponentContainer * parentObject)
: QmitkFunctionalityComponentContainer(parent, parentName, updateSelector, showSelector),
m_ImageCropperImageNode(NULL),
m_MultiWidget(mitkStdMultiWidget),
m_BoundingObjectExists(false),
m_ParentObject(parentObject),
m_ImageCropperComponentGUI(NULL),
m_ImageCropperNodeExisting(false)
{
	SetDataTreeIterator(it);
	SetAvailability(true);

	SetComponentName("ImageCropper");
	m_Node = it->Get();
}

/***************        DESTRUCTOR      ***************/
QmitkImageCropperComponent::~QmitkImageCropperComponent()
{

}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkImageCropperComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
	m_DataTreeIterator = it;
	m_Node = m_DataTreeIterator->Get();
}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkImageCropperComponent::SetSelectorVisibility(bool visibility)
{
	if(m_ImageCropperComponentGUI)
	{
		m_ImageCropperComponentGUI->GetSelectDataGroupBox()->setShown(visibility);
	}
}

/***************   GET IMAGE CONTENT   ***************/
QGroupBox* QmitkImageCropperComponent::GetImageContent()
{
	return (QGroupBox*) m_ImageCropperComponentGUI->GetImageContent();
}


/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkImageCropperComponent::GetTreeNodeSelector()
{
	return m_ImageCropperComponentGUI->GetTreeNodeSelector();
}

/***************       CONNECTIONS      ***************/
void QmitkImageCropperComponent::CreateConnections()
{
	if ( m_ImageCropperComponentGUI )
	{
		connect( (QObject*)(m_ImageCropperComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
		connect( (QObject*)(m_ImageCropperComponentGUI->GetImageCropperFinderGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageCropperContent(bool)));     
		connect( (QObject*)(m_ImageCropperComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool)));  
		connect( (QObject*)(m_ImageCropperComponentGUI->GetImageCropperFinderGroupBox()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 

		// click on the crop button
		connect( (QObject*)(m_ImageCropperComponentGUI->GetCropImageButton()), SIGNAL(pressed()), (QObject*) this, SLOT(CropImage())); 

		connect( (QObject*)(m_ImageCropperComponentGUI->GetCubeButton()), SIGNAL(pressed()), (QObject*) this, SLOT(CubePressed()));
		connect( (QObject*)(m_ImageCropperComponentGUI->GetEllipsoidButton()), SIGNAL(pressed()), (QObject*) this, SLOT(EllipsoidPressed()));
		connect( (QObject*)(m_ImageCropperComponentGUI->GetConeButton()), SIGNAL(pressed()), (QObject*) this, SLOT(ConePressed()));
		connect( (QObject*)(m_ImageCropperComponentGUI->GetCylinderButton()), SIGNAL(pressed()), (QObject*) this, SLOT(CylinderPressed()));
	}
}

void QmitkImageCropperComponent::CubePressed()
{
	CreateBoundingObject(0);
}
void QmitkImageCropperComponent::EllipsoidPressed()
{
	CreateBoundingObject(1);
}
void QmitkImageCropperComponent::ConePressed()
{
	CreateBoundingObject(2);
}
void QmitkImageCropperComponent::CylinderPressed()
{
	CreateBoundingObject(3);
}



/***************     IMAGE SELECTED     ***************/
void QmitkImageCropperComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
	m_SelectedItem = imageIt;
	mitk::DataTreeFilter::Item* currentItem(NULL);
	if(m_ImageCropperComponentGUI)
	{
		if(mitk::DataTreeFilter* filter = m_ImageCropperComponentGUI->GetTreeNodeSelector()->GetFilter())
		{
			if(imageIt)
			{
				currentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( imageIt->GetNode() ) );
			}
		}
	}
	if(currentItem)
	{
		currentItem->SetSelected(true);
	}
	if(m_ImageCropperComponentGUI != NULL)
	{
		for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
		{
			QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->ImageSelected(m_SelectedItem);
		}
	}
	//m_Node = const_cast<mitk::DataTreeNode*>(m_SelectedItem->GetNode());

	mitk::DataTreeFilter* filter = m_ImageCropperComponentGUI->GetTreeNodeSelector()->GetFilter();
	m_MitkImageIterator = filter->GetIteratorToSelectedItem();

	if(m_MitkImageIterator.GetPointer())
	{
		m_MitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());

		//AddBoundingObjectToNode( m_MitkImageIterator );
		m_MitkImageIterator->Get()->SetVisibility(true);
		m_MultiWidget->InitializeStandardViews(m_DataTreeIterator.GetPointer());
		//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}

}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkImageCropperComponent::CreateControlWidget(QWidget* parent)
{
	m_ImageCropperComponentGUI = new QmitkImageCropperComponentGUI(parent);
	m_GUI = m_ImageCropperComponentGUI;

	m_ImageCropperComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());

	if(m_ShowSelector)
	{
		m_ImageCropperComponentGUI->GetImageContent()->setShown(m_ImageCropperComponentGUI->GetSelectDataGroupBox()->isChecked());
	}
	else
	{
		m_ImageCropperComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
	}

	m_ImageCropperComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));

	return m_ImageCropperComponentGUI;
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkImageCropperComponent::GetContentContainer()
{
	return m_ImageCropperComponentGUI->GetContainerContent();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkImageCropperComponent::GetMainCheckBoxContainer()
{
	return m_ImageCropperComponentGUI->GetImageCropperFinderGroupBox();
}

/***************        ACTIVATED       ***************/
void QmitkImageCropperComponent::Activated()
{
	QmitkBaseFunctionalityComponent::Activated();
	m_Active = true;
	//  CreateBoundingObject();
	for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
	{
		m_AddedChildList[i]->Activated();
	} 
}

/***************       DEACTIVATED      ***************/
void QmitkImageCropperComponent::Deactivated()
{
	QmitkBaseFunctionalityComponent::Deactivated();
	m_Active = false;
	for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
	{
		m_AddedChildList[i]->Deactivated();
	} 
	RemoveBoundingObjectFromNode();
}


///************ SHOW IMAGE CROPPER CONTENT ***********/
void QmitkImageCropperComponent::ShowImageCropperContent(bool)
{
	m_ImageCropperComponentGUI->GetContainerContent()->setShown(m_ImageCropperComponentGUI->GetImageCropperFinderGroupBox()->isChecked());


	if(m_ShowSelector)
	{
		m_ImageCropperComponentGUI->GetSelectDataGroupBox()->setShown(m_ImageCropperComponentGUI->GetImageCropperFinderGroupBox()->isChecked());
	}
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkImageCropperComponent::ShowImageContent(bool)
{
	m_ImageCropperComponentGUI->GetImageContent()->setShown(m_ImageCropperComponentGUI->GetSelectDataGroupBox()->isChecked());

	if(m_ShowSelector)
	{
		m_ImageCropperComponentGUI->GetImageContent()->setShown(m_ImageCropperComponentGUI->GetSelectDataGroupBox()->isChecked());
	}
	else
	{
		m_ImageCropperComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
	}
}


/************************************************************************/
/***************************************************************************/

void QmitkImageCropperComponent::ExecuteOperation (mitk::Operation *operation)
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

void QmitkImageCropperComponent::CropImage()
{
	if(!m_CroppingObject)
	{
		CreateBoundingObject(0);
	}
	if (!m_MitkImage) 
	{
		return;
	}

	if (!m_CroppingObjectNode->IsVisible(NULL)) return;

	mitk::BoundingObjectCutter::Pointer cutter = mitk::BoundingObjectCutter::New();
	cutter->SetBoundingObject( m_CroppingObject );
	cutter->SetInput( m_MitkImage );
	cutter->AutoOutsideValueOn();

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

	mitk::Image::Pointer resultImage = cutter->GetOutput();
	resultImage->DisconnectPipeline();

	RemoveBoundingObjectFromNode();

	{
		opExchangeNodes*  doOp   = new opExchangeNodes(OP_EXCHANGE, m_MitkImageIterator->Get().GetPointer(), 
			m_MitkImageIterator->Get()->GetData(), 
			resultImage); 
	//	opExchangeNodes* undoOp  = new opExchangeNodes(OP_EXCHANGE, m_MitkImageIterator->Get().GetPointer(), 
	//		resultImage, 
	//		m_MitkImageIterator->Get()->GetData());

	//	//if(m_ParentObject){m_ParentObject->TreeChanged();}
	//	mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( 
	//		new mitk::OperationEvent(this, doOp, undoOp, "Crop image") ); // tell the undo controller about the action
		ExecuteOperation(doOp); // execute action

		m_MitkImage = resultImage;


		if(m_MitkImageIterator.GetPointer())
		{
			m_MitkImageIterator->Get()->SetData(resultImage);
			if(m_ParentObject)
			{
				m_ParentObject->SetTreeIterator(m_MitkImageIterator);  
			}
		}
		emit mitkImageChanged();
	}
}

void QmitkImageCropperComponent::CreateBoundingObject(int type)
{
	if(m_Active)
	{
		//if(! m_BoundingObjectExists)
		//{
		switch(type)
		{
		case 0:
			m_CroppingObject = mitk::Cuboid::New();
			break;
		case 1:
			m_CroppingObject = mitk::Ellipsoid::New();
			break;
		case 2:
			m_CroppingObject = mitk::Cone::New();
			break;
		case 3:
			m_CroppingObject = mitk::Cylinder::New();
			break;
		default:
			m_CroppingObject = mitk::Cuboid::New();
			break;
		}
		//m_CroppingObject = mitk::Cuboid::New();
		//m_CroppingObject = mitk::Ellipsoid::New();

		m_CroppingObjectNode = mitk::DataTreeNode::New(); 
		mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties( m_CroppingObjectNode );
		m_CroppingObjectNode->SetData( m_CroppingObject );
		m_CroppingObjectNode->SetProperty( "name", mitk::StringProperty::New( "CroppingObject" ) );
		m_CroppingObjectNode->SetProperty( "color", mitk::ColorProperty::New(1.0, 1.0, 0.0) );
		m_CroppingObjectNode->SetProperty( "opacity", mitk::FloatProperty::New(0.4) );
		m_CroppingObjectNode->SetProperty( "layer", mitk::IntProperty::New(99) ); // arbitrary, copied from segmentation functionality
		m_CroppingObjectNode->SetProperty( "selected",  mitk::BoolProperty::New(true) );
		m_BoundingObjectExists = true;
		//}
		if(m_MitkImageIterator.GetPointer())
		{
			m_MitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
		}
		if(m_MitkImageIterator.IsNotNull())
		{
			AddBoundingObjectToNode( m_MitkImageIterator );
			m_MitkImageIterator->Get()->SetVisibility(true);
			m_MultiWidget->InitializeStandardViews(m_DataTreeIterator.GetPointer());
		}
	}
}
//
//
void QmitkImageCropperComponent::AddBoundingObjectToNode(mitk::DataTreeIteratorClone& iterToNode)
{
	if(m_Active)
	{
		if(m_BoundingObjectExists)
		{
			m_ImageToCrop = dynamic_cast<mitk::Image*>(iterToNode->Get()->GetData());
			m_CroppingObjectNode->SetVisibility(true);

			mitk::DataTreeIteratorClone iteratorToCroppingObject = mitk::DataTreeHelper::FindIteratorToNode(iterToNode.GetPointer(),  m_CroppingObjectNode);
			if (iteratorToCroppingObject->IsAtEnd())
			{
				iterToNode->Add(m_CroppingObjectNode);
				m_CroppingObject->FitGeometry(m_ImageToCrop->GetTimeSlicedGeometry());

				if (m_AffineInteractor.IsNull())
				{
					m_AffineInteractor = mitk::AffineInteractor::New("AffineInteractions ctrl-drag", m_CroppingObjectNode);
					mitk::GlobalInteraction::GetInstance()->AddInteractor( m_AffineInteractor );
				}
			}
		}
	}    
}


void QmitkImageCropperComponent::RemoveBoundingObjectFromNode()
{
	if (m_CroppingObjectNode.IsNotNull())
	{
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_AffineInteractor);
		mitk::DataTreeIteratorClone iteratorToCroppingObject = mitk::DataTreeHelper::FindIteratorToNode(m_DataTreeIterator.GetPointer(),  m_CroppingObjectNode);
		if (iteratorToCroppingObject.IsNotNull())
			iteratorToCroppingObject->Remove();
		m_CroppingObjectNode->SetVisibility(false);
	}
}
