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


#ifndef BERRYPLUGINACTIVATOR_H
#define BERRYPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace berry {

class org_blueberry_core_commands_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_blueberry_core_commands")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:
  org_blueberry_core_commands_Activator();

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);
};

typedef org_blueberry_core_commands_Activator PluginActivator;

}

#endif // BERRYPLUGINACTIVATOR_H
