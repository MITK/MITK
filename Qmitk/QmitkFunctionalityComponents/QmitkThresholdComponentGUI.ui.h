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



void QmitkThresholdComponentGUI::SetDataTreeIterator( mitk::DataTreeIteratorBase * it )
{
   m_DataTreeIterator = it;
//TODO   m_TreeNodeSelector->SetDataTreeNodeIterator(it);
   //m_VolumetryWidget->SetDataTreeNodeIterator(it);
}




QmitkDataTreeComboBox * QmitkThresholdComponentGUI::GetTreeNodeSelector()
{
 return m_TreeNodeSelector;
}

int QmitkThresholdComponentGUI::GetSliderValue()
{
    int value = m_ThresholdInputSlider->value();
    return value;
}

int QmitkThresholdComponentGUI::GetNumberValue()
{
    int value = atoi(m_ThresholdInputNumber->text());
    return value;
}

QSlider* QmitkThresholdComponentGUI::GetThresholdInputSlider()
{
return m_ThresholdInputSlider;
}

QLineEdit* QmitkThresholdComponentGUI::GetThresholdInputNumber()
{
return m_ThresholdInputNumber;
}


QCheckBox* QmitkThresholdComponentGUI::GetShowThresholdCheckBox()
{
  return m_ShowThresholdCheckBox;
}
