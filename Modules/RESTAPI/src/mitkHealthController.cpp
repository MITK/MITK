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

void HealthController::SetUptimeCallback(UptimeCallback callback)
{
  m_UptimeCallback = std::move(callback);
}

void HealthController::HandleGET_health(const httplib::Request& /*req*/, httplib::Response& res)
{
  nlohmann::json response;
  response["data"]["status"] = "healthy";

  nlohmann::json checks;
  checks["datastorage"] = m_Bridge.HasDataStorage() ? "ok" : "unavailable";
  // Note: rendering check is planned for a future version
  response["data"]["checks"] = checks;

  // Add uptime if available
  if (m_UptimeCallback)
  {
    auto uptime = m_UptimeCallback();
    if (uptime.has_value())
    {
      response["data"]["uptime_seconds"] = uptime.value();
    }
  }

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
  response["data"]["documentation_url"] = "https://docs.mitk.org/api/v1";

  nlohmann::json capabilities;
  capabilities["transfer_modes"] = nlohmann::json::array({"direct", "file-reference"});
  capabilities["authentication"] = nlohmann::json::array({"api-token"});

  response["data"]["capabilities"] = capabilities;

  res.status = 200;
  res.set_content(response.dump(), "application/json");
}

}
