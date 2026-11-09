/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPluginActivator_h
#define mitkPluginActivator_h

#include <ctkPluginActivator.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace mitk
{
  struct IRestServerService;

  class PluginActivator : public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.mitk.gui.qt.measurementtoolbox")
    Q_INTERFACES(ctkPluginActivator)

  public:
    static ctkPluginContext* GetContext();

    void start(ctkPluginContext* context) override;
    void stop(ctkPluginContext* context) override;

  private:
    static ctkPluginContext* m_Context;

    /** \brief The server that offers the plugin's REST endpoints, if there is one. */
    IRestServerService* m_RestServer = nullptr;
    std::vector<std::uint64_t> m_RestRoutes;

    /** \brief Parent of what the REST endpoints keep in the GUI thread, like the timers that end pulses; goes with them. */
    std::unique_ptr<QObject> m_RestRouteParent;
  };
}

#endif
