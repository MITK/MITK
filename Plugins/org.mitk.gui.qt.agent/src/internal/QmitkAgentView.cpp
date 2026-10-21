/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAgentView.h"
#include "QmitkAgentMemory.h"
#include "QmitkAgentPluginActivator.h"

#include <ctkPlugin.h>
#include <ctkPluginContext.h>
#include <ctkPluginException.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkIRestServerService.h>
#include <mitkLog.h>
#include <mitkMcpEndpoint.h>
#include <mitkMcpRestApiTools.h>
#include <mitkRestGuiThread.h>

#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleRegistry.h>

#include <QmitkIconTheme.h>
#include <QmitkPartLock.h>
#include <QmitkSpeechBubbleFrame.h>

#include <berryIWorkbenchPage.h>

#include <QAbstractTextDocumentLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QStyledItemDelegate>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextFrame>
#include <QTextImageFormat>
#include <QTextList>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QtMath>

#include <algorithm>

const std::string QmitkAgentView::VIEW_ID = "org.mitk.views.agent";

namespace
{
  const QString McpServerName = "mitk";
  const QString ToolPrefix = "mcp__" + McpServerName + "__";
  const std::string McpPath = "/mcp";

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

  /** The server's URL for a client on the same machine. */
  QString GetLocalUrl(const mitk::RestServerConfig& config)
  {
    auto host = QString::fromStdString(config.host);

    if (host == "::" || host == "0.0.0.0" || host == "localhost")
    {
      host = "127.0.0.1";
    }
    else if (host.contains(':'))
    {
      host = '[' + host + ']';
    }

    return QString("%1://%2:%3").arg(config.httpsEnabled ? "https" : "http").arg(host).arg(config.port);
  }
  const QString ImageScheme = "agent-image";
  const QString StepsScheme = "agent-steps";

  // Gestures in render windows return a screenshot each, so thumbnails stay small.
  const QSize ThumbnailSize(160, 100);

  // Around groups of steps and around thumbnails. The same for both: a hidden
  // block still passes its bottom margin on to the next visible one, so a
  // larger thumbnail margin would widen the space after a collapsed group.
  constexpr int StepSpacing = 6;

  // Between the steps of a list, as a step runs over a line for its call and
  // more for its result.
  constexpr int StepItemSpacing = 3;

  constexpr int MessagePadding = 6;

  // Marks the frames around the user's messages, unlike those of tables.
  constexpr int MessageFrameProperty = QTextFormat::UserProperty;

  // The conversation and the input are set larger than the rest of the
  // application, like the VoxTell prompts. Notes and steps keep its size.
  constexpr qreal TextScale = 1.2;

  const QColor NoteColor("gray");
  const QColor ErrorColor("#d9534f");

  // Like the corners of the input's speech bubble.
  constexpr qreal MessageRadius = 8;

  /** \param fontSize In points; 0 for the default font. */
  QString GetSpanHtml(const QString& text, const QColor& color, qreal fontSize)
  {
    const auto size = fontSize > 0
      ? QString(" font-size: %1pt;").arg(fontSize)
      : QString();

    return QString("<span style=\"color: %1;%2\">%3</span>").arg(color.name(), size, text.toHtmlEscaped());
  }

  QString GetNoteHtml(const QString& text, qreal fontSize)
  {
    return GetSpanHtml(text, NoteColor, fontSize);
  }

  QString GetErrorHtml(const QString& text, qreal fontSize)
  {
    return GetSpanHtml(text, ErrorColor, fontSize);
  }

  /** \param fontSize In points; 0 for the default font. */
  QTextCharFormat GetStepFormat(const QColor& color, qreal fontSize)
  {
    QTextCharFormat format;
    format.setForeground(color);

    if (fontSize > 0)
      format.setFontPointSize(fontSize);

    return format;
  }

  /** The frame around a message of the user, whose outline ConversationBrowser draws. */
  QTextFrameFormat GetMessageFrameFormat()
  {
    QTextFrameFormat format;
    format.setProperty(MessageFrameProperty, true);
    format.setPadding(MessagePadding);
    format.setTopMargin(StepSpacing);
    format.setBottomMargin(StepSpacing);
    return format;
  }

  /**
   * Draws the frames around the user's messages with round corners, like the
   * input, which the frames of rich text cannot have.
   */
  class ConversationBrowser : public QTextBrowser
  {
  public:
    using QTextBrowser::QTextBrowser;

