/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPluginActivator_h
#define mitkPluginActivator_h

#include <mitkMeasurementToolboxPluginConfig.h>

#include <ctkPluginActivator.h>

#include <memory>

#if MITK_HAS_RESTAPI
#include <mitkRestHelpers.h>
#endif

namespace mitk
{
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

#if MITK_HAS_RESTAPI
    /** \brief Offers the plugin's REST endpoints, if the application has a server. */
    std::unique_ptr<RestRouteRegistration> m_RestRoutes;
#endif

    /** \brief Parent of what the REST endpoints keep in the GUI thread, like the timers that end pulses; goes with them. */
    std::unique_ptr<QObject> m_RestRouteParent;
  };
}

#endif
