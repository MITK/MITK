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
#include "QmitkInteractionFunctionalityComponent.h"
#include <QmitkDataTreeComboBox.h>



QmitkInteractionFunctionalityComponent::QmitkInteractionFunctionalityComponent(QObject *parent, const char * parentName, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* dataIt, bool updateSelector, bool showSelector)
: QmitkBaseFunctionalityComponent(parent, parentName, dataIt),
m_GUI(NULL),
m_Active(true),
m_SelectedImage(NULL)
{

}

QmitkInteractionFunctionalityComponent::~QmitkInteractionFunctionalityComponent()
{

}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkInteractionFunctionalityComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/***************   SET COMPONENT NAME   ***************/
void QmitkInteractionFunctionalityComponent::SetComponentName(QString name)
{
  m_Name = name;
}

/***************   GET COMPONENT NAME   ***************/
QString QmitkInteractionFunctionalityComponent::SetComponentName()
{
  return m_Name;
}

/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkInteractionFunctionalityComponent::GetTreeNodeSelector()
{
  return (QmitkDataTreeComboBox*) NULL;
}

/***************         GET GUI        ***************/
QWidget* QmitkInteractionFunctionalityComponent::GetGUI()
{
  return m_GUI;
}

/*************** TREE CHANGED (       ) ***************/
void QmitkInteractionFunctionalityComponent::TreeChanged()
{
    UpdateDataTreeComboBoxes();
}

/************ Update DATATREECOMBOBOX(ES) *************/
void QmitkInteractionFunctionalityComponent::UpdateDataTreeComboBoxes()
{
  if(GetTreeNodeSelector() != NULL)
  {
    GetTreeNodeSelector()->Update();
  }
}
