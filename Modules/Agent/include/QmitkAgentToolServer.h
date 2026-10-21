/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentToolServer_h
#define QmitkAgentToolServer_h

#include <MitkAgentExports.h>

#include <QString>
#include <QStringList>

#include <memory>
#include <string>

class QmitkAgentMemory;

namespace mitk
{
  class McpEndpoint;
  struct IRestServerService;
}

/**
 * \brief Offers the Workbench tools to the agent: an MCP endpoint of the REST
 *        API with the routes of the REST API and the agent's memory.
 *
 * Starts a REST server for the agent if none runs, whatever the REST API
 * preferences say: only the agent's token gets in, no client may name files,
 * and nothing outside this computer can reach it. A server that runs already
 * is used as it is, which GetRunningServerRisks() lets the user judge first.
 * The agent's token has no access to the file system.
 */
class MITKAGENT_EXPORT QmitkAgentToolServer
{
public:
  /** \param memory What the tools remember and forget change. Must outlive this object. */
  explicit QmitkAgentToolServer(QmitkAgentMemory& memory);

  /** \brief Withdraws the tools. */
  ~QmitkAgentToolServer();

  QmitkAgentToolServer(const QmitkAgentToolServer&) = delete;
  QmitkAgentToolServer& operator=(const QmitkAgentToolServer&) = delete;

  /**
   * \brief Why the REST server that runs already lets others in as well.
   *
   * Empty if no server runs, or if the one that runs lets in only clients
   * with a token, on this computer, which cannot name files.
   */
  QStringList GetRunningServerRisks() const;

  /**
   * \brief Offers the tools, through the REST server that runs or one of the agent's own.
   *
   * \return Why the tools are not offered; empty if they are.
   */
  QString Offer();

  /**
   * \brief Withdraws the tools and stops the server that Offer() started.
   *
   * Waits for the calls that run, which wait for the GUI thread, by running
   * its event loop. Call it from the GUI thread.
   */
  void Withdraw();

  bool IsOffered() const;

  /** \brief Where the agent reaches the tools; empty unless they are offered. */
  QString GetUrl() const;

  /** \brief The agent's token for the REST server; empty unless the tools are offered. */
  QString GetToken() const;

  /** \brief See mitk::McpEndpoint::RefuseCalls(). Does nothing unless the tools are offered. */
  void RefuseCalls(const std::string& reason);

  /** \brief See mitk::McpEndpoint::AcceptCalls(). Does nothing unless the tools are offered. */
  void AcceptCalls();

  /** \brief The tool calls that run; 0 unless the tools are offered. */
  int GetRunningCallCount() const;

private:
  /** \brief The tools remember and forget, which keep the agent's notes across conversations. */
  void AddMemoryTools();

  QmitkAgentMemory* m_Memory;
  mitk::IRestServerService* m_RestServer;
  std::unique_ptr<mitk::McpEndpoint> m_Endpoint;

  /** \brief Whether Offer() started the REST server, which Withdraw() stops again. */
  bool m_StartedRestServer;

  QString m_Token;
  QString m_Url;
};

#endif
