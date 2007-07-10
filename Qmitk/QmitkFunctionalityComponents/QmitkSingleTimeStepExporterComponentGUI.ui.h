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



QGroupBox* QmitkSingleTimeStepExporterComponentGUI::GetShowTreeNodeSelectorGroupBox()
{
  return m_ShowTreeNodeSelecotrGroupBox;
}


QGroupBox* QmitkSingleTimeStepExporterComponentGUI::GetImageContent()
{
  return m_ImageContent;
}


void QmitkSingleTimeStepExporterComponentGUI::SetDataTreeIterator( mitk::DataTreeIteratorBase * it )
{
  m_DataTreeIterator = it;
}


QmitkDataTreeComboBox * QmitkSingleTimeStepExporterComponentGUI::GetTreeNodeSelector()
{
  return m_TreeNodeSelector;
}


QGroupBox * QmitkSingleTimeStepExporterComponentGUI::GetComponentContent()
{
  return m_ComponentContent;
}


QGroupBox* QmitkSingleTimeStepExporterComponentGUI::GetShowComponent()
{
  return m_SingleTimeStepExporter;
}



QmitkSliderNavigator* QmitkSingleTimeStepExporterComponentGUI::GetQmitkSliderNavigator()
{
  return m_QmitkSliderNavigator;
}


QPushButton* QmitkSingleTimeStepExporterComponentGUI::GetExportButton()
{
 return m_ExportTimeSliceButton;
}
