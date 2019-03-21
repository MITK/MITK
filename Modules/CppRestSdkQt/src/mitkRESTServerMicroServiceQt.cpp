#include "mitkRESTServerMicroServiceQt.h"

mitk::RESTServerMicroServiceQt::RESTServerMicroServiceQt(const web::uri &uri) : RESTServer(uri)
{
}

mitk::RESTServerMicroServiceQt::~RESTServerMicroServiceQt() {}

void mitk::RESTServerMicroServiceQt::OpenListener()
{
  mitk::RESTServer::OpenListener();
}

void mitk::RESTServerMicroServiceQt::CloseListener()
{
  mitk::RESTServer::CloseListener();
}
