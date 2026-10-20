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

#include <cstdint>
#include <vector>

namespace mitk
{
  struct IRestServerService;

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

    /** \brief The server that offers the plugin's REST endpoints, if there is one. */
    IRestServerService* m_RestServer = nullptr;
    std::vector<std::uint64_t> m_RestRoutes;
  };
}

#endif
