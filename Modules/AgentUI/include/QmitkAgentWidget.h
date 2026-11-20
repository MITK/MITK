/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentWidget_h
#define QmitkAgentWidget_h

#include <MitkAgentUIExports.h>

#include <QmitkAgentSession.h>

#include <QHash>
#include <QWidget>

#include <memory>

class QTimer;

namespace mitk
{
  class IPreferences;
}

namespace Ui
{
  class QmitkAgentWidget;
}

/**
 * \brief The controls of a conversation with the agent: its status, the
 *        conversation, the input, and the choice of model and effort.
 *
 * Shows a QmitkAgentSession, which must outlive the widget's use. The
 * preferences keep what the user chooses here, and the settings of Claude
 * Code that the agent's preference page sets.
 */
class MITKAGENTUI_EXPORT QmitkAgentWidget : public QWidget
{
  Q_OBJECT

public:
  /** \brief The preference page that sets how Claude Code runs and signs in. */
  static const std::string PREFERENCE_PAGE_ID;

  /** \brief Applies the settings in \p preferences to \p session. */
  QmitkAgentWidget(QmitkAgentSession* session, mitk::IPreferences* preferences, QWidget* parent = nullptr);
  ~QmitkAgentWidget() override;

  /** \brief Applies the settings of Claude Code after a change of the preferences. */
  void OnPreferencesChanged();

  /** \brief Adds a note to the conversation. */
  void AddNote(const QString& text);

signals:
  /**
   * \brief The Workbench tools were offered to the agent just now.
   *
   * Happens right after the widget was created, and again when a message or
   * a new conversation finds the tools withdrawn or not yet offered.
   */
  void ToolsOffered();

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  /**
   * \brief Makes sure the Workbench tools are offered to the agent.
   *
   * Asks the user first if a REST server runs already with a risk that the
   * user did not accept yet. Explains in the conversation why the tools are
   * not offered.
   *
   * \return Whether the tools are offered.
   */
  bool OfferTools();

  /** \brief Offers the tools and lets the session start Claude Code ahead of the first message. */
  void PrepareAgent();

  /**
   * \brief Whether the agent may use a REST server that runs already with these risks.
   *
   * Asks, unless the user accepted all of them before and chose not to be
   * asked again.
   */
  bool AcceptRunningServer(const QStringList& risks);

  void OnStatusChanged(const QmitkAgentSession::Status& status);

  /** \brief Shows the status of Claude Code, followed by a link to the settings unless \p linkText is empty. */
  void SetStatus(const QString& text, const QString& linkText = QString());

  void OpenSettings();
  void Send();
  void StartNewConversation();
  void ShowMemory();
  void ClearMemory();
  void UpdateControls();
  void UpdateActivityLabel();
  void UpdateCostLabel();

  void OnModelsChanged(const QList<QmitkClaudeCodeProcess::ModelInfo>& models);
  void OnDefaultEffortReported(const QString& model, const QString& effort);
  void OnModelSelected();
  void OnEffortSelected();
  void UpdateEffortBox();
  void OnShowStepsToggled(bool show);

  std::unique_ptr<Ui::QmitkAgentWidget> m_Ui;
  QmitkAgentSession* m_Session;
  mitk::IPreferences* m_Preferences;
  QTimer* m_ActivityTimer;

  /** \brief What the status label shows, to show it again in the colors of another theme. */
  QString m_Status;
  QString m_StatusLinkText;

  QList<QmitkClaudeCodeProcess::ModelInfo> m_Models;

  /**
   * \brief Claude Code's default effort per resolved model.
   *
   * Claude Code names it only while it runs without an effort, so it is kept
   * for when the user chooses one.
   */
  QHash<QString, QString> m_DefaultEfforts;
};

#endif
