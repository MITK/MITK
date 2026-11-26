/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkReportsPluginActivator_h
#define QmitkReportsPluginActivator_h

#include <mitkReportsPluginConfig.h>

#include <ctkPluginActivator.h>

#include <memory>

#if MITK_HAS_RESTAPI
#include <mitkRestHelpers.h>
#endif

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
#if MITK_HAS_RESTAPI
  /** \brief Offers the plugin's REST endpoints, if the application has a server. */
  std::unique_ptr<mitk::RestRouteRegistration> m_RestRoutes;
#endif
};

#endif
