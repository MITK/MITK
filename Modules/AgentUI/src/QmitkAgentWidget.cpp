/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkAgentWidget.h>
#include <ui_QmitkAgentWidget.h>

#include "QmitkAgentDataNoticeDialog.h"

#include <QmitkAgentMemory.h>
#include <QmitkAgentMemoryDialog.h>
#include <QmitkAgentToolServer.h>
#include <QmitkIconTheme.h>
#include <QmitkInfoCard.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>

#include <QCheckBox>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QTimer>

#include <algorithm>

namespace
{
  QString GetEffortName(const QString& level)
  {
    return level == "xhigh"
      ? QString("extra high")
      : level;
  }

  QString GetEffortText(const QString& level)
  {
    const auto name = GetEffortName(level);
    return name.left(1).toUpper() + name.mid(1) + " effort";
  }

  /** \brief The display name, but with the model that Claude Code's default stands for, like "Default (Opus 5.5)". */
  QString GetModelText(const QmitkClaudeCodeProcess::ModelInfo& info, const QList<QmitkClaudeCodeProcess::ModelInfo>& models)
  {
    if (info.Value != "default" || info.ResolvedModel.isEmpty())
      return info.DisplayName;

    const auto model = std::find_if(models.begin(), models.end(), [&info](const auto& other) {
      return other.Value != info.Value && other.ResolvedModel == info.ResolvedModel;
    });

    return QString("Default (%1)").arg(model != models.end() ? model->DisplayName : info.ResolvedModel);
  }
}

const std::string QmitkAgentWidget::PREFERENCE_PAGE_ID = "org.mitk.AgentPreferencePage";

