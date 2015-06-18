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


#include "berryPluginActivator.h"

#include "berryObjectBrowserView.h"

#include <QtPlugin>

namespace berry {

void org_blueberry_ui_qt_objectinspector_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(berry::ObjectBrowserView, context)
}

void org_blueberry_ui_qt_objectinspector_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_blueberry_ui_qt_objectinspector, berry::org_blueberry_ui_qt_objectinspector_Activator)
#endif
