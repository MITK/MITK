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



void QmitkImageCropperComponentGUI::SetDataTreeIterator( mitk::DataTreeIteratorBase * it )
{
   m_DataTreeIterator = it;
//TODO   m_TreeNodeSelector->SetDataTreeNodeIterator(it);
   //m_VolumetryWidget->SetDataTreeNodeIterator(it);
}




QmitkDataTreeComboBox * QmitkImageCropperComponentGUI::GetTreeNodeSelector()
{
 return m_TreeNodeSelector;
}


QGroupBox* QmitkImageCropperComponentGUI::GetShowImageCropperGroupBox()
{
  return m_ShowThresholdGroupBox;
}

QGroupBox* QmitkImageCropperComponentGUI::GetImageCropperFinderGroupBox()
{
  return m_ThresholdFinder;
}

QGroupBox* QmitkImageCropperComponentGUI::GetSelectDataGroupBox()
{
  return m_ThresholdSelectDataGroupBox;
}

QGroupBox* QmitkImageCropperComponentGUI::GetImageCropperValueContent()
{
  return m_ThresholdValueContent;
}

QGroupBox* QmitkImageCropperComponentGUI::GetImageContent()
{
  return m_ImageContent;
}



QGroupBox* QmitkImageCropperComponentGUI::GetContainerContent()
{
  return m_ContainerContent;
}


QPushButton * QmitkImageCropperComponentGUI::GetCropImageButton()
{
  return m_CropImageButton;
}

QPushButton* QmitkImageCropperComponentGUI::GetCubeButton()
{
  return m_CubeButton;
}


QPushButton* QmitkImageCropperComponentGUI::GetEllipsoidButton()
{
  return m_EllipsoidButton;
}


QPushButton* QmitkImageCropperComponentGUI::GetCylinderButton()
{
  return m_CylinderButton;
}


QPushButton* QmitkImageCropperComponentGUI::GetConeButton()
{
 return m_ConeButton;
}