QmitkAgentWidget::QmitkAgentWidget(QmitkAgentSession* session, mitk::IPreferences* preferences, QWidget* parent)
  : QWidget(parent),
    m_Ui(std::make_unique<Ui::QmitkAgentWidget>()),
    m_Session(session),
    m_Preferences(preferences),
    m_ActivityTimer(new QTimer(this))
{
  m_Ui->setupUi(this);

  this->UpdateDataNoticeCard();
  m_Ui->dataNoticeCard->SetClosable(true);
  connect(m_Ui->dataNoticeCard, &QmitkInfoCard::LinkActivated, this, &QmitkAgentWidget::ShowDataNotice);

  connect(m_Ui->statusLabel, &QLabel::linkActivated, this, &QmitkAgentWidget::OpenSettings);
  this->SetStatus("Looking for Claude Code...");

  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, [this]() {
    this->SetStatus(m_Status, m_StatusLinkText);
    this->UpdateDataNoticeCard();
  });

  m_Ui->showStepsCheckBox->setChecked(m_Preferences->GetBool("showSteps", false));
  m_Ui->conversation->SetStepsShown(m_Ui->showStepsCheckBox->isChecked());

  auto* memoryMenu = new QMenu(m_Ui->memoryButton);
  auto* showMemoryAction = memoryMenu->addAction("Show...");
  auto* clearMemoryAction = memoryMenu->addAction("Clear...");
  m_Ui->memoryButton->setMenu(memoryMenu);

  auto* input = m_Ui->messageInput;
  input->ensurePolished();

  QString inputStyleSheet = "QPlainTextEdit { background: transparent; }";

  if (const auto fontSize = input->font().pointSizeF(); fontSize > 0)
    inputStyleSheet += QString("QPlainTextEdit { font-size: %1pt; }").arg(fontSize * QmitkAgentConversationWidget::TextScale);

  input->setStyleSheet(inputStyleSheet);
  input->setMaximumHeight(input->fontMetrics().lineSpacing() * 5);
  input->installEventFilter(this);
  this->setFocusProxy(input);

  // Wide enough for the longest entry if there is room, but in a narrow view
  // they may shrink to the width of their placeholder, instead of making the
  // whole view wider than its pane.
  for (auto* box : { m_Ui->modelBox, m_Ui->effortBox })
  {
    box->ensurePolished();
    box->setMinimumWidth(box->minimumSizeHint().width());
  }

  m_ActivityTimer->setInterval(1000);

  connect(memoryMenu, &QMenu::aboutToShow, this, [this, showMemoryAction, clearMemoryAction]() {
    const auto count = m_Session->GetMemory().GetNotes().size();
    showMemoryAction->setText(count == 1 ? QString("Show 1 note...") : QString("Show %1 notes...").arg(count));
    clearMemoryAction->setEnabled(count > 0);
  });

  connect(m_Ui->showStepsCheckBox, &QCheckBox::toggled, this, &QmitkAgentWidget::OnShowStepsToggled);
  connect(showMemoryAction, &QAction::triggered, this, &QmitkAgentWidget::ShowMemory);
  connect(clearMemoryAction, &QAction::triggered, this, &QmitkAgentWidget::ClearMemory);
  connect(m_Ui->newConversationButton, &QPushButton::clicked, this, &QmitkAgentWidget::StartNewConversation);
  connect(m_Ui->stopButton, &QPushButton::clicked, this, &QmitkAgentWidget::OnStopClicked);
  m_Ui->stopButton->installEventFilter(this);
  connect(m_Ui->sendButton, &QPushButton::clicked, this, &QmitkAgentWidget::Send);
  connect(m_Ui->modelBox, &QComboBox::currentIndexChanged, this, &QmitkAgentWidget::OnModelSelected);
  connect(m_Ui->effortBox, &QComboBox::currentIndexChanged, this, &QmitkAgentWidget::OnEffortSelected);
  connect(m_ActivityTimer, &QTimer::timeout, this, &QmitkAgentWidget::UpdateActivityLabel);

  auto* conversation = m_Ui->conversation;

  connect(m_Session, &QmitkAgentSession::StatusChanged, this, &QmitkAgentWidget::OnStatusChanged);
  connect(m_Session, &QmitkAgentSession::ModelsChanged, this, &QmitkAgentWidget::OnModelsChanged);
  connect(m_Session, &QmitkAgentSession::DefaultEffortReported, this, &QmitkAgentWidget::OnDefaultEffortReported);
  connect(m_Session, &QmitkAgentSession::TurnStateChanged, this, &QmitkAgentWidget::UpdateControls);
  connect(m_Session, &QmitkAgentSession::ActivityChanged, this, &QmitkAgentWidget::UpdateActivityLabel);
  connect(m_Session, &QmitkAgentSession::CostChanged, this, &QmitkAgentWidget::UpdateCostLabel);
  connect(m_Session, &QmitkAgentSession::UserMessageAdded, conversation, &QmitkAgentConversationWidget::AppendUserMessage);
  connect(m_Session, &QmitkAgentSession::TextStarted, conversation, &QmitkAgentConversationWidget::BeginStreamedText);
  connect(m_Session, &QmitkAgentSession::TextAdded, conversation, &QmitkAgentConversationWidget::AppendStreamedText);
  connect(m_Session, &QmitkAgentSession::TextFinished, conversation, &QmitkAgentConversationWidget::EndStreamedText);
  connect(m_Session, &QmitkAgentSession::TurnsFinished, conversation, &QmitkAgentConversationWidget::ForgetStreamedText);
  connect(m_Session, &QmitkAgentSession::StepFinished, conversation, &QmitkAgentConversationWidget::AppendStep);
  connect(m_Session, &QmitkAgentSession::ConversationEnded, conversation, &QmitkAgentConversationWidget::Clear);
  connect(m_Session, &QmitkAgentSession::NoteAdded, conversation, &QmitkAgentConversationWidget::AppendNote);
  connect(m_Session, &QmitkAgentSession::ErrorAdded, conversation, &QmitkAgentConversationWidget::AppendError);

  // The activity of a turn is the session's, which may end before the widget.
  connect(m_Session, &QObject::destroyed, m_ActivityTimer, &QTimer::stop);

  m_Session->SetModel(QString::fromStdString(m_Preferences->Get("model", "")));
  m_Session->SetEffort(QString::fromStdString(m_Preferences->Get("effort", "")));
  m_Session->SetExecutable(QString::fromStdString(m_Preferences->Get("executable", "")));
  m_Session->SetIgnoreApiKeyVariable(m_Preferences->GetBool("ignoreApiKeyVariable", false));

  this->UpdateCostLabel();
  this->UpdateControls();

  // Later, as offering the tools may ask the user, which a widget should not
  // do while it is being created, at startup for example.
  QTimer::singleShot(0, this, &QmitkAgentWidget::PrepareAgent);
}

QmitkAgentWidget::~QmitkAgentWidget() = default;

