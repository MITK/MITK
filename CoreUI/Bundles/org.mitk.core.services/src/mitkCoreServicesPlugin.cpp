#include "mitkCoreServicesPlugin.h"

#include "internal/mitkDataStorageService.h"

namespace mitk
{

const std::string CoreServicesPlugin::PLUGIN_ID = "org.mitk.core.services";

void CoreServicesPlugin::Start(berry::IBundleContext::Pointer context)
{
  DataStorageService::Pointer service(new DataStorageService());
  context->RegisterService(IDataStorageService::ID, service);
}

}
