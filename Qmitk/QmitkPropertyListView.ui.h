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
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
//
#include <qlabel.h>
#include <qlayout.h>
#include <assert.h>
#include <qgroupbox.h>
#include <qobjectlist.h>
#include <vector>
#include <qpushbutton.h>

#include "QmitkPropertyListViewItem.h"
#include "itkCommand.h"

void QmitkPropertyListView::init()
{
  std::cout << "init()" << std::endl;
  m_Group->setColumns(3);
  m_PropertyList = NULL;
}
void QmitkPropertyListView::SetPropertyList( mitk::PropertyList *propertyList )
{
  if (propertyList != m_PropertyList)
  {
    if (m_PropertyList)
    {
      m_PropertyList->RemoveObserver(m_ObserverTag);
    }
    m_PropertyList = propertyList;
    itk::SimpleMemberCommand<QmitkPropertyListView>::Pointer propertyListModifiedCommand =
      itk::SimpleMemberCommand<QmitkPropertyListView>::New();
    propertyListModifiedCommand->SetCallbackFunction(this, &QmitkPropertyListView::PropertyListModified);
    m_ObserverTag = m_PropertyList->AddObserver(itk::ModifiedEvent(), propertyListModifiedCommand);
    int row = 0;
    const mitk::PropertyList::PropertyMap* propertyMap = propertyList->GetMap();

    // clear the group
    for (std::map<std::string,QmitkPropertyListViewItem*>::iterator it = m_Items.begin() ; it != m_Items.end() ; it++)
    {
      delete it->second->m_EnabledButton;
      delete it->second->m_Label;
      delete it->second->m_Control;
    }
    m_Items.clear();
    for (mitk::PropertyList::PropertyMap::const_iterator iter = propertyMap->begin(); iter!=propertyMap->end(); iter++)
    {
      QmitkPropertyListViewItem* item = QmitkPropertyListViewItem::CreateInstance(propertyList,iter->first,m_Group);
      m_Items.insert(std::make_pair(item->m_Name,item));
    }
  }
}
void QmitkPropertyListView::PropertyListModified()
{
  std::cout << "PropertyListModified()" << std::endl;
  const mitk::PropertyList::PropertyMap* propertyMap = m_PropertyList->GetMap();
  for (std::map<std::string,QmitkPropertyListViewItem*>::iterator it = m_Items.begin() ; it != m_Items.end() ; it++)
  {
    // update existing properties and check for removed ones
    if (propertyMap->find(it->first) != propertyMap->end())
    {
      it->second->UpdateView();
    }
    else
    {
      delete it->second->m_EnabledButton;
      delete it->second->m_Label;
      delete it->second->m_Control;
      m_Items.erase(it);
    }
    // now add new ones if they exist
    for (mitk::PropertyList::PropertyMap::const_iterator iter = propertyMap->begin(); iter!=propertyMap->end(); iter++)
    {
      if (m_Items.find(iter->first) == m_Items.end())
      {
        QmitkPropertyListViewItem* item = QmitkPropertyListViewItem::CreateInstance(m_PropertyList,iter->first,m_Group);
        m_Items.insert(std::make_pair(item->m_Name,item));
      }
    }
  }
}

