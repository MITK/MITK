/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentView_h
#define QmitkAgentView_h

#include <QmitkAbstractView.h>
#include <QmitkClaudeCodeProcess.h>

#include <QDir>
#include <QElapsedTimer>
#include <QHash>
#include <QJsonObject>
#include <QStringList>

#include <memory>

class QmitkAgentConversationWidget;
class QmitkAgentMemory;
class QCheckBox;
class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTimer;

namespace mitk
{
  class McpEndpoint;
  struct IRestServerService;
}

/**
 * \brief Chat with an agent that operates the Workbench.
 *
 * The agent is the user's own Claude Code installation, started headless
 * with the Workbench tools as its only tools. The view offers the tools
 * through an MCP endpoint of the REST API for as long as it exists, and
 * starts the REST server for that if it does not run. The agent's token
 * has no access to the file system.
 */
class QmitkAgentView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkAgentView();
  ~QmitkAgentView() override;

  /**
   * \brief The empty directory in which Claude Code runs, created if missing.
   *
   * Claude Code reads project settings and CLAUDE.md files from there.
   */
  static QString GetWorkspaceDirectory();

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;
  void OnPreferencesChanged(const mitk::IPreferences* preferences) override;

private:
  void OfferTools();
  void WithdrawTools();
  bool AreToolsOffered() const;

  /**
   * \brief Whether the agent may use a REST server that runs already.
   *
   * Asks the user if the server lets others in as well, unless the user
   * chose not to be asked again.
   */
  bool AcceptRunningServer();

  /** \brief The tools remember and forget, which keep the agent's notes across conversations. */
  void AddMemoryTools(mitk::McpEndpoint& endpoint);

  /** \brief How Claude Code is run, as far as the settings decide it. */
  QmitkClaudeCodeProcess::Options GetBaseOptions() const;

  void QueryAuthStatus();

  /**
   * \brief Finds and asks Claude Code again after its settings or sign-in changed.
   *
   * Restarts the process, which the conversation continues in. A running
   * turn keeps its process, and the check follows when the turn ends.
   */
  void CheckClaudeCodeAgain();

  /** \brief Shows the status of Claude Code, followed by a link to the settings unless \p linkText is empty. */
  void SetStatus(const QString& text, const QString& linkText = QString());

  void OpenSettings();
  void StartProcess();
  void RestartIdleProcess();
  void Send();

  /**
   * \brief Interrupts the running turn and refuses the agent's tool calls
   *        until the next message, so that none still on its way changes the
   *        Workbench after the user stopped the agent.
   */
  void Stop();

  void StartNewConversation();
  void ShowMemory();
  void ClearMemory();
  void UpdateControls();

  bool IsTurnRunning() const;

  /**
   * \brief Locks the other views and the editors while the agent works.
   *
   * The lock lasts until the agent's turns have ended and its tool calls no
   * longer run: an interrupted turn ends before the call it interrupted.
   */
  void UpdateLock();

  void OnInitialized(const QList<QmitkClaudeCodeProcess::ModelInfo>& models);
  void OnModelSelected();
  void OnEffortSelected();
  void UpdateEffortBox();

  void OnMessageReceived(const QJsonObject& message);
  void OnSystemMessage(const QJsonObject& message);
  void OnStreamEvent(const QJsonObject& message);
  void OnAssistantMessage(const QJsonObject& message);
  void OnToolResults(const QJsonObject& message);
  void OnResult(const QJsonObject& message);
  void OnProcessFinished(int exitCode, const QString& errorOutput);
  void OnProcessError(const QString& description);

  /**
   * \brief Let the user see the answer when the agent's turn ends.
   *
   * Shows the view again if the agent's actions covered it with another
   * view of its tab group, for example by opening the Data Manager.
   */
  void CleanUpAfterTurn();

  void SetActivity(const QString& activity);
  void UpdateActivityLabel();
  void UpdateCostLabel();
  void OnShowStepsToggled(bool show);

  QLabel* m_StatusLabel;

  /** \brief What the status label shows, to show it again in the colors of another theme. */
  QString m_Status;
  QString m_StatusLinkText;

  QCheckBox* m_ShowStepsCheckBox;
  QPushButton* m_MemoryButton;
  QPushButton* m_NewConversationButton;
  QmitkAgentConversationWidget* m_Conversation;
  QLabel* m_ActivityLabel;
  QLabel* m_CostLabel;
  QPlainTextEdit* m_Input;
  QComboBox* m_ModelBox;
  QComboBox* m_EffortBox;
  QPushButton* m_StopButton;
  QPushButton* m_SendButton;
  QTimer* m_ActivityTimer;

  /** \brief Looks again whether the views can be unlocked while tool calls run after the turns ended. */
  QTimer* m_UnlockTimer;

  std::unique_ptr<QmitkAgentMemory> m_Memory;
  std::unique_ptr<mitk::McpEndpoint> m_Endpoint;
  mitk::IRestServerService* m_RestServer;

  /** \brief Whether the view started the REST server, and stops it again. */
  bool m_StartedRestServer;

  QmitkClaudeCodeProcess* m_Process;

  QDir m_Directory;
  QString m_Token;
  QString m_McpUrl;
  QString m_Executable;
  QString m_SessionId;
  bool m_SignedIn;

  /** \brief The settings that Claude Code runs with; an empty executable is looked for. */
  QString m_ConfiguredExecutable;
  bool m_IgnoreApiKeyVariable;

  /** \brief Whether CheckClaudeCodeAgain() waits for the running turn to end. */
  bool m_CheckPending;

  /** \brief Counts the queries of the sign-in, so that only the last one counts. */
  int m_AuthQueries;

  /**
   * \brief The messages sent whose turns have not ended.
   *
   * Claude Code runs a message sent during a turn as a turn of its own
   * afterwards, and an interrupt ends only the running turn.
   */
  int m_PendingTurns;

  /** \brief Whether the user stopped the agent, whose tool calls are refused until the next message. */
  bool m_Stopping;

  bool m_Killing;

  QList<QmitkClaudeCodeProcess::ModelInfo> m_Models;

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
