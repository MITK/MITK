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



void QmitkSurfaceCreatorComponentGUI::SetDataTreeIterator( mitk::DataTreeIteratorBase * it )
{
   m_DataTreeIterator = it;
//TODO   m_TreeNodeSelector->SetDataTreeNodeIterator(it);
   //m_VolumetryWidget->SetDataTreeNodeIterator(it);
}

QmitkDataTreeComboBox * QmitkSurfaceCreatorComponentGUI::GetTreeNodeSelector()
{
 return m_TreeNodeSelector;
}

QGroupBox* QmitkSurfaceCreatorComponentGUI::GetSelectDataGroupBox()
{
  return m_SurfaceDataGroupBox;
}


QGroupBox * QmitkSurfaceCreatorComponentGUI::GetImageContent()
{
   return m_ImageContent;
}


QGroupBox * QmitkSurfaceCreatorComponentGUI::GetSurfaceCreatorGroupBox()
{
  return m_SurfaceCreatorGroupBox;
}
