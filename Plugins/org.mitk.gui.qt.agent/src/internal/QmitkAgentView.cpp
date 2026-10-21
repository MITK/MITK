/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAgentView.h"
#include "QmitkAgentPluginActivator.h"

#include <QmitkAgentConversationWidget.h>
#include <QmitkAgentMemory.h>
#include <QmitkAgentMemoryDialog.h>
#include <QmitkAgentToolServer.h>

#include <ctkPlugin.h>
#include <ctkPluginContext.h>
#include <ctkPluginException.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkLog.h>

#include <QmitkIconTheme.h>
#include <QmitkPartLock.h>
#include <QmitkSpeechBubbleFrame.h>

#include <berryIWorkbenchPage.h>

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <algorithm>

const std::string QmitkAgentView::VIEW_ID = "org.mitk.views.agent";

namespace
{
  const QString McpServerName = "mitk";
  const QString ToolPrefix = "mcp__" + McpServerName + "__";

  // As in plugin.xml.
  const std::string PreferencePageId = "org.mitk.AgentPreferencePage";

  /** Start a plugin now that would otherwise start only when one of its classes is used. */
  void StartPlugin(const QString& symbolicName)
  {
    auto* context = QmitkAgentPluginActivator::GetContext();

    if (context == nullptr)
      return;

    for (const auto& plugin : context->getPlugins())
    {
      if (plugin->getSymbolicName() != symbolicName || plugin->getState() == ctkPlugin::ACTIVE)
        continue;

      try
      {
        plugin->start(ctkPlugin::START_TRANSIENT);
      }
      catch (const ctkPluginException& e)
      {
        MITK_WARN << "Cannot start " << symbolicName.toStdString() << ": " << e.what();
      }
    }
  }

  QString GetToolName(const QString& name)
  {
    return name.startsWith(ToolPrefix)
      ? name.mid(ToolPrefix.size())
      : name;
  }

