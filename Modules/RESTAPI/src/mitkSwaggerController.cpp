/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSwaggerController.h"
#include <mitkLog.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <iterator>

namespace mitk
{

SwaggerController::SwaggerController()
{
  // Pre-load all static resources into the cache. After construction the cache
  // is read-only, so handler threads can access it concurrently without locking.
  auto* context = us::GetModuleContext();
  if (context == nullptr)
  {
    MITK_WARN << "SwaggerController: No module context available; documentation endpoints will return 404";
    return;
  }

  for (const auto& resourcePath : {"swagger/index.html",
                                    "swagger/swagger-ui.css",
                                    "swagger/swagger-ui-bundle.js",
                                    "openapi.json"})
  {
    us::ModuleResource resource = context->GetModule()->GetResource(resourcePath);
    if (!resource.IsValid())
    {
      MITK_WARN << "SwaggerController: Resource not found: " << resourcePath;
      continue;
    }

    us::ModuleResourceStream stream(resource, std::ios::binary);
    m_ResourceCache[resourcePath] = std::string((std::istreambuf_iterator<char>(stream)),
                                                std::istreambuf_iterator<char>{});
  }
}

std::string SwaggerController::LoadResource(const std::string& resourcePath) const
{
  const auto it = m_ResourceCache.find(resourcePath);
  if (it != m_ResourceCache.end())
  {
    return it->second;
  }
  return "";
}

void SwaggerController::HandleGET_docs(const httplib::Request& /*req*/, httplib::Response& res)
{
  const std::string html = this->LoadResource("swagger/index.html");
  if (html.empty())
  {
    res.status = 404;
    res.set_content("Swagger UI resources not available", "text/plain");
    return;
  }

  res.status = 200;
  res.set_content(html, "text/html");
}

void SwaggerController::HandleGET_docs_css(const httplib::Request& /*req*/, httplib::Response& res)
{
  const std::string css = this->LoadResource("swagger/swagger-ui.css");
  if (css.empty())
  {
    res.status = 404;
    res.set_content("swagger-ui.css not found", "text/plain");
    return;
  }

  res.status = 200;
  res.set_content(css, "text/css");
}

void SwaggerController::HandleGET_docs_js(const httplib::Request& /*req*/, httplib::Response& res)
{
  const std::string js = this->LoadResource("swagger/swagger-ui-bundle.js");
  if (js.empty())
  {
    res.status = 404;
    res.set_content("swagger-ui-bundle.js not found", "text/plain");
    return;
  }

  res.status = 200;
  res.set_content(js, "application/javascript");
}

void SwaggerController::HandleGET_openapi(const httplib::Request& /*req*/, httplib::Response& res)
{
  const std::string spec = this->LoadResource("openapi.json");
  if (spec.empty())
  {
    res.status = 404;
    res.set_content("openapi.json not found", "text/plain");
    return;
  }

  res.status = 200;
  res.set_content(spec, "application/json");
}

}
