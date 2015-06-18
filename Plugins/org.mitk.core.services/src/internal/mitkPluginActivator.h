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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_mitk_core_services")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  static const std::string PLUGIN_ID;

  org_mitk_core_services_Activator();
  ~org_mitk_core_services_Activator();

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
