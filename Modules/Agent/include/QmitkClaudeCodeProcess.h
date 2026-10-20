/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkClaudeCodeProcess_h
#define QmitkClaudeCodeProcess_h

#include <MitkAgentExports.h>

#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QProcess>
#include <QStringList>

/**
 * \brief Runs the user's own Claude Code installation as a headless agent.
 *
 * The process reads user messages and control requests as JSON lines on
 * stdin and reports every event of the conversation as a JSON line on
 * stdout (the stream-json format of "claude -p"), including the text of
 * the agent's answers while it is written. Built-in tools are removed, so
 * the agent can only use the MCP servers it is given.
 *
 * Authentication is entirely Claude Code's own: this class never sees
 * credentials. Which method Claude Code uses can be queried with
 * QueryAuthStatus().
 */
class MITKAGENT_EXPORT QmitkClaudeCodeProcess : public QObject
{
  Q_OBJECT

public:
  struct Options
  {
    /** \brief The claude executable. */
    QString Executable;

    /**
     * \brief Where Claude Code runs.
     *
     * Claude Code reads project settings and CLAUDE.md files from here, so
     * this should be an empty directory.
     */
    QString WorkingDirectory;

    /** \brief A JSON file in the format of "claude --mcp-config". */
    QString McpConfigFile;

    /** \brief Tools that run without asking, for example "mcp__mitk__*". */
    QStringList AllowedTools;

    /** \brief A text file with instructions appended to Claude Code's system prompt. */
    QString AppendSystemPromptFile;

    /** \brief A model alias or name; empty for Claude Code's default. */
    QString Model;

    /** \brief One of low, medium, high, xhigh, max; empty for the default. */
    QString Effort;

    /** \brief A session to continue; empty for a new one. */
    QString ResumeSessionId;
  };

  struct AuthStatus
  {
    bool LoggedIn = false;

    /** \brief How Claude Code authenticates, for example "claude.ai" or "api_key". */
    QString Method;

    /** \brief The subscription, for example "team"; empty for API keys. */
    QString Subscription;

    /** \brief Why the status could not be read; empty on success. */
    QString Error;
  };

  /** \brief A model that the account can use. */
  struct ModelInfo
  {
    /** \brief What to pass as Options::Model, for example "opus"; "default" for Claude Code's default. */
    QString Value;

    QString DisplayName;
    QString Description;

    /** \brief The values that Options::Effort takes with this model; empty if it takes none. */
    QStringList EffortLevels;
  };

  explicit QmitkClaudeCodeProcess(QObject* parent = nullptr);

  /** \brief Kills the process if it is still running. */
  ~QmitkClaudeCodeProcess() override;

  /** \brief Find the claude executable on the PATH or in its default install location. */
  static QString FindExecutable();

  /** \brief Ask Claude Code how it would authenticate. Blocks for up to a few seconds. */
  static AuthStatus QueryAuthStatus(const QString& executable);

  /**
   * \brief The environment Claude Code is started with.
   *
   * The current environment without the variables a parent Claude Code
   * session sets, which would otherwise make the child behave as nested,
   * and with 127.0.0.1 exempt from any HTTP proxy.
   */
  static QProcessEnvironment GetEnvironment();

  /** \brief Start Claude Code. Emits ErrorOccurred() if it cannot be started. */
  void Start(const Options& options);

  bool IsRunning() const;

  /** \brief Send a user message. It starts a turn or is queued behind the running one. */
  void SendUserMessage(const QString& text);

  /** \brief Ask Claude Code to end the running turn. */
  void Interrupt();

  /** \brief Kill the process. */
  void Kill();

signals:
  /** \brief One event of the conversation, as Claude Code reported it. */
  void MessageReceived(const QJsonObject& message);

  /** \brief Claude Code is ready for messages, and lists the models the account can use. */
  void Initialized(const QList<QmitkClaudeCodeProcess::ModelInfo>& models);

  /** \brief The process ended. \p errorOutput is the end of what it wrote to stderr. */
  void Finished(int exitCode, const QString& errorOutput);

  void ErrorOccurred(const QString& description);

private:
  void WriteLine(const QJsonObject& object);
  QString SendControlRequest(const QJsonObject& request);
  void OnControlResponse(const QJsonObject& message);
  void OnReadyReadStandardOutput();
  void OnReadyReadStandardError();
  void OnFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void OnErrorOccurred(QProcess::ProcessError error);

  QProcess* m_Process;
  QByteArray m_OutputBuffer;
  QByteArray m_ErrorOutput;
  unsigned int m_RequestCounter;
  QString m_InitializeRequestId;
};

#endif
