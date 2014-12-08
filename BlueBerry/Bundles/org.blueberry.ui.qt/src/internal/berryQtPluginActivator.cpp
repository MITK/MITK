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

#include "berryQtPluginActivator.h"

#include "berryQtStyleManager.h"

#include "berryQtDnDTweaklet.h"
#include "berryQtImageTweaklet.h"
#include "berryQtMessageDialogTweaklet.h"
#include "berryQtWorkbenchTweaklet.h"
#include "berryQtWorkbenchPageTweaklet.h"
#include "berryQtWidgetsTweaklet.h"
#include "berryQtStylePreferencePage.h"
#include "defaultpresentation/berryQtWorkbenchPresentationFactory.h"

namespace berry {

QtPluginActivator::QtPluginActivator()
{

}

QtPluginActivator::~QtPluginActivator()
{

}

void
QtPluginActivator::start(ctkPluginContext* context)
{
  AbstractUICTKPlugin::start(context);

  BERRY_REGISTER_EXTENSION_CLASS(QtDnDTweaklet, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtImageTweaklet, context);
  BERRY_REGISTER_EXTENSION_CLASS(QtMessageDialogTweaklet, context);
  BERRY_REGISTER_EXTENSION_CLASS(QtWidgetsTweaklet, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtWorkbenchTweaklet, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtWorkbenchPageTweaklet, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtWorkbenchPresentationFactory, context)

  BERRY_REGISTER_EXTENSION_CLASS(QtStylePreferencePage, context)

  QtStyleManager* manager = new QtStyleManager();
  styleManager = IQtStyleManager::Pointer(manager);
  context->registerService<berry::IQtStyleManager>(manager);

}

void QtPluginActivator::stop(ctkPluginContext* context)
{
  styleManager = 0;

  AbstractUICTKPlugin::stop(context);
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_blueberry_ui_qt, berry::QtPluginActivator)
#endif