  QString GetEffortText(const QString& level)
  {
    if (level == "low")
      return "Low effort";

    if (level == "medium")
      return "Medium effort";

    if (level == "high")
      return "High effort";

    if (level == "xhigh")
      return "Extra high effort";

    if (level == "max")
      return "Max effort";

    return level + " effort";
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

QmitkAgentView::QmitkAgentView()
  : m_StatusLabel(nullptr),
    m_ShowStepsCheckBox(nullptr),
    m_MemoryButton(nullptr),
    m_NewConversationButton(nullptr),
    m_Conversation(nullptr),
    m_ActivityLabel(nullptr),
    m_CostLabel(nullptr),
    m_Input(nullptr),
    m_ModelBox(nullptr),
    m_EffortBox(nullptr),
    m_StopButton(nullptr),
    m_SendButton(nullptr),
    m_ActivityTimer(nullptr),
    m_UnlockTimer(nullptr),
    m_Process(nullptr),
    m_SignedIn(false),
    m_IgnoreApiKeyVariable(false),
    m_CheckPending(false),
    m_AuthQueries(0),
    m_PendingTurns(0),
    m_Stopping(false),
    m_Killing(false),
    m_ConversationCost(0.0)
{
}

QmitkAgentView::~QmitkAgentView()
{
  if (m_Process != nullptr)
  {
    m_Process->disconnect(this);
    m_Process->Kill();
  }

  // Withdrawing the tools runs the event loop, in which the timer must not lock again.
  if (m_UnlockTimer != nullptr)
    m_UnlockTimer->stop();

  if (m_ToolServer != nullptr)
    m_ToolServer->Withdraw();

  QmitkPartLock::GetInstance()->Unlock();
}

QString QmitkAgentView::GetWorkspaceDirectory()
{
  const QDir directory(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  directory.mkpath("Agent/workspace");
  return directory.filePath("Agent/workspace");
}

void QmitkAgentView::CreateQtPartControl(QWidget* parent)
{
  auto* layout = new QVBoxLayout(parent);

  m_StatusLabel = new QLabel(parent);
  m_StatusLabel->setObjectName("statusLabel");
  m_StatusLabel->setWordWrap(true);
  m_StatusLabel->setTextFormat(Qt::RichText);
  connect(m_StatusLabel, &QLabel::linkActivated, this, &QmitkAgentView::OpenSettings);
  layout->addWidget(m_StatusLabel);
  this->SetStatus("Looking for Claude Code...");

  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, [this]() {
    this->SetStatus(m_Status, m_StatusLinkText);
  });

  auto* header = new QHBoxLayout;
  m_ShowStepsCheckBox = new QCheckBox("Show steps", parent);
  m_ShowStepsCheckBox->setObjectName("showStepsCheckBox");
  m_ShowStepsCheckBox->setToolTip("Show the tools the agent uses, their results, and screenshots. Otherwise each run of steps is one line that you can click to show it.");
  m_ShowStepsCheckBox->setChecked(GetPreferences()->GetBool("showSteps", false));
  m_MemoryButton = new QPushButton("Memory", parent);
  m_MemoryButton->setObjectName("memoryButton");
  m_MemoryButton->setToolTip("Notes that the agent saved to remember in later conversations.");
  auto* memoryMenu = new QMenu(m_MemoryButton);
  auto* showMemoryAction = memoryMenu->addAction("Show...");
  auto* clearMemoryAction = memoryMenu->addAction("Clear...");
  m_MemoryButton->setMenu(memoryMenu);
  m_NewConversationButton = new QPushButton("New conversation", parent);
  m_NewConversationButton->setObjectName("newConversationButton");
  header->addWidget(m_ShowStepsCheckBox);
  header->addStretch();
  header->addWidget(m_MemoryButton);
  header->addWidget(m_NewConversationButton);
  layout->addLayout(header);

  m_Conversation = new QmitkAgentConversationWidget(parent);
  m_Conversation->setObjectName("conversation");
  m_Conversation->setPlaceholderText("The conversation appears here.");
  m_Conversation->SetStepsShown(m_ShowStepsCheckBox->isChecked());
  layout->addWidget(m_Conversation, 1);

  auto* activity = new QHBoxLayout;
  m_ActivityLabel = new QLabel(parent);
  m_ActivityLabel->setObjectName("activityLabel");
  m_ActivityLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_CostLabel = new QLabel(parent);
  m_CostLabel->setObjectName("costLabel");
  m_CostLabel->setToolTip("What the conversation would cost with an API key. A subscription is not charged per token, but the conversation counts toward its usage limits.");
  activity->addWidget(m_ActivityLabel, 1);
  activity->addWidget(m_CostLabel);
  layout->addLayout(activity);

  auto* inputBubble = new QmitkSpeechBubbleFrame(parent);
  auto* inputLayout = new QVBoxLayout(inputBubble);
  inputLayout->setContentsMargins(0, 0, 0, 0);

  m_Input = new QPlainTextEdit(inputBubble);
  m_Input->setObjectName("messageInput");
  m_Input->setFrameShape(QFrame::NoFrame);
  m_Input->ensurePolished();

  QString inputStyleSheet = "QPlainTextEdit { background: transparent; }";

  if (const auto fontSize = m_Input->font().pointSizeF(); fontSize > 0)
    inputStyleSheet += QString("QPlainTextEdit { font-size: %1pt; }").arg(fontSize * QmitkAgentConversationWidget::TextScale);

  m_Input->setStyleSheet(inputStyleSheet);
  m_Input->setPlaceholderText("Describe what you want to do. Enter sends, Shift+Enter starts a new line.");
  m_Input->setMaximumHeight(m_Input->fontMetrics().lineSpacing() * 5);
  m_Input->installEventFilter(this);
  inputLayout->addWidget(m_Input);
  layout->addWidget(inputBubble);

  auto* buttons = new QHBoxLayout;
  m_ModelBox = new QComboBox(parent);
  m_ModelBox->setObjectName("modelBox");
  m_ModelBox->setPlaceholderText("Model");
  m_ModelBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  m_ModelBox->setToolTip("The model the agent runs on. A change applies from the next message on, and the conversation continues.");
  m_EffortBox = new QComboBox(parent);
  m_EffortBox->setObjectName("effortBox");
  m_EffortBox->setPlaceholderText("Effort");
  m_EffortBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  m_EffortBox->setToolTip("How thoroughly the model thinks before it acts. More effort takes longer and uses more of the usage limits.");

  // Wide enough for the longest entry if there is room, but in a narrow view
  // they may shrink to the width of their placeholder, instead of making the
  // whole view wider than its pane.
  for (auto* box : { m_ModelBox, m_EffortBox })
  {
    box->ensurePolished();
    box->setMinimumWidth(box->minimumSizeHint().width());
  }

  m_StopButton = new QPushButton("Stop", parent);
  m_StopButton->setObjectName("stopButton");
  m_SendButton = new QPushButton("Send", parent);
  m_SendButton->setObjectName("sendButton");
  buttons->addWidget(m_ModelBox);
  buttons->addWidget(m_EffortBox);
  buttons->addStretch();
  buttons->addWidget(m_StopButton);
  buttons->addWidget(m_SendButton);
  layout->addLayout(buttons);

  m_ActivityTimer = new QTimer(this);
  m_ActivityTimer->setInterval(1000);

  m_UnlockTimer = new QTimer(this);
  m_UnlockTimer->setInterval(100);

  connect(memoryMenu, &QMenu::aboutToShow, this, [this, showMemoryAction, clearMemoryAction]() {
    const auto count = m_Memory->GetNotes().size();
    showMemoryAction->setText(count == 1 ? QString("Show 1 note...") : QString("Show %1 notes...").arg(count));
    clearMemoryAction->setEnabled(count > 0);
  });
  connect(m_ShowStepsCheckBox, &QCheckBox::toggled, this, &QmitkAgentView::OnShowStepsToggled);
  connect(showMemoryAction, &QAction::triggered, this, &QmitkAgentView::ShowMemory);
  connect(clearMemoryAction, &QAction::triggered, this, &QmitkAgentView::ClearMemory);
  connect(m_NewConversationButton, &QPushButton::clicked, this, &QmitkAgentView::StartNewConversation);
  connect(m_StopButton, &QPushButton::clicked, this, &QmitkAgentView::Stop);
  connect(m_SendButton, &QPushButton::clicked, this, &QmitkAgentView::Send);
  connect(m_ModelBox, &QComboBox::currentIndexChanged, this, &QmitkAgentView::OnModelSelected);
  connect(m_EffortBox, &QComboBox::currentIndexChanged, this, &QmitkAgentView::OnEffortSelected);
  connect(m_ActivityTimer, &QTimer::timeout, this, &QmitkAgentView::UpdateActivityLabel);
  connect(m_UnlockTimer, &QTimer::timeout, this, &QmitkAgentView::UpdateLock);

  m_Process = new QmitkClaudeCodeProcess(this);
  connect(m_Process, &QmitkClaudeCodeProcess::MessageReceived, this, &QmitkAgentView::OnMessageReceived);
  connect(m_Process, &QmitkClaudeCodeProcess::Initialized, this, &QmitkAgentView::OnInitialized);
  connect(m_Process, &QmitkClaudeCodeProcess::Finished, this, &QmitkAgentView::OnProcessFinished);
  connect(m_Process, &QmitkClaudeCodeProcess::ErrorOccurred, this, &QmitkAgentView::OnProcessError);

  m_Directory.setPath(GetWorkspaceDirectory());
  m_Directory.cdUp();

  m_Memory = std::make_unique<QmitkAgentMemory>(m_Directory.filePath("memory.json"));
  m_ToolServer = std::make_unique<QmitkAgentToolServer>(*m_Memory);

  const auto* preferences = GetPreferences();
  m_Model = QString::fromStdString(preferences->Get("model", ""));
  m_Effort = QString::fromStdString(preferences->Get("effort", ""));
  m_ConfiguredExecutable = QString::fromStdString(preferences->Get("executable", ""));
  m_IgnoreApiKeyVariable = preferences->GetBool("ignoreApiKeyVariable", false);

  if (auto* activator = QmitkAgentPluginActivator::GetInstance(); activator != nullptr)
    connect(activator, &QmitkAgentPluginActivator::ClaudeCodeSignedIn, this, &QmitkAgentView::CheckClaudeCodeAgain);

  this->UpdateCostLabel();
  this->UpdateControls();

  // Later, as offering the tools may ask the user, which a view should not do
  // while it is being created, at startup for example.
  QTimer::singleShot(0, this, [this]() {
    this->OfferTools();
    this->QueryAuthStatus();
  });
}

void QmitkAgentView::SetFocus()
{
  m_Input->setFocus();
}

bool QmitkAgentView::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == m_Input && event->type() == QEvent::KeyPress)
  {
    const auto* keyEvent = static_cast<QKeyEvent*>(event);
    const bool isEnter = keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter;

    if (isEnter && !keyEvent->modifiers().testFlag(Qt::ShiftModifier))
    {
      this->Send();
      return true;
    }
  }

