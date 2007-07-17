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
#include "QmitkFunctionalityComponentContainer.h"
//#include "QmitkBaseFunctionalityComponent.h"

#include <QmitkDataTreeComboBox.h>
#include <mitkDataTreeFilter.h>
#include "mitkDataTreeFilterFunctions.h"

/*****Qt-Elements***/
#include <qgroupbox.h>
#include <qlayout.h>
#include <itkCommand.h>
#include <qobjectlist.h>
#include <qcombobox.h>
#include <qlabel.h>
//#include <qlayout.h>

#include <vector>


const QSizePolicy preferred(QSizePolicy::Preferred, QSizePolicy::Preferred);

/***************       CONSTRUCTOR      ***************/
QmitkFunctionalityComponentContainer::QmitkFunctionalityComponentContainer(QObject *parent, const char *parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkBaseFunctionalityComponent(parent, parentName, it),
m_UpdateSelector(updateSelector), 
m_ShowSelector(showSelector),
m_GUI(NULL),
//m_ParentMitkImage(NULL),
//m_ParentMitkImageIterator(NULL),
m_Active(false),
m_SelectedItem(NULL),
m_FunctionalityComponentContainerGUI(NULL),
m_Parent(parent), 
//m_ParentName(parentName), 
m_ComponentName("ComponentContainer"),
//m_MultiWidget(mitkStdMultiWidget),
m_Spacer(NULL),
m_MulitWidget(mitkStdMultiWidget),
m_BackButton(NULL),
m_NextButton(NULL),
m_MaximumWidgedStackSize(-1)
{
	SetDataTreeIterator(it);
	SetAvailability(true);
}

/***************        DESTRUCTOR      ***************/
QmitkFunctionalityComponentContainer::~QmitkFunctionalityComponentContainer()
{
	if(m_DataTreeIterator.IsNotNull() )
	{
		m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
	}
}

/***************   SET COMPONENT NAME   ***************/
void QmitkFunctionalityComponentContainer::SetComponentName(QString name)
{
	m_ComponentName = name;
}

/***************   GET COMPONENT NAME   ***************/
QString QmitkFunctionalityComponentContainer::GetComponentName()
{
	return m_ComponentName;
}


/************* SET SHOW TREE NODE SELECTOR ***********/
void QmitkFunctionalityComponentContainer::SetShowTreeNodeSelector(bool show)
{
	GetImageContent()->setShown(show);
}

/***************   GET IMAGE CONTENT   ***************/
QGroupBox*  QmitkFunctionalityComponentContainer::GetImageContent()
{
	return (QGroupBox*) m_FunctionalityComponentContainerGUI->GetImageContent();
}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkFunctionalityComponentContainer::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
	if(m_DataTreeIterator.IsNotNull() )
	{
		m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
	}
	m_DataTreeIterator = it;
	if(m_DataTreeIterator.IsNotNull())
	{
		itk::ReceptorMemberCommand<QmitkFunctionalityComponentContainer>::Pointer command = itk::ReceptorMemberCommand<QmitkFunctionalityComponentContainer>::New();
		command->SetCallbackFunction(this, &QmitkFunctionalityComponentContainer::TreeChanged);
		m_ObserverTag = m_DataTreeIterator->GetTree()->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
	}
}

/***************         GET GUI        ***************/
QWidget* QmitkFunctionalityComponentContainer::GetGUI()
{
	return m_GUI;
}

/*************** GET PARTENT MITK IMAGE ***************/
mitk::Image* QmitkFunctionalityComponentContainer::GetParentMitkImage()
{
	return m_ParentMitkImage;
}
/******** ******* GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkFunctionalityComponentContainer::GetTreeNodeSelector()
{
	if(m_FunctionalityComponentContainerGUI)
	{
		return m_FunctionalityComponentContainerGUI->GetTreeNodeSelector();
	}
	else return NULL;
}

/******** *******    GET MULTI WIDGET    ***************/
QmitkStdMultiWidget * QmitkFunctionalityComponentContainer::GetMultiWidget()
{
	return m_MulitWidget;
}

