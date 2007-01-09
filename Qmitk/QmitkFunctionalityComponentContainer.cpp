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

//#include <qlayout.h>

#include <vector>


const QSizePolicy preferred(QSizePolicy::Preferred, QSizePolicy::Preferred);

/***************       CONSTRUCTOR      ***************/
QmitkFunctionalityComponentContainer::QmitkFunctionalityComponentContainer(QObject *parent, const char *parentName, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool updateSelector, bool showSelector)
: QmitkBaseFunctionalityComponent(parent, parentName, it),
m_GUI(NULL),
m_Active(false),
m_UpdateSelector(updateSelector), 
m_ShowSelector(showSelector),
m_SelectedItem(NULL),
m_FunctionalityComponentContainerGUI(NULL),
m_Parent(parent), 
//m_ParentName(parentName), 
m_ComponentName("ComponentContainer"),
//m_MultiWidget(mitkStdMultiWidget),
m_Spacer(NULL),
m_MulitWidget(mitkStdMultiWidget)
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


/******** ******* GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkFunctionalityComponentContainer::GetTreeNodeSelector()
{
  if(m_FunctionalityComponentContainerGUI)
  {
    return m_FunctionalityComponentContainerGUI->GetTreeNodeSelector();
  }
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

/***************     IMAGE SELECTED     ***************/
void QmitkFunctionalityComponentContainer::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedItem = imageIt;
  if(m_FunctionalityComponentContainerGUI != NULL)
  {
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
    {
      m_AddedChildList[i]->ImageSelected(m_SelectedItem);
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
    m_FunctionalityComponentContainerGUI->GetContainerBorder()->setTitle("Container");
    m_FunctionalityComponentContainerGUI->GetContainerBorder()->setLineWidth(0);
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



