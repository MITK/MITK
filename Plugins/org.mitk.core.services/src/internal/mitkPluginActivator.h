/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCORESERVICESPLUGIN_H_
#define MITKCORESERVICESPLUGIN_H_

#include <ctkPluginActivator.h>

#include <berrySmartPointer.h>

#include <usServiceEvent.h>

namespace us
{
class ModuleContext;
}

namespace mitk
{

class DataStorageService;

class org_mitk_core_services_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_core_services")
  Q_INTERFACES(ctkPluginActivator)

public:

  static const std::string PLUGIN_ID;

  org_mitk_core_services_Activator();
  ~org_mitk_core_services_Activator() override;

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

  void MitkServiceChanged(const us::ServiceEvent event);

private:

  QScopedPointer<DataStorageService> dataStorageService;

  QMap<long, QObject*> mapMitkIdToAdapter;
  QMap<long, ctkServiceRegistration> mapMitkIdToRegistration;

  us::ModuleContext* mitkContext;
  ctkPluginContext* pluginContext;

  void AddMitkService(const us::ServiceReferenceU& ref);

  ctkDictionary CreateServiceProperties(const us::ServiceReferenceU& ref);
};

typedef org_mitk_core_services_Activator PluginActivator;

}

#endif /*MITKCORESERVICESPLUGIN_H_*/
