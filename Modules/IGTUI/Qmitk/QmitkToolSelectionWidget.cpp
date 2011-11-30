/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-03-21 19:27:37 +0100 (Sa, 21 Mrz 2009) $
Version:   $Revision: 16719 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkToolSelectionWidget.h"



QmitkToolSelectionWidget::QmitkToolSelectionWidget(QWidget* parent)  
: QWidget(parent), m_Controls(NULL)
{
  this->CreateQtPartControl( this );
}

QmitkToolSelectionWidget::~QmitkToolSelectionWidget()
{
  m_Controls = NULL;
}


void QmitkToolSelectionWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToolSelectionWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}


void QmitkToolSelectionWidget::CreateConnections()
{
  connect ( m_Controls->m_UseToolCheckBox, SIGNAL(toggled(bool)), this, SLOT(CheckBoxToggled(bool)) );
  //connect ( m_Controls->m_ToolSelectionComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(void SelectedToolChanged(int)) );

  //connect( (QObject*)(m_Controls->m_StartNavigationBtn), SIGNAL(clicked()), this, SLOT(OnStartTimer()) );
  //connect( (QObject*)(m_Controls->m_StopNavigationBtn), SIGNAL(clicked()), this, SLOT(OnStopTimer()) );
  //connect(  m_Controls->m_UpdateRateSB, SIGNAL(valueChanged(int)), this, SLOT(OnChangeTimerInterval(int)) );
}


int QmitkToolSelectionWidget::GetCurrentSelectedIndex()
{
  return m_Controls->m_ToolSelectionComboBox->currentIndex();
}

void QmitkToolSelectionWidget::SetToolNames( const QStringList& toolNames )
{
  m_Controls->m_ToolSelectionComboBox->addItems(toolNames);
}

void QmitkToolSelectionWidget::AddToolName( const QString& toolName)
{
  m_Controls->m_ToolSelectionComboBox->addItem(toolName);
}

void QmitkToolSelectionWidget::ChangeToolName( int index, const QString& toolName )
{
  m_Controls->m_ToolSelectionComboBox->insertItem(index, toolName);
}

void QmitkToolSelectionWidget::RemoveToolName( const QString& toolName )
{
  for(int i=0; i < m_Controls->m_ToolSelectionComboBox->count(); ++i)
  {
    if(m_Controls->m_ToolSelectionComboBox->itemText(i).compare(toolName) == 0)
      m_Controls->m_ToolSelectionComboBox->removeItem(i);
  }
}

void QmitkToolSelectionWidget::RemoveToolName( int index )
{
  m_Controls->m_ToolSelectionComboBox->removeItem(index);
}

void QmitkToolSelectionWidget::ClearToolNames()
{
  m_Controls->m_ToolSelectionComboBox->clear();
}


void  QmitkToolSelectionWidget::SetCheckboxtText( const QString& text)
{
  m_Controls->m_UseToolCheckBox->setText(text);
}

void QmitkToolSelectionWidget::EnableWidget()
{
  this->setEnabled( true );
}

void QmitkToolSelectionWidget::DisableWidget()
{
  this->setEnabled( false );
}


void QmitkToolSelectionWidget::CheckBoxToggled(bool checked)
{
  if(checked)
    m_Controls->m_ToolSelectionComboBox->setEnabled(false);
  else
    m_Controls->m_ToolSelectionComboBox->setEnabled(true);

  emit SignalUseTool(m_Controls->m_ToolSelectionComboBox->currentIndex(), checked);
}

bool QmitkToolSelectionWidget::IsSelectedToolActivated()
{
  return m_Controls->m_UseToolCheckBox->isChecked();
}