  return QmitkAbstractView::eventFilter(watched, event);
}

void QmitkAgentView::OnPreferencesChanged(const mitk::IPreferences* preferences)
{
  if (preferences == nullptr)
    return;

  // Also called for the choices that the view stores itself, like the model.
  const auto executable = QString::fromStdString(preferences->Get("executable", ""));
  const auto ignoreApiKeyVariable = preferences->GetBool("ignoreApiKeyVariable", false);

  if (executable == m_ConfiguredExecutable && ignoreApiKeyVariable == m_IgnoreApiKeyVariable)
    return;

  m_ConfiguredExecutable = executable;
  m_IgnoreApiKeyVariable = ignoreApiKeyVariable;
  this->CheckClaudeCodeAgain();
}

void QmitkAgentView::OfferTools()
{
  if (const auto risks = m_ToolServer->GetRunningServerRisks(); !risks.isEmpty() && !this->AcceptRunningServer(risks))
  {
    m_Conversation->AppendNote("The agent does not work in the Workbench for now. To let the Agent view start a REST API "
      "server of its own, which only the agent can use, stop the running server in the REST API view and reopen "
      "the Agent view. Or require a token in Preferences > REST API.");
    return;
  }

  if (const auto error = m_ToolServer->Offer(); !error.isEmpty())
  {
    m_Conversation->AppendError(error);
    return;
  }

  // These plugins add the endpoints of the Workbench, of segmentation, of
  // image statistics and of reports when they start, which they otherwise do
  // only with one of their views or editors. They must not start eagerly with
  // the application: those that require org.mitk.gui.qt.common would start
  // it before the workbench runs, which leaves the views without their
  // lifecycle notifications.
  StartPlugin("org.mitk.gui.qt.restapi");
  StartPlugin("org.mitk.gui.qt.segmentation");
  StartPlugin("org.mitk.gui.qt.measurementtoolbox");
  StartPlugin("org.mitk.gui.qt.reports");
}

