#include "mitkRESTServerMicroServiceQt.h"

mitk::RESTServerMicroServiceQt::RESTServerMicroServiceQt(const web::uri &uri) : RESTServerMicroService(uri)
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
