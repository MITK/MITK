/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtWorkbenchAdvisor.h"
#include "internal/berryQtGlobalEventFilter.h"
#include "berryWorkbenchPlugin.h"
#include "berryQtPreferences.h"

#include <berryPlatform.h>
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryIQtStyleManager.h>

#include <QApplication>
#include <QString>
#include <QTextCodec>

#include <Poco/File.h>
#include <Poco/FileStream.h>

#include <vector>

namespace berry
{

void QtWorkbenchAdvisor::Initialize(IWorkbenchConfigurer::Pointer configurer)
{
  WorkbenchAdvisor::Initialize(configurer);

  IPreferencesService* prefService = WorkbenchPlugin::GetDefault()->GetPreferencesService();
  IPreferences::Pointer prefs = prefService->GetSystemPreferences()->Node(QtPreferences::QT_STYLES_NODE);
  QString styleName = prefs->Get(QtPreferences::QT_STYLE_NAME, "");
  QString fontName = prefs->Get(QtPreferences::QT_FONT_NAME, "Open Sans");
  QString fontSize = prefs->Get(QtPreferences::QT_FONT_SIZE, "9");

  ctkServiceReference serviceRef = WorkbenchPlugin::GetDefault()->GetPluginContext()->getServiceReference<IQtStyleManager>();
  if (serviceRef)
  {
    IQtStyleManager* styleManager = WorkbenchPlugin::GetDefault()->GetPluginContext()->getService<IQtStyleManager>(serviceRef);
    styleManager->SetStyle(styleName);
    styleManager->SetFont(fontName);
    styleManager->SetFontSize(fontSize.toInt());
    styleManager->UpdateWorkbenchFont();
  }

  QObject* eventFilter = new QtGlobalEventFilter(qApp);
  qApp->installEventFilter(eventFilter);
}

}
