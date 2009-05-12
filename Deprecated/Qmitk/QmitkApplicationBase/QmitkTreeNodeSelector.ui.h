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

/**
  \class QmitkTreeNodeSelector
  
 \attention This class is deprecated and should be replaced by QmitkDataTreeComboBox, 
            as it provides the same functionality but better separation of GUI and 
            non-GUI tasks.
            For information about how to use the new widget, refer to QmitkDataTreeComboBox, 
            there is quite some information. If unsure, ask the MITK mailinglist.
  
 A simple ComboXBox for selecting an image node
*/
#include "mitkDataTreeNode.h"
#include "QmitkCommonFunctionality.h"
#include "qlistbox.h"

void QmitkTreeNodeSelector::SetDataTreeNodeIterator( mitk::DataTreeIteratorClone it )
{
  m_DataTreeIteratorClone = it;
  UpdateContent();
}

void QmitkTreeNodeSelector::UpdateContent()
{
  // iteriere ueber Baum und wende filter funktion an
  QString currentText = m_ComboBox->currentText();
  m_ComboBox->clear();
  m_TreeNodes.clear();
  if ( m_DataTreeIteratorClone.IsNotNull() ) {
    CommonFunctionality::DataTreeIteratorVector images = CommonFunctionality::FilterNodes(m_DataTreeIteratorClone,m_FilterFunction);
   
    for (CommonFunctionality::DataTreeIteratorVector::iterator it = images.begin(); it != images.end() ; it++ )
    {
      std::string name;
      if ((*it)->Get()->GetName(name))
      {
        /** TODO: create a better kind of uniqueness */
        // this line adds spaces to the name, until it is unique within the list
        while ( m_ComboBox->listBox() && m_ComboBox->listBox()->findItem( QString(name.c_str()), Qt::ExactMatch|Qt::CaseSensitive ) )
          name += ' ';
        
        m_ComboBox->insertItem( name.c_str() );
        m_TreeNodes[name] = *it;
        if (currentText == name.c_str())
        {
          m_ComboBox->setCurrentText(currentText);
        }
      }
    }
  }
  QString newText = m_ComboBox->currentText();
  if ( (currentText != newText) )
  {
    if ( !newText.isNull() ) 
    {
      TreeNodeSelected(newText);
    }
    else
    {
      emit TreeNodeChanged();
    }
  }
}


bool QmitkTreeNodeSelector::IsImageNode( mitk::DataTreeNode * node )
{
  return node && node->GetData() && dynamic_cast<mitk::Image*>(node->GetData());
}

void QmitkTreeNodeSelector::init()
{
  m_FilterFunction = QmitkTreeNodeSelector::IsImageNode;
}


void QmitkTreeNodeSelector::TreeNodeSelected( const QString &name )
{
  emit TreeNodeChanged();
  emit Activated(m_TreeNodes[name.ascii()]);
  emit Activated(m_TreeNodes[name.ascii()]->Get());
}


mitk::DataTreeNode * QmitkTreeNodeSelector::GetSelectedNode() const
{
  if (m_ComboBox && !m_ComboBox->currentText().isEmpty())
  {
    return m_TreeNodes.find(m_ComboBox->currentText().ascii())->second->Get();
  }
  else
  {
    return NULL;
  }
}


const mitk::DataTreeIteratorClone * QmitkTreeNodeSelector::GetSelectedIterator() const
{
  if (m_ComboBox && !m_ComboBox->currentText().isEmpty())
  {
    return &( m_TreeNodes.find(m_ComboBox->currentText().ascii())->second);
  }
  else
  {
    return NULL;
  }
}


bool QmitkTreeNodeSelector::SelectNode(mitk::DataTreeNode* node)
{
  // TODO: this is not optimal, since the tree could possibly have changed since the last call of UpdateContent().
  // But iterating over the class internal map will yield a wrong order of items sometimes
  
  // iterate over matching data tree nodes
  //   if tree node == node, mark this item as selected, return true
  // if node is not in tree, return false
  CommonFunctionality::DataTreeIteratorVector images = CommonFunctionality::FilterNodes(m_DataTreeIteratorClone,m_FilterFunction);
  CommonFunctionality::DataTreeIteratorVector::iterator it;
  int i;
  for (it = images.begin(), i = 0; 
       it != images.end(); 
       ++it, ++i )
  {
    if ((*it)->Get() == node)
    {
      m_ComboBox->setCurrentItem(i);
      emit TreeNodeChanged();
      emit Activated(*it);
      emit Activated(node);
    }
  }
  
  // nothing found
  return false;
}

