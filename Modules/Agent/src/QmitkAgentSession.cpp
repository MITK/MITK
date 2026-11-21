/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkAgentSession.h>

#include <QmitkAgentMemory.h>
#include <QmitkAgentToolServer.h>

#include <mitkLog.h>

#include <QCoreApplication>
#include <QFile>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>
#include <QtConcurrent>

#include <algorithm>

namespace
{
  const QString McpServerName = "mitk";
  const QString ToolPrefix = "mcp__" + McpServerName + "__";

  // Named per process: a Workbench instance that starts its agent at the same time would
  // otherwise replace the file before this instance's Claude Code has read it.
  QString GetConfigPath(const QDir& directory)
  {
    return directory.filePath(QString("mcp-%1.json").arg(QCoreApplication::applicationPid()));
  }

  QString GetPromptPath(const QDir& directory)
  {
    return directory.filePath(QString("system-prompt-%1.md").arg(QCoreApplication::applicationPid()));
  }

  QString GetToolName(const QString& name)
  {
    return name.startsWith(ToolPrefix)
      ? name.mid(ToolPrefix.size())
      : name;
  }

  QString Abbreviate(const QString& text, qsizetype maxLength)
  {
    const auto simplified = text.simplified();

    return simplified.size() > maxLength
      ? simplified.left(maxLength) + "..."
      : simplified;
  }

  QString ToolResultText(const QJsonValue& content)
  {
    if (content.isString())
      return content.toString();

    QStringList parts;

    for (const auto& item : content.toArray())
    {
      const auto object = item.toObject();
      const auto text = object.value("text").toString();

      // Where Claude Code saved an image; the thumbnail shows the image itself.
      if (object.value("type").toString() == "text" && !text.startsWith("[Image: source: "))
        parts << text;
    }

    return parts.join(' ');
  }

  /**
   * A call in one line: method and path for the tools of the REST API, the
   * arguments for other tools, like the note that the agent remembers.
   */
  QString DescribeToolCall(const QString& toolName, const QJsonObject& input)
  {
    if (input.value("path").isString())
      return QString("%1 %2").arg(input.value("method").toString("GET"), input.value("path").toString());

    QStringList arguments;

    for (const auto& argument : input)
    {
      arguments << (argument.isString()
        ? argument.toString()
        : QString::fromUtf8(QJsonDocument(QJsonArray{ argument }).toJson(QJsonDocument::Compact)).mid(1).chopped(1));
    }

    return arguments.isEmpty()
      ? toolName
      : QString("%1: %2").arg(toolName, arguments.join(", "));
  }

  /** Why a call failed: the status and message of an error of the REST API, the result as it is otherwise. */
  QString DescribeFailure(const QString& result)
  {
    QString status;
    auto body = result;

    // A status of the REST API other than 200 comes first, on a line of its own.
    if (const auto end = result.indexOf('\n'); result.startsWith("HTTP ") && end > 0)
    {
      status = result.left(end);
      body = result.mid(end + 1);
    }

    const auto message = QJsonDocument::fromJson(body.toUtf8()).object().value("error").toObject().value("message").toString();
    const auto reason = message.isEmpty() ? body : message;

    return status.isEmpty()
      ? reason
      : QString("%1: %2").arg(status, reason);
  }
}

QmitkAgentSession::QmitkAgentSession(QObject* parent)
  : QObject(parent),
    m_Process(new QmitkClaudeCodeProcess(this)),
    m_IgnoreApiKeyVariable(false),
    m_SignedIn(false),
    m_CheckPending(false),
    m_RestartPending(false),
    m_AuthQueries(0),
    m_PendingTurns(0),
    m_InitReported(false),
    m_ConversationLost(false),
    m_Stopping(false),
    m_Killing(false),
    m_Prepared(false),
    m_ConversationCost(0.0)
{
  m_Directory.setPath(GetWorkspaceDirectory());
  m_Directory.cdUp();

  m_Memory = std::make_unique<QmitkAgentMemory>(m_Directory.filePath("memory.json"));
  m_ToolServer = std::make_unique<QmitkAgentToolServer>(*m_Memory);

  connect(m_Process, &QmitkClaudeCodeProcess::MessageReceived, this, &QmitkAgentSession::OnMessageReceived);
  connect(m_Process, &QmitkClaudeCodeProcess::Initialized, this, &QmitkAgentSession::ModelsChanged);
  connect(m_Process, &QmitkClaudeCodeProcess::DefaultEffortReported, this, &QmitkAgentSession::DefaultEffortReported);
  connect(m_Process, &QmitkClaudeCodeProcess::Finished, this, &QmitkAgentSession::OnProcessFinished);
  connect(m_Process, &QmitkClaudeCodeProcess::ErrorOccurred, this, &QmitkAgentSession::ErrorAdded);
}

