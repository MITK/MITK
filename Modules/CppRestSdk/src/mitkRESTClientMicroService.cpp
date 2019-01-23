#include "mitkRESTClientMicroService.h"
#include <mitkCommon.h>

mitk::RESTClientMicroService::RESTClientMicroService(utility::string_t url)
{

}

mitk::RESTClientMicroService::~RESTClientMicroService() 
{
}

void mitk::RESTClientMicroService::TestFunctionClient()
{
  MITK_INFO << "Test for Client";
}