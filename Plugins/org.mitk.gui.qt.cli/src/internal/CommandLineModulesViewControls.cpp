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
: m_LayoutForAbout(NULL)
, m_LayoutForHelp(NULL)
{
  this->setupUi(parent);
  this->m_RunButton->setIcon(QIcon(":/CommandLineModulesResources/run.png"));
  this->m_StopButton->setIcon(QIcon(":/CommandLineModulesResources/stop.png"));
  this->m_RestoreDefaults->setIcon(QIcon(":/CommandLineModulesResources/undo.png"));

  m_LayoutForAbout = new QHBoxLayout(m_TabAbout);
  m_LayoutForAbout->addWidget(m_AboutBrowser);
  m_LayoutForAbout->setContentsMargins(0,0,0,0);
  m_LayoutForAbout->setSpacing(0);
  m_LayoutForHelp = new QHBoxLayout(m_TabHelp);
  m_LayoutForHelp->addWidget(m_HelpBrowser);
  m_LayoutForHelp->setContentsMargins(0,0,0,0);
  m_LayoutForHelp->setSpacing(0);

  m_AboutBrowser->setReadOnly(true);
  m_HelpBrowser->setReadOnly(true);

  m_GridLayout->setRowStretch(0, 0);
  m_GridLayout->setRowStretch(1, 1);
  m_GridLayout->setRowStretch(2, 5);

}


//-----------------------------------------------------------------------------
CommandLineModulesViewControls::~CommandLineModulesViewControls()
{

}
