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

#include "QmitkTreeNodeSelector.h"

/*****Qt-Elements***/
#include <qgroupbox.h>
#include <qlayout.h>
#include <itkCommand.h>


QmitkFunctionalityComponentContainer::QmitkFunctionalityComponentContainer(QObject *parent, const char *name, mitk::DataTreeIteratorBase* it)
: QmitkBaseFunctionalityComponent(name, it),m_GUI(NULL),m_Name(name)
{
  //SetAvailability(true);
  //  CreateContainerWidget(parent);
  SetDataTreeIterator(it);

}

QmitkFunctionalityComponentContainer::~QmitkFunctionalityComponentContainer()
{

}

QString QmitkFunctionalityComponentContainer::GetFunctionalityName()
{
  return m_Name;
}

void QmitkFunctionalityComponentContainer::SetFunctionalityName(QString name)
{
  m_Name = name;
}

mitk::DataTreeIteratorBase* QmitkFunctionalityComponentContainer::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
}

void QmitkFunctionalityComponentContainer::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  //if(m_DataTreeIterator.IsNotNull() )
  //{
  //  m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
  //}
  //m_DataTreeIterator = it;
  //if(m_DataTreeIterator.IsNotNull())
  //{
  //  itk::ReceptorMemberCommand<QmitkFunctionalityComponentContainer>::Pointer command = itk::ReceptorMemberCommand<QmitkFunctionalityComponentContainer>::New();
  //  command->SetCallbackFunction(this, &QmitkFunctionalityComponentContainer::TreeChanged);
  //  m_ObserverTag = m_DataTreeIterator->GetTree()->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
  //}
}

void QmitkFunctionalityComponentContainer::TreeChanged()
{
  if(m_GUI != NULL)
  {
    m_GUI->GetTreeNodeSelector()->SetDataTreeNodeIterator(this->GetDataTreeIterator());
    for(int i = 0;  i < m_Qbfc.size(); i++)
    {
      //m_Qbfc[i]->SetDataTree(this->GetDataTreeIterator());
      m_Qbfc[i]->TreeChanged(this->GetDataTreeIterator());
      TreeChanged(GetDataTreeIterator());
    }
  }

}

void QmitkFunctionalityComponentContainer::TreeChanged(mitk::DataTreeIteratorBase* it)
{
if(m_GUI != NULL)
  {
    m_GUI->GetTreeNodeSelector()->SetDataTreeNodeIterator(it);
    for(int i = 0;  i < m_Qbfc.size(); i++)
    {
      m_Qbfc[i]->SetDataTreeIterator(this->GetDataTreeIterator());
      m_Qbfc[i]->TreeChanged(it);
    }
  }
}

void QmitkFunctionalityComponentContainer::CreateConnections()
{
  if ( m_GUI )
  {
    connect( (QObject*)(m_GUI->m_TreeNodeSelector), SIGNAL(Activated(mitk::DataTreeIteratorClone)),(QObject*) this, SLOT(ImageSelected(mitk::DataTreeIteratorClone)) );
  }
}


QString QmitkFunctionalityComponentContainer::GetFunctionalityComponentName()
{
  return m_Name;
}

void QmitkFunctionalityComponentContainer::ImageSelected(mitk::DataTreeIteratorClone imageIt)
{
  TreeChanged();
}


/******************************************************************************************/
QWidget* QmitkFunctionalityComponentContainer::CreateContainerWidget(QWidget* parent)
{
  if (m_GUI == NULL)
  {
    m_GUI = new QmitkFunctionalityComponentContainerGUI(parent);
    m_GUI->GetCompContBox()->setTitle("Container");
  }
  return m_GUI;
}

void QmitkFunctionalityComponentContainer::Activated()
{
  m_Activated = true;
  if(m_TreeChangedWhileInActive)
  {
    TreeChanged();
    m_TreeChangedWhileInActive = false;
  }
}

void QmitkFunctionalityComponentContainer::AddComponent(QmitkFunctionalityComponentContainer* componentContainer)
{  
  if(componentContainer!=NULL)
  {
    //QWidget* componentWidget = componentContainer->CreateContainerWidget(m_GUI->GetCompContBox());
    m_Qbfc.push_back(componentContainer);

    //QLayout* containerLayout = m_GUI->GetContainerLayout();
    //m_GUI->insertChild(component->GetWidget());
  }

}

QWidget* QmitkFunctionalityComponentContainer::GetGUI()
{
  return m_GUI;
}