/*************** TREE CHANGED ( EVENT ) ***************/
void QmitkFunctionalityComponentContainer::TreeChanged(const itk::EventObject & /*treeChangedEvent*/)
{
	if(IsActivated())
	{
		TreeChanged();
	}
	else
		TreeChanged();
}

/*************** TREE CHANGED (       ) ***************/
void QmitkFunctionalityComponentContainer::TreeChanged()
{
	UpdateDataTreeComboBoxes();

	for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
	{
		m_AddedChildList[i]->TreeChanged();
	} 
}

/************ Update DATATREECOMBOBOX(ES) *************/
void QmitkFunctionalityComponentContainer::UpdateDataTreeComboBoxes()
{
	if(GetTreeNodeSelector() != NULL)
	{
		GetTreeNodeSelector()->Update();
	}
}

/***************       CONNECTIONS      ***************/
void QmitkFunctionalityComponentContainer::CreateConnections()
{
	if ( m_FunctionalityComponentContainerGUI )
	{
		connect( (QObject*)(m_FunctionalityComponentContainerGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
		connect( (QObject*)(m_FunctionalityComponentContainerGUI->GetContainerBorder()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool)));    
	}
}

void QmitkFunctionalityComponentContainer::SetTreeIterator(mitk::DataTreeIteratorClone dataIt)
{
	if(dataIt.IsNotNull())
	{
		if(m_FunctionalityComponentContainerGUI)
		{
			//mitk::DataTreeFilter* filter = m_FunctionalityComponentContainerGUI->GetTreeNodeSelector()->GetFilter();
			m_MitkImageIterator  = dataIt;

			if(m_MitkImageIterator.GetPointer())
			{
				m_MitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
			}

			if(m_FunctionalityComponentContainerGUI != NULL)
			{
				for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
				{
          QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
          if (functionalityComponent != NULL)
          {
            functionalityComponent->m_DataTreeIterator = m_MitkImageIterator;
            functionalityComponent->TreeChanged();
          }
        }   
      }
		}
	}
}

/***************     IMAGE SELECTED     ***************/
void QmitkFunctionalityComponentContainer::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
	m_SelectedItem = imageIt;
	if(m_FunctionalityComponentContainerGUI != NULL)
	{
		for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
		{
			QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->ImageSelected(m_SelectedItem);
		}   
	}

	if(m_FunctionalityComponentContainerGUI)
	{
		mitk::DataTreeFilter* filter = m_FunctionalityComponentContainerGUI->GetTreeNodeSelector()->GetFilter();
		m_MitkImageIterator = filter->GetIteratorToSelectedItem();

		if(m_MitkImageIterator.GetPointer())
		{
			m_ParentMitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
		}

		if(m_FunctionalityComponentContainerGUI != NULL)
		{
			for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
			{
        QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
        if (functionalityComponent != NULL)
        {
          functionalityComponent->m_ParentMitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
				  functionalityComponent->m_MitkImageIterator = m_MitkImageIterator;
        }
			}   
		}
	}
	TreeChanged();
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkFunctionalityComponentContainer::CreateControlWidget(QWidget* parent)
{
	if (m_FunctionalityComponentContainerGUI == NULL)
	{
		m_FunctionalityComponentContainerGUI = new QmitkFunctionalityComponentContainerGUI(parent);
		m_GUI = m_FunctionalityComponentContainerGUI;

		m_FunctionalityComponentContainerGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
		//m_FunctionalityComponentContainerGUI->GetContainerBorder()->setTitle("<bold>Select Image<\bold>");
		//m_FunctionalityComponentContainerGUI->GetContainerBorder()->setLineWidth(0);
	}
	CreateConnections();
	m_FunctionalityComponentContainerGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));
	return m_GUI;
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkFunctionalityComponentContainer::GetContentContainer()
{
	return m_FunctionalityComponentContainerGUI->GetImageContent();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkFunctionalityComponentContainer::GetMainCheckBoxContainer()
{
	return m_FunctionalityComponentContainerGUI->GetContainerBorder();
}

/*********** SET CONTENT CONTAINER VISIBLE ************/
void QmitkFunctionalityComponentContainer::SetContentContainerVisibility(bool)
{
	if(GetMainCheckBoxContainer() != NULL)
	{
		if(GetMainCheckBoxContainer()->isChecked())
		{
			Activated();
		}
		else
		{
			Deactivated();
		}
	}
	for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
	{
		if(m_AddedChildList[i]->GetContentContainer() != NULL)
		{
			m_AddedChildList[i]->GetContentContainer()->setShown(GetMainCheckBoxContainer()->isChecked());
		}
		if(m_AddedChildList[i]->GetMainCheckBoxContainer() != NULL)
		{
			m_AddedChildList[i]->GetMainCheckBoxContainer()->setChecked(GetMainCheckBoxContainer()->isChecked());
		}
		m_AddedChildList[i]->SetContentContainerVisibility(GetMainCheckBoxContainer()->isChecked());
	} 
}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkFunctionalityComponentContainer::SetSelectorVisibility(bool visibility)
{
	if(m_GUI)
	{
		m_FunctionalityComponentContainerGUI->GetImageContent()->setShown(visibility);
	}
	m_ShowSelector = visibility;
}

/***************        ACTIVATED       ***************/
void QmitkFunctionalityComponentContainer::Activated()
{
	TreeChanged();
	QmitkBaseFunctionalityComponent::Activated();
	m_Active = true;
	for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
	{
		m_AddedChildList[i]->Activated();
	} 
}

/***************      DEACTIVATED       ***************/
void QmitkFunctionalityComponentContainer::Deactivated()
{
	QmitkBaseFunctionalityComponent::Deactivated();
	m_Active = false;
	for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
	{
		m_AddedChildList[i]->Deactivated();
	} 
}

/***************      ADD COMPONENT     ***************/
void QmitkFunctionalityComponentContainer::AddComponent(QmitkBaseFunctionalityComponent* component)
{  
	if(component!=NULL)
	{
		QWidget* componentWidget = component->CreateControlWidget(m_GUI);
		AddComponentListener(component);
		m_GUI->layout()->add(componentWidget);
		component->CreateConnections();
		if(m_Spacer != NULL)
		{
			m_GUI->layout()->removeItem(m_Spacer);
		}
		QSpacerItem*  spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
		m_Spacer = spacer;
		m_GUI->layout()->addItem( m_Spacer ); 
		m_GUI->repaint();
	}
}
/** \brief Method to return the NextButton to switch to the next widgetStackPage*/
QPushButton* QmitkFunctionalityComponentContainer::GetNextButton()
{
	return m_NextButton;
}

/** \brief Method to return the BackButton to switch to the last widgetStackPage*/
QPushButton* QmitkFunctionalityComponentContainer::GetBackButton()
{
	return m_BackButton;
}


/***************      ADD COMPONENT     ***************/
void QmitkFunctionalityComponentContainer::AddComponent(QmitkBaseFunctionalityComponent* component, int stackPage)
{  
	if(component!=NULL)
	{
		QWidget* visibleWidget = m_FunctionalityComponentContainerGUI->GetWidgetStack()->visibleWidget();
		int idVisibleWidget = m_FunctionalityComponentContainerGUI->GetWidgetStack()->id(visibleWidget);
		if(idVisibleWidget > m_MaximumWidgedStackSize)
		{
			m_MaximumWidgedStackSize = idVisibleWidget;
		}
		if(m_MaximumWidgedStackSize < stackPage)
		{
			QWidget* w = new QWidget(m_FunctionalityComponentContainerGUI->GetWidgetStack());
			m_FunctionalityComponentContainerGUI->GetWidgetStack()->addWidget(w, stackPage);
			m_MaximumWidgedStackSize++;
			m_FunctionalityComponentContainerGUI->GetWidgetStack()->raiseWidget(w);
			visibleWidget = m_FunctionalityComponentContainerGUI->GetWidgetStack()->visibleWidget();
			idVisibleWidget = m_FunctionalityComponentContainerGUI->GetWidgetStack()->id(visibleWidget);
			QLayout* visibleWidgetLayout = new QVBoxLayout(visibleWidget, QBoxLayout::TopToBottom);
		}

		QLayout* layout;
		if(m_FunctionalityComponentContainerGUI->GetWidgetStack()->layout() == 0)
		{
			layout = new QVBoxLayout(m_FunctionalityComponentContainerGUI->GetWidgetStack(), QBoxLayout::TopToBottom);
		}
		else 
		{
			layout = m_FunctionalityComponentContainerGUI->GetWidgetStack()->layout();
		}


		QWidget* componentWidget = component->CreateControlWidget(m_FunctionalityComponentContainerGUI->GetWidgetStack()->visibleWidget());
		AddComponentListener(component);
		m_FunctionalityComponentContainerGUI->GetWidgetStack()->widget(stackPage)->layout()->add(componentWidget);
		m_FunctionalityComponentContainerGUI->GetWidgetStack()->setShown(true);
		m_FunctionalityComponentContainerGUI->GetWidgetStack()->updateGeometry();
		m_FunctionalityComponentContainerGUI->GetWidgetStack()->layout()->activate();
		componentWidget->setShown(true);
		component->CreateConnections();

		m_GUI->repaint();
	}
}

void QmitkFunctionalityComponentContainer::CreateNavigationButtons()
{
	//QBoxLayout * buttonLayout = new QHBoxLayout(GetImageContent()->layout());
	QBoxLayout * buttonLayout = new QHBoxLayout(m_FunctionalityComponentContainerGUI->layout());
	//if(m_BackButton==NULL)
	//{
	//	m_BackButton = new QPushButton("<<", GetImageContent());
	//}
	//if(m_NextButton==NULL)
	//{
	//	m_NextButton = new QPushButton(">>", GetImageContent());
	//}
	if(m_BackButton==NULL)
	{
		m_BackButton = new QPushButton("<<", m_FunctionalityComponentContainerGUI);
	}
	if(m_NextButton==NULL)
	{
		m_NextButton = new QPushButton(">>", m_FunctionalityComponentContainerGUI);
	}
	buttonLayout->addWidget(m_BackButton);
	buttonLayout->addWidget(m_NextButton);
	//m_GUI->layout()->addChildLayout(buttonLayout);
	m_BackButton->setShown(true);
	m_NextButton->setShown(true);
	m_GUI->layout()->AlignTop;
	m_GUI->layout()->activate();
	m_GUI->repaint();

//	connect( (QObject*)(m_NextButton),  SIGNAL(pressed()), (QObject*) this, SLOT(NextButtonPressed())); 
//	connect( (QObject*)(m_BackButton),  SIGNAL(pressed()), (QObject*) this, SLOT(BackButtonPressed())); 

	m_FunctionalityComponentContainerGUI->GetWidgetStack()->raiseWidget(1);
	SetWizardText("");
	GetImageContent()->updateGeometry();
	if(m_Spacer != NULL)
	{
		m_GUI->layout()->removeItem(m_Spacer);
	}
	QSpacerItem*  spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
	m_Spacer = spacer;
	m_GUI->layout()->addItem( m_Spacer );
	m_GUI->updateGeometry();
	m_GUI->repaint();
}

void QmitkFunctionalityComponentContainer::SetWizardText(const QString & text)
{
	//m_FunctionalityComponentContainerGUI->GetWizardTextLabel()->setText(text);
	//m_FunctionalityComponentContainerGUI->GetWizardTextLabel()->setAlignment(Qt::WordBreak);
	GetImageContent()->updateGeometry();
	GetImageContent()->repaint();
	m_GUI->updateGeometry();
	m_GUI->layout()->AlignTop;
	m_GUI->layout()->activate();
	m_GUI->repaint();

}

//void QmitkFunctionalityComponentContainer::ChooseWizardText(int page)
//{
//	switch(page)
//	{
//	case 1:
//		SetWizardText("<b>Step 1   Minimize the dataset:</b><br><b>a)</b> If you have a 4D-Dataset export one time step with the TimeStepExporter.<br><b>b)</b> Crop the image by selecting a shape and placing it (STRG + Mouse) around the relevant areas. Everything around the shape will be cut off.");
//		break;
//	case 2:
//		SetWizardText("<b>Step 2   Create an STL-Model:</b><br><b>a)</b> Choose a threshold with the Threshold Finder where all areas that shall be used for the model are marked (green).<br><b>b)</b> If there are special areas that have to be manipulated use the PixelGreyValueManipulator and do not forget to select the new created image.><br><b>c)</b> Create a Surface with the SurfaceCreator.");
//		break;
//	case 3:
//		SetWizardText("<b>Step 3 Surface Finish:</b><br><b>a)</b> If you have your surface you can use the connectivity filter where you can mark all connected areas in different colours or delete alle areas instead of the biggest.");
//		break;
//	default:
//		SetWizardText("<b>Step 1   Minimize the dataset:</b><br><b>a)</b> If you have a 4D-Dataset export one time step with the TimeStepExporter.<br><b>b)</b> Crop the image by selecting a shape and placing it (STRG + Mouse) around the relevant areas. Everything around the shape will be cut off.");
//		break;
//	};
//}

//void QmitkFunctionalityComponentContainer::NextButtonPressed()
//{
//	int actualPage = m_FunctionalityComponentContainerGUI->GetWidgetStack()->id(m_FunctionalityComponentContainerGUI->GetWidgetStack()->visibleWidget());
//	switch(actualPage)
//	{
//	case 1:
//		m_FunctionalityComponentContainerGUI->GetWidgetStack()->raiseWidget(2);
//		ChooseWizardText(2);
//		break;
//	case 2:
//		m_FunctionalityComponentContainerGUI->GetWidgetStack()->raiseWidget(3);
//		ChooseWizardText(3);
//		break;
//	case 3:
//		m_FunctionalityComponentContainerGUI->GetWidgetStack()->raiseWidget(1);
//		ChooseWizardText(1);
//		break;
//	default:
//		m_FunctionalityComponentContainerGUI->GetWidgetStack()->raiseWidget(1);
//		ChooseWizardText(1);
//		break;
//	};
//
//}
//void QmitkFunctionalityComponentContainer::BackButtonPressed()
//{
//	int actualPage = m_FunctionalityComponentContainerGUI->GetWidgetStack()->id(m_FunctionalityComponentContainerGUI->GetWidgetStack()->visibleWidget());
//	switch(actualPage)
//	{
//	case 1:
//		m_FunctionalityComponentContainerGUI->GetWidgetStack()->raiseWidget(3);
//		ChooseWizardText(3);
//		break;
//	case 2:
//		m_FunctionalityComponentContainerGUI->GetWidgetStack()->raiseWidget(1);
//		ChooseWizardText(1);
//		break;
//	case 3:
//		m_FunctionalityComponentContainerGUI->GetWidgetStack()->raiseWidget(2);
//		ChooseWizardText(2);
//		break;
//	default:
//		m_FunctionalityComponentContainerGUI->GetWidgetStack()->raiseWidget(1);
//		ChooseWizardText(1);
//		break;
//	};
//}


QmitkFunctionalityComponentContainerGUI * QmitkFunctionalityComponentContainer::GetFunctionalityComponentContainerGUI()
{
	return m_FunctionalityComponentContainerGUI;
}