bool QmitkAgentView::AcceptRunningServer(const QStringList& risks)
{
  auto* preferences = GetPreferences();

  if (!preferences->GetBool("warnAboutOpenRestServer", true))
    return true;

  QMessageBox box(QMessageBox::Warning, "Agent", QString(
    "<p>The REST API server, through which the agent works, already runs with settings that let others in "
    "as well:</p><ul><li>%1</li></ul>"
    "<p>The agent uses it with a token of its own, without access to your files. If you continue, others can "
    "use the server while the agent works, as they could before.</p>")
    .arg(risks.join("</li><li>")), QMessageBox::NoButton, m_Conversation->window());

  auto* continueButton = box.addButton("Continue", QMessageBox::AcceptRole);
  box.addButton(QMessageBox::Cancel);
  box.setDefaultButton(continueButton);

  auto* doNotShowAgain = new QCheckBox("Do not show again", &box);
  box.setCheckBox(doNotShowAgain);
  box.exec();

  const bool accepted = box.clickedButton() == continueButton;

  if (accepted && doNotShowAgain->isChecked())
    preferences->PutBool("warnAboutOpenRestServer", false);

  return accepted;
}

QmitkClaudeCodeProcess::Options QmitkAgentView::GetBaseOptions() const
{
  QmitkClaudeCodeProcess::Options options;
  options.Executable = m_Executable;
  options.WorkingDirectory = GetWorkspaceDirectory();
  options.IgnoreApiKeyVariable = m_IgnoreApiKeyVariable;
  return options;
}

