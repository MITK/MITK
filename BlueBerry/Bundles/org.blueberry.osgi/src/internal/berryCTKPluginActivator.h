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

#ifndef BERRYCTKPLUGINACTIVATOR_H
#define BERRYCTKPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>
#include <QObject>

#include <org_blueberry_osgi_Export.h>

namespace berry {

// We need to export this activator, because it is referenced
// in the templated method berry::ServiceRegistry::GetServiceById<>(...)
class BERRY_OSGI org_blueberry_osgi_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_blueberry_osgi")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static ctkPluginContext* getPluginContext();

private:

  static ctkPluginContext* context;

};

typedef org_blueberry_osgi_Activator CTKPluginActivator;

}

#endif // BERRYCTKPLUGINACTIVATOR_H
