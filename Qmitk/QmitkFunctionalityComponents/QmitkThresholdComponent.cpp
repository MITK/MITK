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
#include "QmitkThresholdComponent.h"
#include "QmitkThresholdComponentGUI.h"

#include "QmitkTreeNodeSelector.h"
#include <qcombobox.h>

QmitkThresholdComponent::QmitkThresholdComponent( )
{
  m_GUI = NULL;
  //m_Name = name;
  //SetAvailability(true);

}

QmitkThresholdComponent::~QmitkThresholdComponent()
{

}

QString QmitkThresholdComponent::GetFunctionalityName()
{
  return m_Name;
}

QWidget* QmitkThresholdComponent::CreateContainerWidget(QWidget* parent)
{

  m_GUI = new QmitkThresholdComponentGUI(parent);
  return m_GUI;

}

void QmitkThresholdComponent::CreateConnections()
{

}


QString QmitkThresholdComponent::GetFunctionalityComponentName()
{
  return m_Name;
}

/*************** TREE CHANGED (       ) ***************/
void QmitkThresholdComponent::TreeChanged()
{
  if(m_GUI != NULL)
  {
    m_GUI->GetTreeNodeSelector()->SetDataTreeNodeIterator(this->GetDataTreeIterator());
    //TODO
    UpdateTreeNodeSelector(this->GetDataTreeIterator());
    for(int i = 0;  i < m_Qbfc.size(); i++) 
    {
      //m_Qbfc[i]->SetDataTree(this->GetDataTreeIterator());
      //m_Obfc[i]->TreeChanged(this->GetDataTreeIterator());
    }
  }

}

/*************** TREE CHANGED(ITERATOR) ***************/
void QmitkThresholdComponent::TreeChanged(mitk::DataTreeIteratorBase* it)
{
  if(m_GUI != NULL)
  {
    SetDataTreeIterator(it);
    m_GUI->GetTreeNodeSelector()->SetDataTreeNodeIterator(it);
    //TODO
    UpdateTreeNodeSelector(it);
  }
}


QWidget* QmitkThresholdComponent::GetGUI()
{
  return m_GUI;
}



void QmitkThresholdComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}


void QmitkThresholdComponent::UpdateTreeNodeSelector(mitk::DataTreeIteratorClone imageIt)
{
  //m_GUI->SetDataTreeIterator(it);
  //QmitkTreeNodeSelector* guiSelector = m_GUI->GetTreeNodeSelector();
  //guiSelector = selector;
  //m_GUI->GetTreeNodeSelector()->UpdateContent();
  m_GUI->GetTreeNodeSelector()->SetDataTreeNodeIterator(imageIt);
}

void QmitkThresholdComponent::UpdateTreeNodeSelector(mitk::DataTreeIteratorBase* it)
{
  //m_GUI->SetDataTreeIterator(it);
  m_GUI->GetTreeNodeSelector()->UpdateContent();
}

void QmitkThresholdComponent::UpdateSelector(QString name)
{
  m_GUI->GetTreeNodeSelector()->TreeNodeSelected(name);
}
