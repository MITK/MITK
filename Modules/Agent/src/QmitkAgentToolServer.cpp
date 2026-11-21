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
#include <mitkRestServer.h>

#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleRegistry.h>

#include <QCoreApplication>
#include <QStringList>
#include <QTimer>

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

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

    // "localhost" stays: the server may be bound to ::1 only, which 127.0.0.1 does not reach.
    if (host == "::" || host == "0.0.0.0")
    {
      host = "127.0.0.1";
    }
    else if (host.contains(':'))
    {
      host = '[' + host + ']';
    }

    return QString("%1://%2:%3").arg(config.httpsEnabled ? "https" : "http").arg(host).arg(config.port);
  }

  /**
   * Why Claude Code would not take the certificate of an HTTPS server for the
   * host of GetLocalUrl(); empty if it would. Node checks an IP address
   * against the IP entries of a certificate only, and a name against its
   * names, as OpenSSL's checks do.
   */
  QString GetRefusalOfCertificate(const mitk::RestServerConfig& config)
  {
    if (!config.httpsEnabled)
      return QString();

    const auto host = config.host == "::" || config.host == "0.0.0.0" ? std::string("127.0.0.1") : config.host;

    const std::unique_ptr<BIO, decltype(&BIO_free)> file(BIO_new_file(config.sslCertPath.c_str(), "r"), &BIO_free);
    const std::unique_ptr<X509, decltype(&X509_free)> certificate(
      file != nullptr ? PEM_read_bio_X509(file.get(), nullptr, nullptr, nullptr) : nullptr, &X509_free);

    if (certificate == nullptr)
    {
      return QString("The agent cannot work in the Workbench: the certificate of the REST API server that runs (%1) "
        "cannot be read. Stop the server, or start it with a readable certificate, and reopen the Agent view.")
        .arg(QString::fromStdString(config.sslCertPath));
    }

    // A self-signed certificate, trusted as it is, still counts only while it
    // is valid. The server itself serves it at any time.
    const auto* validity = X509_cmp_current_time(X509_get0_notAfter(certificate.get())) < 0
      ? "has expired"
      : X509_cmp_current_time(X509_get0_notBefore(certificate.get())) > 0
        ? "is not valid yet"
        : nullptr;

    if (validity != nullptr)
    {
      return QString("The agent cannot work in the Workbench: the certificate of the REST API server that runs (%1) "
        "%2. Restart the server with a valid certificate, or stop it, and reopen the Agent view.")
        .arg(QString::fromStdString(config.sslCertPath)).arg(validity);
    }

    // -2: no IP address, so a name.
    const auto ipMatch = X509_check_ip_asc(certificate.get(), host.c_str(), 0);
    const bool named = ipMatch == 1 || (ipMatch == -2 && X509_check_host(certificate.get(), host.c_str(), host.size(), 0, nullptr) == 1);

    if (!named)
    {
      return QString("The agent cannot work in the Workbench: it connects to the REST API server that runs at %1 over "
        "HTTPS, but the server's certificate does not name %1. Restart the server with a certificate that names it, or "
        "on a host that the certificate names, like localhost, or stop the server, and reopen the Agent view.")
        .arg(QString::fromStdString(host));
    }

    return QString();
  }

  QString GetMcpUrl(const mitk::RestServerConfig& config)
  {
    return GetLocalUrl(config) + "/api/v1" + QString::fromStdString(McpPath);
  }

  /**
   * The address that the agent's requests to GetLocalUrl() arrive from, as
   * the server reports it: the one the server listens on, which for a name
   * like "localhost" is a single one of its addresses. A dual-stack socket
   * reports 127.0.0.1 as ::ffff:127.0.0.1.
   */
  std::string GetAgentAddress(const std::string& host)
  {
    std::string address;

    try
    {
      address = mitk::RestServer::GetListeningAddress(host);
    }
    catch (const mitk::Exception&)
    {
      // The server resolved it when it started, so this hardly happens.
      return host;
    }

    if (address == "::")
      return "::ffff:127.0.0.1";

    if (address == "0.0.0.0" || address.rfind("127.", 0) == 0)
      return "127.0.0.1";

    return address;
  }

  /** Why the server does not accept requests from this machine; empty if it does. */
  QString GetRefusalOfThisMachine(const mitk::RestServerConfig& config)
  {
    if (config.clientAccessMode == mitk::ClientAccessMode::Whitelist)
    {
      // The server compares the client's address as a string, so a name like
      // "localhost" on the list lets nobody in, and only the exact spelling of
      // the agent's address does.
      const auto& allowed = config.allowedClientIPs;
      const auto address = GetAgentAddress(config.host);

      if (std::find(allowed.begin(), allowed.end(), address) == allowed.end())
      {
        return QString("The agent cannot work in the Workbench: the REST API server that runs accepts only the clients "
          "on its list, and the agent's requests from this computer arrive from %1 there. Add %1 to the list in the "
          "REST API view, or stop the server, and reopen the Agent view.").arg(QString::fromStdString(address));
      }
    }
    else if (config.clientAccessMode == mitk::ClientAccessMode::LocalhostOnly)
    {
      // Bound to any other address, requests from this computer arrive from
      // that address, which is not a local one to the server.
      const auto& host = config.host;
      const auto address = GetAgentAddress(host);
      const bool reachable = address == "127.0.0.1" || address == "::ffff:127.0.0.1" || address == "::1";

      if (!reachable)
      {
        return QString("The agent cannot work in the Workbench: the REST API server that runs listens only on %1 and "
          "accepts only clients on this computer, which cannot reach it there. Change the server's host to localhost "
          "in the REST API view, or stop the server, and reopen the Agent view.").arg(QString::fromStdString(host));
      }
    }

    return QString();
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
  else if (const auto config = m_RestServer->GetRunningConfig(); config.has_value())
  {
    if (const auto refusal = GetRefusalOfThisMachine(*config); !refusal.isEmpty())
      return refusal;

    if (const auto refusal = GetRefusalOfCertificate(*config); !refusal.isEmpty())
      return refusal;
  }

  m_Endpoint = std::make_unique<mitk::McpEndpoint>("MITK Workbench", "0.2");
  mitk::AddRestApiTools(*m_Endpoint, m_RestServer);
  this->AddMemoryTools();

  try
  {
    m_Token = QString::fromStdString(m_RestServer->CreateToken("Agent view", mitk::RestTokenRights()));
    m_Endpoint->RequireToken(m_Token.toStdString());
    m_Endpoint->Offer(m_RestServer, McpPath);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << e.GetDescription();
    this->Withdraw();
    return QString("The Workbench tools are not available: %1").arg(e.GetDescription());
  }

  const auto config = m_RestServer->GetRunningConfig().value();
  m_Url = GetMcpUrl(config);

  if (config.httpsEnabled)
    m_CertificateFile = QString::fromStdString(config.sslCertPath);

  return QString();
}

void QmitkAgentToolServer::Withdraw()
{
  m_Url.clear();
  m_CertificateFile.clear();

  if (m_Endpoint != nullptr)
  {
    m_Endpoint->Withdraw();

    // Calls that run may wait for the GUI thread, which is this one. User
    // input stays out, as the Agent view may be closing. The timer wakes the
    // wait, so that the count is looked at again. Not a nested QEventLoop,
    // whose exec() returns at once while the application quits.
    QTimer wakeUp;
    wakeUp.start(10);

    while (m_Endpoint->GetRunningCallCount() > 0)
      QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents);

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

QString QmitkAgentToolServer::GetCertificateFile() const
{
  return m_CertificateFile;
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
