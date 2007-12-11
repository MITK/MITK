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

#include "QmitkTreeNodeSelector.h"

void QmitkFunctionalityComponentContainerGUI::SetDataTreeIterator( mitk::DataTreeIteratorBase * it )
{
   m_DataTreeIterator = it;
//   m_TreeNodeSelector->SetDataTreeNodeIterator(it);
}

QGroupBox* QmitkFunctionalityComponentContainerGUI::GetContainerBorder()
{
    return m_ContainerBorder;
}

QLayout* QmitkFunctionalityComponentContainerGUI::GetContainerLayout()
{
 return NULL;
}

QmitkDataTreeComboBox*  QmitkFunctionalityComponentContainerGUI::GetTreeNodeSelector()
{
    return m_TreeNodeSelector;
}


QGroupBox * QmitkFunctionalityComponentContainerGUI::GetImageContent()
{
  return m_ImageContent;
}


QWidgetStack* QmitkFunctionalityComponentContainerGUI::GetWidgetStack()
{
  return m_WidgetStack;
}
