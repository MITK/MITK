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
#include "QmitkSurfaceCreatorComponent.h"
#include "QmitkSurfaceCreatorComponentGUI.h"

#include <QmitkDataTreeComboBox.h>

#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>


/***************       CONSTRUCTOR      ***************/
QmitkSurfaceCreatorComponent::QmitkSurfaceCreatorComponent(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool updateSelector, bool showSelector)
{
  SetDataTreeIterator(it);
  m_GUI = NULL;
  m_MultiWidget= mitkStdMultiWidget;
  m_Active = false;
  m_UpdateSelector = updateSelector;
  m_ShowSelector = showSelector;
}

/***************        DESTRUCTOR      ***************/
QmitkSurfaceCreatorComponent::~QmitkSurfaceCreatorComponent()
{

}

/*************** GET FUNCTIONALITY NAME ***************/
QString QmitkSurfaceCreatorComponent::GetFunctionalityName()
{
  return m_Name;
}

/*************** SET FUNCTIONALITY NAME ***************/
void QmitkSurfaceCreatorComponent::SetFunctionalityName(QString name)
{
  m_Name = name;
}

/*************** GET FUNCCOMPONENT NAME ***************/
QString QmitkSurfaceCreatorComponent::GetFunctionalityComponentName()
{
  return m_Name;
}

/*************** GET DATA TREE ITERATOR ***************/
mitk::DataTreeIteratorBase* QmitkSurfaceCreatorComponent::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkSurfaceCreatorComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/***************         GET GUI        ***************/
QWidget* QmitkSurfaceCreatorComponent::GetGUI()
{
  return m_GUI;
}

/*************** TREE CHANGED (       ) ***************/
void QmitkSurfaceCreatorComponent::TreeChanged()
{

}

/***************       CONNECTIONS      ***************/
void QmitkSurfaceCreatorComponent::CreateConnections()
{
  if ( m_GUI )
  {
    connect( (QObject*)(m_GUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));        
    connect( (QObject*)(m_GUI->GetSurfaceCreatorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowSurfaceCreatorContent(bool)));
    connect( (QObject*)(m_GUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool)));
  }
}

/***************     IMAGE SELECTED     ***************/
void QmitkSurfaceCreatorComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedImage = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_GUI)
  {
    if(mitk::DataTreeFilter* filter = m_GUI->GetTreeNodeSelector()->GetFilter())
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
QWidget* QmitkSurfaceCreatorComponent::CreateContainerWidget(QWidget* parent)
{
  m_GUI = new QmitkSurfaceCreatorComponentGUI(parent);
  m_GUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
 
  if(!m_ShowSelector)
  {
    m_GUI->GetSelectDataGroupBox()->setShown(false);
  }

  return m_GUI;

}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkSurfaceCreatorComponent::SetSelectorVisibility(bool visibility)
{
  if(m_GUI)
  {
    m_GUI->GetSelectDataGroupBox()->setShown(visibility);
  }
  m_ShowSelector = visibility;
}


/***************        ACTIVATED       ***************/
void QmitkSurfaceCreatorComponent::Activated()
{
  m_Active = true;
}

/***************       DEACTIVATED      ***************/
void QmitkSurfaceCreatorComponent::Deactivated()
{
  m_Active = false;
}

///************ SHOW SURFACE CREATOR CONTENT ************/
void QmitkSurfaceCreatorComponent::ShowSurfaceCreatorContent(bool)
{
  if(m_ShowSelector)
  {
    m_GUI->GetSelectDataGroupBox()->setShown(m_GUI->GetSurfaceCreatorGroupBox()->isChecked());
  }
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkSurfaceCreatorComponent::ShowImageContent(bool)
{
  m_GUI->GetImageContent()->setShown(m_GUI->GetSelectDataGroupBox()->isChecked());
}