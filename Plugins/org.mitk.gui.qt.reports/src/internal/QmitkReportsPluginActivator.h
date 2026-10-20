/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkReportsPluginActivator_h
#define QmitkReportsPluginActivator_h

#include <ctkPluginActivator.h>

#include <cstdint>
#include <vector>

namespace mitk
{
  struct IRestServerService;
}

class QmitkReportStore;

class QmitkReportsPluginActivator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_reports")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

  /** \brief The reports of the session, or nullptr while the plugin is not active. */
  static QmitkReportStore* GetStore();

private:
  /** \brief The server that offers the plugin's REST endpoints, if there is one. */
  mitk::IRestServerService* m_RestServer = nullptr;
  std::vector<std::uint64_t> m_RestRoutes;
};

#endif