void QmitkAgentView::QueryAuthStatus()
{
  const auto query = ++m_AuthQueries;

  m_Executable = m_ConfiguredExecutable.isEmpty()
    ? QmitkClaudeCodeProcess::FindExecutable()
    : m_ConfiguredExecutable;

  m_StatusLabel->setToolTip(QString());

  if (m_Executable.isEmpty())
  {
    this->SetStatus("Claude Code was not found. Install it, or set where it is.", "Settings...");
    return;
  }

  if (!QmitkClaudeCodeProcess::IsUsableExecutable(m_Executable))
  {
    const auto text = QString("The agent cannot run %1 as Claude Code.").arg(QDir::toNativeSeparators(m_Executable));
    this->SetStatus(text, "Settings...");
    m_Executable.clear();
    return;
  }

  this->SetStatus("Checking how Claude Code signs in...");

  auto* watcher = new QFutureWatcher<QmitkClaudeCodeProcess::AuthStatus>(this);

  connect(watcher, &QFutureWatcher<QmitkClaudeCodeProcess::AuthStatus>::finished, this, [this, watcher, query]() {
    const auto status = watcher->result();
    watcher->deleteLater();

    // The settings changed meanwhile, and a newer query follows.
    if (query != m_AuthQueries)
      return;

    if (!status.Error.isEmpty())
    {
      this->SetStatus(status.Error, "Settings...");
    }
    else if (!status.LoggedIn)
    {
      this->SetStatus("Claude Code is not signed in.", "Sign in...");
    }
    else
    {
      this->SetStatus(QString("Claude Code, signed in %1.").arg(status.Describe()), "Settings...");
      m_SignedIn = true;

      if (status.ApiKeySource == "ANTHROPIC_API_KEY")
      {
        m_StatusLabel->setToolTip("Claude Code prefers this key over a Claude subscription. To use a subscription instead, sign in "
          "with it in the settings and choose to use it even if ANTHROPIC_API_KEY is set.");
      }

      // Ahead of the first message, so that it gets a faster answer and the
      // models to choose from are known.
      if (!m_Process->IsRunning())
        this->StartProcess();
    }
  });

  watcher->setFuture(QtConcurrent::run(&QmitkClaudeCodeProcess::QueryAuthStatus, this->GetBaseOptions()));
}

void QmitkAgentView::CheckClaudeCodeAgain()
{
  if (this->IsTurnRunning())
  {
    m_CheckPending = true;
    return;
  }

  m_CheckPending = false;
  m_SignedIn = false;

  m_Killing = true;
  m_Process->Kill();
  m_Killing = false;

  this->QueryAuthStatus();
}

void QmitkAgentView::SetStatus(const QString& text, const QString& linkText)
{
  m_Status = text;
  m_StatusLinkText = linkText;

  auto html = text.toHtmlEscaped();

  // In the accent color of the theme, as the default link color is hard to
  // read in the dark theme, and its style sheet overrides the palette.
  if (!linkText.isEmpty())
  {
    html += QString(" <a href=\"settings\"><span style=\"color: %1;\">%2</span></a>")
      .arg(QmitkIconTheme::GetAccentColor(), linkText.toHtmlEscaped());
  }

  m_StatusLabel->setText(html);
}

void QmitkAgentView::OpenSettings()
{
  mitk::CoreServices::GetPreferencesService()->OpenPreferencesDialog(PreferencePageId);

  // Claude Code may have been installed meanwhile, which changes no setting.
  if (!m_SignedIn)
    this->CheckClaudeCodeAgain();
}

void QmitkAgentView::StartProcess()
{
  if (!m_ToolServer->IsOffered())
    return;

  const QJsonObject server = {
    { "type", "http" },
    { "url", m_ToolServer->GetUrl() },
    { "headers", QJsonObject{ { "Authorization", "Bearer " + m_ToolServer->GetToken() } } }
  };

  const auto configPath = m_Directory.filePath("mcp.json");
  QFile config(configPath);

  if (!config.open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
    m_Conversation->AppendError(QString("Cannot write %1.").arg(configPath));
    return;
  }

  config.write(QJsonDocument(QJsonObject{ { "mcpServers", QJsonObject{ { McpServerName, server } } } }).toJson());
  config.close();

  QFile promptResource(":/org.mitk.gui.qt.agent/AgentSystemPrompt.md");

  if (!promptResource.open(QIODevice::ReadOnly))
  {
    m_Conversation->AppendError("The agent's instructions are missing from the plugin.");
    return;
  }

  const auto promptPath = m_Directory.filePath("system-prompt.md");
  QFile prompt(promptPath);

  // Earlier versions copied the resource, which left a read-only file.
  if (prompt.exists())
    prompt.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

  if (!prompt.open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
    m_Conversation->AppendError(QString("Cannot write %1.").arg(promptPath));
    return;
  }

  prompt.write(promptResource.readAll());
  prompt.write(m_Memory->Describe().toUtf8());
  prompt.close();

  auto options = this->GetBaseOptions();
  options.McpConfigFile = configPath;
  options.AllowedTools = { ToolPrefix + "*" };
  options.AppendSystemPromptFile = promptPath;
  options.Model = m_Model;
  options.Effort = m_Effort;
  options.ResumeSessionId = m_SessionId;

  m_Process->Start(options);
}

