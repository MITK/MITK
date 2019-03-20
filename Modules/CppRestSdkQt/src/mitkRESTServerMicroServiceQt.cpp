#include "mitkRESTServerMicroServiceQt.h"
#include <mitkCommon.h>

mitk::RESTServerMicroServiceQt::RESTServerMicroServiceQt(web::uri uri) : RESTServerMicroService(uri)
{
}

mitk::RESTServerMicroServiceQt::~RESTServerMicroServiceQt() {}

void mitk::RESTServerMicroServiceQt::OpenListener()
{
  mitk::RESTServerMicroService::OpenListener();
}

void mitk::RESTServerMicroServiceQt::CloseListener()
{
  mitk::RESTServerMicroService::CloseListener();
}
