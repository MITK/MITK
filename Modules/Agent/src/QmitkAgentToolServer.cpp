/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkAgentToolServer.h>

#include <QmitkAgentMemory.h>

#include <mitkIRestServerService.h>
#include <mitkLog.h>
#include <mitkMcpEndpoint.h>
#include <mitkMcpRestApiTools.h>
#include <mitkRestGuiThread.h>

#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleRegistry.h>

#include <QCoreApplication>

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace
{
  const std::string McpPath = "/mcp";
  const std::string StartedBy = "Agent view";

  mitk::IRestServerService* GetRestServer()
  {
    auto* context = us::ModuleRegistry::GetModule(1)->GetModuleContext();
    const auto reference = context->GetServiceReference<mitk::IRestServerService>();

    return reference
      ? context->GetService(reference)
      : nullptr;
  }

  /** The server's URL for a client on the same machine. */
  QString GetLocalUrl(const mitk::RestServerConfig& config)
  {
    auto host = QString::fromStdString(config.host);

    if (host == "::" || host == "0.0.0.0" || host == "localhost")
    {
      host = "127.0.0.1";
    }
    else if (host.contains(':'))
    {
      host = '[' + host + ']';
    }

    return QString("%1://%2:%3").arg(config.httpsEnabled ? "https" : "http").arg(host).arg(config.port);
  }

  QString GetMcpUrl(const mitk::RestServerConfig& config)
  {
    return GetLocalUrl(config) + "/api/v1" + QString::fromStdString(McpPath);
  }
}

QmitkAgentToolServer::QmitkAgentToolServer(QmitkAgentMemory& memory)
  : m_Memory(&memory),
    m_RestServer(GetRestServer()),
    m_StartedRestServer(false)
{
}

QmitkAgentToolServer::~QmitkAgentToolServer()
{
  this->Withdraw();
}

QStringList QmitkAgentToolServer::GetRunningServerRisks() const
{
  if (m_RestServer == nullptr || !m_RestServer->IsRunning())
    return {};

  const auto config = m_RestServer->GetRunningConfig();

  if (!config.has_value())
    return {};

  QStringList risks;
  const bool remote = config->clientAccessMode != mitk::ClientAccessMode::LocalhostOnly;

  if (!config->requireAuth)
    risks << "Programs on this computer can use it without a token, and web pages in your browser can use parts of it.";

  if (remote)
    risks << "Other computers can reach it.";

  if (config->fileAccessMode == mitk::FileAccessMode::Unrestricted && (!config->requireAuth || remote))
    risks << "These clients can load any file you can read into the Workbench.";

  return risks;
}

QString QmitkAgentToolServer::Offer()
{
  if (this->IsOffered())
    return QString();

  if (m_RestServer == nullptr)
    return "The agent cannot work in the Workbench: this build of MITK has no REST API, through which the agent operates it.";

  if (!m_RestServer->IsRunning())
  {
    // A server for the agent only: the main token is empty.
    mitk::RestServerConfig config;
    config.enabled = true;
    config.host = "127.0.0.1";
    config.port = 0;
    config.clientAccessMode = mitk::ClientAccessMode::LocalhostOnly;
    config.requireAuth = true;
    config.apiToken.clear();
    config.fileAccessMode = mitk::FileAccessMode::AllowedDirectories;
    config.allowedFileDirectories.clear();
    config.httpsEnabled = false;

    if (!m_RestServer->Start(config, StartedBy))
    {
      const auto error = QString::fromStdString(m_RestServer->GetLastError().value_or("unknown error"));

      return QString("The agent cannot work in the Workbench, as the REST API server it works through does not start: "
        "%1. The log may tell more. You can also start the server yourself in the REST API view and reopen the Agent "
        "view.").arg(error);
    }

    m_StartedRestServer = true;
  }

  m_Endpoint = std::make_unique<mitk::McpEndpoint>("MITK Workbench", "0.2");
  mitk::AddRestApiTools(*m_Endpoint, m_RestServer);
  this->AddMemoryTools();

  try
  {
    m_Token = QString::fromStdString(m_RestServer->CreateToken("Agent view", mitk::RestTokenRights()));
    m_Endpoint->Offer(m_RestServer, McpPath);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << e.GetDescription();
    this->Withdraw();
    return QString("The Workbench tools are not available: %1").arg(e.GetDescription());
  }

  m_Url = GetMcpUrl(m_RestServer->GetRunningConfig().value());
  return QString();
}

