#include "mitkRESTClient.h"
#include "mitkRESTUtil.h"
#include <mitkCommon.h>

mitk::RESTClient::RESTClient() {}

mitk::RESTClient::~RESTClient() {}

pplx::task<web::json::value> mitk::RESTClient::Get(const web::uri &uri)
{
  //Create new HTTP client
  MitkClient *client = new MitkClient(uri);

  MITK_INFO << "Calling GET with " << mitk::RESTUtil::convertToUtf8(uri.path()) << " on client "
            << mitk::RESTUtil::convertToUtf8(uri.authority().to_string());

  //create get request
  MitkRequest getRequest(MitkRESTMethods::GET);

  //make request
  return client->request(getRequest).then([=](pplx::task<MitkResponse> responseTask) {
    try
    {
      //get response of the request
      MitkResponse response = responseTask.get();
      auto status = response.status_code();
      MITK_INFO << " status: " << status;

      if (status != MitkRestStatusCodes::OK)
      {
        //throw if something went wrong (e.g. invalid uri)
        //this exception can be handled by client
        mitkThrow() << "response was not OK";
      }
      try
      {
        //parse content type to application/json if it isn't already
        //this is important if the content type is e.g. application/dicom+json
        utility::string_t requestContentType = response.headers().content_type();
        if (requestContentType != L"application/json")
        {
          response.headers().set_content_type(L"application/json");
        }
        //return json answer
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

pplx::task<web::json::value> mitk::RESTClient::Get(const web::uri &uri, const utility::string_t &filePath)
{
  // Create new HTTP client
  MitkClient *client = new MitkClient(uri);

  MITK_INFO << "Calling GET with " << mitk::RESTUtil::convertToUtf8(uri.path()) << " on client "
            << mitk::RESTUtil::convertToUtf8(uri.authority().to_string()) << " save into "
             << mitk::RESTUtil::convertToUtf8(filePath);
  
  //create new file buffer
  auto fileBuffer = std::make_shared<concurrency::streams::streambuf<uint8_t>>();
  // create get request
  MitkRequest getRequest(MitkRESTMethods::GET);

  //open file stream for the specified file path
  return concurrency::streams::file_buffer<uint8_t>::open(filePath, std::ios::out)
    .then([=](concurrency::streams::streambuf<uint8_t> outFile) -> pplx::task<MitkResponse> {
      *fileBuffer = outFile;
      //make the get request
      return client->request(MitkRESTMethods::GET);
    })
    // Write the response body into the file buffer.
    .then([=](MitkResponse response) -> pplx::task<size_t> {
      auto status = response.status_code();
      MITK_DEBUG << "Status code: " << status;

      if (status != web::http::status_codes::OK)
      {
        // throw if something went wrong (e.g. invalid uri)
        // this exception can be handled by client
        mitkThrow() << "GET ended up with response " << mitk::RESTUtil::convertToUtf8(response.to_string());
      }
      
      return response.body().read_to_end(*fileBuffer);
    })
    // Close the file buffer.
    .then([=](size_t) { return fileBuffer->close(); })
    .then([=]() {
      //return empty json object
      web::json::value data;
      return data;
    });
}

pplx::task<web::json::value> mitk::RESTClient::PUT(const web::uri &uri, const web::json::value &content)
{
  // Create new HTTP client
  MitkClient *client = new MitkClient(uri);

  MITK_INFO << "Calling PUT with " << mitk::RESTUtil::convertToUtf8(uri.path()) << " on client "
            << mitk::RESTUtil::convertToUtf8(uri.authority().to_string());

  // create put request
  MitkRequest putRequest(MitkRESTMethods::PUT);
  //set body of the put request with data given by client
  if (content != NULL)
  {
    putRequest.set_body(content);
  } 
  //make put request
  return client->request(putRequest).then([=](pplx::task<MitkResponse> responseTask) {
    try
    {
      // get response of the request 
      MitkResponse response = responseTask.get();
      auto status = response.status_code();
      MITK_INFO << " status: " << status;
      if (status != MitkRestStatusCodes::OK)
      {
        // throw if something went wrong (e.g. invalid uri)
        // this exception can be handled by client
        mitkThrow() << "response was not OK";
      }

      try
      {
        // parse content type to application/json if it isn't already
        // this is important if the content type is e.g. application/dicom+json
        utility::string_t requestContentType = response.headers().content_type();
        if (requestContentType != L"application/json")
        {
          response.headers().set_content_type(L"application/json");
        }
        // return json answer
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

pplx::task<web::json::value> mitk::RESTClient::POST(const web::uri &uri, const web::json::value &content)
{
  // Create new HTTP client
  MitkClient *client = new MitkClient(uri);
  MITK_INFO << "Calling POST with " << mitk::RESTUtil::convertToUtf8(uri.path()) << " on client "
            << mitk::RESTUtil::convertToUtf8(uri.authority().to_string());

  // Create post request
  MitkRequest postRequest(MitkRESTMethods::POST);
  // set body of the put request with data given by client
  if (content != NULL)
  {
    postRequest.set_body(content);
  }

  //make post request
  return client->request(postRequest).then([=](pplx::task<MitkResponse> responseTask) {
    try
    {
      // get response of the request 
      MitkResponse response = responseTask.get();
      auto status = response.status_code();
      MITK_INFO << " status: " << status;

      if (status != MitkRestStatusCodes::Created)
      {
        // throw if something went wrong (e.g. invalid uri)
        // this exception can be handled by client
        mitkThrow() << "response was not Created";
      }

      try
      {
        // parse content type to application/json if it isn't already
        // this is important if the content type is e.g. application/dicom+json
        utility::string_t requestContentType = response.headers().content_type();
        if (requestContentType != L"application/json")
        {
          response.headers().set_content_type(L"application/json");
        }
        // return json answer
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
