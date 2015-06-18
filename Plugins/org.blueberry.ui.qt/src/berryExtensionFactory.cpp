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

#include "berryExtensionFactory.h"

#include "berryIConfigurationElement.h"

#include "berryCoreException.h"
#include "berryObjectString.h"
#include "berryPlatformUI.h"
#include "berryStatus.h"

#include "internal/berryQtStylePreferencePage.h"
#include "internal/dialogs/berryPerspectivesPreferencePage.h"

namespace berry {

const QString ExtensionFactory::STYLE_PREFERENCE_PAGE = "stylePreferencePage";
const QString ExtensionFactory::PERSPECTIVES_PREFERENCE_PAGE = "perspectivesPreferencePage";

ExtensionFactory::~ExtensionFactory()
{
}

QObject* ExtensionFactory::Create()
{
  if (STYLE_PREFERENCE_PAGE == id)
  {
    return Configure(new QtStylePreferencePage());
  }
  if (PERSPECTIVES_PREFERENCE_PAGE == id)
  {
    return Configure(new PerspectivesPreferencePage());
  }
//  if (SHOW_IN_CONTRIBUTION == id)
//  {
//    ShowInMenu showInMenu = new ShowInMenu();
//    return showInMenu;
//  }

  IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, PlatformUI::PLUGIN_ID(), 0,
                                     QString("Unknown id in data argument for ") + this->metaObject()->className(),
                                     BERRY_STATUS_LOC));
  throw CoreException(status);
}

void ExtensionFactory::SetInitializationData(const SmartPointer<IConfigurationElement>& config, const QString& propertyName, const Object::Pointer& data)
{
  if (ObjectString::Pointer strData = data.Cast<ObjectString>())
  {
    id = *strData;
  }
  else
  {
    IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, PlatformUI::PLUGIN_ID(), 0,
                                       QString("Data argument must be a String for ") + this->metaObject()->className(),
                                       BERRY_STATUS_LOC));
    throw CoreException(status);
  }
  this->config = config;
  this->propertyName = propertyName;
}

QObject*ExtensionFactory::Configure(QObject* obj)
{
  if (IExecutableExtension* execExt = qobject_cast<IExecutableExtension*>(obj))
  {
    execExt->SetInitializationData(config, propertyName, Object::Pointer());
  }
  return obj;
}

}
