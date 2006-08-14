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

#include <QmitkDataTreeComboBox.h>


/***************       CONSTRUCTOR      ***************/
QmitkThresholdComponent::QmitkThresholdComponent(QObject *parent, const char *name, mitk::DataTreeIteratorBase* it)
{
  SetDataTreeIterator(it);
  m_GUI = NULL;
  //m_Name = name;
  //SetAvailability(true);
}

/***************        DESTRUCTOR      ***************/
QmitkThresholdComponent::~QmitkThresholdComponent()
{

}

/*************** GET FUNCTIONALITY NAME ***************/
QString QmitkThresholdComponent::GetFunctionalityName()
{
  return m_Name;
}

/*************** SET FUNCTIONALITY NAME ***************/
void QmitkThresholdComponent::SetFunctionalityName(QString name)
{
  m_Name = name;
}

/*************** GET FUNCCOMPONENT NAME ***************/
QString QmitkThresholdComponent::GetFunctionalityComponentName()
{
  return m_Name;
}

/*************** GET DATA TREE ITERATOR ***************/
mitk::DataTreeIteratorBase* QmitkThresholdComponent::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkThresholdComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/***************         GET GUI        ***************/
QWidget* QmitkThresholdComponent::GetGUI()
{
  return m_GUI;
}

/*************** TREE CHANGED (       ) ***************/
void QmitkThresholdComponent::TreeChanged()
{
  if(m_GUI != NULL)
  {
    
    for(unsigned int i = 0;  i < m_Qbfc.size(); i++) 
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
    //    m_GUI->GetTreeNodeSelector()->SetDataTreeNodeIterator(it);
    //TODO
    // UpdateTreeNodeSelector(it);
  }
}

/***************       CONNECTIONS      ***************/
void QmitkThresholdComponent::CreateConnections()
{
  if ( m_GUI )
  {
    connect( (QObject*)(m_GUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
  }
}

/***************     IMAGE SELECTED     ***************/
void QmitkThresholdComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedImage = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if ( m_GUI )
  {
    if ( mitk::DataTreeFilter* filter = m_GUI->GetTreeNodeSelector()->GetFilter() )
    {
      if ( imageIt )
      {
        currentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( imageIt->GetNode() ) );
      }
    }
  }
  
  if( currentItem )
  {
    currentItem->SetSelected(true);
  }

  TreeChanged();
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkThresholdComponent::CreateContainerWidget(QWidget* parent)
{
  m_GUI = new QmitkThresholdComponentGUI(parent);
  m_GUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
  return m_GUI;
}
















