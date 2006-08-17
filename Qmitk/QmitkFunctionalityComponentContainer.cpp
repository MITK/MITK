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
#include "QmitkFunctionalityComponentContainerGUI.h"

#include <QmitkDataTreeComboBox.h>
#include <mitkDataTreeFilter.h>

/*****Qt-Elements***/
#include <qgroupbox.h>
#include <qlayout.h>
#include <itkCommand.h>
#include <qobjectlist.h>
#include <qcombobox.h>

const QSizePolicy preferred(QSizePolicy::Preferred, QSizePolicy::Preferred);

/***************       CONSTRUCTOR      ***************/
QmitkFunctionalityComponentContainer::QmitkFunctionalityComponentContainer(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkBaseFunctionalityComponent(name, it),m_Parent(parent), m_GUI(NULL),m_Name(name), m_MultiWidget(mitkStdMultiWidget), m_SelectedImage(NULL), m_UpdateSelector(true), m_ShowSelector(true)
{
  SetDataTreeIterator(it);
  m_Active = false;
}

/***************       CONSTRUCTOR      ***************/
QmitkFunctionalityComponentContainer::QmitkFunctionalityComponentContainer(QObject *parent, const char *name, mitk::DataTreeIteratorBase* it, bool updateSelector, bool showSelector)
: QmitkBaseFunctionalityComponent(name, it),m_Parent(parent), m_GUI(NULL),m_Name(name), m_SelectedImage(NULL), m_UpdateSelector(updateSelector), m_ShowSelector(showSelector)
{
  SetDataTreeIterator(it);
}

/***************        DESTRUCTOR      ***************/
QmitkFunctionalityComponentContainer::~QmitkFunctionalityComponentContainer()
{
  if(m_DataTreeIterator.IsNotNull() )
  {
    //m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
  }
}

/*************** GET FUNCTIONALITY NAME ***************/
QString QmitkFunctionalityComponentContainer::GetFunctionalityName()
{
  return m_Name;
}

/*************** SET FUNCTIONALITY NAME ***************/
void QmitkFunctionalityComponentContainer::SetFunctionalityName(QString name)
{
  m_Name = name;
}

/*************** GET FUNCCOMPONENT NAME ***************/
QString QmitkFunctionalityComponentContainer::GetFunctionalityComponentName()
{
  return m_Name;
}

/*************** GET DATA TREE ITERATOR ***************/
mitk::DataTreeIteratorBase* QmitkFunctionalityComponentContainer::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
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

/*************** TREE CHANGED ( EVENT ) ***************/
void QmitkFunctionalityComponentContainer::TreeChanged(const itk::EventObject & /*treeChangedEvent*/)
{
  if(IsActivated())
  {
    m_TreeChangedWhileInActive = false;
    TreeChanged();
  }
  else
    m_TreeChangedWhileInActive = true;
  TreeChanged();
}

/*************** TREE CHANGED (       ) ***************/
void QmitkFunctionalityComponentContainer::TreeChanged()
{
  
}

/*************** TREE CHANGED(ITERATOR) ***************/
void QmitkFunctionalityComponentContainer::TreeChanged(mitk::DataTreeIteratorBase* it)
{

}

/***************       CONNECTIONS      ***************/
void QmitkFunctionalityComponentContainer::CreateConnections()
{
  if ( m_GUI )
  {
    connect( (QObject*)(m_GUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
  }
}

/***************     IMAGE SELECTED     ***************/
void QmitkFunctionalityComponentContainer::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedImage = imageIt;
    if(m_GUI != NULL)
  {
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
    {
      m_AddedChildList[i]->ImageSelected(m_SelectedImage);
    }   
  }
  TreeChanged();
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkFunctionalityComponentContainer::CreateContainerWidget(QWidget* parent)
{
  if (m_GUI == NULL)
  {
    m_GUI = new QmitkFunctionalityComponentContainerGUI(parent);
    m_GUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
    m_GUI->GetContainerBorder()->setTitle("Container");
    m_GUI->GetContainerBorder()->setLineWidth(0);
  }
  return m_GUI;
}

/***************        ACTIVATED       ***************/
void QmitkFunctionalityComponentContainer::Activated()
{
  m_Activated = true;
  m_Active = true;
  if(m_TreeChangedWhileInActive)
  {
    TreeChanged();
    m_TreeChangedWhileInActive = false;
  }
}


/***************      ADD COMPONENT     ***************/
void QmitkFunctionalityComponentContainer::AddComponent(QmitkFunctionalityComponentContainer* componentContainer)
{  
  if(componentContainer!=NULL)
  {
    QWidget* componentWidget = componentContainer->CreateContainerWidget(m_GUI);
    m_AddedChildList.push_back(componentContainer);
    m_GUI->layout()->add(componentWidget);
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    m_GUI->layout()->addItem( spacer );

  }

}



