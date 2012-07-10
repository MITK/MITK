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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>

// Qmitk
#include "CommandLineModulesView.h"
#include "CommandLineModulesPreferencesPage.h"

// Qt
#include <QMessageBox>

const std::string CommandLineModulesView::VIEW_ID = "org.mitk.gui.qt.cli";

CommandLineModulesView::CommandLineModulesView()
{
}

CommandLineModulesView::~CommandLineModulesView()
{
}

void CommandLineModulesView::SetFocus()
{
}

void CommandLineModulesView::CreateQtPartControl( QWidget *parent )
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  // Loads the preferences into member variables.
  this->RetrievePreferenceValues();
}

void CommandLineModulesView::RetrievePreferenceValues()
{
  berry::IPreferencesService::Pointer prefService
      = berry::Platform::GetServiceRegistry()
      .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  assert( prefService );

  std::string id = "/" + CommandLineModulesView::VIEW_ID;
  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node(id))
        .Cast<berry::IBerryPreferences>();

  assert( prefs );

  m_TemporaryDirectoryName = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::TEMPORARY_DIRECTORY_NODE_NAME, ""));
  m_ModulesDirectoryName = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::MODULES_DIRECTORY_NODE_NAME, ""));
}

void CommandLineModulesView::OnPreferencesChanged(const berry::IBerryPreferences* /*prefs*/)
{
  // Loads the preferences into member variables.
  this->RetrievePreferenceValues();
}


