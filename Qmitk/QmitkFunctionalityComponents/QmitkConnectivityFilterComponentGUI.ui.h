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



QGroupBox* QmitkConnectivityFilterComponentGUI::GetShowTreeNodeSelectorGroupBox()
{
  return m_ShowTreeNodeSelecotrGroupBox;
}


QGroupBox* QmitkConnectivityFilterComponentGUI::GetImageContent()
{
  return m_ImageContent;
}


void QmitkConnectivityFilterComponentGUI::SetDataTreeIterator( mitk::DataTreeIteratorBase * it )
{
  m_DataTreeIterator = it;
}


//QmitkDataTreeComboBox * QmitkThresholdComponentGUI::GetTreeNodeSelector()
//{
// return m_TreeNodeSelector;
//}


QmitkDataTreeComboBox * QmitkConnectivityFilterComponentGUI::GetTreeNodeSelector()
{
  return m_TreeNodeSelector;
}


QGroupBox * QmitkConnectivityFilterComponentGUI::GetComponentContent()
{
  return m_ComponentContent;
}


QGroupBox* QmitkConnectivityFilterComponentGUI::GetShowComponent()
{
  return m_ConnectivityFilter;
}


QComboBox*  QmitkConnectivityFilterComponentGUI::GetFilterModeComboBox()
{
  return m_ParameterModeComboBox;
}


QLineEdit* QmitkConnectivityFilterComponentGUI::GetParameterLineEdit()
{
  return m_ParamterLineEdit;
}


QPushButton* QmitkConnectivityFilterComponentGUI::GetStartFilterPushButton()
{
  return m_StartFilterPushButton;
}


QLineEdit* QmitkConnectivityFilterComponentGUI::GetParameterLabel()
{
  return m_ParamterLineEdit;
}


QGroupBox* QmitkConnectivityFilterComponentGUI::GetParameterPart()
{
  return m_ExtraParameterGroupBox;
}


QCheckBox* QmitkConnectivityFilterComponentGUI::GetReplaceSurfaceCheckBox()
{
  return m_ReplaceSurfaceCheckBox;
}



