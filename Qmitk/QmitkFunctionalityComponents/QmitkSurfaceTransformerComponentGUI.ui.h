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



void QmitkSurfaceTransformerComponentGUI::SetDataTreeIterator( mitk::DataTreeIteratorBase * it )
{
   m_DataTreeIterator = it;
//TODO   m_TreeNodeSelector->SetDataTreeNodeIterator(it);
   //m_VolumetryWidget->SetDataTreeNodeIterator(it);
}




QmitkDataTreeComboBox * QmitkSurfaceTransformerComponentGUI::GetTreeNodeSelector()
{
 return m_TreeNodeSelector;
}


QGroupBox* QmitkSurfaceTransformerComponentGUI::GetShowSurfaceTransformerGroupBox()
{
  return m_ShowSurfaceTransformerGroupBox;
}

QGroupBox* QmitkSurfaceTransformerComponentGUI::GetSurfaceTransformerFinderGroupBox()
{
  return m_SurfaceTransformer;
}

QGroupBox* QmitkSurfaceTransformerComponentGUI::GetSelectDataGroupBox()
{
  return m_SurfaceTransformerSelectDataGroupBox;
}

QGroupBox* QmitkSurfaceTransformerComponentGUI::GetSurfaceTransformerValueContent()
{
  return m_SurfaceTransformerValueContent;
}

QGroupBox* QmitkSurfaceTransformerComponentGUI::GetImageContent()
{
  return m_ImageContent;
}


QGroupBox* QmitkSurfaceTransformerComponentGUI::GetContainerContent()
{
  return m_ContainerContent;
}


QPushButton* QmitkSurfaceTransformerComponentGUI::GetMoveButton()
{
  return m_MoveButton;
}


QPushButton* QmitkSurfaceTransformerComponentGUI::GetRotateButton()
{
  return m_RotateButton;
}


QPushButton* QmitkSurfaceTransformerComponentGUI::GetScaleButton()
{
  return m_ScaleButton;
}