void QmitkAgentView::RestartIdleProcess()
{
  // Model and effort can only be chosen at the start. A process that is not
  // running picks the choice up with the next message.
  if (this->IsTurnRunning() || !m_Process->IsRunning())
    return;

  m_Killing = true;
  m_Process->Kill();
  m_Killing = false;

  this->StartProcess();
}

void QmitkAgentView::Send()
{
  const auto text = m_Input->toPlainText().trimmed();

  if (text.isEmpty() || m_Executable.isEmpty())
    return;

  if (!m_ToolServer->IsOffered())
  {
    m_Conversation->AppendNote("The agent cannot work in the Workbench right now, see the messages above.");
    return;
  }

  m_Input->clear();
  m_Conversation->AppendUserMessage(text);

  if (!m_Process->IsRunning())
    this->StartProcess();

  if (!m_Process->IsRunning())
    return;

  if (m_Stopping)
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
  this->UpdateControls();
}

void QmitkAgentView::Stop()
{
  if (!this->IsTurnRunning() || m_Stopping)
    return;

  m_Stopping = true;
  m_ToolServer->RefuseCalls("The user stopped you, so this call did nothing. Wait for the user's next message.");
  m_Process->Interrupt();

  m_Activity = "Stopping";
  this->UpdateControls();
}

void QmitkAgentView::StartNewConversation()
{
  m_Killing = true;
  m_Process->Kill();
  m_Killing = false;

  m_SessionId.clear();
  m_PendingTurns = 0;
  m_Conversation->Clear();
  m_ToolCalls.clear();
  m_ConversationCost = 0.0;
  this->UpdateCostLabel();
  this->UpdateControls();

  if (m_CheckPending)
  {
    this->CheckClaudeCodeAgain();
  }
  else if (m_SignedIn)
  {
    this->StartProcess();
  }
}

void QmitkAgentView::ShowMemory()
{
  // In the size of the conversation.
  QmitkAgentMemoryDialog dialog(*m_Memory, std::max(0.0, m_Conversation->font().pointSizeF()), m_Conversation);
  dialog.exec();

  if (dialog.HasChanged())
    this->RestartIdleProcess();
}

void QmitkAgentView::ClearMemory()
{
  const auto count = m_Memory->GetNotes().size();

  const auto question = count == 1
    ? QString("Delete the note of the agent's memory? This cannot be undone.")
    : QString("Delete all %1 notes of the agent's memory? This cannot be undone.").arg(count);

  if (QMessageBox::question(m_Conversation, "Agent Memory", question) != QMessageBox::Yes)
    return;

  try
  {
    m_Memory->Clear();
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(m_Conversation, "Agent Memory", QString::fromStdString(e.what()));
    return;
  }

  this->RestartIdleProcess();
}

void QmitkAgentView::UpdateControls()
{
  const bool turnRunning = this->IsTurnRunning();

  m_StopButton->setEnabled(turnRunning && !m_Stopping);
  m_MemoryButton->setEnabled(!turnRunning);
  m_ModelBox->setEnabled(!turnRunning && m_ModelBox->count() > 0);
  m_EffortBox->setEnabled(!turnRunning && m_EffortBox->count() > 1);

  if (turnRunning)
  {
    m_ActivityTimer->start();
  }
  else
  {
    m_ActivityTimer->stop();
  }

  this->UpdateActivityLabel();
  this->UpdateLock();
}

bool QmitkAgentView::IsTurnRunning() const
{
  return m_PendingTurns > 0;
}