  protected:
    void paintEvent(QPaintEvent* event) override
    {
      {
        QPainter painter(this->viewport());
        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(-this->horizontalScrollBar()->value(), -this->verticalScrollBar()->value());

        // Read at every paint, so that the frames follow a change of the theme.
        painter.setPen(QPen(QmitkSpeechBubbleFrame::GetBorderColor(this->palette()), 1));
        painter.setBrush(QmitkSpeechBubbleFrame::GetFillColor(this->palette()));

        const auto* layout = this->document()->documentLayout();

        for (auto* frame : this->document()->rootFrame()->childFrames())
        {
          const auto format = frame->frameFormat();

          if (!format.boolProperty(MessageFrameProperty))
            continue;

          // The bounding rectangle includes the margins.
          const auto rect = layout->frameBoundingRect(frame).adjusted(
            format.leftMargin() + 0.5, format.topMargin() + 0.5, -format.rightMargin() - 0.5, -format.bottomMargin() - 0.5);

          painter.drawRoundedRect(rect, MessageRadius, MessageRadius);
        }
      }

      QTextBrowser::paintEvent(event);
    }
  };

  /**
   * Shows the text of each item as a paragraph in a frame with slightly
   * rounded corners. Edits it in place, and the row grows with the text.
   * Enter confirms, as the text is one paragraph.
   */
  class ParagraphDelegate : public QStyledItemDelegate
  {
  public:
    explicit ParagraphDelegate(QListView* view)
      : QStyledItemDelegate(view),
        m_View(view),
        m_EditorHeight(0)
    {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
      const bool editing = m_EditedIndex == index;

      QPainterPath frame;
      frame.addRoundedRect(QRectF(option.rect).adjusted(0.5, 0.5, -0.5, -0.5), Radius, Radius);

      painter->save();
      painter->setRenderHint(QPainter::Antialiasing);
      painter->fillPath(frame, QmitkSpeechBubbleFrame::GetFillColor(option.palette));

      // The selection and hover of the theme, in the shape of the frame.
      if (!editing)
      {
        painter->save();
        painter->setClipPath(frame, Qt::IntersectClip);
        m_View->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
        painter->restore();
      }

      painter->setPen(editing
        ? option.palette.color(QPalette::Highlight)
        : QmitkSpeechBubbleFrame::GetBorderColor(option.palette));

      painter->drawPath(frame);
      painter->restore();

      // The editor shows the text.
      if (editing)
        return;

      // Without the states that the frame shows already.
      auto textOption = option;
      textOption.rect = option.rect.marginsRemoved(PaddingMargins);
      textOption.state &= ~(QStyle::State_Selected | QStyle::State_MouseOver | QStyle::State_HasFocus);
      QStyledItemDelegate::paint(painter, textOption, index);
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
      // A width of 0 leaves the view to stretch the row to its width.
      if (m_EditedIndex == index && m_EditorHeight > 0)
        return QSize(0, m_EditorHeight + 2 * Padding);

      // Wrapped at the width of the rows that the view paints. It lays them
      // out at that width, but asks for their size at the width of its
      // contents, which may be wider. The text then takes fewer lines there,
      // and the view would paint it elided and leave a gap below.
      auto textOption = option;
      textOption.rect.setWidth(m_View->viewport()->width() - 2 * m_View->spacing());
      textOption.rect = textOption.rect.marginsRemoved(PaddingMargins);
      return QSize(0, QStyledItemDelegate::sizeHint(textOption, index).height() + 2 * Padding);
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const override
    {
      auto* editor = new QTextEdit(parent);
      editor->setAcceptRichText(false);
      editor->setTabChangesFocus(true);
      editor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      editor->setFrameShape(QFrame::NoFrame);
      editor->setStyleSheet("QTextEdit { background: transparent; }");
      editor->document()->setDocumentMargin(0);

      m_EditedIndex = index;
      m_EditorHeight = 0;

      // Queued, as the document signals in the middle of its layout.
      connect(editor->document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, editor,
        [this, editor, index = QPersistentModelIndex(index)](const QSizeF& size) {
          // The view hides an editor before it destroys it.
          if (editor->isHidden())
            return;

          m_EditorHeight = qCeil(size.height()) + 2 * editor->frameWidth();
          m_View->doItemsLayout();
          m_View->scrollTo(index);
        },
        Qt::QueuedConnection);

      return editor;
    }

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const override
    {
      // Where the list shows the text, which the style indents by this margin.
      const auto textMargin = m_View->style()->pixelMetric(QStyle::PM_FocusFrameHMargin, &option, option.widget) + 1;
      editor->setGeometry(option.rect.marginsRemoved(PaddingMargins).adjusted(textMargin, 0, -textMargin, 0));
    }

    void destroyEditor(QWidget* editor, const QModelIndex& index) const override
    {
      m_EditedIndex = QPersistentModelIndex();
      QStyledItemDelegate::destroyEditor(editor, index);

      // Back to the text, once the view has closed the editor.
      QMetaObject::invokeMethod(m_View, &QAbstractItemView::doItemsLayout, Qt::QueuedConnection);
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
      auto* textEdit = static_cast<QTextEdit*>(editor);
      textEdit->setPlainText(index.data(Qt::EditRole).toString());
      textEdit->moveCursor(QTextCursor::End);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {
      model->setData(index, static_cast<QTextEdit*>(editor)->toPlainText(), Qt::EditRole);
    }

  protected:
    bool eventFilter(QObject* object, QEvent* event) override
    {
      if (event->type() == QEvent::KeyPress)
      {
        if (const auto key = static_cast<QKeyEvent*>(event)->key(); key == Qt::Key_Return || key == Qt::Key_Enter)
        {
          // Queued, like for a line edit. A message box from the commit takes
          // the focus, upon which the view closes and destroys the editor.
          QMetaObject::invokeMethod(object, [this, editor = QPointer<QWidget>(static_cast<QWidget*>(object))]() {
            emit this->commitData(editor);

            if (editor != nullptr)
              emit this->closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
          }, Qt::QueuedConnection);

          return true;
        }
      }

      return QStyledItemDelegate::eventFilter(object, event);
    }

  private:
    static constexpr int Padding = 6;
    static constexpr QMargins PaddingMargins = QMargins(Padding, Padding, Padding, Padding);
    static constexpr qreal Radius = 4;

    QListView* m_View;
    mutable QPersistentModelIndex m_EditedIndex;
    mutable int m_EditorHeight;
  };

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
    m_RestServer(nullptr),
    m_StartedRestServer(false),
    m_Process(nullptr),
    m_SignedIn(false),
    m_IgnoreApiKeyVariable(false),
    m_CheckPending(false),
    m_AuthQueries(0),
    m_PendingTurns(0),
    m_Stopping(false),
    m_Killing(false),
    m_StreamBlock(-1),
    m_FollowEnd(true),
    m_NoteFontSize(0.0),
    m_StepGroupOpen(false),
    m_ConversationCost(0.0),
    m_NextImageId(1)
{
}

QmitkAgentView::~QmitkAgentView()
{
  if (m_Process != nullptr)
  {
    m_Process->disconnect(this);
    m_Process->Kill();
  }

  // WithdrawTools() runs the event loop, in which the timer must not lock again.
  if (m_UnlockTimer != nullptr)
    m_UnlockTimer->stop();

  this->WithdrawTools();
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

  m_Conversation = new ConversationBrowser(parent);
  m_Conversation->setObjectName("conversation");
  m_Conversation->setOpenLinks(false);
  m_Conversation->setPlaceholderText("The conversation appears here.");
  layout->addWidget(m_Conversation, 1);

  // In a style sheet, as the application sets its font size in one, which a
  // font set on the widget would not override.
  m_Conversation->ensurePolished();
  m_NoteFontSize = m_Conversation->font().pointSizeF();

  if (m_NoteFontSize > 0)
    m_Conversation->setStyleSheet(QString("QTextBrowser { font-size: %1pt; }").arg(m_NoteFontSize * TextScale));
  else
    m_NoteFontSize = 0.0;

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
    inputStyleSheet += QString("QPlainTextEdit { font-size: %1pt; }").arg(fontSize * TextScale);

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
  connect(m_Conversation, &QTextBrowser::anchorClicked, this, &QmitkAgentView::OnLinkClicked);

  connect(m_Conversation->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
    m_FollowEnd = value == m_Conversation->verticalScrollBar()->maximum();
  });

  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, m_Conversation->viewport(), qOverload<>(&QWidget::update));

