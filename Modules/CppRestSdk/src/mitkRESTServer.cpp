#include "mitkRESTServer.h"

#include <mitkCommon.h>

mitk::RESTServer::RESTServer()
{
}

mitk::RESTServer::RESTServer(utility::string_t url) : m_Listener(url)
{
}

mitk::RESTServer::~RESTServer()
{
}

void mitk::RESTServer::HandleError(pplx::task<void>& t)
{
  try
  {
    t.get();
  }
  catch (...)
  {
    mitkThrow() << "An error occured.";
  }
}