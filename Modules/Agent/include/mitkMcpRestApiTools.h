/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMcpRestApiTools_h
#define mitkMcpRestApiTools_h

#include <MitkAgentExports.h>

namespace mitk
{
  class McpEndpoint;
  struct IRestServerService;

  /**
   * \brief Add the tools api_get and api_call, which make requests to the
   *        REST API of a server on behalf of the MCP client.
   *
   * The requests carry the client's Authorization header, so they have the
   * rights of the client's token. JSON and text responses become text,
   * images become images.
   *
   * \param server Must outlive the endpoint's offer.
   */
  MITKAGENT_EXPORT void AddRestApiTools(McpEndpoint& endpoint, IRestServerService* server);
}

#endif