void QmitkAgentWidget::AddNote(const QString& text)
{
  m_Ui->conversation->AppendNote(text);
}

bool QmitkAgentWidget::OfferTools()
{
  auto& toolServer = m_Session->GetToolServer();

  if (toolServer.IsOffered())
    return true;

  if (const auto risks = toolServer.GetRunningServerRisks(); !risks.isEmpty() && !this->AcceptRunningServer(risks))
  {
    m_Ui->conversation->AppendNote("The agent does not work in the Workbench for now. To let the Agent view start a REST API "
      "server of its own, which only the agent can use, stop the running server in the REST API view (or let it "
      "require a token and restart it), then start a new conversation.");
    return false;
  }

  if (const auto error = toolServer.Offer(); !error.isEmpty())
  {
    m_Ui->conversation->AppendError(error);
    return false;
  }

  emit this->ToolsOffered();
  return true;
}

void QmitkAgentWidget::PrepareAgent()
{
  if (this->OfferTools())
    m_Session->PrepareForMessages();
}

void QmitkAgentWidget::OnPreferencesChanged()
{
  // Also called for the choices that the widget stores itself, like the model.
  const auto executable = QString::fromStdString(m_Preferences->Get("executable", ""));
  const auto ignoreApiKeyVariable = m_Preferences->GetBool("ignoreApiKeyVariable", false);

  if (executable == m_Session->GetExecutable() && ignoreApiKeyVariable == m_Session->GetIgnoreApiKeyVariable())
    return;

  m_Session->SetExecutable(executable);
  m_Session->SetIgnoreApiKeyVariable(ignoreApiKeyVariable);
  m_Session->CheckClaudeCodeAgain();
}

bool QmitkAgentWidget::eventFilter(QObject* watched, QEvent* event)
{
  // A button clicks again on the second press of a double click, and a second
  // Stop ends Claude Code. Qt tells a double click by the times of the presses,
  // which a busy GUI thread does not shift.
  if (watched == m_Ui->stopButton && event->type() == QEvent::MouseButtonDblClick)
    return true;

  if (watched == m_Ui->messageInput && event->type() == QEvent::KeyPress)
  {
    const auto* keyEvent = static_cast<QKeyEvent*>(event);
    const bool isEnter = keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter;

    if (isEnter && !keyEvent->modifiers().testFlag(Qt::ShiftModifier))
    {
      this->Send();
      return true;
    }
  }

  return QWidget::eventFilter(watched, event);
}

bool QmitkAgentWidget::IsDataNoticeAccepted() const
{
  return m_Preferences->GetInt("acceptedDataNotice", 0) >= QmitkAgentDataNoticeDialog::VERSION;
}

bool QmitkAgentWidget::AcceptDataNotice()
{
  if (this->IsDataNoticeAccepted())
    return true;

  QmitkAgentDataNoticeDialog dialog(QmitkAgentDataNoticeDialog::Mode::Ask, this->window());

  if (dialog.exec() != QDialog::Accepted)
    return false;

  m_Preferences->PutInt("acceptedDataNotice", QmitkAgentDataNoticeDialog::VERSION);
  return true;
}

void QmitkAgentWidget::ShowDataNotice()
{
  if (!this->IsDataNoticeAccepted())
  {
    this->AcceptDataNotice();
    return;
  }

  QmitkAgentDataNoticeDialog dialog(QmitkAgentDataNoticeDialog::Mode::Show, this->window());
  dialog.exec();
}

void QmitkAgentWidget::UpdateDataNoticeCard()
{
  // In the accent color of the theme, like the link of the status.
  m_Ui->dataNoticeCard->SetMessage(QString("Your messages, what the agent reads and its screenshots leave this "
    "computer. <a href=\"details\"><span style=\"color: %1;\">Details</span></a>")
    .arg(QmitkIconTheme::GetAccentColor()), QmitkInfoCard::Severity::Warning);
}

