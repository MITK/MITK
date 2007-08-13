/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-07-17 17:59:12 +0200 (Di, 17 Jul 2007) $
Version:   $Revision: 11316 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkSurfaceTransformerComponent.h"
#include "QmitkSurfaceTransformerComponentGUI.h"


#include <QmitkDataTreeComboBox.h>

#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"
#include "QmitkStdMultiWidget.h"

#include <vtkLinearTransform.h>
#include <vtkMatrix4x4.h>

#include <qgroupbox.h>
#include <qlineedit.h>
#include <qslider.h>

#include <vnl/vnl_math.h>




/***************       CONSTRUCTOR      ***************/
QmitkSurfaceTransformerComponent::QmitkSurfaceTransformerComponent(QObject * parent, const char * parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget * mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionalityComponentContainer(parent, parentName, updateSelector, showSelector),
m_SurfaceTransformerImageNode(NULL),
m_SurfaceTransformerComponentGUI(NULL),
m_SurfaceTransformerNodeExisting(false),
m_SurfaceNode(NULL),
m_TransformationMode(0),
m_MultiWidget(mitkStdMultiWidget)
{
	SetDataTreeIterator(it);
	SetAvailability(true);

	SetComponentName("SurfaceTransformer");
	m_Node = it->Get();
}

/***************        DESTRUCTOR      ***************/
QmitkSurfaceTransformerComponent::~QmitkSurfaceTransformerComponent()
{

}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkSurfaceTransformerComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
	m_DataTreeIterator = it;
	m_Node = m_DataTreeIterator->Get();
}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkSurfaceTransformerComponent::SetSelectorVisibility(bool visibility)
{
	if(m_SurfaceTransformerComponentGUI)
	{
		m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->setShown(visibility);
	}
}

/***************   GET IMAGE CONTENT   ***************/
QGroupBox* QmitkSurfaceTransformerComponent::GetImageContent()
{
	return (QGroupBox*) m_SurfaceTransformerComponentGUI->GetImageContent();
}


/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkSurfaceTransformerComponent::GetTreeNodeSelector()
{
	return m_SurfaceTransformerComponentGUI->GetTreeNodeSelector();
}

/****************** GET SURFACE NODE ******************/
mitk::DataTreeNode::Pointer QmitkSurfaceTransformerComponent::GetSurfaceNode()
{
	return m_SurfaceNode;
}

/****************** SET SURFACE NODE ******************/
void QmitkSurfaceTransformerComponent::SetSurfaceNode(mitk::DataTreeNode::Pointer node)
{
	m_SurfaceNode = node;
}

/***************       CONNECTIONS      ***************/
void QmitkSurfaceTransformerComponent::CreateConnections()
{
	if ( m_SurfaceTransformerComponentGUI )
	{
		connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
		connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetSurfaceTransformerFinderGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowSurfaceTransformerContent(bool)));     
		connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool))); 
		connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetSurfaceTransformerFinderGroupBox()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 
		connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetMoveButton()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(TransformationModeMove(bool))); 
		connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetRotateButton()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(TransformationModeRotate(bool)));
	}
}


/************** TRANSFORMATION MODE MOVE **************/
void QmitkSurfaceTransformerComponent::TransformationModeMove(bool toggleflag)
{
	if(toggleflag == true)
	{
		m_SurfaceTransformerComponentGUI->GetRotateButton()->setOn(false);
	}

	if(m_SurfaceNode)
	{
		//activate the connection between the arrow-keys and the translation-mehtods.
		m_Left->setEnabled(true);
		m_Right->setEnabled(true);
		m_Up->setEnabled(true);
		m_Down->setEnabled(true);
		m_Forward->setEnabled(true);
		m_Backward->setEnabled(true);

		//deactivate the connction between the keys and the rotation-methods
		m_RotX->setEnabled(false);
		m_RotInvX->setEnabled(false);
		m_RotY->setEnabled(false);
		m_RotInvY->setEnabled(false);
		m_RotZ->setEnabled(false);
		m_RotInvZ->setEnabled(false);

	}
}

