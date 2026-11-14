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
 * QueryAuthStatus(). Claude Code reads no user settings, so credentials
 * or providers configured there do not apply, only those that "claude
 * auth login" stores and environment variables.
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

    /**
     * \brief Whether Claude Code does not get the environment variable ANTHROPIC_API_KEY.
     *
     * Claude Code prefers the key over a signed-in subscription and bills
     * per token.
     */
    bool IgnoreApiKeyVariable = false;
  };

  struct MITKAGENT_EXPORT AuthStatus
  {
    bool LoggedIn = false;

    /** \brief How Claude Code authenticates, for example "claude.ai" or "api_key". */
    QString Method;

    /** \brief The subscription, for example "team"; empty for API keys. */
    QString Subscription;

    /** \brief Where an API key comes from, for example "ANTHROPIC_API_KEY"; empty without one. */
    QString ApiKeySource;

    /** \brief Who serves the models, for example "firstParty" or "bedrock". */
    QString ApiProvider;

    /** \brief Why the status could not be read; empty on success. */
    QString Error;

    /** \brief How Claude Code signs in and who pays, for example "with a Claude subscription (team)". */
    QString Describe() const;
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

  /** \brief Find the claude executable on the PATH or in its usual install locations. */
  static QString FindExecutable();

  /**
   * \brief Whether \p path is a claude executable that Start() can run.
   *
   * On Windows, only an .exe: the .cmd wrapper of an npm install would run
   * through cmd.exe, which reinterprets the arguments.
   */
  static bool IsUsableExecutable(const QString& path);

  /**
   * \brief Ask Claude Code how it would authenticate if started with \p options.
   *
   * Reads the settings that Start() lets Claude Code read, from the same
   * working directory, so that the answer holds for the process. Blocks for
   * up to a few seconds.
   */
  static AuthStatus QueryAuthStatus(const Options& options);

  /**
   * \brief Set \p process up to run "claude auth" with \p arguments, for example "status" or "login".
   *
   * With the executable, working directory, settings and environment of
   * \p options, as Start() runs Claude Code, so that signing in, the status
   * and the agent agree on what Claude Code reads. The caller starts the
   * process.
   */
  static void SetUpAuthCommand(QProcess& process, const Options& options, const QStringList& arguments);

  /**
   * \brief The environment Claude Code is started with.
   *
   * The current environment without the variables a parent Claude Code
   * session sets, which would otherwise make the child behave as nested,
   * with 127.0.0.1 exempt from any HTTP proxy, and with the directory of
   * Options::Executable in front of the PATH.
   */
  static QProcessEnvironment GetEnvironment(const Options& options);

  /** \brief Start Claude Code. Emits ErrorOccurred() if it cannot be started. */
  void Start(const Options& options);

  bool IsRunning() const;

  /**
   * \brief Send a user message. It starts a turn or is queued behind the running one.
   *
   * The text is always a message for the model, never one of Claude Code's
   * slash commands.
   */
  void SendUserMessage(const QString& text);

  /**
   * \brief Ask Claude Code to end the running turn.
   *
   * Claude Code answers with InterruptAcknowledged() at once and ends the
   * turn with a result message. Without a running turn, only the
   * acknowledgment comes.
   */
  void Interrupt();

  /** \brief Kill the process. */
  void Kill();

signals:
  /** \brief One event of the conversation, as Claude Code reported it. */
  void MessageReceived(const QJsonObject& message);

  /**
   * \brief Claude Code is ready for messages, and lists the models the account can use.
   *
   * Not emitted if it lists no model.
   */
  void Initialized(const QList<QmitkClaudeCodeProcess::ModelInfo>& models);

  /** \brief Claude Code took the request of Interrupt(). */
  void InterruptAcknowledged();

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
  QString m_InterruptRequestId;
};

#endif
