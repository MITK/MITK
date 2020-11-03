/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "CommandLineModulesViewControls.h"
#include <QIcon>
#include <QSize>
#include <QHBoxLayout>

//-----------------------------------------------------------------------------
CommandLineModulesViewControls::CommandLineModulesViewControls(QWidget *parent)
{
  this->setupUi(parent);
  this->m_ChooseLabel->setVisible(false);
  this->m_ComboBox->setToolTip("Choose a command line module");
  this->m_RunButton->setIcon(QIcon(":/CommandLineModulesResources/run.png"));
  this->m_RestoreDefaults->setIcon(QIcon(":/CommandLineModulesResources/undo.png"));
}


//-----------------------------------------------------------------------------
CommandLineModulesViewControls::~CommandLineModulesViewControls()
{

}


//-----------------------------------------------------------------------------
void CommandLineModulesViewControls::SetAdvancedWidgetsVisible(const bool& isVisible)
{
  this->m_ClearXMLCache->setVisible(isVisible);
  this->m_ReloadModules->setVisible(isVisible);
}