/************* TRANSFORMATION MODE ROTATE *************/
void QmitkSurfaceTransformerComponent::TransformationModeRotate(bool toggleflag)
{
	if(toggleflag == true)
	{
		m_SurfaceTransformerComponentGUI->GetMoveButton()->setOn(false);

		//activate the connection between the arrow-keys and the rotation-mehtods.
		m_RotX->setEnabled(true);
		m_RotInvX->setEnabled(true);
		m_RotY->setEnabled(true);
		m_RotInvY->setEnabled(true);
		m_RotZ->setEnabled(true);
		m_RotInvZ->setEnabled(true);

		//deactivate the connction between the keys and the translation-methods
		m_Left->setEnabled(false);
		m_Right->setEnabled(false);
		m_Up->setEnabled(false);
		m_Down->setEnabled(false);
		m_Forward->setEnabled(false);
		m_Backward->setEnabled(false);
	}
}


/*****************************************************************************************************************************/
/****************************************     TRANSLATION      ***************************************************************/
/*****************************************************************************************************************************/

/*************          MOVE LEFT         *************/
void QmitkSurfaceTransformerComponent::MoveLeft()
{
	if(m_SurfaceNode)
	{
		double val = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(0,3);
		std::cout<<"Wert vorher: "<<val<<std::endl;
		val = val -1;
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,val);
		RepaintRenderWindows();
	}
}                     

/*************         MOVE RIGHT         *************/
void QmitkSurfaceTransformerComponent::MoveRight()
{
	if(m_SurfaceNode)
	{
		double val = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(0,3);
		std::cout<<"Wert vorher: "<<val<<std::endl;
		val = val +1;
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,val);
		RepaintRenderWindows();
	}
}                    

/*************          MOVE UP           *************/
void QmitkSurfaceTransformerComponent::MoveUp()
{
	if(m_SurfaceNode)
	{
		double val = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(1,3);
		std::cout<<"Wert vorher: "<<val<<std::endl;
		val = val +1;
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(1,3,val);
		RepaintRenderWindows();
	}
}                      

/*************         MOVE DOWN          *************/
void QmitkSurfaceTransformerComponent::MoveDown()
{
	if(m_SurfaceNode)
	{
		double val = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(1,3);
		std::cout<<"Wert vorher: "<<val<<std::endl;
		val = val -1;
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(1,3,val);
		RepaintRenderWindows();
	}
}  

/*************       MOVE FOREWARD        *************/
void QmitkSurfaceTransformerComponent::MoveForward()
{
	if(m_SurfaceNode)
	{
		double val = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(2,3);
		std::cout<<"Wert vorher: "<<val<<std::endl;
		val = val +1;
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(2,3,val);
		RepaintRenderWindows();
	}
}                      

/*************        MOVE BACKWARD       *************/
void QmitkSurfaceTransformerComponent::MoveBackward()
{
	if(m_SurfaceNode)
	{
		double val = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(2,3);
		std::cout<<"Wert vorher: "<<val<<std::endl;
		val = val -1;
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(2,3,val);
		RepaintRenderWindows();
	}
} 
/*****************************************************************************************************************************/
/****************************************       ROTATION       ***************************************************************/
/*****************************************************************************************************************************/

///*************    ROTATE ARROUND X-AXIS   *************/
void QmitkSurfaceTransformerComponent::RotX()
{
	if(m_SurfaceNode)
	{
		double valX = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(0,3);
		double valY = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(1,3);
		double valZ = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(2,3);

		double alpha = 5; 

		/*the alpha value is in grad. So grad must be converted to rad:*/
		/* grad |   rad | const*/
		/* 360° |   2pi | 2*vnl_math::pi        */
		/* 270° | 3/2pi | 3*vnl_math::pi_over_2 */
		/* 180° |    pi |   vnl_math::pi        */
		/*  90° | 1/2pi |   vnl_math::pi_over_2 */

		double newValX = valX;
		double newValY = valY * cos ((alpha/180)*vnl_math::pi) - valZ * sin ((alpha/180)*vnl_math::pi);
		double newValZ = valY * sin ((alpha/180)*vnl_math::pi) + valZ * cos ((alpha/180)*vnl_math::pi);

		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,newValX);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(1,3,newValY);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(2,3,newValZ);

		RepaintRenderWindows();
	}
}                     

