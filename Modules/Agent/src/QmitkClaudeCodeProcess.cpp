/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkClaudeCodeProcess.h>

#include <mitkLog.h>

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRandomGenerator>
#include <QStandardPaths>

namespace
{
  constexpr qsizetype MaxErrorOutputSize = 8192;

  bool IsUsableExecutable(const QString& path)
  {
    if (path.isEmpty())
      return false;

    const QFileInfo info(path);

#ifdef Q_OS_WIN
    // A .cmd or .bat wrapper (an npm install) would run through cmd.exe,
    // which reinterprets the arguments.
    if (info.suffix().compare("exe", Qt::CaseInsensitive) != 0)
      return false;
#endif

    return info.isFile() && info.isExecutable();
  }
}

QmitkClaudeCodeProcess::QmitkClaudeCodeProcess(QObject* parent)
  : QObject(parent),
    m_Process(new QProcess(this)),
    m_RequestCounter(0)
{
  connect(m_Process, &QProcess::readyReadStandardOutput, this, &QmitkClaudeCodeProcess::OnReadyReadStandardOutput);
  connect(m_Process, &QProcess::readyReadStandardError, this, &QmitkClaudeCodeProcess::OnReadyReadStandardError);
  connect(m_Process, &QProcess::finished, this, &QmitkClaudeCodeProcess::OnFinished);
  connect(m_Process, &QProcess::errorOccurred, this, &QmitkClaudeCodeProcess::OnErrorOccurred);
}

QmitkClaudeCodeProcess::~QmitkClaudeCodeProcess()
{
  m_Process->disconnect(this);
  this->Kill();
}

QString QmitkClaudeCodeProcess::FindExecutable()
{
  const auto onPath = QStandardPaths::findExecutable("claude");

  if (IsUsableExecutable(onPath))
    return onPath;

  // Where the native installer puts it, in case the PATH of the application
  // differs from the one of the user's shell.
  const QDir localBin(QDir::home().filePath(".local/bin"));

#ifdef Q_OS_WIN
  const auto installed = localBin.filePath("claude.exe");
#else
  const auto installed = localBin.filePath("claude");
#endif

  return IsUsableExecutable(installed) ? installed : QString();
}

QmitkClaudeCodeProcess::AuthStatus QmitkClaudeCodeProcess::QueryAuthStatus(const QString& executable)
{
  AuthStatus status;

  QProcess process;
  process.setProcessEnvironment(GetEnvironment());
  process.start(executable, { "auth", "status" });

  if (!process.waitForFinished(10000))
  {
    process.kill();
    status.Error = "Claude Code did not report its authentication status.";
    return status;
  }

  const auto document = QJsonDocument::fromJson(process.readAllStandardOutput());

  if (!document.isObject())
  {
    status.Error = "Claude Code reported its authentication status in an unknown format.";
    return status;
  }

  const auto object = document.object();
  status.LoggedIn = object.value("loggedIn").toBool();
  status.Method = object.value("authMethod").toString();
  status.Subscription = object.value("subscriptionType").toString();

  return status;
}

QProcessEnvironment QmitkClaudeCodeProcess::GetEnvironment()
{
  auto environment = QProcessEnvironment::systemEnvironment();

  for (const auto& key : environment.keys())
  {
    const bool setByParentSession = key == "CLAUDECODE" || key == "CLAUDE_PID" || key == "CLAUDE_EFFORT" ||
      (key.startsWith("CLAUDE_CODE_") && key != "CLAUDE_CODE_GIT_BASH_PATH" && !key.startsWith("CLAUDE_CODE_USE_"));

    if (setByParentSession)
      environment.remove(key);
  }

  // The MCP server listens on 127.0.0.1, which a configured HTTP proxy would
  // look for on its own host. Which spelling Claude Code reads first is not
  // known, so both get the combined list.
  auto noProxy = environment.value("no_proxy", environment.value("NO_PROXY"));
  noProxy = noProxy.isEmpty() ? QString("127.0.0.1") : noProxy + ",127.0.0.1";
  environment.insert("no_proxy", noProxy);
  environment.insert("NO_PROXY", noProxy);

  return environment;
}

