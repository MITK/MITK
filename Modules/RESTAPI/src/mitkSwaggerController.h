/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSwaggerController_h
#define mitkSwaggerController_h

#include <httplib.h>

#include <MitkRESTAPIExports.h>

#include <string>
#include <unordered_map>

namespace mitk
{
  /**
   * \brief Serves Swagger UI and OpenAPI specification for API documentation.
   *
   * Endpoints:
   * - GET /api/v1/docs             -> Swagger UI HTML page
   * - GET /api/v1/docs/swagger-ui.css       -> Swagger UI stylesheet
   * - GET /api/v1/docs/swagger-ui-bundle.js -> Swagger UI JavaScript bundle
   * - GET /api/v1/openapi.json      -> OpenAPI 3.0 specification
   *
   * All assets are loaded from CppMicroServices bundled resources and cached
   * in memory after first access.
   */
  class MITKRESTAPI_EXPORT SwaggerController
  {
  public:
    SwaggerController();

    /** \brief Handle GET /docs - serves Swagger UI HTML page. */
    void HandleGET_docs(const httplib::Request& req, httplib::Response& res);

    /** \brief Handle GET /docs/swagger-ui.css - serves Swagger UI stylesheet. */
    void HandleGET_docs_css(const httplib::Request& req, httplib::Response& res);

    /** \brief Handle GET /docs/swagger-ui-bundle.js - serves Swagger UI JavaScript. */
    void HandleGET_docs_js(const httplib::Request& req, httplib::Response& res);

    /** \brief Handle GET /openapi.json - serves the OpenAPI specification. */
    void HandleGET_openapi(const httplib::Request& req, httplib::Response& res);

  private:
    /**
     * \brief Load a CppMicroServices bundled resource as a string.
     *
     * All resources are pre-loaded into m_ResourceCache in the constructor,
     * so this method is read-only after construction and safe to call from
     * concurrent handler threads without additional locking.
     *
     * \pre Resource must exist in the module bundle.
     * \param resourcePath Path within the resource bundle.
     * 
eturn Resource content, or empty string if not found.
     */
    std::string LoadResource(const std::string& resourcePath) const;

    /** \brief Read-only after construction; safe for concurrent handler thread access. */
    std::unordered_map<std::string, std::string> m_ResourceCache;
  };
}

#endif