/*********** ROTATE INVERSE ARROUND X-AXIS ************/
void QmitkSurfaceTransformerComponent::RotInvX()
{
	if(m_SurfaceNode)
	{
		double valX = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(0,3);
		double valY = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(1,3);
		double valZ = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(2,3);

		double alpha = -5; 

		/*the alpha value is in grad. So grad must be converted to rad:*/
		/* grad |   rad | const*/
		/* 360° |   2pi | 2*vnl_math::pi        */
		/* 270° | 3/2pi | 3*vnl_math::pi_over_2 */
		/* 180° |    pi |   vnl_math::pi        */
		/*  90° | 1/2pi |   vnl_math::pi_over_2 */

		double newValX = valX;
		double newValY = valY * cos ((alpha/180)*vnl_math::pi) - valZ * sin ((alpha/180)*vnl_math::pi);
		double newValZ = valY * sin ((alpha/180)*vnl_math::pi) + valZ * cos ((alpha/180)*vnl_math::pi);

		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,newValX);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(1,3,newValY);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(2,3,newValZ);

		RepaintRenderWindows();
	}
}                    

/*************    ROTATE ARROUND Y-AXIS   *************/
void QmitkSurfaceTransformerComponent::RotY()
{
	if(m_SurfaceNode)
	{
		double valX = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(0,3);
		double valY = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(1,3);
		double valZ = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(2,3);

		double alpha = 5; 

		/*the alpha value is in grad. So grad must be converted to rad:*/
		/* grad |   rad | const*/
		/* 360° |   2pi | 2*vnl_math::pi        */
		/* 270° | 3/2pi | 3*vnl_math::pi_over_2 */
		/* 180° |    pi |   vnl_math::pi        */
		/*  90° | 1/2pi |   vnl_math::pi_over_2 */

		double newValX = valX * cos ((alpha/180)*vnl_math::pi) + valZ * sin ((alpha/180)*vnl_math::pi);
		double newValY = valY;
		double newValZ = -valX * sin ((alpha/180)*vnl_math::pi) + valZ * cos ((alpha/180)*vnl_math::pi);

		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,newValX);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(1,3,newValY);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(2,3,newValZ);

		RepaintRenderWindows();
	}
}                      

/*********** ROTATE INVERSE ARROUND Y-AXIS ************/
void QmitkSurfaceTransformerComponent::RotInvY()
{
	if(m_SurfaceNode)
	{
		double valX = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(0,3);
		double valY = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(1,3);
		double valZ = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(2,3);

		double alpha = -5; 

		/*the alpha value is in grad. So grad must be converted to rad:*/
		/* grad |   rad | const*/
		/* 360° |   2pi | 2*vnl_math::pi        */
		/* 270° | 3/2pi | 3*vnl_math::pi_over_2 */
		/* 180° |    pi |   vnl_math::pi        */
		/*  90° | 1/2pi |   vnl_math::pi_over_2 */

		double newValX = valX * cos ((alpha/180)*vnl_math::pi) + valZ * sin ((alpha/180)*vnl_math::pi);
		double newValY = valY;
		double newValZ = -valX * sin ((alpha/180)*vnl_math::pi) + valZ * cos ((alpha/180)*vnl_math::pi);

		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,newValX);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(1,3,newValY);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(2,3,newValZ);

		RepaintRenderWindows();
	}
} 

/*************    ROTATE ARROUND Z-AXIS   *************/
void QmitkSurfaceTransformerComponent::RotZ()
{
	if(m_SurfaceNode)
	{
		double valX = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(0,3);
		double valY = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(1,3);
		double valZ = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(2,3);

		double alpha = 5; 

		/*the alpha value is in grad. So grad must be converted to rad:*/
		/* grad |   rad | const*/
		/* 360° |   2pi | 2*vnl_math::pi        */
		/* 270° | 3/2pi | 3*vnl_math::pi_over_2 */
		/* 180° |    pi |   vnl_math::pi        */
		/*  90° | 1/2pi |   vnl_math::pi_over_2 */

		double newValX = valX * cos ((alpha/180)*vnl_math::pi) - valY * sin ((alpha/180)*vnl_math::pi);
		double newValY = valX * sin ((alpha/180)*vnl_math::pi) + valY * cos ((alpha/180)*vnl_math::pi);
		double newValZ = valZ;

		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,newValX);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(1,3,newValY);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(2,3,newValZ);

		RepaintRenderWindows();
	}
}                      

