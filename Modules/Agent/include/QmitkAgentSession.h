/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentSession_h
#define QmitkAgentSession_h

#include <MitkAgentExports.h>

#include <QmitkAgentStep.h>
#include <QmitkClaudeCodeProcess.h>

#include <QDir>
#include <QElapsedTimer>
#include <QHash>
#include <QJsonObject>
#include <QObject>

#include <memory>

class QmitkAgentMemory;
class QmitkAgentToolServer;

/**
 * \brief A conversation with an agent that operates the Workbench.
 *
 * The agent is the user's own Claude Code installation, started headless
 * with the Workbench tools as its only tools (see QmitkAgentToolServer).
 * The session finds Claude Code, checks how it signs in, starts it, and
 * turns its events into signals for the parts of a conversation: messages,
 * streamed text and steps. A conversation continues across restarts of the
 * process, for example after another model was chosen.
 *
 * Use it from the GUI thread.
 */
class MITKAGENT_EXPORT QmitkAgentSession : public QObject
{
  Q_OBJECT

public:
  enum class StatusKind
  {
    NotFound,
    Unusable,
    Checking,
    Error,
    NotSignedIn,
    SignedIn
  };

  /** \brief How far Claude Code is ready. */
  struct Status
  {
    StatusKind Kind = StatusKind::Checking;
    QString Text;

    /** \brief Whether Claude Code signs in with the key in ANTHROPIC_API_KEY, which it prefers over a subscription. */
    bool UsesApiKeyVariable = false;
  };

  explicit QmitkAgentSession(QObject* parent = nullptr);

  /** \brief Kills Claude Code and withdraws the tools. */
  ~QmitkAgentSession() override;

  /**
   * \brief The empty directory in which Claude Code runs, created if missing.
   *
   * Claude Code reads project settings and CLAUDE.md files from there.
   */
  static QString GetWorkspaceDirectory();

  /** \brief The notes that the agent keeps across conversations. */
  QmitkAgentMemory& GetMemory() const;

  QmitkAgentToolServer& GetToolServer() const;

  /** \brief The claude executable to run; empty to look for it. Takes effect with the next check. */
  void SetExecutable(const QString& executable);
  QString GetExecutable() const;

  /** \see QmitkClaudeCodeProcess::Options::IgnoreApiKeyVariable. Takes effect with the next check. */
  void SetIgnoreApiKeyVariable(bool ignore);
  bool GetIgnoreApiKeyVariable() const;

  /** \brief The model; empty for Claude Code's default. Takes effect when the process starts next. */
  void SetModel(const QString& model);
  QString GetModel() const;

  /** \brief The effort; empty for Claude Code's default. Takes effect when the process starts next. */
  void SetEffort(const QString& effort);
  QString GetEffort() const;

  /**
   * \brief Finds Claude Code and asks it how it signs in.
   *
   * Starts it only after PrepareForMessages(), so that a session nobody
   * turns to runs no process.
   */
  void CheckClaudeCode();

  /**
   * \brief Starts Claude Code ahead of the first message, if it is signed in
   *        and does not run, so that the message gets a faster answer and the
   *        models to choose from are known.
   *
   * To be called when the user turns to the agent. If the sign-in is still
   * being checked, Claude Code starts once the check has found it signed in.
   */
  void PrepareForMessages();

  /**
   * \brief Finds and asks Claude Code again after its settings or sign-in changed.
   *
   * Restarts the process, which the conversation continues in. A running
   * turn keeps its process, and the check follows when the turn ends.
   */
  void CheckClaudeCodeAgain();

  bool IsSignedIn() const;

  /**
   * \brief Sends a message of the user, which starts a turn or follows the running one.
   *
   * \return Whether the message was taken. It is not without Claude Code,
   *         which an error explains if it cannot be started, or without the
   *         tools, which a note explains.
   */
  bool Send(const QString& text);

  /**
   * \brief Interrupts the running turn and refuses the agent's tool calls
   *        until the next message, so that none still on its way changes the
   *        Workbench after the user stopped the agent.
   *
   * Called again while the turn still has not ended, ends Claude Code, so
   * that the user always gets out of a turn; the next message continues the
   * conversation in a new process.
   */
  void Stop();

  /** \brief Ends the conversation; the next message starts a new one. */
  void StartNewConversation();

  /**
   * \brief Starts the process again if it runs without a turn, to pick up the
   *        model, the effort, or the memory, which go in at the start.
   *
   * A process that is not running picks them up with the next message. While
   * a turn runs, the restart waits for the turns to end.
   */
  void RestartIdleProcess();

  bool IsTurnRunning() const;
  bool IsStopping() const;