void QmitkAgentView::UpdateLock()
{
  auto* lock = QmitkPartLock::GetInstance();
  const bool callsRunning = m_ToolServer != nullptr && m_ToolServer->GetRunningCallCount() > 0;

  if (!this->IsTurnRunning() && !callsRunning)
  {
    m_UnlockTimer->stop();
    lock->Unlock();
    return;
  }

  lock->Lock(QString::fromStdString(VIEW_ID), "Locked while the agent works. You can stop it in the Agent view.");

  if (!this->IsTurnRunning())
    m_UnlockTimer->start();
}

void QmitkAgentView::OnInitialized(const QList<QmitkClaudeCodeProcess::ModelInfo>& models)
{
  const auto model = m_Model;
  const auto effort = m_Effort;

  m_Models = models;

  {
    const QSignalBlocker blocker(m_ModelBox);
    m_ModelBox->clear();

    for (const auto& info : models)
    {
      m_ModelBox->addItem(info.DisplayName, info.Value);
      m_ModelBox->setItemData(m_ModelBox->count() - 1, info.Description, Qt::ToolTipRole);
    }

    auto index = m_ModelBox->findData(m_Model.isEmpty() ? QString("default") : m_Model);

    // A model the account cannot use (anymore).
    if (index < 0)
    {
      m_Model.clear();
      GetPreferences()->Put("model", "");
      index = std::max(m_ModelBox->findData("default"), 0);
    }

    m_ModelBox->setCurrentIndex(index);
  }

  this->UpdateEffortBox();
  this->UpdateControls();

  if (m_Model != model || m_Effort != effort)
    this->RestartIdleProcess();
}

void QmitkAgentView::OnModelSelected()
{
  const auto value = m_ModelBox->currentData().toString();

  m_Model = value == "default" ? QString() : value;
  GetPreferences()->Put("model", m_Model.toStdString());

  this->UpdateEffortBox();
  this->UpdateControls();
  this->RestartIdleProcess();
}

void QmitkAgentView::OnEffortSelected()
{
  m_Effort = m_EffortBox->currentData().toString();
  GetPreferences()->Put("effort", m_Effort.toStdString());

  this->RestartIdleProcess();
}

void QmitkAgentView::UpdateEffortBox()
{
  const auto modelIndex = m_ModelBox->currentIndex();

  const auto levels = modelIndex >= 0 && modelIndex < m_Models.size()
    ? m_Models[modelIndex].EffortLevels
    : QStringList();

  const QSignalBlocker blocker(m_EffortBox);
  m_EffortBox->clear();
  m_EffortBox->addItem("Default effort", QString());

  for (const auto& level : levels)
    m_EffortBox->addItem(GetEffortText(level), level);

  auto index = m_EffortBox->findData(m_Effort);

  // An effort the chosen model does not take.
  if (index < 0)
  {
    m_Effort.clear();
    GetPreferences()->Put("effort", "");
    index = 0;
  }

  m_EffortBox->setCurrentIndex(index);
}

void QmitkAgentView::OnMessageReceived(const QJsonObject& message)
{
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

void QmitkAgentView::OnSystemMessage(const QJsonObject& message)
{
  const auto subtype = message.value("subtype").toString();

  // A request to the model, at the start of a turn and after each tool result.
  if (subtype == "status" && message.value("status").toString() == "requesting")
    this->SetActivity("Thinking");

  if (subtype != "init")
    return;

  m_SessionId = message.value("session_id").toString();

  for (const auto& server : message.value("mcp_servers").toArray())
  {
    const auto object = server.toObject();

    if (object.value("name").toString() == McpServerName && object.value("status").toString() != "connected")
      m_Conversation->AppendError(QString("Claude Code could not connect to the Workbench tools (%1).").arg(object.value("status").toString()));
  }

  // Built-in tools would give the agent the user's files and a shell.
  for (const auto& tool : message.value("tools").toArray())
  {
    if (!tool.toString().startsWith(ToolPrefix))
    {
      m_Conversation->AppendError(QString("Claude Code offers the tool \"%1\", which the Agent view does not allow. Stopping.").arg(tool.toString()));
      m_Killing = true;
      m_Process->Kill();
      m_Killing = false;
      m_PendingTurns = 0;
      this->UpdateControls();
      return;
    }
  }
}

void QmitkAgentView::OnStreamEvent(const QJsonObject& message)
{
  if (!message.value("parent_tool_use_id").isNull())
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
      m_Conversation->BeginStreamedText();
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
      m_Conversation->AppendStreamedText(delta.value("text").toString());
  }
}

