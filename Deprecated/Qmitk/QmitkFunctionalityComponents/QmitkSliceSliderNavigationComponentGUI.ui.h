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



void QmitkSliceSliderNavigationComponentGUI::SetDataTreeIterator( mitk::DataTreeIteratorBase * it )
{
   m_DataTreeIterator = it;
//TODO   m_TreeNodeSelector->SetDataTreeNodeIterator(it);
   //m_VolumetryWidget->SetDataTreeNodeIterator(it);
}




QmitkDataTreeComboBox * QmitkSliceSliderNavigationComponentGUI::GetTreeNodeSelector()
{
 return m_TreeNodeSelector;
}


QGroupBox* QmitkSliceSliderNavigationComponentGUI::GetImageContent()
{
  return m_ImageContent;
}




QmitkSliderNavigator* QmitkSliceSliderNavigationComponentGUI::GetTransversalSlider()
{
  return m_SliderNavigatorTransversal;
}


QmitkSliderNavigator* QmitkSliceSliderNavigationComponentGUI::GetSagittalSlider()
{
return m_SliderNavigatorSagittal;
}


QmitkSliderNavigator* QmitkSliceSliderNavigationComponentGUI::GetCoronalSlider()
{
return m_SliderNavigatorCoronal;
}


QmitkSliderNavigator* QmitkSliceSliderNavigationComponentGUI::GetTemporalSlider()
{
return m_SliderNavigatorTemporal;
}


QGroupBox* QmitkSliceSliderNavigationComponentGUI::GetSliceSliderNavigatorContentGroupBox()
{
return m_SliceSliderNavigatorContentGroupBox;
}


QGroupBox* QmitkSliceSliderNavigationComponentGUI::GetSliceSliderNavigator()
{
return m_SliceSliderNavigator;
}


QGroupBox* QmitkSliceSliderNavigationComponentGUI::GetSelectDataGroupBox()
{
 return m_SelectDataGroupBox;
}


QLabel* QmitkSliceSliderNavigationComponentGUI::GetTextLabelTemporal()
{
 return m_TextLabelTemporal;
}