/*********** ROTATE INVERSE ARROUND Z-AXIS ************/
void QmitkSurfaceTransformerComponent::RotInvZ()
{
	if(m_SurfaceNode)
	{
		double valX = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(0,3);
		double valY = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(1,3);
		double valZ = m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->GetElement(2,3);

		double alpha = -5; 

		/*the alpha value is in grad. So grad must be converted to rad:*/
		/* grad |   rad | const*/
		/* 360° |   2pi | 2*vnl_math::pi        */
		/* 270° | 3/2pi | 3*vnl_math::pi_over_2 */
		/* 180° |    pi |   vnl_math::pi        */
		/*  90° | 1/2pi |   vnl_math::pi_over_2 */

		double newValX = valX * cos ((alpha/180)*vnl_math::pi) - valY * sin ((alpha/180)*vnl_math::pi);
		double newValY = valX * sin ((alpha/180)*vnl_math::pi) + valY * cos ((alpha/180)*vnl_math::pi);
		double newValZ = valZ;

		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,newValX);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(1,3,newValY);
		m_SurfaceNode->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(2,3,newValZ);

		RepaintRenderWindows();
	}
}

/*************    REPAINT RENDERWINDOWS   *************/
void QmitkSurfaceTransformerComponent::RepaintRenderWindows()
{
	m_MultiWidget->GetRenderWindow1()->repaint();
	m_MultiWidget->GetRenderWindow2()->repaint();
	m_MultiWidget->GetRenderWindow3()->repaint();
	m_MultiWidget->GetRenderWindow4()->repaint();
}