void QmitkAgentView::OnAssistantMessage(const QJsonObject& message)
{
  // Messages of subagents are not part of the conversation shown.
  if (!message.value("parent_tool_use_id").isNull())
    return;

  for (const auto& block : message.value("message").toObject().value("content").toArray())
  {
    const auto object = block.toObject();
    const auto type = object.value("type").toString();

    if (type == "text")
    {
      m_Conversation->EndStreamedText(object.value("text").toString());
    }
    else if (type == "tool_use")
    {
      m_ToolCalls.insert(object.value("id").toString(), object);
    }
  }
}

void QmitkAgentView::OnToolResults(const QJsonObject& message)
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

    m_Conversation->AppendStep(step);
  }
}

void QmitkAgentView::OnResult(const QJsonObject& message)
{
  m_PendingTurns = std::max(0, m_PendingTurns - 1);
  this->UpdateControls();

  if (!this->IsTurnRunning())
  {
    this->CleanUpAfterTurn();

    // Later, as the process is still reporting.
    if (m_CheckPending)
      QTimer::singleShot(0, this, &QmitkAgentView::CheckClaudeCodeAgain);
  }

  const auto subtype = message.value("subtype").toString();
  const auto seconds = message.value("duration_ms").toDouble() / 1000.0;
  auto cost = 0.0;

  // The total of the session so far, which a resumed process continues.
  if (const auto total = message.value("total_cost_usd"); total.isDouble())
  {
    cost = std::max(0.0, total.toDouble() - m_ConversationCost);
    m_ConversationCost = total.toDouble();
    this->UpdateCostLabel();
  }

  if (subtype == "success")
  {
    m_Conversation->AppendNote(QString("Done in %1 s (API-equivalent cost $%2).").arg(seconds, 0, 'f', 1).arg(cost, 0, 'f', 2));
  }
  else if (subtype == "error_during_execution")
  {
    m_Conversation->AppendNote("Stopped.");
  }
  else
  {
    QStringList errors;

    for (const auto& error : message.value("errors").toArray())
      errors << error.toString();

    m_Conversation->AppendError(QString("The agent stopped (%1). %2").arg(subtype, errors.join(' ')));
  }
}

void QmitkAgentView::OnProcessFinished(int exitCode, const QString& errorOutput)
{
  m_PendingTurns = 0;
  this->UpdateControls();

  if (m_Killing)
    return;

  this->CleanUpAfterTurn();

  if (m_CheckPending)
    QTimer::singleShot(0, this, &QmitkAgentView::CheckClaudeCodeAgain);

  auto text = QString("Claude Code ended (exit code %1).").arg(exitCode);

  if (!errorOutput.isEmpty())
    text += " " + Abbreviate(errorOutput, 500);

  m_Conversation->AppendError(text + " The next message continues the conversation.");
}

void QmitkAgentView::OnProcessError(const QString& description)
{
  m_Conversation->AppendError(description);
}

void QmitkAgentView::CleanUpAfterTurn()
{
  const auto page = this->GetSite()->GetPage();
  const berry::IWorkbenchPart::Pointer part(this);

  if (page.IsNotNull() && !page->IsPartVisible(part))
    page->BringToTop(part);
}

void QmitkAgentView::SetActivity(const QString& activity)
{
  m_Activity = activity;
  this->UpdateActivityLabel();
}

void QmitkAgentView::UpdateActivityLabel()
{
  if (!this->IsTurnRunning())
  {
    m_ActivityLabel->clear();
    return;
  }

  m_ActivityLabel->setText(QString("%1... %2 s").arg(m_Activity).arg(m_TurnTimer.elapsed() / 1000));
}

void QmitkAgentView::UpdateCostLabel()
{
  m_CostLabel->setText(QString("Conversation: $%1").arg(m_ConversationCost, 0, 'f', 2));
}

void QmitkAgentView::OnShowStepsToggled(bool show)
{
  GetPreferences()->PutBool("showSteps", show);
  m_Conversation->SetStepsShown(show);
}