bool QmitkAgentWidget::AcceptRunningServer(const QStringList& risks)
{
  // The texts of the risks that the user accepted without being asked again.
  const auto acceptedRisks = QString::fromStdString(m_Preferences->Get("acceptedServerRisks", "")).split('\n', Qt::SkipEmptyParts);

  const bool allAccepted = std::all_of(risks.begin(), risks.end(), [&acceptedRisks](const QString& risk) {
    return acceptedRisks.contains(risk);
  });

  if (allAccepted)
    return true;

  QMessageBox box(QMessageBox::Warning, "Agent", QString(
    "<p>The REST API server, through which the agent works, already runs with settings that let others in "
    "as well:</p><ul><li>%1</li></ul>"
    "<p>The agent uses it with a token of its own, without access to your files. If you continue, others can "
    "use the server while the agent works, as they could before.</p>")
    .arg(risks.join("</li><li>")), QMessageBox::NoButton, this->window());

  auto* continueButton = box.addButton("Continue", QMessageBox::AcceptRole);
  auto* cancelButton = box.addButton(QMessageBox::Cancel);
  box.setDefaultButton(cancelButton);
  box.setEscapeButton(cancelButton);

  auto* doNotShowAgain = new QCheckBox("Do not show again for these risks", &box);
  box.setCheckBox(doNotShowAgain);
  box.exec();

  const bool accepted = box.clickedButton() == continueButton;

  if (accepted && doNotShowAgain->isChecked())
  {
    auto stored = acceptedRisks;

    for (const auto& risk : risks)
    {
      if (!stored.contains(risk))
        stored << risk;
    }

    m_Preferences->Put("acceptedServerRisks", stored.join('\n').toStdString());
  }

  return accepted;
}

void QmitkAgentWidget::OnStatusChanged(const QmitkAgentSession::Status& status)
{
  using Kind = QmitkAgentSession::StatusKind;

  if (status.Kind == Kind::Checking)
  {
    this->SetStatus(status.Text);
  }
  else if (status.Kind == Kind::NotSignedIn)
  {
    this->SetStatus(status.Text, "Sign in...");
  }
  else
  {
    this->SetStatus(status.Text, "Settings...");
  }

  m_Ui->statusLabel->setToolTip(status.UsesApiKeyVariable
    ? QString("Claude Code prefers this key over a Claude subscription. To use a subscription instead, sign in "
        "with it in the settings and choose to ignore ANTHROPIC_API_KEY.")
    : QString());
}

void QmitkAgentWidget::SetStatus(const QString& text, const QString& linkText)
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

  m_Ui->statusLabel->setText(html);
}

void QmitkAgentWidget::OpenSettings()
{
  mitk::CoreServices::GetPreferencesService()->OpenPreferencesDialog(PREFERENCE_PAGE_ID);

  // Claude Code may have been installed meanwhile, which changes no setting.
  if (!m_Session->IsSignedIn())
    m_Session->CheckClaudeCodeAgain();
}

void QmitkAgentWidget::Send()
{
  const auto text = m_Ui->messageInput->toPlainText().trimmed();

  if (text.isEmpty())
    return;

  if (!this->AcceptDataNotice())
    return;

  // Asks again if the tools were not offered, for example after the question
  // about a running server was canceled.
  if (!this->OfferTools())
    return;

  if (m_Session->Send(text))
    m_Ui->messageInput->clear();
}

void QmitkAgentWidget::StartNewConversation()
{
  m_Ui->conversation->Clear();
  m_Session->StartNewConversation();
  this->PrepareAgent();
}

void QmitkAgentWidget::ShowMemory()
{
  // In the size of the conversation.
  QmitkAgentMemoryDialog dialog(m_Session->GetMemory(), std::max(0.0, m_Ui->conversation->font().pointSizeF()), m_Ui->conversation);
  dialog.exec();

  if (dialog.HasChanged())
    m_Session->RestartIdleProcess();
}

