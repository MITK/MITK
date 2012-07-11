/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "CommandLineModulesViewControls.h"
#include <QIcon>

CommandLineModulesViewControls::CommandLineModulesViewControls(QWidget *parent)
{
  this->setupUi(parent);
  this->m_RunButton->setIcon(QIcon(":/CommandLineModulesResources/run.png"));
  this->m_StopButton->setIcon(QIcon(":/CommandLineModulesResources/stop.png"));
}

CommandLineModulesViewControls::~CommandLineModulesViewControls()
{

}
