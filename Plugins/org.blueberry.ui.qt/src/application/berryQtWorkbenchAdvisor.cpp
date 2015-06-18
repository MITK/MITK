/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQtWorkbenchAdvisor.h"
#include "internal/berryQtGlobalEventFilter.h"
#include "internal/berryWorkbenchPlugin.h"
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

  ctkServiceReference serviceRef = WorkbenchPlugin::GetDefault()->GetPluginContext()->getServiceReference<IQtStyleManager>();
  if (serviceRef)
  {
    IQtStyleManager* styleManager = WorkbenchPlugin::GetDefault()->GetPluginContext()->getService<IQtStyleManager>(serviceRef);
    styleManager->SetStyle(styleName);
  }

  QObject* eventFilter = new QtGlobalEventFilter(qApp);
  qApp->installEventFilter(eventFilter);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  // character strings should be interpreted as UTF-8 encoded strings
  // e.g. plugin.xml files are UTF-8 encoded
  QTextCodec* utf8Codec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForCStrings(utf8Codec);
  QTextCodec::setCodecForTr(utf8Codec);
#endif
}

}
