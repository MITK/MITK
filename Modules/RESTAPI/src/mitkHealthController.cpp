/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkHealthController.h"
#include <mitkVersion.h>

namespace mitk
{

HealthController::HealthController(DataStorageBridge& bridge)
  : m_Bridge(bridge)
{
}

void HealthController::HandleGET_health(const httplib::Request& /*req*/, httplib::Response& res)
{
  nlohmann::json response;
  response["data"]["status"] = "healthy";

  nlohmann::json checks;
  checks["datastorage"] = m_Bridge.HasDataStorage() ? "ok" : "unavailable";
  response["data"]["checks"] = checks;

  res.status = 200;
  res.set_content(response.dump(), "application/json");
}

void HealthController::HandleGET_info(const httplib::Request& /*req*/, httplib::Response& res)
{
  nlohmann::json response;

  response["data"]["name"] = "MITK Workbench REST API";
  response["data"]["version"] = "1.0.0";
  response["data"]["api_version"] = "v1";
  response["data"]["mitk_version"] = MITK_VERSION_STRING;

  nlohmann::json capabilities;
  capabilities["transfer_modes"] = nlohmann::json::array({"direct", "file-reference"});
  capabilities["authentication"] = nlohmann::json::array({"api-token"});

  response["data"]["capabilities"] = capabilities;
  response["data"]["datastorage_available"] = m_Bridge.HasDataStorage();

  res.status = 200;
  res.set_content(response.dump(), "application/json");
}

}