/***************     IMAGE SELECTED     ***************/
void QmitkSurfaceTransformerComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
	m_SelectedItem = imageIt;
	mitk::DataTreeFilter::Item* currentItem(NULL);
	if(m_SurfaceTransformerComponentGUI)
	{
		if(mitk::DataTreeFilter* filter = m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()->GetFilter())
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
	if(m_SurfaceTransformerComponentGUI != NULL)
	{
		for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
		{
			QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
			if (functionalityComponent != NULL)
				functionalityComponent->ImageSelected(m_SelectedItem);
		}
	}
	m_Node = const_cast<mitk::DataTreeNode*>(m_SelectedItem->GetNode());
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkSurfaceTransformerComponent::CreateControlWidget(QWidget* parent)
{
	m_SurfaceTransformerComponentGUI = new QmitkSurfaceTransformerComponentGUI(parent);
	m_GUI = m_SurfaceTransformerComponentGUI;

	m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());

	if(m_ShowSelector)
	{
		m_SurfaceTransformerComponentGUI->GetImageContent()->setShown(m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->isChecked());
	}
	else
	{
		m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
	}


	//create connection between keys and translation methods

	m_Left = new QAccel(m_GUI);        // create accels for multiWidget
	m_Left->connectItem( m_Left->insertItem(Qt::Key_Left), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(MoveLeft()));                     // MoveLeft() slot

	m_Right = new QAccel(m_GUI);        // create accels for multiWidget
	m_Right->connectItem( m_Right->insertItem(Qt::Key_Right), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(MoveRight()));                     // MoveRight() slot

	m_Up = new QAccel(m_GUI);        // create accels for multiWidget
	m_Up->connectItem( m_Up->insertItem(Qt::Key_Up), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(MoveUp()));                     // MoveUp() slot

	m_Down = new QAccel(m_GUI);        // create accels for multiWidget
	m_Down->connectItem( m_Down->insertItem(Qt::Key_Down), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(MoveDown()));                     // MoveDown() slot

	m_Forward = new QAccel(m_GUI);        // create accels for multiWidget
	m_Forward->connectItem( m_Forward->insertItem(Qt::Key_A), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(MoveForward()));                     // MoveForward() slot

	m_Backward = new QAccel(m_GUI);        // create accels for multiWidget
	m_Backward->connectItem( m_Backward->insertItem(Qt::Key_S), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(MoveBackward()));                     // MoveBackward() slot



	//create connection between keys and rotation methods

	m_RotX = new QAccel(m_GUI);        // create accels for multiWidget
	m_RotX->connectItem( m_RotX->insertItem(Qt::Key_Left), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(RotX()));                     // RotY() slot

	m_RotInvX = new QAccel(m_GUI);        // create accels for multiWidget
	m_RotInvX->connectItem( m_RotInvX->insertItem(Qt::Key_Right), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(RotInvX()));                     // RotInvX() slot

	m_RotY = new QAccel(m_GUI);        // create accels for multiWidget
	m_RotY->connectItem( m_RotY->insertItem(Qt::Key_Up), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(RotY()));                     // RotY() slot

	m_RotInvY = new QAccel(m_GUI);        // create accels for multiWidget
	m_RotInvY->connectItem( m_RotInvY->insertItem(Qt::Key_Down), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(RotInvY()));                     // RotInvY() slot

	m_RotZ = new QAccel(m_GUI);        // create accels for multiWidget
	m_RotZ->connectItem( m_RotZ->insertItem(Qt::Key_A), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(RotZ()));                     // RotZ() slot

	m_RotInvZ = new QAccel(m_GUI);        // create accels for multiWidget
	m_RotInvZ->connectItem( m_RotInvZ->insertItem(Qt::Key_S), // adds SHIFT+ALT accelerator
		this,                                       // connected to SurfaceCreator
		SLOT(RotInvZ()));                     // RotInvZ() slot



	/*  see list of Keys at http://doc.trolltech.com/3.3/qt.html#Key-enum */

	return m_SurfaceTransformerComponentGUI;
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkSurfaceTransformerComponent::GetContentContainer()
{
	return m_SurfaceTransformerComponentGUI->GetContainerContent();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkSurfaceTransformerComponent::GetMainCheckBoxContainer()
{
	return m_SurfaceTransformerComponentGUI->GetSurfaceTransformerFinderGroupBox();
}

/***************        ACTIVATED       ***************/
void QmitkSurfaceTransformerComponent::Activated()
{
	QmitkBaseFunctionalityComponent::Activated();
	m_Active = true;
	for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
	{
		m_AddedChildList[i]->Activated();
	} 

	m_SurfaceTransformerComponentGUI->GetMoveButton()->setOn(false);
	m_SurfaceTransformerComponentGUI->GetRotateButton()->setOn(false);

	m_Left->setEnabled(false);
	m_Right->setEnabled(false);
	m_Up->setEnabled(false);
	m_Down->setEnabled(false);
	m_Forward->setEnabled(false);
	m_Backward->setEnabled(false);

	m_RotX->setEnabled(false);
	m_RotInvX->setEnabled(false);
	m_RotY->setEnabled(false);
	m_RotInvY->setEnabled(false);
	m_RotZ->setEnabled(false);
	m_RotInvZ->setEnabled(false);

}

/***************       DEACTIVATED      ***************/
void QmitkSurfaceTransformerComponent::Deactivated()
{
	QmitkBaseFunctionalityComponent::Deactivated();
	m_Active = false;
	for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
	{
		m_AddedChildList[i]->Deactivated();
	} 

	m_Left->setEnabled(false);
	m_Right->setEnabled(false);
	m_Up->setEnabled(false);
	m_Down->setEnabled(false);
	m_Forward->setEnabled(false);
	m_Backward->setEnabled(false);

	m_RotX->setEnabled(false);
	m_RotInvX->setEnabled(false);
	m_RotY->setEnabled(false);
	m_RotInvY->setEnabled(false);
	m_RotZ->setEnabled(false);
	m_RotInvZ->setEnabled(false);
}


///************ SHOW SurfaceTransformer CONTENT ***********/
void QmitkSurfaceTransformerComponent::ShowSurfaceTransformerContent(bool)
{
	m_SurfaceTransformerComponentGUI->GetContainerContent()->setShown(m_SurfaceTransformerComponentGUI->GetSurfaceTransformerFinderGroupBox()->isChecked());

	if(m_ShowSelector)
	{
		m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->setShown(m_SurfaceTransformerComponentGUI->GetSurfaceTransformerFinderGroupBox()->isChecked());
	}
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkSurfaceTransformerComponent::ShowImageContent(bool)
{
	m_SurfaceTransformerComponentGUI->GetImageContent()->setShown(m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->isChecked());

	if(m_ShowSelector)
	{
		m_SurfaceTransformerComponentGUI->GetImageContent()->setShown(m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->isChecked());
	}
	else
	{
		m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
	}
}


///***************  DELETE SurfaceTransformerD NODE  **************/
void QmitkSurfaceTransformerComponent::DeleteSurfaceTransformerNode()
{
	if(m_SurfaceTransformerImageNode)
	{
		mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
		while ( !iteratorClone->IsAtEnd() )
		{
			mitk::DataTreeNode::Pointer node = iteratorClone->Get();

			std::string name;
			node->GetName(name);

			if(name == "SurfaceTransformer image")
			{
				iteratorClone->Disconnect();
				m_SurfaceTransformerNodeExisting = false;
			}
			++iteratorClone;
		}
	}
}
