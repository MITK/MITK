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

// Parent classes
#include <berryAbstractUICTKPlugin.h>

namespace mitk
{
  class PluginActivator : public berry::AbstractUICTKPlugin
  {
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_segmentation")
#endif
    Q_INTERFACES(ctkPluginActivator)

  public:

    PluginActivator();
    ~PluginActivator();

    void start(ctkPluginContext *context) override;
    void stop(ctkPluginContext *context) override;

    static PluginActivator* getDefault();

    static ctkPluginContext* getContext();

  private:

    static ctkPluginContext* m_context;
    static PluginActivator* m_Instance;
  };
}

#endif
