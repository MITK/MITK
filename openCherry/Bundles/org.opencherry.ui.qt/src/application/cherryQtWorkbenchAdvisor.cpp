/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryQtWorkbenchAdvisor.h"
#include "../internal/cherryQtGlobalEventFilter.h"
#include "../cherryQtPreferences.h"

#include <cherryPlatform.h>
#include <cherryIPreferencesService.h>
#include <cherryIQtStyleManager.h>

#include <QApplication>
#include <QString>

#include <Poco/File.h>
#include <Poco/FileStream.h>

#include <vector>

namespace cherry
{

void QtWorkbenchAdvisor::Initialize(IWorkbenchConfigurer::Pointer configurer)
{
  WorkbenchAdvisor::Initialize(configurer);

  IPreferencesService::Pointer prefService = Platform::GetServiceRegistry().GetServiceById<IPreferencesService>(
      IPreferencesService::ID);
  IPreferences::Pointer prefs = prefService->GetSystemPreferences()->Node(QtPreferences::QT_STYLES_NODE);
  QString styleName = QString::fromStdString(prefs->Get(QtPreferences::QT_STYLE_NAME, ""));

  IQtStyleManager::Pointer styleManager = Platform::GetServiceRegistry().GetServiceById<IQtStyleManager>(IQtStyleManager::ID);
  styleManager->SetStyle(styleName);

  QObject* eventFilter = new QtGlobalEventFilter(qApp);
  qApp->installEventFilter(eventFilter);
}

}