  /** \brief What the agent does in the running turn, for example "Thinking". */
  QString GetActivity() const;

  /** \brief Milliseconds since the running turns started. */
  qint64 GetTurnDuration() const;

  /** \brief What the conversation would cost with an API key, in US dollars. */
  double GetConversationCost() const;

signals:
  void StatusChanged(const QmitkAgentSession::Status& status);

  /** \brief Claude Code started and lists the models that the account can use. */
  void ModelsChanged(const QList<QmitkClaudeCodeProcess::ModelInfo>& models);

  /** \see QmitkClaudeCodeProcess::DefaultEffortReported() */
  void DefaultEffortReported(const QString& model, const QString& effort);

  /** \brief A turn started or ended, or the agent was stopped. */
  void TurnStateChanged();

  /** \brief The last running turn ended. */
  void TurnsFinished();

  void ActivityChanged();
  void CostChanged();

  void UserMessageAdded(const QString& text);

  /** \brief The agent starts to write a text, which TextAdded() continues and TextFinished() completes. */
  void TextStarted();
  void TextAdded(const QString& text);
  void TextFinished(const QString& markdown);

  void StepFinished(const QmitkAgentStep& step);

  /** \brief Something about the conversation that the user should know. */
  void NoteAdded(const QString& text);

  void ErrorAdded(const QString& text);

private:
  /** \brief How Claude Code is run, as far as the settings decide it. */
  QmitkClaudeCodeProcess::Options GetBaseOptions() const;

  void StartProcess();
  void KillProcess();
  void SetStatus(StatusKind kind, const QString& text, bool usesApiKeyVariable = false);
  void SetActivity(const QString& activity);

  void OnMessageReceived(const QJsonObject& message);
  void OnSystemMessage(const QJsonObject& message);
  void OnStreamEvent(const QJsonObject& message);
  void OnAssistantMessage(const QJsonObject& message);
  void OnToolResults(const QJsonObject& message);
  void OnResult(const QJsonObject& message);
  void OnInterruptAcknowledged();
  void OnProcessFinished(int exitCode, const QString& errorOutput);

  std::unique_ptr<QmitkAgentMemory> m_Memory;
  std::unique_ptr<QmitkAgentToolServer> m_ToolServer;
  QmitkClaudeCodeProcess* m_Process;

  /** \brief Where the memory, the MCP configuration and the instructions of the agent are stored. */
  QDir m_Directory;

  /** \brief The executable found or set, which runs; empty if there is none. */
  QString m_Executable;

  /** \brief The executable that the settings name; empty to look for one. */
  QString m_ConfiguredExecutable;

  bool m_IgnoreApiKeyVariable;
  bool m_SignedIn;

  /** \brief Whether CheckClaudeCodeAgain() waits for the running turn to end. */
  bool m_CheckPending;

  /** \brief Whether RestartIdleProcess() waits for the running turn to end. */
  bool m_RestartPending;

  /** \brief Counts the queries of the sign-in, so that only the last one counts. */
  int m_AuthQueries;

  QString m_SessionId;

  /**
   * \brief The messages sent whose turns have not ended.
   *
   * Claude Code runs a message sent during a turn as a turn of its own
   * afterwards, and an interrupt ends only the running turn.
   */
  int m_PendingTurns;

  /**
   * \brief Whether Claude Code has reported the start of a turn and not yet its result.
   *
   * Tells an interrupt that ends a turn from one that finds none: only the
   * first is followed by a result.
   */
  bool m_TurnInFlight;

  /**
   * \brief Whether the running process has reported its first init.
   *
   * A process that cannot take up the conversation it is asked to resume
   * ends with a result before any init.
   */
  bool m_InitReported;

  /** \brief Whether the running process could not take up the conversation, which was dropped. */
  bool m_ConversationLost;

  /** \brief Whether the user stopped the agent, whose tool calls are refused until the next message. */
  bool m_Stopping;

  bool m_Killing;

  /** \brief Whether the user has turned to the agent, see PrepareForMessages(). */
  bool m_Prepared;

  /** \brief The chosen model and effort; empty for Claude Code's defaults. */
  QString m_Model;
  QString m_Effort;

  QElapsedTimer m_TurnTimer;
  QString m_Activity;

  /**
   * \brief The tool calls that wait for their results, by id.
   *
   * A step shows a call together with its result. Calls that the agent makes
   * at once get their results after all of them.
   */
  QHash<QString, QJsonObject> m_ToolCalls;

  /** \brief Claude Code reports the cost of the whole session, even across processes. */
  double m_ConversationCost;
};

#endif
