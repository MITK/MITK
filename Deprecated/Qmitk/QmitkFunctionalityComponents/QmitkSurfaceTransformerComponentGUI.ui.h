/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: $
Version:   $Revision: $
 
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


QPushButton* QmitkSurfaceTransformerComponentGUI::GetMirrorButton()
{
 return m_MirrorButton;
}


QLineEdit* QmitkSurfaceTransformerComponentGUI::GetXBox()
{
  return m_XBox;
}


QLineEdit* QmitkSurfaceTransformerComponentGUI::GetYBox()
{
  return m_YBox;
}


QLineEdit* QmitkSurfaceTransformerComponentGUI::GetZBox()
{
  return m_ZBox;
}
