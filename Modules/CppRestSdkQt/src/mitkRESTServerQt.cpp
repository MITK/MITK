#include "mitkRESTServerQt.h"

mitk::RESTServerQt::RESTServerQt(const web::uri &uri) : RESTServer(uri)
{
}

mitk::RESTServerQt::~RESTServerQt() {}

void mitk::RESTServerQt::OpenListener()
{
  mitk::RESTServer::OpenListener();
}

void mitk::RESTServerQt::CloseListener()
{
  mitk::RESTServer::CloseListener();
}