  m_Process = new QmitkClaudeCodeProcess(this);
  connect(m_Process, &QmitkClaudeCodeProcess::MessageReceived, this, &QmitkAgentView::OnMessageReceived);
  connect(m_Process, &QmitkClaudeCodeProcess::Initialized, this, &QmitkAgentView::OnInitialized);
  connect(m_Process, &QmitkClaudeCodeProcess::Finished, this, &QmitkAgentView::OnProcessFinished);
  connect(m_Process, &QmitkClaudeCodeProcess::ErrorOccurred, this, &QmitkAgentView::OnProcessError);

  m_Directory.setPath(GetWorkspaceDirectory());
  m_Directory.cdUp();

  m_Memory = std::make_unique<QmitkAgentMemory>(m_Directory.filePath("memory.json"));

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
  auto* context = us::ModuleRegistry::GetModule(1)->GetModuleContext();
  const auto reference = context->GetServiceReference<mitk::IRestServerService>();
  m_RestServer = reference ? context->GetService(reference) : nullptr;

  if (m_RestServer == nullptr)
  {
    this->AppendError("The agent cannot work in the Workbench: this build of MITK has no REST API, through which "
      "the agent operates it.");
    return;
  }

  if (m_RestServer->IsRunning())
  {
    if (!this->AcceptRunningServer())
    {
      this->AppendNote("The agent does not work in the Workbench for now. To let the Agent view start a REST API "
        "server of its own, which only the agent can use, stop the running server in the REST API view and reopen "
        "the Agent view. Or require a token in Preferences > REST API.");
      return;
    }
  }
  else
  {
    // A server for the agent only, whatever the REST API preferences say:
    // only the agent's token gets in, as the main token is empty, no client
    // may name files, and nothing outside this computer can reach it.
    mitk::RestServerConfig config;
    config.enabled = true;
    config.host = "127.0.0.1";
    config.port = 0;
    config.clientAccessMode = mitk::ClientAccessMode::LocalhostOnly;
    config.requireAuth = true;
    config.apiToken.clear();
    config.fileAccessMode = mitk::FileAccessMode::AllowedDirectories;
    config.allowedFileDirectories.clear();
    config.httpsEnabled = false;

    if (!m_RestServer->Start(config, "Agent view"))
    {
      const auto error = QString::fromStdString(m_RestServer->GetLastError().value_or("unknown error"));

      this->AppendError(QString("The agent cannot work in the Workbench, as the REST API server it works through does "
        "not start: %1. The log may tell more. You can also start the server yourself in the REST API view and reopen "
        "the Agent view.").arg(error));
      return;
    }

    m_StartedRestServer = true;
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

  m_Endpoint = std::make_unique<mitk::McpEndpoint>("MITK Workbench", "0.2");
  mitk::AddRestApiTools(*m_Endpoint, m_RestServer);
  this->AddMemoryTools(*m_Endpoint);

  try
  {
    m_Token = QString::fromStdString(m_RestServer->CreateToken("Agent view", mitk::RestTokenRights()));
    m_Endpoint->Offer(m_RestServer, McpPath);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << e.GetDescription();
    this->AppendError(QString("The Workbench tools are not available: %1").arg(e.GetDescription()));
    return;
  }

  m_McpUrl = GetLocalUrl(m_RestServer->GetRunningConfig().value()) + "/api/v1" + QString::fromStdString(McpPath);
}

void QmitkAgentView::WithdrawTools()
{
  m_McpUrl.clear();

  if (m_Endpoint != nullptr)
  {
    m_Endpoint->Withdraw();

    // Calls that run wait for the GUI thread, which is this one.
    while (m_Endpoint->GetRunningCallCount() > 0)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 50);

    m_Endpoint.reset();
  }