void QmitkClaudeCodeProcess::Start(const Options& options)
{
  if (this->IsRunning())
  {
    emit ErrorOccurred("Claude Code is already running.");
    return;
  }

  QStringList arguments = {
    "-p",
    "--input-format", "stream-json",
    "--output-format", "stream-json",
    "--verbose",
    "--include-partial-messages",
    "--tools", "",
    "--strict-mcp-config",
    "--mcp-config", options.McpConfigFile,
    "--permission-mode", "dontAsk",
    "--setting-sources=project",
    // Claude Code's own memory would ask the agent to write files with the
    // built-in tools, which are removed.
    "--settings", R"({"disableClaudeAiConnectors": true, "autoMemoryEnabled": false})"
  };

  if (!options.AllowedTools.isEmpty())
    arguments << "--allowedTools" << options.AllowedTools.join(',');

  if (!options.AppendSystemPromptFile.isEmpty())
    arguments << "--append-system-prompt-file" << options.AppendSystemPromptFile;

  if (!options.Model.isEmpty())
    arguments << "--model" << options.Model;

  if (!options.Effort.isEmpty())
    arguments << "--effort" << options.Effort;

  // The equals form keeps a value starting with a dash from being parsed as a flag.
  if (!options.ResumeSessionId.isEmpty())
    arguments << QString("--resume=%1").arg(options.ResumeSessionId);

  m_OutputBuffer.clear();
  m_ErrorOutput.clear();

  m_Process->setProgram(options.Executable);
  m_Process->setArguments(arguments);
  m_Process->setWorkingDirectory(options.WorkingDirectory);
  m_Process->setProcessEnvironment(GetEnvironment());
  m_Process->start();

  if (!m_Process->waitForStarted(10000))
    return; // OnErrorOccurred() reported it.

  // Like the Agent SDK, open the control channel before the first message.
  m_InitializeRequestId = this->SendControlRequest({ { "subtype", "initialize" }, { "hooks", QJsonValue::Null } });
}

bool QmitkClaudeCodeProcess::IsRunning() const
{
  return m_Process->state() != QProcess::NotRunning;
}

void QmitkClaudeCodeProcess::SendUserMessage(const QString& text)
{
  const QJsonObject message = {
    { "role", "user" },
    { "content", text }
  };

  this->WriteLine({
    { "type", "user" },
    { "message", message },
    { "parent_tool_use_id", QJsonValue::Null },
    { "session_id", "default" }
  });
}

void QmitkClaudeCodeProcess::Interrupt()
{
  this->SendControlRequest({ { "subtype", "interrupt" } });
}

void QmitkClaudeCodeProcess::Kill()
{
  if (!this->IsRunning())
    return;

  m_Process->kill();
  m_Process->waitForFinished(3000);
}

void QmitkClaudeCodeProcess::WriteLine(const QJsonObject& object)
{
  if (!this->IsRunning())
  {
    emit ErrorOccurred("Claude Code is not running.");
    return;
  }

  m_Process->write(QJsonDocument(object).toJson(QJsonDocument::Compact) + '\n');
}

QString QmitkClaudeCodeProcess::SendControlRequest(const QJsonObject& request)
{
  const auto requestId = QString("req_%1_%2")
    .arg(++m_RequestCounter)
    .arg(QRandomGenerator::global()->generate(), 8, 16, QChar('0'));

  this->WriteLine({
    { "type", "control_request" },
    { "request_id", requestId },
    { "request", request }
  });

  return requestId;
}

void QmitkClaudeCodeProcess::OnControlResponse(const QJsonObject& message)
{
  const auto response = message.value("response").toObject();

  if (response.value("request_id").toString() != m_InitializeRequestId || response.value("subtype").toString() != "success")
    return;

  QList<ModelInfo> models;

  for (const auto& entry : response.value("response").toObject().value("models").toArray())
  {
    const auto object = entry.toObject();

    ModelInfo model;
    model.Value = object.value("value").toString();
    model.DisplayName = object.value("displayName").toString();
    model.Description = object.value("description").toString();

    if (object.value("supportsEffort").toBool())
    {
      for (const auto& level : object.value("supportedEffortLevels").toArray())
        model.EffortLevels << level.toString();
    }

    if (!model.Value.isEmpty())
      models << model;
  }

  emit Initialized(models);
}

void QmitkClaudeCodeProcess::OnReadyReadStandardOutput()
{
  m_OutputBuffer += m_Process->readAllStandardOutput();

  qsizetype lineEnd;

  while ((lineEnd = m_OutputBuffer.indexOf('\n')) >= 0)
  {
    const auto line = m_OutputBuffer.left(lineEnd).trimmed();
    m_OutputBuffer.remove(0, lineEnd + 1);

    if (line.isEmpty())
      continue;

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(line, &error);

    if (document.isObject())
    {
      const auto object = document.object();

      if (object.value("type").toString() == "control_response")
        this->OnControlResponse(object);

      emit MessageReceived(object);
    }
    else
    {
      MITK_WARN << "Unexpected output of Claude Code: " << line.left(200).toStdString();
    }
  }
}

void QmitkClaudeCodeProcess::OnReadyReadStandardError()
{
  m_ErrorOutput += m_Process->readAllStandardError();

  if (m_ErrorOutput.size() > MaxErrorOutputSize)
    m_ErrorOutput = m_ErrorOutput.right(MaxErrorOutputSize);
}

void QmitkClaudeCodeProcess::OnFinished(int exitCode, QProcess::ExitStatus)
{
  this->OnReadyReadStandardOutput();
  this->OnReadyReadStandardError();

  emit Finished(exitCode, QString::fromUtf8(m_ErrorOutput).trimmed());
}

void QmitkClaudeCodeProcess::OnErrorOccurred(QProcess::ProcessError error)
{
  if (error == QProcess::FailedToStart)
    emit ErrorOccurred(QString("Cannot start Claude Code (%1): %2").arg(m_Process->program(), m_Process->errorString()));
}