void QmitkAgentToolServer::Withdraw()
{
  m_Url.clear();

  if (m_Endpoint != nullptr)
  {
    m_Endpoint->Withdraw();

    // Calls that run may wait for the GUI thread, which is this one.
    while (m_Endpoint->GetRunningCallCount() > 0)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 50);

    m_Endpoint.reset();
  }

  if (m_RestServer == nullptr)
    return;

  if (!m_Token.isEmpty())
  {
    m_RestServer->RemoveToken(m_Token.toStdString());
    m_Token.clear();
  }

  // The server that runs now may be one that the user started after this one.
  if (m_StartedRestServer && m_RestServer->GetStartedBy() == StartedBy)
    m_RestServer->Stop();

  m_StartedRestServer = false;
}

bool QmitkAgentToolServer::IsOffered() const
{
  return !m_Url.isEmpty();
}

bool QmitkAgentToolServer::HasLostServer() const
{
  if (!this->IsOffered())
    return false;

  const auto config = m_RestServer->IsRunning() ? m_RestServer->GetRunningConfig() : std::nullopt;

  return !config.has_value() || GetMcpUrl(*config) != m_Url;
}

QString QmitkAgentToolServer::GetUrl() const
{
  return m_Url;
}

QString QmitkAgentToolServer::GetToken() const
{
  return m_Token;
}

void QmitkAgentToolServer::RefuseCalls(const std::string& reason)
{
  if (m_Endpoint != nullptr)
    m_Endpoint->RefuseCalls(reason);
}

void QmitkAgentToolServer::AcceptCalls()
{
  if (m_Endpoint != nullptr)
    m_Endpoint->AcceptCalls();
}

int QmitkAgentToolServer::GetRunningCallCount() const
{
  return m_Endpoint != nullptr
    ? m_Endpoint->GetRunningCallCount()
    : 0;
}

void QmitkAgentToolServer::AddMemoryTools()
{
  auto* memory = m_Memory;
  const auto* server = m_RestServer;

  mitk::McpTool remember;
  remember.Name = "remember";
  remember.Description =
    "Save a note to your memory. You get your notes at the start of every later conversation. Save what you "
    "learned about operating the Workbench, from the user or by trying, that will help you next time: how a "
    "tool behaves, a step that is easy to get wrong, or what the user prefers. One fact per note, short and "
    "general. Never save patient data, image contents, or findings. The user sees each note in the chat and "
    "can edit or delete it.";
  remember.InputSchema = {
    { "type", "object" },
    { "properties", { { "note", { { "type", "string" } } } } },
    { "required", { "note" } }
  };
  remember.Handler = [memory, server](const nlohmann::json& arguments, const mitk::RestRequest&) {
    const auto note = QString::fromStdString(arguments.at("note").get<std::string>());

    return mitk::CallInGuiThread(*server, [memory, note]() {
      const auto id = memory->Add(note);
      return mitk::McpToolResult::FromText(QString("Saved as note %1.").arg(id).toStdString());
    });
  };
  m_Endpoint->AddTool(remember);

  mitk::McpTool forget;
  forget.Name = "forget";
  forget.Description =
    "Delete a note from your memory, for example one that turned out to be wrong or outdated. To correct a "
    "note, delete it and save the corrected one.";
  forget.InputSchema = {
    { "type", "object" },
    { "properties", { { "id", { { "type", "integer" }, { "description", "id of the note" } } } } },
    { "required", { "id" } }
  };
  forget.Handler = [memory, server](const nlohmann::json& arguments, const mitk::RestRequest&) {
    const auto& idArgument = arguments.at("id");

    // get<int>() alone would turn a fraction, a boolean or a too large number into the id of another note.
    if (!idArgument.is_number_integer() ||
        idArgument.get<std::int64_t>() < 1 ||
        idArgument.get<std::int64_t>() > std::numeric_limits<int>::max())
    {
      throw std::invalid_argument("The id of a note is a positive integer.");
    }

    const auto id = idArgument.get<int>();

    return mitk::CallInGuiThread(*server, [memory, id]() {
      memory->Remove(id);
      return mitk::McpToolResult::FromText(QString("Deleted note %1.").arg(id).toStdString());
    });
  };
  m_Endpoint->AddTool(forget);
}