void QmitkAgentWidget::ClearMemory()
{
  auto& memory = m_Session->GetMemory();
  const auto count = memory.GetNotes().size();

  const auto question = count == 1
    ? QString("Delete the note of the agent's memory? This cannot be undone.")
    : QString("Delete all %1 notes of the agent's memory? This cannot be undone.").arg(count);

  const auto answer = QMessageBox::question(m_Ui->conversation, "Agent Memory", question,
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (answer != QMessageBox::Yes)
    return;

  try
  {
    memory.Clear();
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(m_Ui->conversation, "Agent Memory", QString::fromStdString(e.what()));
    return;
  }

  m_Session->RestartIdleProcess();
}

void QmitkAgentWidget::OnStopClicked()
{
  m_Session->Stop();
}

void QmitkAgentWidget::UpdateControls()
{
  const bool turnRunning = m_Session->IsTurnRunning();

  // Also while the agent stops: a second Stop ends Claude Code if the turn does not end.
  m_Ui->stopButton->setEnabled(turnRunning);
  m_Ui->memoryButton->setEnabled(!turnRunning);
  m_Ui->modelBox->setEnabled(!turnRunning && m_Ui->modelBox->count() > 0);
  m_Ui->effortBox->setEnabled(!turnRunning && m_Ui->effortBox->count() > 1);

  if (turnRunning)
  {
    m_ActivityTimer->start();
  }
  else
  {
    m_ActivityTimer->stop();
  }

  this->UpdateActivityLabel();
}

void QmitkAgentWidget::UpdateActivityLabel()
{
  if (!m_Session->IsTurnRunning())
  {
    m_Ui->activityLabel->clear();
    return;
  }

  m_Ui->activityLabel->setText(QString("%1... %2 s").arg(m_Session->GetActivity()).arg(m_Session->GetTurnDuration() / 1000));
}

void QmitkAgentWidget::UpdateCostLabel()
{
  m_Ui->costLabel->setText(QString("Conversation: $%1").arg(m_Session->GetConversationCost(), 0, 'f', 2));
}

void QmitkAgentWidget::OnModelsChanged(const QList<QmitkClaudeCodeProcess::ModelInfo>& models)
{
  const auto model = m_Session->GetModel();
  const auto effort = m_Session->GetEffort();

  m_Models = models;

  {
    auto* box = m_Ui->modelBox;
    const QSignalBlocker blocker(box);
    box->clear();

    for (const auto& info : models)
    {
      box->addItem(GetModelText(info, models), info.Value);
      box->setItemData(box->count() - 1, info.Description, Qt::ToolTipRole);
    }

    auto index = box->findData(model.isEmpty() ? QString("default") : model);

    // A model the account cannot use (anymore).
    if (index < 0)
    {
      m_Session->SetModel(QString());
      m_Preferences->Put("model", "");
      index = std::max(box->findData("default"), 0);
    }

    box->setCurrentIndex(index);
  }

  this->UpdateEffortBox();
  this->UpdateControls();

  if (m_Session->GetModel() != model || m_Session->GetEffort() != effort)
    m_Session->RestartIdleProcess();
}

void QmitkAgentWidget::OnDefaultEffortReported(const QString& model, const QString& effort)
{
  m_DefaultEfforts.insert(model, effort);
  this->UpdateEffortBox();
}

void QmitkAgentWidget::OnModelSelected()
{
  const auto value = m_Ui->modelBox->currentData().toString();

  m_Session->SetModel(value == "default" ? QString() : value);
  m_Preferences->Put("model", m_Session->GetModel().toStdString());

  this->UpdateEffortBox();
  this->UpdateControls();
  m_Session->RestartIdleProcess();
}

void QmitkAgentWidget::OnEffortSelected()
{
  m_Session->SetEffort(m_Ui->effortBox->currentData().toString());
  m_Preferences->Put("effort", m_Session->GetEffort().toStdString());

  m_Session->RestartIdleProcess();
}

void QmitkAgentWidget::UpdateEffortBox()
{
  const auto modelIndex = m_Ui->modelBox->currentIndex();
  const auto hasModel = modelIndex >= 0 && modelIndex < m_Models.size();

  const auto levels = hasModel
    ? m_Models[modelIndex].EffortLevels
    : QStringList();

  const auto defaultEffort = hasModel
    ? m_DefaultEfforts.value(m_Models[modelIndex].ResolvedModel)
    : QString();

  auto* box = m_Ui->effortBox;
  const QSignalBlocker blocker(box);
  box->clear();

  box->addItem(defaultEffort.isEmpty()
    ? QString("Default effort")
    : QString("Default effort (%1)").arg(GetEffortName(defaultEffort)), QString());

  for (const auto& level : levels)
    box->addItem(GetEffortText(level), level);

  auto index = box->findData(m_Session->GetEffort());

  // An effort the chosen model does not take.
  if (index < 0)
  {
    m_Session->SetEffort(QString());
    m_Preferences->Put("effort", "");
    index = 0;
  }

  box->setCurrentIndex(index);
}

void QmitkAgentWidget::OnShowStepsToggled(bool show)
{
  m_Preferences->PutBool("showSteps", show);
  m_Ui->conversation->SetStepsShown(show);
}