QmitkAgentSession::~QmitkAgentSession()
{
  m_Process->disconnect(this);
  m_Process->Kill();

  m_ToolServer->Withdraw();

  QFile::remove(GetConfigPath(m_Directory));
  QFile::remove(GetPromptPath(m_Directory));
}

QString QmitkAgentSession::GetWorkspaceDirectory()
{
  const QDir directory(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  directory.mkpath("Agent/workspace");
  QFile::setPermissions(directory.filePath("Agent"), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
  return directory.filePath("Agent/workspace");
}

QmitkAgentMemory& QmitkAgentSession::GetMemory() const
{
  return *m_Memory;
}

QmitkAgentToolServer& QmitkAgentSession::GetToolServer() const
{
  return *m_ToolServer;
}

void QmitkAgentSession::SetExecutable(const QString& executable)
{
  m_ConfiguredExecutable = executable;
}

QString QmitkAgentSession::GetExecutable() const
{
  return m_ConfiguredExecutable;
}

void QmitkAgentSession::SetIgnoreApiKeyVariable(bool ignore)
{
  m_IgnoreApiKeyVariable = ignore;
}

bool QmitkAgentSession::GetIgnoreApiKeyVariable() const
{
  return m_IgnoreApiKeyVariable;
}

void QmitkAgentSession::SetModel(const QString& model)
{
  m_Model = model;
}

QString QmitkAgentSession::GetModel() const
{
  return m_Model;
}

void QmitkAgentSession::SetEffort(const QString& effort)
{
  m_Effort = effort;
}

QString QmitkAgentSession::GetEffort() const
{
  return m_Effort;
}

void QmitkAgentSession::CheckClaudeCode()
{
  const auto query = ++m_AuthQueries;

  m_Executable = m_ConfiguredExecutable.isEmpty()
    ? QmitkClaudeCodeProcess::FindExecutable()
    : m_ConfiguredExecutable;

  if (m_Executable.isEmpty())
  {
    this->SetStatus(StatusKind::NotFound, "Claude Code was not found. Install it, or set where it is.");
    return;
  }

  if (!QmitkClaudeCodeProcess::IsUsableExecutable(m_Executable))
  {
    this->SetStatus(StatusKind::Unusable, QString("The agent cannot run %1 as Claude Code.").arg(QDir::toNativeSeparators(m_Executable)));
    m_Executable.clear();
    return;
  }

  this->SetStatus(StatusKind::Checking, "Checking how Claude Code signs in...");

  auto* watcher = new QFutureWatcher<QmitkClaudeCodeProcess::AuthStatus>(this);

  connect(watcher, &QFutureWatcher<QmitkClaudeCodeProcess::AuthStatus>::finished, this, [this, watcher, query]() {
    const auto status = watcher->result();
    watcher->deleteLater();

    // The settings changed meanwhile, and a newer query follows.
    if (query != m_AuthQueries)
      return;

    if (!status.Error.isEmpty())
    {
      this->SetStatus(StatusKind::Error, status.Error);
    }
    else if (!status.LoggedIn)
    {
      this->SetStatus(StatusKind::NotSignedIn, "Claude Code is not signed in.");
    }
    else
    {
      m_SignedIn = true;
      this->SetStatus(StatusKind::SignedIn, QString("Claude Code, signed in %1.").arg(status.Describe()), status.ApiKeySource == "ANTHROPIC_API_KEY");

      if (m_Prepared && !m_Process->IsRunning())
        this->StartProcess();
    }
  });

  watcher->setFuture(QtConcurrent::run(&QmitkClaudeCodeProcess::QueryAuthStatus, this->GetBaseOptions()));
}

void QmitkAgentSession::CheckClaudeCodeAgain()
{
  if (this->IsTurnRunning())
  {
    m_CheckPending = true;
    return;
  }

  m_CheckPending = false;
  m_SignedIn = false;

  this->KillProcess();
  this->CheckClaudeCode();
}

void QmitkAgentSession::PrepareForMessages()
{
  m_Prepared = true;

  if (m_SignedIn && !m_Process->IsRunning())
    this->StartProcess();
}

bool QmitkAgentSession::IsSignedIn() const
{
  return m_SignedIn;
}

bool QmitkAgentSession::Send(const QString& text)
{
  if (text.isEmpty() || m_Executable.isEmpty())
    return false;

  if (!m_ToolServer->IsOffered())
  {
    emit NoteAdded("The agent cannot work in the Workbench right now, see the messages above.");
    return false;
  }

  if (m_ToolServer->HasLostServer())
  {
    emit NoteAdded("The REST API server that the agent works through was stopped or restarted with other settings. "
      "Reopen the Agent view to continue.");
    return false;
  }

  m_Prepared = true;

  if (!m_Process->IsRunning())
    this->StartProcess();

  if (!m_Process->IsRunning())
    return false;

  emit UserMessageAdded(text);

  // While the stopped turn runs on, its calls stay refused and a further Stop
  // ends Claude Code. OnResult() ends the stop once this message's turn follows.
  if (m_Stopping && !this->IsTurnRunning())
  {
    m_Stopping = false;
    m_ToolServer->AcceptCalls();
  }

  m_Process->SendUserMessage(text);

  if (!this->IsTurnRunning())
  {
    m_TurnTimer.start();
    m_Activity = "Thinking";
  }

  ++m_PendingTurns;
  emit TurnStateChanged();

  return true;
}

void QmitkAgentSession::Stop()
{
  if (!this->IsTurnRunning())
    return;

  if (m_Stopping)
  {
    // The turn did not end on the interrupt. Without its process it cannot
    // go on, and the session id lets the next message continue.
    this->KillProcess();

    m_PendingTurns = 0;
    m_ToolCalls.clear();

    emit NoteAdded("Claude Code did not stop and was ended. Your next message continues the conversation.");
    emit TurnStateChanged();
    emit TurnsFinished();
    this->FollowUpEndedTurns();
    return;
  }

  m_Stopping = true;
  m_ToolServer->RefuseCalls("The user stopped you, so this call did nothing. Wait for the user's next message.");
  m_Process->Interrupt();

  m_Activity = "Stopping";
  emit TurnStateChanged();
}

void QmitkAgentSession::StartNewConversation()
{
  this->KillProcess();

  m_SessionId.clear();
  m_PendingTurns = 0;
  m_ToolCalls.clear();
  m_ConversationCost = 0.0;
  emit CostChanged();
  emit TurnStateChanged();

  if (m_CheckPending)
  {
    this->CheckClaudeCodeAgain();
  }
  else if (m_SignedIn && m_Prepared)
  {
    this->StartProcess();
  }
}

void QmitkAgentSession::RestartIdleProcess()
{
  if (!m_Process->IsRunning())
    return;

  if (this->IsTurnRunning())
  {
    m_RestartPending = true;
    return;
  }

  this->KillProcess();
  this->StartProcess();
}

bool QmitkAgentSession::IsTurnRunning() const
{
  return m_PendingTurns > 0;
}

bool QmitkAgentSession::IsStopping() const
{
  return m_Stopping;
}

QString QmitkAgentSession::GetActivity() const
{
  return m_Activity;
}

qint64 QmitkAgentSession::GetTurnDuration() const
{
  return m_TurnTimer.elapsed();
}

double QmitkAgentSession::GetConversationCost() const
{
  return m_ConversationCost;
}

QmitkClaudeCodeProcess::Options QmitkAgentSession::GetBaseOptions() const
{
  QmitkClaudeCodeProcess::Options options;
  options.Executable = m_Executable;
  options.WorkingDirectory = GetWorkspaceDirectory();
  options.IgnoreApiKeyVariable = m_IgnoreApiKeyVariable;
  return options;
}

void QmitkAgentSession::StartProcess()
{
  if (!m_ToolServer->IsOffered() || m_ToolServer->HasLostServer())
    return;

  const QJsonObject server = {
    { "type", "http" },
    { "url", m_ToolServer->GetUrl() },
    { "headers", QJsonObject{ { "Authorization", "Bearer " + m_ToolServer->GetToken() } } }
  };

  const auto configPath = GetConfigPath(m_Directory);
  QSaveFile config(configPath);

  if (!config.open(QIODevice::WriteOnly) || !config.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner))
  {
    emit ErrorAdded(QString("Cannot write %1.").arg(configPath));
    return;
  }

  config.write(QJsonDocument(QJsonObject{ { "mcpServers", QJsonObject{ { McpServerName, server } } } }).toJson());

  if (!config.commit())
  {
    emit ErrorAdded(QString("Cannot write %1.").arg(configPath));
    return;
  }

  QFile promptResource(":/Agent/AgentSystemPrompt.md");

  if (!promptResource.open(QIODevice::ReadOnly))
  {
    emit ErrorAdded("The agent's instructions are missing from the Agent module.");
    return;
  }

  const auto promptPath = GetPromptPath(m_Directory);
  QSaveFile prompt(promptPath);

  if (!prompt.open(QIODevice::WriteOnly))
  {
    emit ErrorAdded(QString("Cannot write %1.").arg(promptPath));
    return;
  }

  prompt.write(promptResource.readAll());
  prompt.write(m_Memory->Describe().toUtf8());

  if (!prompt.commit())
  {
    emit ErrorAdded(QString("Cannot write %1.").arg(promptPath));
    return;
  }

  auto options = this->GetBaseOptions();
  options.McpConfigFile = configPath;
  options.AllowedTools = { ToolPrefix + "*" };
  options.AppendSystemPromptFile = promptPath;
  options.Model = m_Model;
  options.Effort = m_Effort;
  options.ResumeSessionId = m_SessionId;
  options.TrustedCertificateFile = m_ToolServer->GetCertificateFile();

  m_RestartPending = false;
  m_InitReported = false;
  m_ConversationLost = false;

  m_Process->Start(options);
}

void QmitkAgentSession::FollowUpEndedTurns()
{
  if (m_CheckPending)
  {
    QTimer::singleShot(0, this, &QmitkAgentSession::CheckClaudeCodeAgain);
  }
  else if (m_RestartPending)
  {
    // Returns at once if the process has ended.
    QTimer::singleShot(0, this, &QmitkAgentSession::RestartIdleProcess);
  }
}

void QmitkAgentSession::KillProcess()
{
  m_RestartPending = false;
  m_Killing = true;
  m_Process->Kill();
  m_Killing = false;
}

void QmitkAgentSession::SetStatus(StatusKind kind, const QString& text, bool usesApiKeyVariable)
{
  emit StatusChanged({ kind, text, usesApiKeyVariable });
}

void QmitkAgentSession::SetActivity(const QString& activity)
{
  m_Activity = activity;
  emit ActivityChanged();
}

void QmitkAgentSession::OnMessageReceived(const QJsonObject& message)
{
  // Killing the process delivers what it still had to say, which is no
  // longer part of the conversation.
  if (m_Killing)
    return;

  const auto type = message.value("type").toString();

  if (type == "system")
  {
    this->OnSystemMessage(message);
  }
  else if (type == "stream_event")
  {
    this->OnStreamEvent(message);
  }
  else if (type == "assistant")
  {
    this->OnAssistantMessage(message);
  }
  else if (type == "user")
  {
    this->OnToolResults(message);
  }
  else if (type == "result")
  {
    this->OnResult(message);
  }
}

void QmitkAgentSession::OnSystemMessage(const QJsonObject& message)
{
  const auto subtype = message.value("subtype").toString();

  // A request to the model, at the start of a turn and after each tool result.
  if (subtype == "status" && message.value("status").toString() == "requesting")
    this->SetActivity("Thinking");

  if (subtype != "init")
    return;

  m_InitReported = true;
  m_ConversationLost = false;

  // Claude Code reports it at the start of every turn.
  m_SessionId = message.value("session_id").toString();

  bool serverListed = false;

  for (const auto& server : message.value("mcp_servers").toArray())
  {
    const auto object = server.toObject();

    if (object.value("name").toString() != McpServerName)
      continue;

    serverListed = true;

    if (object.value("status").toString() != "connected")
      emit ErrorAdded(QString("Claude Code could not connect to the Workbench tools (%1).").arg(object.value("status").toString()));
  }

  if (!serverListed)
    emit ErrorAdded("Claude Code does not list the Workbench tools.");

  // Built-in tools would give the agent the user's files and a shell. A list
  // that cannot be read is no proof that there are none.
  const auto tools = message.value("tools");
  QString error;

  if (!tools.isArray())
  {
    error = "Claude Code does not report its tools, which the Agent view has to check. Stopping.";
  }
  else
  {
    for (const auto& tool : tools.toArray())
    {
      if (!tool.toString().startsWith(ToolPrefix))
      {
        error = QString("Claude Code offers the tool \"%1\", which the Agent view does not allow. Stopping.").arg(tool.toString());
        break;
      }
    }
  }

  if (error.isEmpty())
    return;

  emit ErrorAdded(error);
  this->KillProcess();
  m_PendingTurns = 0;
  emit TurnStateChanged();
}

void QmitkAgentSession::OnStreamEvent(const QJsonObject& message)
{
  if (message.value("parent_tool_use_id").isString())
    return;

  const auto event = message.value("event").toObject();
  const auto type = event.value("type").toString();

  if (type == "content_block_start")
  {
    const auto block = event.value("content_block").toObject();
    const auto blockType = block.value("type").toString();

    if (blockType == "text")
    {
      this->SetActivity("Writing");
      emit TextStarted();
    }
    else if (blockType == "tool_use")
    {
      this->SetActivity(QString("Using %1").arg(GetToolName(block.value("name").toString())));
    }
  }
  else if (type == "content_block_delta")
  {
    const auto delta = event.value("delta").toObject();

    if (delta.value("type").toString() == "text_delta")
      emit TextAdded(delta.value("text").toString());
  }
}

void QmitkAgentSession::OnAssistantMessage(const QJsonObject& message)
{
  // Messages of subagents are not part of the conversation shown.
  if (message.value("parent_tool_use_id").isString())
    return;

  for (const auto& block : message.value("message").toObject().value("content").toArray())
  {
    const auto object = block.toObject();
    const auto type = object.value("type").toString();

    if (type == "text")
    {
      emit TextFinished(object.value("text").toString());
    }
    else if (type == "tool_use")
    {
      m_ToolCalls.insert(object.value("id").toString(), object);
    }
  }
}

void QmitkAgentSession::OnToolResults(const QJsonObject& message)
{
  for (const auto& block : message.value("message").toObject().value("content").toArray())
  {
    const auto object = block.toObject();

    if (object.value("type").toString() != "tool_result")
      continue;

    const auto call = m_ToolCalls.take(object.value("tool_use_id").toString());
    const auto name = GetToolName(call.value("name").toString());
    const auto content = object.value("content");

    QmitkAgentStep step;
    step.Failed = object.value("is_error").toBool();

    if (!call.isEmpty())
      step.Call = Abbreviate(DescribeToolCall(name, call.value("input").toObject()), 200);

    // The result only where it says why a call failed. The transcript of
    // Claude Code keeps all of them.
    if (step.Failed || call.isEmpty())
    {
      const auto text = ToolResultText(content);
      step.Result = Abbreviate(step.Failed ? DescribeFailure(text) : text, 300);
    }

    for (const auto& item : content.toArray())
    {
      const auto image = item.toObject();
      const auto source = image.value("source").toObject();

      if (image.value("type").toString() == "image" && source.value("type").toString() == "base64")
        step.Images << QByteArray::fromBase64(source.value("data").toString().toLatin1());
    }

    if (name == "remember")
    {
      step.Memory = QmitkAgentStep::MemoryChange::NoteSaved;
    }
    else if (name == "forget")
    {
      step.Memory = QmitkAgentStep::MemoryChange::NoteDeleted;
    }

    emit StepFinished(step);
  }
}

void QmitkAgentSession::OnResult(const QJsonObject& message)
{
  // A process that was started ahead and cannot resume ends with a result
  // before any turn.
  if (!this->IsTurnRunning() && m_InitReported)
    MITK_WARN << "Claude Code ended a turn that the Agent view did not count as running.";

  m_PendingTurns = std::max(0, m_PendingTurns - 1);

  // Stop ended the turn that ran. One that the user sent after it, often to
  // steer the agent, runs as usual.
  if (m_Stopping && this->IsTurnRunning())
  {
    m_Stopping = false;
    m_ToolServer->AcceptCalls();
    m_Activity = "Thinking";
  }

  // A process that reports a result before its first init could not take up
  // the conversation it was asked to resume.
  if (!m_InitReported && !m_SessionId.isEmpty())
  {
    m_SessionId.clear();
    m_ConversationLost = true;
    m_ConversationCost = 0.0;
    emit CostChanged();
  }

  emit TurnStateChanged();

  if (!this->IsTurnRunning())
  {
    emit TurnsFinished();
    this->FollowUpEndedTurns();
  }

  const auto subtype = message.value("subtype").toString();
  const auto seconds = message.value("duration_ms").toDouble() / 1000.0;
  auto cost = 0.0;

  // The total of the session so far, which a resumed process continues.
  if (const auto total = message.value("total_cost_usd"); total.isDouble())
  {
    cost = std::max(0.0, total.toDouble() - m_ConversationCost);
    m_ConversationCost = total.toDouble();
    emit CostChanged();
  }

  if (subtype == "success")
  {
    // Claude Code reports a refused model like this, after an assistant message with the reason.
    if (message.value("is_error").toBool())
    {
      emit ErrorAdded("Failed.");
    }
    else
    {
      emit NoteAdded(QString("Done in %1 s (API-equivalent cost $%2).").arg(seconds, 0, 'f', 1).arg(cost, 0, 'f', 2));
    }
  }
  else if (subtype == "error_during_execution")
  {
    // The process says why it ends, see OnProcessFinished().
    if (!m_ConversationLost)
      emit NoteAdded("Stopped.");
  }
  else
  {
    QStringList errors;

    for (const auto& error : message.value("errors").toArray())
      errors << error.toString();

    emit ErrorAdded(QString("The agent stopped (%1). %2").arg(subtype, errors.join(' ')));
  }
}

void QmitkAgentSession::OnProcessFinished(int exitCode, const QString& errorOutput)
{
  const bool turnWasRunning = this->IsTurnRunning();

  m_PendingTurns = 0;
  emit TurnStateChanged();

  if (m_Killing)
    return;

  if (turnWasRunning)
    emit TurnsFinished();

  this->FollowUpEndedTurns();

  auto text = m_ConversationLost
    ? QString("Claude Code could not continue the conversation (exit code %1).").arg(exitCode)
    : QString("Claude Code ended (exit code %1).").arg(exitCode);

  if (auto output = errorOutput.simplified(); !output.isEmpty())
  {
    // The end, where a process says why it ends.
    if (output.size() > 500)
      output = "..." + output.right(500);

    text += " " + output;
  }

  emit ErrorAdded(m_ConversationLost
    ? text + " Your next message starts a new conversation, in which the agent does not know the earlier messages."
    : text + " The next message continues the conversation.");
}