  if (m_RestServer == nullptr)
    return;

  if (!m_Token.isEmpty())
  {
    m_RestServer->RemoveToken(m_Token.toStdString());
    m_Token.clear();
  }

  if (m_StartedRestServer)
  {
    m_RestServer->Stop();
    m_StartedRestServer = false;
  }
}

bool QmitkAgentView::AreToolsOffered() const
{
  return !m_McpUrl.isEmpty();
}

bool QmitkAgentView::AcceptRunningServer()
{
  const auto config = m_RestServer->GetRunningConfig();
  auto* preferences = GetPreferences();

  if (!config.has_value() || !preferences->GetBool("warnAboutOpenRestServer", true))
    return true;

  QStringList risks;
  const bool remote = config->clientAccessMode != mitk::ClientAccessMode::LocalhostOnly;

  if (!config->requireAuth)
    risks << "Programs on this computer can use it without a token, and web pages in your browser can use parts of it.";

  if (remote)
    risks << "Other computers can reach it.";

  if (config->fileAccessMode == mitk::FileAccessMode::Unrestricted && (!config->requireAuth || remote))
    risks << "These clients can load any file you can read into the Workbench.";

  if (risks.isEmpty())
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

void QmitkAgentView::AddMemoryTools(mitk::McpEndpoint& endpoint)
{
  auto* memory = m_Memory.get();
  const auto* server = m_RestServer;

  mitk::McpTool remember;
  remember.Name = "remember";
  remember.Description =
    "Save a note to your memory. You get your notes at the start of every later conversation. Save what you "
    "learned about operating the Workbench, from the user or by trying, that will help you next time: how a "
    "tool behaves, a step that is easy to get wrong, or what the user prefers. One fact per note, short and "
    "general. Never save patient data, image contents, or findings. The user sees each note in the chat and "
    "can edit or delete it.";
  remember.InputSchema = {
    { "type", "object" },
    { "properties", { { "note", { { "type", "string" } } } } },
    { "required", { "note" } }
  };
  remember.Handler = [memory, server](const nlohmann::json& arguments, const mitk::RestRequest&) {
    const auto note = QString::fromStdString(arguments.at("note").get<std::string>());

    return mitk::CallInGuiThread(*server, [memory, note]() {
      const auto id = memory->Add(note);
      return mitk::McpToolResult::FromText(QString("Saved as note %1.").arg(id).toStdString());
    });
  };
  endpoint.AddTool(remember);

  mitk::McpTool forget;
  forget.Name = "forget";
  forget.Description =
    "Delete a note from your memory, for example one that turned out to be wrong or outdated. To correct a "
    "note, delete it and save the corrected one.";
  forget.InputSchema = {
    { "type", "object" },
    { "properties", { { "id", { { "type", "integer" }, { "description", "id of the note" } } } } },
    { "required", { "id" } }
  };
  forget.Handler = [memory, server](const nlohmann::json& arguments, const mitk::RestRequest&) {
    const auto id = arguments.at("id").get<int>();

    return mitk::CallInGuiThread(*server, [memory, id]() {
      memory->Remove(id);
      return mitk::McpToolResult::FromText(QString("Deleted note %1.").arg(id).toStdString());
    });
  };
  endpoint.AddTool(forget);
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
  if (!this->AreToolsOffered())
    return;

  const QJsonObject server = {
    { "type", "http" },
    { "url", m_McpUrl },
    { "headers", QJsonObject{ { "Authorization", "Bearer " + m_Token } } }
  };

  const auto configPath = m_Directory.filePath("mcp.json");
  QFile config(configPath);

  if (!config.open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
    this->AppendError(QString("Cannot write %1.").arg(configPath));
    return;
  }

  config.write(QJsonDocument(QJsonObject{ { "mcpServers", QJsonObject{ { McpServerName, server } } } }).toJson());
  config.close();

  QFile promptResource(":/org.mitk.gui.qt.agent/AgentSystemPrompt.md");

  if (!promptResource.open(QIODevice::ReadOnly))
  {
    this->AppendError("The agent's instructions are missing from the plugin.");
    return;
  }

  const auto promptPath = m_Directory.filePath("system-prompt.md");
  QFile prompt(promptPath);

  // Earlier versions copied the resource, which left a read-only file.
  if (prompt.exists())
    prompt.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

  if (!prompt.open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
    this->AppendError(QString("Cannot write %1.").arg(promptPath));
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

  if (!this->AreToolsOffered())
  {
    this->AppendNote("The agent cannot work in the Workbench right now, see the messages above.");
    return;
  }

  m_Input->clear();
  m_FollowEnd = true;
  this->AppendUserMessage(text);

  if (!m_Process->IsRunning())
    this->StartProcess();

  if (!m_Process->IsRunning())
    return;

  if (m_Stopping)
  {
    m_Stopping = false;
    m_Endpoint->AcceptCalls();
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
  m_Endpoint->RefuseCalls("The user stopped you, so this call did nothing. Wait for the user's next message.");
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
  m_Conversation->clear();
  m_Images.clear();
  m_StreamBlock = -1;
  m_StepGroups.clear();
  m_StepGroupOpen = false;
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
  QDialog dialog(m_Conversation);
  dialog.setWindowTitle("Agent Memory");

  auto* description = new QLabel("The agent gets these notes at the start of every conversation. Double-click a note to edit it.", &dialog);
  description->setWordWrap(true);

  auto* list = new QListWidget(&dialog);
  list->setWordWrap(true);
  list->setSpacing(6);
  list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  list->setItemDelegate(new ParagraphDelegate(list));

  // In the size of the conversation, in a style sheet like there. The editor
  // needs its own rule, as the application sets the size of every widget.
  if (const auto fontSize = m_Conversation->font().pointSizeF(); fontSize > 0)
    list->setStyleSheet(QString("QListWidget, QTextEdit { font-size: %1pt; }").arg(fontSize));

  for (const auto& note : m_Memory->GetNotes())
  {
    auto* item = new QListWidgetItem(note.Text, list);
    item->setData(Qt::UserRole, note.Id);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
  }

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  auto* deleteButton = buttons->addButton("Delete", QDialogButtonBox::ActionRole);
  deleteButton->setEnabled(false);

  auto* layout = new QVBoxLayout(&dialog);
  layout->addWidget(description);
  layout->addWidget(list);
  layout->addWidget(buttons);

  bool changed = false;

  connect(list, &QListWidget::itemSelectionChanged, &dialog, [list, deleteButton]() {
    deleteButton->setEnabled(!list->selectedItems().isEmpty());
  });

  connect(list, &QListWidget::itemChanged, &dialog, [this, &dialog, &changed, list](QListWidgetItem* item) {
    const auto id = item->data(Qt::UserRole).toInt();

    try
    {
      m_Memory->SetText(id, item->text());
      changed = true;
    }
    catch (const std::exception& e)
    {
      QMessageBox::warning(&dialog, "Agent Memory", QString::fromStdString(e.what()));
    }

    // Show the note as stored: normalized, or unchanged if it was rejected.
    for (const auto& note : m_Memory->GetNotes())
    {
      if (note.Id == id)
      {
        const QSignalBlocker blocker(list);
        item->setText(note.Text);
      }
    }
  });

  connect(deleteButton, &QPushButton::clicked, &dialog, [this, &dialog, &changed, list]() {
    for (auto* item : list->selectedItems())
    {
      try
      {
        m_Memory->Remove(item->data(Qt::UserRole).toInt());
        delete item;
        changed = true;
      }
      catch (const std::exception& e)
      {
        QMessageBox::warning(&dialog, "Agent Memory", QString::fromStdString(e.what()));
        return;
      }
    }
  });

  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  dialog.resize(560, 400);
  dialog.exec();

  if (changed)
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
  const bool callsRunning = m_Endpoint != nullptr && m_Endpoint->GetRunningCallCount() > 0;

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
      this->AppendError(QString("Claude Code could not connect to the Workbench tools (%1).").arg(object.value("status").toString()));
  }

  // Built-in tools would give the agent the user's files and a shell.
  for (const auto& tool : message.value("tools").toArray())
  {
    if (!tool.toString().startsWith(ToolPrefix))
    {
      this->AppendError(QString("Claude Code offers the tool \"%1\", which the Agent view does not allow. Stopping.").arg(tool.toString()));
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
      m_StreamBlock = this->AppendBlock().blockNumber();
    }
    else if (blockType == "tool_use")
    {
      this->SetActivity(QString("Using %1").arg(GetToolName(block.value("name").toString())));
    }
  }
  else if (type == "content_block_delta" && m_StreamBlock >= 0)
  {
    const auto delta = event.value("delta").toObject();

    if (delta.value("type").toString() != "text_delta")
      return;

    QTextCursor cursor(m_Conversation->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(delta.value("text").toString(), QTextCharFormat());
    this->ScrollToEnd();
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
      const auto text = object.value("text").toString();

      if (m_StreamBlock >= 0)
      {
        // The complete text replaces the streamed one, formatted now.
        QTextCursor cursor(m_Conversation->document()->findBlockByNumber(m_StreamBlock));
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.insertMarkdown(text);
        m_StreamBlock = -1;
        this->ScrollToEnd();
      }
      else
      {
        this->AppendMarkdown(text);
      }
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
    const auto text = ToolResultText(content);
    const auto failed = object.value("is_error").toBool();

    auto cursor = this->AppendStepItem();

    if (!call.isEmpty())
      cursor.insertText(Abbreviate(DescribeToolCall(name, call.value("input").toObject()), 200), GetStepFormat(NoteColor, m_NoteFontSize));

    // The result only where it says why a call failed. The transcript of
    // Claude Code keeps all of them.
    if (failed || call.isEmpty())
    {
      if (!call.isEmpty())
        cursor.insertText(QString(QChar::LineSeparator));

      cursor.insertText(Abbreviate(failed ? DescribeFailure(text) : text, 300), GetStepFormat(failed ? ErrorColor : NoteColor, m_NoteFontSize));
    }

    const auto thumbnails = this->CreateThumbnails(content);

    if (!thumbnails.isEmpty())
    {
      cursor = this->AppendStepImageBlock();

      for (qsizetype i = 0; i < thumbnails.size(); ++i)
      {
        if (i > 0)
          cursor.insertText(" ");

        cursor.insertImage(thumbnails[i]);
      }
    }

    this->FinishStep(name, failed);
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
    this->AppendNote(QString("Done in %1 s (API-equivalent cost $%2).").arg(seconds, 0, 'f', 1).arg(cost, 0, 'f', 2));
  }
  else if (subtype == "error_during_execution")
  {
    this->AppendNote("Stopped.");
  }
  else
  {
    QStringList errors;

    for (const auto& error : message.value("errors").toArray())
      errors << error.toString();

    this->AppendError(QString("The agent stopped (%1). %2").arg(subtype, errors.join(' ')));
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

  this->AppendError(text + " The next message continues the conversation.");
}

void QmitkAgentView::OnProcessError(const QString& description)
{
  this->AppendError(description);
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

void QmitkAgentView::OnLinkClicked(const QUrl& url)
{
  if (url.scheme() == ImageScheme)
  {
    this->ShowImage(url.path().toInt());
  }
  else if (url.scheme() == StepsScheme)
  {
    const auto index = url.path().toULongLong();

    if (index < m_StepGroups.size())
      this->SetStepGroupExpanded(index, !m_StepGroups[index].Expanded);
  }
  // Links in the agent's answers must not open local files or programs.
  else if (url.scheme() == "https" || url.scheme() == "http")
  {
    QDesktopServices::openUrl(url);
  }
}

QList<QTextImageFormat> QmitkAgentView::CreateThumbnails(const QJsonValue& content)
{
  QList<QTextImageFormat> thumbnails;
  const auto pixelRatio = m_Conversation->devicePixelRatioF();

  for (const auto& item : content.toArray())
  {
    const auto object = item.toObject();
    const auto source = object.value("source").toObject();

    if (object.value("type").toString() != "image" || source.value("type").toString() != "base64")
      continue;

    const auto data = QByteArray::fromBase64(source.value("data").toString().toLatin1());
    QImage image;

    if (!image.loadFromData(data))
      continue;

    const auto size = image.size().scaled(ThumbnailSize, Qt::KeepAspectRatio).boundedTo(image.size());
    auto thumbnail = image.scaled(size * pixelRatio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    thumbnail.setDevicePixelRatio(pixelRatio);

    const auto id = m_NextImageId++;
    const auto url = QString("%1:%2").arg(ImageScheme).arg(id);

    m_Images.insert(id, data);
    m_Conversation->document()->addResource(QTextDocument::ImageResource, QUrl(url), thumbnail);

    QTextImageFormat format;
    format.setName(url);
    format.setWidth(size.width());
    format.setHeight(size.height());
    format.setAnchor(true);
    format.setAnchorHref(url);
    thumbnails << format;
  }

  return thumbnails;
}

void QmitkAgentView::ShowImage(int id)
{
  QPixmap pixmap;

  if (!pixmap.loadFromData(m_Images.value(id)))
    return;

  // Not modal, so that the agent can go on working and the user can compare.
  auto* dialog = new QDialog(m_Conversation);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowTitle("Screenshot");

  auto* image = new QLabel(dialog);
  image->setPixmap(pixmap);

  auto* scrollArea = new QScrollArea(dialog);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setWidget(image);

  auto* layout = new QVBoxLayout(dialog);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(scrollArea);

  const auto available = m_Conversation->screen()->availableGeometry().size() * 0.9;
  dialog->resize(pixmap.size().boundedTo(available));
  dialog->show();
}

QTextCursor QmitkAgentView::AppendBlock()
{
  // The space after a group of steps, like the one before its summary.
  QTextBlockFormat format;

  if (m_StepGroupOpen)
    format.setTopMargin(StepSpacing);

  m_StreamBlock = -1;
  m_StepGroupOpen = false;

  QTextCursor cursor(m_Conversation->document());
  cursor.movePosition(QTextCursor::End);

  // The last block is empty at the start, and after a frame like the one
  // around a message of the user, which is always followed by a block.
  if (cursor.block().length() > 1)
  {
    cursor.insertBlock(format, QTextCharFormat());
  }
  else
  {
    cursor.setBlockFormat(format);
    cursor.setBlockCharFormat(QTextCharFormat());
  }

  return cursor;
}

void QmitkAgentView::AppendUserMessage(const QString& text)
{
  m_StreamBlock = -1;
  m_StepGroupOpen = false;

  auto* document = m_Conversation->document();
  QTextCursor cursor(document);
  cursor.movePosition(QTextCursor::End);

  // A frame starts after the block at the cursor. Empty, as at the start and
  // after an earlier frame, that block would show as an empty line above it.
  const auto emptyBlock = cursor.block().length() == 1
    ? cursor.blockNumber()
    : -1;

  cursor.insertFrame(GetMessageFrameFormat());
  cursor.insertHtml(QString("<b>You:</b> %1").arg(text.toHtmlEscaped().replace('\n', "<br>")));

  if (emptyBlock >= 0)
  {
    auto block = document->findBlockByNumber(emptyBlock);
    block.setVisible(false);
    document->markContentsDirty(block.position(), block.length());
  }

  this->ScrollToEnd();
}

QTextCursor QmitkAgentView::AppendStepItem()
{
  if (!m_StepGroupOpen)
  {
    auto summary = this->AppendBlock();

    QTextBlockFormat format;
    format.setTopMargin(StepSpacing);
    summary.setBlockFormat(format);

    m_StepGroups.push_back({ summary.blockNumber(), summary.blockNumber(), nullptr, 0, 0, 0, 0, m_ShowStepsCheckBox->isChecked() });
    m_StepGroupOpen = true;
  }

  auto& group = m_StepGroups.back();

  QTextCursor cursor(m_Conversation->document());
  cursor.movePosition(QTextCursor::End);

  QTextBlockFormat itemFormat;
  itemFormat.setBottomMargin(StepItemSpacing);

  // The bullet takes the block's character format.
  cursor.insertBlock(itemFormat, GetStepFormat(NoteColor, m_NoteFontSize));

  if (group.List == nullptr)
  {
    QTextListFormat format;
    format.setStyle(QTextListFormat::ListDisc);
    format.setIndent(1);
    group.List = cursor.createList(format);
  }
  else
  {
    group.List->add(cursor.block());
  }

  return cursor;
}

QTextCursor QmitkAgentView::AppendStepImageBlock()
{
  // Not part of the list, but indented as far as the text of its items.
  QTextBlockFormat format;
  format.setIndent(1);
  format.setTopMargin(StepSpacing);
  format.setBottomMargin(StepSpacing);

  QTextCursor cursor(m_Conversation->document());
  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock(format, QTextCharFormat());
  return cursor;
}

void QmitkAgentView::FinishStep(const QString& toolName, bool failed)
{
  auto& group = m_StepGroups.back();
  auto* document = m_Conversation->document();

  if (!group.Expanded)
  {
    const auto first = document->findBlockByNumber(group.LastBlock + 1);

    for (auto block = first; block.isValid(); block = block.next())
      block.setVisible(false);

    document->markContentsDirty(first.position(), document->characterCount() - first.position());
  }

  group.LastBlock = document->lastBlock().blockNumber();
  ++group.Steps;

  if (failed)
  {
    ++group.Failures;
  }
  else if (toolName == "remember")
  {
    ++group.SavedNotes;
  }
  else if (toolName == "forget")
  {
    ++group.DeletedNotes;
  }

  this->UpdateStepSummary(m_StepGroups.size() - 1);
  this->ScrollToEnd();
}

void QmitkAgentView::UpdateStepSummary(std::size_t index)
{
  const auto& group = m_StepGroups[index];

  auto action = QString("%1 %2")
    .arg(group.Expanded ? "Hide" : "Show")
    .arg(group.Steps == 1 ? QString("1 step") : QString("%1 steps").arg(group.Steps));

  if (group.Failures > 0)
    action += QString(", %1 failed").arg(group.Failures);

  QTextCursor cursor(m_Conversation->document()->findBlockByNumber(group.SummaryBlock));
  cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

  QTextCharFormat format;
  format.setAnchor(true);
  format.setAnchorHref(QString("%1:%2").arg(StepsScheme).arg(index));
  format.setForeground(NoteColor);
  format.setFontUnderline(true);

  if (m_NoteFontSize > 0)
    format.setFontPointSize(m_NoteFontSize);

  cursor.insertText(action, format);

  // Most steps go through the same tools of the REST API, so naming them says
  // little. Changes of the memory are worth a look, though.
  QStringList memory;

  if (group.SavedNotes > 0)
    memory << (group.SavedNotes == 1 ? QString("saved a note") : QString("saved %1 notes").arg(group.SavedNotes));

  if (group.DeletedNotes > 0)
    memory << (group.DeletedNotes == 1 ? QString("deleted a note") : QString("deleted %1 notes").arg(group.DeletedNotes));

  if (!memory.isEmpty())
  {
    format.setFontUnderline(false);
    cursor.insertText(QString(" (%1)").arg(memory.join(", ")), format);
  }
}

void QmitkAgentView::SetStepGroupExpanded(std::size_t index, bool expanded)
{
  auto& group = m_StepGroups[index];
  group.Expanded = expanded;

  if (group.LastBlock > group.SummaryBlock)
  {
    auto* document = m_Conversation->document();
    const auto first = document->findBlockByNumber(group.SummaryBlock + 1);
    const auto last = document->findBlockByNumber(group.LastBlock);

    for (auto block = first; block.isValid() && block.blockNumber() <= group.LastBlock; block = block.next())
      block.setVisible(expanded);

    document->markContentsDirty(first.position(), last.position() + last.length() - first.position());
  }

  this->UpdateStepSummary(index);
}

void QmitkAgentView::OnShowStepsToggled(bool show)
{
  GetPreferences()->PutBool("showSteps", show);

  for (std::size_t i = 0; i < m_StepGroups.size(); ++i)
    this->SetStepGroupExpanded(i, show);
}

void QmitkAgentView::AppendHtml(const QString& html)
{
  auto cursor = this->AppendBlock();
  cursor.insertHtml(html);
  this->ScrollToEnd();
}

void QmitkAgentView::AppendMarkdown(const QString& markdown)
{
  auto cursor = this->AppendBlock();
  cursor.insertMarkdown(markdown);
  this->ScrollToEnd();
}

void QmitkAgentView::AppendNote(const QString& text)
{
  this->AppendHtml(GetNoteHtml(text, m_NoteFontSize));
}

void QmitkAgentView::AppendError(const QString& text)
{
  this->AppendHtml(GetErrorHtml(text, m_NoteFontSize));
}

void QmitkAgentView::ScrollToEnd()
{
  if (m_FollowEnd)
    m_Conversation->verticalScrollBar()->setValue(m_Conversation->verticalScrollBar()->maximum());
}
