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
#include <berryIQtStyleManager.h>
#include <berryQtPreferences.h>

#include <QApplication>
#include <QString>
#include <QTextCodec>

#include <Poco/File.h>
#include <Poco/FileStream.h>

#include <vector>

#include <mitkIPreferences.h>

namespace berry
{

void QtWorkbenchAdvisor::Initialize(IWorkbenchConfigurer::Pointer configurer)
{
  WorkbenchAdvisor::Initialize(configurer);

  auto* prefs = WorkbenchPlugin::GetDefault()->GetPreferences()->Node(QtPreferences::QT_STYLES_NODE);
  auto styleName = QString::fromStdString(prefs->Get(QtPreferences::QT_STYLE_NAME, ""));
  auto fontName = QString::fromStdString(prefs->Get(QtPreferences::QT_FONT_NAME, "Open Sans"));
  auto fontSize = QString::fromStdString(prefs->Get(QtPreferences::QT_FONT_SIZE, "9"));

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
