/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "CommandLineModulesViewControls.h"
#include <QIcon>
#include <QSize>
#include <QHBoxLayout>

//-----------------------------------------------------------------------------
CommandLineModulesViewControls::CommandLineModulesViewControls(QWidget *parent)
{
  this->setupUi(parent);
  this->m_ConsoleOutput->setCollapsed(true);
  this->m_RunButton->setIcon(QIcon(":/CommandLineModulesResources/run.png"));
  this->m_StopButton->setIcon(QIcon(":/CommandLineModulesResources/stop.png"));
  this->m_RestoreDefaults->setIcon(QIcon(":/CommandLineModulesResources/undo.png"));
  this->Initial();
}


//-----------------------------------------------------------------------------
CommandLineModulesViewControls::~CommandLineModulesViewControls()
{

}


//-----------------------------------------------------------------------------
void CommandLineModulesViewControls::Initial()
{
  this->m_RunButton->setIcon(QIcon(":/CommandLineModulesResources/run.png"));
  this->m_RunButton->setToolTip("run application");
  this->m_RunButton->update();
  this->Enable(true);
}


//-----------------------------------------------------------------------------
void CommandLineModulesViewControls::Running()
{
  this->m_RunButton->setIcon(QIcon(":/CommandLineModulesResources/pause.png"));
  this->m_RunButton->setToolTip("pause application");
  this->m_RunButton->update();
  this->Enable(false);
}


//-----------------------------------------------------------------------------
void CommandLineModulesViewControls::Cancel()
{
  this->Initial();
}


//-----------------------------------------------------------------------------
void CommandLineModulesViewControls::Pause()
{
  this->m_RunButton->setIcon(QIcon(":/CommandLineModulesResources/run.png"));
  this->m_RunButton->setToolTip("resume application");
  this->m_RunButton->update();
  this->Enable(false);
}


//-----------------------------------------------------------------------------
void CommandLineModulesViewControls::Resume()
{
  this->Running();
}


//-----------------------------------------------------------------------------
void CommandLineModulesViewControls::Finished()
{
  this->Initial();
}


//-----------------------------------------------------------------------------
void CommandLineModulesViewControls::Enable(bool enable)
{
  this->m_ComboBox->setEnabled(enable);
  this->m_TabWidget->setEnabled(enable);
  this->m_RestoreDefaults->setEnabled(enable);
}
