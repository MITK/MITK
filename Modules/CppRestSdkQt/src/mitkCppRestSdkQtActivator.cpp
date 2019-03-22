#include "mitkCppRestSdkQtActivator.h"
#include <mitkCommon.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleInitialization.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <usModuleSettings.h>
#include <QCoreApplication>

void MitkCppRestSdkQtActivator::Load(us::ModuleContext *context)
{
  // Registration of the RESTManagerMicroservice
  m_RESTManagerQt.reset(new mitk::RESTManagerQt);
 
  us::ServiceProperties props;
  //The standard RESTManager which is used for non-qt applications has a ranking of 5
  if (nullptr != QCoreApplication::instance)
  {
    props[us::ServiceConstants::SERVICE_RANKING()] = 10;
  }
  else
  {
    props[us::ServiceConstants::SERVICE_RANKING()] = 0;
  }
  context->RegisterService<mitk::IRESTManager>(m_RESTManagerQt.get(),props);
}

void MitkCppRestSdkQtActivator::Unload(us::ModuleContext *) {}

US_EXPORT_MODULE_ACTIVATOR(MitkCppRestSdkQtActivator)
