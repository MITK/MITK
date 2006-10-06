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



void QmitkPixelGreyValueManipulatorComponentGUI::SetDataTreeIterator( mitk::DataTreeIteratorBase * it )
{
   m_DataTreeIterator = it;
//TODO   m_TreeNodeSelector->SetDataTreeNodeIterator(it);
   //m_VolumetryWidget->SetDataTreeNodeIterator(it);
}

QmitkDataTreeComboBox * QmitkPixelGreyValueManipulatorComponentGUI::GetTreeNodeSelector()
{
 return m_TreeNodeSelector;
}

QGroupBox* QmitkPixelGreyValueManipulatorComponentGUI::GetSelectDataGroupBox()
{
  return m_PixelManipulatorDataGroupBox;
}


QGroupBox * QmitkPixelGreyValueManipulatorComponentGUI::GetImageContent()
{
   return m_ImageContent;
}


QGroupBox * QmitkPixelGreyValueManipulatorComponentGUI::GetPixelGreyValueManipulatorGroupBox()
{
  return m_PixelGreyValueManipulatorGroupBox;
}

QGroupBox * QmitkPixelGreyValueManipulatorComponentGUI::GetManipulatorContentGroupBox()
{
    return m_ManipulatorContentGroupBox;
}


QComboBox * QmitkPixelGreyValueManipulatorComponentGUI::GetManipulationModeComboBox()
{
  return m_ManipulationModeComboBox;
}



QComboBox * QmitkPixelGreyValueManipulatorComponentGUI::GetManipulationAreaComboBox()
{
  return m_ManipulationAreaComboBox;
}




QLineEdit * QmitkPixelGreyValueManipulatorComponentGUI::GetValue1LineEdit()
{
  return m_Value1LineEdit;
}


QLineEdit * QmitkPixelGreyValueManipulatorComponentGUI::GetValue2LineEdit()
{
  return m_Value2LineEdit;
}


QGroupBox * QmitkPixelGreyValueManipulatorComponentGUI::GetValue2GroupBox()
{
  return m_Value2GroupBox;
}


QPushButton * QmitkPixelGreyValueManipulatorComponentGUI::GetCreateNewManipulatedImageButton()
{
  return m_CreateNewManipulatedImageButton;
}


QGroupBox * QmitkPixelGreyValueManipulatorComponentGUI::GetPixelManipulatorContentGroupBox()
{
   return m_PixelManipulatorContentGroupBox;
}

QGroupBox * QmitkPixelGreyValueManipulatorComponentGUI::GetValue1GroupBox()
{
  return m_Value1GroupBox;
}




QGroupBox* QmitkPixelGreyValueManipulatorComponentGUI::GetBaseThresholdGroupBox()
{
  return m_BaseThresholdGroupBox;
}


int QmitkPixelGreyValueManipulatorComponentGUI::GetBaseThreshold()
{
  int value;
  value = atoi(m_BaseThresholdLineEdit->text());
  return value;
}


QmitkDataTreeComboBox* QmitkPixelGreyValueManipulatorComponentGUI::GetSegmentationSelector()
{
  return m_SegmentationSelector;
}


QGroupBox* QmitkPixelGreyValueManipulatorComponentGUI::GetSegmentationSelectorGroupBox()
{
  return m_SegmentationSelectorGroupBox;
}


void QmitkPixelGreyValueManipulatorComponentGUI::SetThreshold(int threshold)
{
 m_BaseThresholdLineEdit->setText(QString::number(threshold));
}


QmitkPointListWidget* QmitkPixelGreyValueManipulatorComponentGUI::GetQmitkPointListWidget()
{
  return m_QmitkPointListWidget;
}
