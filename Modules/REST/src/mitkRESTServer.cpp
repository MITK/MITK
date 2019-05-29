/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkRESTServer.h>

#include <mitkIRESTManager.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <cpprest/http_listener.h>

using namespace std::placeholders;

using http_listener = web::http::experimental::listener::http_listener;
using http_request = web::http::http_request;
using methods = web::http::methods;
using status_codes = web::http::status_codes;

namespace mitk
{
  class RESTServer::Impl
  {
  public:
    Impl(const web::uri &uri);
    ~Impl();

    void HandleGet(const http_request &request);

    web::http::experimental::listener::http_listener listener;
    web::uri uri;
  };

  RESTServer::Impl::Impl(const web::uri &uri)
    : uri{uri}
  {
  }

  RESTServer::Impl::~Impl()
  {
  }

  void RESTServer::Impl::HandleGet(const http_request &request)
  {
    web::uri_builder builder(this->listener.uri());
    builder.append(request.absolute_uri());

    auto uriString = builder.to_uri().to_string();

    web::json::value content;

    auto context = us::GetModuleContext();
    auto managerRef = context->GetServiceReference<IRESTManager>();

    if (managerRef)
    {
      auto manager = context->GetService(managerRef);
      if (manager)
      {
        auto data = request.extract_json().get();
        content = manager->Handle(builder.to_uri(), data);
      }
    }

    request.reply(content.is_null()
      ? status_codes::NotFound
      : status_codes::OK);
  }
}


mitk::RESTServer::RESTServer(const web::uri &uri)
  : m_Impl{std::make_unique<Impl>(uri)}
{
}

mitk::RESTServer::~RESTServer()
{
}

void mitk::RESTServer::OpenListener()
{
  m_Impl->listener = http_listener(m_Impl->uri);
  m_Impl->listener.support(methods::GET, std::bind(&Impl::HandleGet, m_Impl.get(), _1));
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
