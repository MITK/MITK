/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryQtWorkbenchAdvisor.h"
#include "berryQtAssistantUtil.h"
#include "../internal/berryQtGlobalEventFilter.h"
#include "../berryQtPreferences.h"

#include <berryPlatform.h>
#include <berryIPreferencesService.h>
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

  IPreferencesService::Pointer prefService = Platform::GetServiceRegistry().GetServiceById<IPreferencesService>(
      IPreferencesService::ID);
  IPreferences::Pointer prefs = prefService->GetSystemPreferences()->Node(QtPreferences::QT_STYLES_NODE);
  QString styleName = QString::fromStdString(prefs->Get(QtPreferences::QT_STYLE_NAME, ""));

  IQtStyleManager::Pointer styleManager = Platform::GetServiceRegistry().GetServiceById<IQtStyleManager>(IQtStyleManager::ID);
  styleManager->SetStyle(styleName);

  QObject* eventFilter = new QtGlobalEventFilter(qApp);
  qApp->installEventFilter(eventFilter);

  // character strings should be interpreted as UTF-8 encoded strings
  // e.g. plugin.xml files are UTF-8 encoded
  QTextCodec* utf8Codec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForCStrings(utf8Codec);
  QTextCodec::setCodecForTr(utf8Codec);
}

bool QtWorkbenchAdvisor::PreShutdown()
{
  berry::QtAssistantUtil::CloseAssistant();
  return true;
}

}
