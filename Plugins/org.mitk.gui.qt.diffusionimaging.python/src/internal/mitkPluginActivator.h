/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef MITKPLUGINACTIVATOR_H
#define MITKPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk
{
  class PluginActivator : public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.mitk.gui.qt.diffusionimaging.preprocessing")
    Q_INTERFACES(ctkPluginActivator)

  public:
    static ctkPluginContext* GetContext();

    void start(ctkPluginContext* context) override;
    void stop(ctkPluginContext* context) override;

  private:
    static ctkPluginContext* m_Context;
  };
}

#endif
