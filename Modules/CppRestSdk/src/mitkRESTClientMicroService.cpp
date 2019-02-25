#include "mitkRESTClientMicroService.h"
#include "mitkRESTUtil.h"
#include <mitkCommon.h>

mitk::RESTClientMicroService::RESTClientMicroService() {}

mitk::RESTClientMicroService::~RESTClientMicroService() {}

pplx::task<web::json::value> mitk::RESTClientMicroService::Get(web::uri uri)
{
  MitkClient *client = new MitkClient(uri);

  MITK_INFO << "Calling GET with " << utility::conversions::to_utf8string(uri.path()) << " on client "
            << mitk::RESTUtil::convertToUtf8(uri.authority().to_string());

  MitkRequest getRequest(MitkRESTMethods::GET);

  return client->request(getRequest).then([=](pplx::task<MitkResponse> responseTask) {
    try
    {
      MitkResponse response = responseTask.get();
      auto status = response.status_code();
      MITK_INFO << " status: " << status;

      if (status != MitkRestStatusCodes::OK)
      {
        mitkThrow() << "response was not OK";
      }
      try
      {
        utility::string_t requestContentType = response.headers().content_type();
        if (requestContentType != L"application/json")
        {
          response.headers().set_content_type(L"application/json");
        }
        return response.extract_json().get();
      }
      catch (...)
      {
        mitkThrow() << "extracting json went wrong";
      }
    }
    catch (...)
    {
      mitkThrow() << "getting response went wrong";
    }
  });
}

pplx::task<web::json::value> mitk::RESTClientMicroService::Get(web::uri uri, utility::string_t filePath)
{
  MitkClient *client = new MitkClient(uri);
  MITK_INFO << "Calling GET with " << utility::conversions::to_utf8string(uri.path()) << " on client "
            << mitk::RESTUtil::convertToUtf8(uri.authority().to_string()) << " save into "
             << mitk::RESTUtil::convertToUtf8(filePath);

  auto fileBuffer = std::make_shared<concurrency::streams::streambuf<uint8_t>>();
  MitkRequest getRequest(MitkRESTMethods::GET);
  return concurrency::streams::file_buffer<uint8_t>::open(filePath, std::ios::out)
    .then([=](concurrency::streams::streambuf<uint8_t> outFile) -> pplx::task<MitkResponse> {
      *fileBuffer = outFile;

      return client->request(MitkRESTMethods::GET);
    })
    // Write the response body into the file buffer.
    .then([=](MitkResponse response) -> pplx::task<size_t> {
      auto status = response.status_code();
      MITK_DEBUG << "Status code: " << status;

      if (status != web::http::status_codes::OK)
      {
        mitkThrow() << "GET ended up with response " << mitk::RESTUtil::convertToUtf8(response.to_string());
      }
      
      return response.body().read_to_end(*fileBuffer);
    })
    // Close the file buffer.
    .then([=](size_t) { return fileBuffer->close(); })
    .then([=]() {
      web::json::value data;
      return data;
    });
}

pplx::task<web::json::value> mitk::RESTClientMicroService::PUT(web::uri uri, web::json::value content)
{
  MitkClient *client = new MitkClient(uri);
  MITK_INFO << "Calling PUT with " << utility::conversions::to_utf8string(uri.path()) << " on client "
            << mitk::RESTUtil::convertToUtf8(uri.authority().to_string());

  MitkRequest putRequest(MitkRESTMethods::PUT);
  putRequest.set_body(content);
  return client->request(putRequest).then([=](pplx::task<MitkResponse> responseTask) {
    try
    {
      MitkResponse response = responseTask.get();
      auto status = response.status_code();
      MITK_INFO << " status: " << status;
      if (status != MitkRestStatusCodes::OK)
      {
        mitkThrow() << "response was not OK";
      }

      try
      {
        utility::string_t requestContentType = response.headers().content_type();
        if (requestContentType != L"application/json")
        {
          response.headers().set_content_type(L"application/json");
        }
        return response.extract_json().get();
      }
      catch (...)
      {
        mitkThrow() << "extracting json went wrong";
      }
    }
    catch (...)
    {
      mitkThrow() << "getting response went wrong";
    }
  });
}

pplx::task<web::json::value> mitk::RESTClientMicroService::POST(web::uri uri, web::json::value content)
{
  MitkClient *client = new MitkClient(uri);
  MITK_INFO << "Calling POST with " << utility::conversions::to_utf8string(uri.path()) << " on client "
            << mitk::RESTUtil::convertToUtf8(uri.authority().to_string());

  MitkRequest postRequest(MitkRESTMethods::POST);
  if (content != NULL)
  {
    postRequest.set_body(content);
  }

  return client->request(postRequest).then([=](pplx::task<MitkResponse> responseTask) {
    try
    {
      MitkResponse response = responseTask.get();
      auto status = response.status_code();
      MITK_INFO << " status: " << status;

      if (status != MitkRestStatusCodes::Created)
      {
        mitkThrow() << "response was not Created";
      }

      try
      {
        utility::string_t requestContentType = response.headers().content_type();
        if (requestContentType != L"application/json")
        {
          response.headers().set_content_type(L"application/json");
        }
        return response.extract_json().get();
      }
      catch (...)
      {
        mitkThrow() << "extracting json went wrong";
      }
    }
    catch(...)
    {
      mitkThrow() << "getting response went wrong";
    }
  });
}
