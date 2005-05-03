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
  \class QmitkTreeNodeSelector. A simple ComboXBox for selecting an image node
*/
#include "mitkDataTreeNode.h"
void QmitkTreeNodeSelector::SetDataTreeNodeIterator( mitk::DataTreeIteratorClone it )
{
  m_DataTreeIteratorClone = it;
  UpdateContent();
}

/** TODO: make entries unique if two datatreenodes have the same name */

void QmitkTreeNodeSelector::UpdateContent()
{
  // iteriere ueber Baum und wende filter funktion an
  CommonFunctionality::DataTreeIteratorVector images = CommonFunctionality::FilterNodes(m_DataTreeIteratorClone,m_FilterFunction);
  QString currentText = m_ComboBox->currentText();
  while (m_ComboBox->count()) { m_ComboBox->removeItem(0); }
  m_TreeNodes.clear();
  for (CommonFunctionality::DataTreeIteratorVector::iterator it = images.begin(); it != images.end() ; it++ )
  {
    std::string name;
    if ((*it)->Get()->GetName(name))
    {
      m_ComboBox->insertItem( name.c_str() );
      m_TreeNodes[name] =*it;
      if (currentText.find(name.c_str()) != -1 )
      {
        m_ComboBox->setCurrentText(currentText);
      }
    }
  }
  QString newText = m_ComboBox->currentText();
  if (currentText != newText)
  {
    TreeNodeSelected(newText);
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
  emit Activated(m_TreeNodes[name.ascii()]);
  emit Activated(m_TreeNodes[name.ascii()]->Get());
}

mitk::DataTreeNode * QmitkTreeNodeSelector::GetSelectedNode()
{
  if (m_ComboBox && !m_ComboBox->currentText().isEmpty())
  {
    return m_TreeNodes[m_ComboBox->currentText().ascii()]->Get();
  }
  else
  {
    return NULL;
  }
}

