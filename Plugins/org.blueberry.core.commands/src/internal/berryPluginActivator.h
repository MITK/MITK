/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYPLUGINACTIVATOR_H
#define BERRYPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace berry {

class org_blueberry_core_commands_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_blueberry_core_commands")
  Q_INTERFACES(ctkPluginActivator)

public:
  org_blueberry_core_commands_Activator();

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;
};

typedef org_blueberry_core_commands_Activator PluginActivator;

}

#endif // BERRYPLUGINACTIVATOR_H
