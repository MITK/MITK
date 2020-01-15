/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIRESTManager.h>
#include <mitkRESTServer.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <cpprest/http_listener.h>

using namespace std::placeholders;

using http_listener = web::http::experimental::listener::http_listener;
using http_request = web::http::http_request;
using http_response = web::http::http_response;
using methods = web::http::methods;
using status_codes = web::http::status_codes;

namespace mitk
{
  class RESTServer::Impl
  {
  public:
    Impl(const web::uri &uri);
    ~Impl();

    void HandleRequest(const http_request &request);

    web::http::experimental::listener::http_listener listener;
    web::uri uri;
  };

  RESTServer::Impl::Impl(const web::uri &uri) : uri{uri} {}

  RESTServer::Impl::~Impl() {}

  void RESTServer::Impl::HandleRequest(const http_request &request)
  {
    web::uri_builder builder(this->listener.uri());
    builder.append(request.absolute_uri());

    auto uriString = builder.to_uri().to_string();

    http_response response(status_codes::InternalError);
    response.set_body(U("There went something wrong after receiving the request."));

    auto context = us::GetModuleContext();
    auto managerRef = context->GetServiceReference<IRESTManager>();

    if (managerRef)
    {
      auto manager = context->GetService(managerRef);
      if (manager)
      {
        // not every request contains JSON data
        web::json::value data = {};
        if (request.headers().content_type() == U("application/json"))
        {
          data = request.extract_json().get();
        }

        mitk::RESTUtil::ParamMap headers;
        auto begin = request.headers().begin();
        auto end = request.headers().end();

        for (; begin != end; ++begin)
        {
          headers.insert(mitk::RESTUtil::ParamMap::value_type(begin->first, begin->second));
        }

        response = manager->Handle(builder.to_uri(), data, request.method(), headers);
      }
    }

    request.reply(response);
  }
} // namespace mitk

mitk::RESTServer::RESTServer(const web::uri &uri) : m_Impl{std::make_unique<Impl>(uri)} {}

mitk::RESTServer::~RESTServer() {}

void mitk::RESTServer::OpenListener()
{
  m_Impl->listener = http_listener(m_Impl->uri);
  m_Impl->listener.support(std::bind(&Impl::HandleRequest, m_Impl.get(), _1));
  m_Impl->listener.support(methods::OPTIONS, std::bind(&Impl::HandleRequest, m_Impl.get(), _1));
  m_Impl->listener.open().wait();
}

void mitk::RESTServer::CloseListener()
{
  m_Impl->listener.close().wait();
}

web::uri mitk::RESTServer::GetUri()
{
  return m_Impl->uri;
}
