/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPluginActivator_h
#define mitkPluginActivator_h

// Parent classes
#include <berryAbstractUICTKPlugin.h>

#include <mitkSegmentationPluginConfig.h>

#include <memory>

#if MITK_HAS_RESTAPI
#include <mitkRestHelpers.h>
#endif

namespace mitk
{
  class PluginActivator : public berry::AbstractUICTKPlugin
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_segmentation")
    Q_INTERFACES(ctkPluginActivator)

  public:

    PluginActivator();
    ~PluginActivator() override;

    void start(ctkPluginContext *context) override;
    void stop(ctkPluginContext *context) override;

    static PluginActivator* getDefault();

    static ctkPluginContext* getContext();

  private:

    static ctkPluginContext* m_context;
    static PluginActivator* m_Instance;

#if MITK_HAS_RESTAPI
    /** \brief Offers the plugin's REST endpoints, if the application has a server. */
    std::unique_ptr<RestRouteRegistration> m_RestRoutes;
#endif
  };
}

#endif
