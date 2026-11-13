/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkAgentConversationWidget.h>

#include <QmitkIconTheme.h>
#include <QmitkSpeechBubbleFrame.h>

#include <QAbstractTextDocumentLayout>
#include <QDesktopServices>
#include <QDialog>
#include <QHelpEvent>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextFrame>
#include <QTextImageFormat>
#include <QTextList>
#include <QToolTip>
#include <QVBoxLayout>

namespace
{
  const QString ImageScheme = "agent-image";
  const QString PreviewScheme = "agent-preview";
  const QString StepsScheme = "agent-steps";

  // Gestures in render windows return a screenshot each, so thumbnails stay small.
  const QSize ThumbnailSize(160, 100);

  // Smaller still, as a preview gathers the images of all steps of a group.
  const QSize PreviewSize(96, 60);

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

  /** \param fontSize In points; 0 for the default font. */
  QTextCharFormat GetStepFormat(const QColor& color, qreal fontSize)
  {
    QTextCharFormat format;
    format.setForeground(color);

    if (fontSize > 0)
      format.setFontPointSize(fontSize);

    return format;
  }

  /** The frame around a message of the user, whose outline paintEvent() draws. */
  QTextFrameFormat GetMessageFrameFormat()
  {
    QTextFrameFormat format;
    format.setProperty(MessageFrameProperty, true);
    format.setPadding(MessagePadding);
    format.setTopMargin(StepSpacing);
    format.setBottomMargin(StepSpacing);
    return format;
  }

  /** Inserts the thumbnails in a row that wraps, after those the block already has. */
  void InsertThumbnails(QTextCursor& cursor, const QList<QTextImageFormat>& thumbnails)
  {
    for (const auto& thumbnail : thumbnails)
    {
      // In a format of its own, as the space would take that of the image
      // before it and link to it.
      if (!cursor.atBlockStart())
        cursor.insertText(" ", QTextCharFormat());

      cursor.insertImage(thumbnail);
    }
  }
}

QmitkAgentConversationWidget::QmitkAgentConversationWidget(QWidget* parent)
  : QTextBrowser(parent),
    m_StreamBlock(-1),
    m_StreamLastBlock(-1),
    m_FollowEnd(true),
    m_NoteFontSize(0.0),
    m_StepGroupOpen(false),
    m_StepsShown(false),
    m_NextImageId(1)
{
  this->setOpenLinks(false);

  // In a style sheet, as the application sets its font size in one, which a
  // font set on the widget would not override.
  this->ensurePolished();
  m_NoteFontSize = this->font().pointSizeF();

  if (m_NoteFontSize > 0)
    this->setStyleSheet(QString("QTextBrowser { font-size: %1pt; }").arg(m_NoteFontSize * TextScale));
  else
    m_NoteFontSize = 0.0;

  connect(this, &QTextBrowser::anchorClicked, this, &QmitkAgentConversationWidget::OnLinkClicked);

  connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
    m_FollowEnd = value == this->verticalScrollBar()->maximum();
  });

  // The frames take the theme's colors when painted, the summaries of steps
  // when written.
  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, [this]() {
    for (std::size_t i = 0; i < m_StepGroups.size(); ++i)
      this->UpdateStepSummary(i);

    this->viewport()->update();
  });
}

QmitkAgentConversationWidget::~QmitkAgentConversationWidget() = default;

void QmitkAgentConversationWidget::AppendUserMessage(const QString& text)
{
  // The user's own message brings the conversation back to its end.
  m_FollowEnd = true;
  m_StepGroupOpen = false;
  this->InterruptStreamedText();

  auto* document = this->document();
  QTextCursor cursor(document);
  cursor.movePosition(QTextCursor::End);

  // A frame starts after the block at the cursor. Empty, as at the start and
  // after an earlier frame, that block would show as an empty line above it.
  const auto emptyBlock = cursor.block().length() == 1
    ? cursor.blockNumber()
    : -1;

  cursor.insertFrame(GetMessageFrameFormat());

  QTextCharFormat labelFormat;
  labelFormat.setFontWeight(QFont::Bold);
  cursor.insertText("You:", labelFormat);

  // Not as HTML, which would collapse indentation and runs of spaces.
  cursor.insertText(QChar(' ') + QString(text).replace(QChar('\n'), QChar(QChar::LineSeparator)), QTextCharFormat());

  if (emptyBlock >= 0)
  {
    auto block = document->findBlockByNumber(emptyBlock);
    block.setVisible(false);
    document->markContentsDirty(block.position(), block.length());
  }

  this->ScrollToEnd();
}

void QmitkAgentConversationWidget::AppendMarkdown(const QString& markdown)
{
  auto cursor = this->AppendBlock();
  cursor.insertMarkdown(markdown);
  this->ScrollToEnd();
}

void QmitkAgentConversationWidget::AppendNote(const QString& text)
{
  this->AppendHtml(GetSpanHtml(text, NoteColor, m_NoteFontSize));
}

void QmitkAgentConversationWidget::AppendError(const QString& text)
{
  this->AppendHtml(GetSpanHtml(text, ErrorColor, m_NoteFontSize));
}

void QmitkAgentConversationWidget::BeginStreamedText()
{
  m_StreamBlock = this->AppendBlock().blockNumber();
  m_StreamLastBlock = -1;
}

void QmitkAgentConversationWidget::AppendStreamedText(const QString& text)
{
  if (m_StreamBlock < 0 || m_StreamLastBlock >= 0)
    return;

  QTextCursor cursor(this->document());
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(text, QTextCharFormat());
  this->ScrollToEnd();
}

void QmitkAgentConversationWidget::EndStreamedText(const QString& markdown)
{
  if (m_StreamBlock < 0)
  {
    this->AppendMarkdown(markdown);
    return;
  }

  auto* document = this->document();
  const auto blockCount = document->blockCount();
  auto text = markdown;

  // The complete text replaces the streamed one, formatted now.
  QTextCursor cursor(document->findBlockByNumber(m_StreamBlock));

  if (m_StreamLastBlock < 0)
  {
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
  }
  else
  {
    const auto last = document->findBlockByNumber(m_StreamLastBlock);
    cursor.setPosition(last.position() + last.length() - 1, QTextCursor::KeepAnchor);

    // Above other content, a new line at the end would add an empty block.
    while (text.endsWith('\n'))
      text.chop(1);
  }

  cursor.removeSelectedText();
  cursor.insertMarkdown(text);

  // Groups of steps below the text find their blocks by number.
  const auto addedBlocks = document->blockCount() - blockCount;

  for (auto& group : m_StepGroups)
  {
    if (group.SummaryBlock > m_StreamBlock)
    {
      group.SummaryBlock += addedBlocks;
      group.PreviewBlock += addedBlocks;
      group.LastBlock += addedBlocks;
    }
  }

  m_StreamBlock = -1;
  m_StreamLastBlock = -1;
  this->ScrollToEnd();
}

void QmitkAgentConversationWidget::InterruptStreamedText()
{
  if (m_StreamBlock < 0 || m_StreamLastBlock >= 0)
    return;

  // What follows takes an empty block at the end as its own.
  auto last = this->document()->lastBlock();

  if (last.length() == 1)
    last = last.previous();

  if (last.isValid() && last.blockNumber() >= m_StreamBlock)
  {
    m_StreamLastBlock = last.blockNumber();
  }
  else
  {
    m_StreamBlock = -1;
  }
}

void QmitkAgentConversationWidget::AppendStep(const QmitkAgentStep& step)
{
  auto cursor = this->AppendStepItem();

  if (!step.Call.isEmpty())
    cursor.insertText(step.Call, GetStepFormat(NoteColor, m_NoteFontSize));

  if (!step.Result.isEmpty())
  {
    if (!step.Call.isEmpty())
      cursor.insertText(QString(QChar::LineSeparator));

    cursor.insertText(step.Result, GetStepFormat(step.Failed ? ErrorColor : NoteColor, m_NoteFontSize));
  }

  QList<QTextImageFormat> thumbnails;
  QList<QTextImageFormat> previews;

  for (const auto& data : step.Images)
  {
    QImage image;

    if (!image.loadFromData(data))
      continue;

    const auto id = m_NextImageId++;
    const auto link = QString("%1:%2").arg(ImageScheme).arg(id);
    m_Images.insert(id, data);

    thumbnails << this->CreateThumbnail(image, ThumbnailSize, link, link);
    previews << this->CreateThumbnail(image, PreviewSize, QString("%1:%2").arg(PreviewScheme).arg(id), link);
  }

  if (!thumbnails.isEmpty())
  {
    cursor = this->AppendStepImageBlock();
    InsertThumbnails(cursor, thumbnails);

    this->AppendPreviews(previews);
  }

  this->FinishStep(step);
}

void QmitkAgentConversationWidget::SetStepsShown(bool shown)
{
  m_StepsShown = shown;

  for (std::size_t i = 0; i < m_StepGroups.size(); ++i)
    this->SetStepGroupExpanded(i, shown);
}

void QmitkAgentConversationWidget::Clear()
{
  this->clear();
  m_Images.clear();
  m_StreamBlock = -1;
  m_StreamLastBlock = -1;
  m_StepGroups.clear();
  m_StepGroupOpen = false;
}

void QmitkAgentConversationWidget::paintEvent(QPaintEvent* event)
{
  // Draws the frames around the user's messages with round corners, like the
  // input, which the frames of rich text cannot have.
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

bool QmitkAgentConversationWidget::viewportEvent(QEvent* event)
{
  if (event->type() == QEvent::ToolTip)
  {
    const auto* helpEvent = static_cast<QHelpEvent*>(event);
    const QUrl url(this->anchorAt(helpEvent->pos()));

    if (url.scheme() == "https" || url.scheme() == "http")
    {
      // Encoded, so that a look-alike host shows as what it is.
      QToolTip::showText(helpEvent->globalPos(), QString::fromUtf8(url.toEncoded()), this->viewport());
      return true;
    }
  }

  return QTextBrowser::viewportEvent(event);
}

QTextCursor QmitkAgentConversationWidget::AppendBlock()
{
  // The space after a group of steps, like the one before its summary.
  QTextBlockFormat format;
  const bool afterSteps = m_StepGroupOpen;

  if (afterSteps)
    format.setTopMargin(StepSpacing);

  this->InterruptStreamedText();
  m_StepGroupOpen = false;

  QTextCursor cursor(this->document());
  cursor.movePosition(QTextCursor::End);

  // The last block is empty at the start, and after a frame like the one
  // around a message of the user, which is always followed by a block. The
  // empty block of a step without text stays with its group, though, as it
  // is hidden and shown with it.
  if (afterSteps || cursor.block().length() > 1)
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

QTextCursor QmitkAgentConversationWidget::AppendStepItem()
{
  if (!m_StepGroupOpen)
  {
    auto cursor = this->AppendBlock();

    QTextBlockFormat format;
    format.setTopMargin(StepSpacing);
    cursor.setBlockFormat(format);

    const auto summaryBlock = cursor.blockNumber();

    // The preview has no bottom margin: hidden while the steps show, it would
    // pass it on to the first of them.
    cursor.insertBlock(format, QTextCharFormat());

    m_StepGroups.push_back({
      .SummaryBlock = summaryBlock,
      .PreviewBlock = cursor.blockNumber(),
      .LastBlock = cursor.blockNumber(),
      .List = nullptr,
      .Steps = 0,
      .Failures = 0,
      .SavedNotes = 0,
      .DeletedNotes = 0,
      .Expanded = m_StepsShown });

    m_StepGroupOpen = true;
    this->UpdatePreview(m_StepGroups.back());
  }

  auto& group = m_StepGroups.back();

  QTextCursor cursor(this->document());
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

QTextCursor QmitkAgentConversationWidget::AppendStepImageBlock()
{
  // Not part of the list, but indented as far as the text of its items.
  QTextBlockFormat format;
  format.setIndent(1);
  format.setTopMargin(StepSpacing);
  format.setBottomMargin(StepSpacing);

  QTextCursor cursor(this->document());
  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock(format, QTextCharFormat());
  return cursor;
}

void QmitkAgentConversationWidget::FinishStep(const QmitkAgentStep& step)
{
  auto& group = m_StepGroups.back();
  auto* document = this->document();

  if (!group.Expanded)
  {
    const auto first = document->findBlockByNumber(group.LastBlock + 1);

    for (auto block = first; block.isValid(); block = block.next())
      block.setVisible(false);

    document->markContentsDirty(first.position(), document->characterCount() - first.position());
  }

  group.LastBlock = document->lastBlock().blockNumber();
  ++group.Steps;

  if (step.Failed)
  {
    ++group.Failures;
  }
  else if (step.Memory == QmitkAgentStep::MemoryChange::NoteSaved)
  {
    ++group.SavedNotes;
  }
  else if (step.Memory == QmitkAgentStep::MemoryChange::NoteDeleted)
  {
    ++group.DeletedNotes;
  }

  this->UpdateStepSummary(m_StepGroups.size() - 1);
  this->ScrollToEnd();
}

void QmitkAgentConversationWidget::UpdateStepSummary(std::size_t index)
{
  const auto& group = m_StepGroups[index];

  auto action = QString("%1 %2")
    .arg(group.Expanded ? "Hide" : "Show")
    .arg(group.Steps == 1 ? QString("1 step") : QString("%1 steps").arg(group.Steps));

  if (group.Failures > 0)
    action += QString(", %1 failed").arg(group.Failures);

  QTextCursor cursor(this->document()->findBlockByNumber(group.SummaryBlock));
  cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

  QTextCharFormat format;
  format.setAnchor(true);
  format.setAnchorHref(QString("%1:%2").arg(StepsScheme).arg(index));
  format.setForeground(QColor(QmitkIconTheme::GetAccentColor()));

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
    cursor.insertText(QString(" (%1)").arg(memory.join(", ")), format);
}

void QmitkAgentConversationWidget::SetStepGroupExpanded(std::size_t index, bool expanded)
{
  auto& group = m_StepGroups[index];
  group.Expanded = expanded;

  if (group.LastBlock > group.PreviewBlock)
  {
    auto* document = this->document();
    const auto first = document->findBlockByNumber(group.PreviewBlock + 1);
    const auto last = document->findBlockByNumber(group.LastBlock);

    for (auto block = first; block.isValid() && block.blockNumber() <= group.LastBlock; block = block.next())
      block.setVisible(expanded);

    document->markContentsDirty(first.position(), last.position() + last.length() - first.position());
  }

  this->UpdatePreview(group);
  this->UpdateStepSummary(index);
}

void QmitkAgentConversationWidget::AppendPreviews(const QList<QTextImageFormat>& previews)
{
  const auto& group = m_StepGroups.back();

  QTextCursor cursor(this->document()->findBlockByNumber(group.PreviewBlock));
  cursor.movePosition(QTextCursor::EndOfBlock);
  InsertThumbnails(cursor, previews);

  this->UpdatePreview(group);
}

void QmitkAgentConversationWidget::UpdatePreview(const StepGroup& group)
{
  auto* document = this->document();
  auto block = document->findBlockByNumber(group.PreviewBlock);

  // Empty, it would show as an empty line.
  block.setVisible(!group.Expanded && block.length() > 1);
  document->markContentsDirty(block.position(), block.length());
}

void QmitkAgentConversationWidget::AppendHtml(const QString& html)
{
  auto cursor = this->AppendBlock();
  cursor.insertHtml(html);
  this->ScrollToEnd();
}

QTextImageFormat QmitkAgentConversationWidget::CreateThumbnail(const QImage& image, const QSize& size, const QString& name, const QString& link)
{
  const auto pixelRatio = this->devicePixelRatioF();
  const auto thumbnailSize = image.size().scaled(size, Qt::KeepAspectRatio).boundedTo(image.size());

  auto thumbnail = image.scaled(thumbnailSize * pixelRatio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  thumbnail.setDevicePixelRatio(pixelRatio);

  this->document()->addResource(QTextDocument::ImageResource, QUrl(name), thumbnail);

  QTextImageFormat format;
  format.setName(name);
  format.setWidth(thumbnailSize.width());
  format.setHeight(thumbnailSize.height());
  format.setAnchor(true);
  format.setAnchorHref(link);
  return format;
}

void QmitkAgentConversationWidget::ShowImage(int id)
{
  QPixmap pixmap;

  if (!pixmap.loadFromData(m_Images.value(id)))
    return;

  // Not modal, so that the agent can go on working and the user can compare.
  auto* dialog = new QDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowTitle("Screenshot");

  // Screenshots are in device pixels.
  pixmap.setDevicePixelRatio(this->devicePixelRatioF());

  auto* image = new QLabel(dialog);
  image->setPixmap(pixmap);

  auto* scrollArea = new QScrollArea(dialog);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setWidget(image);

  auto* layout = new QVBoxLayout(dialog);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(scrollArea);

  const auto available = this->screen()->availableGeometry().size() * 0.9;
  dialog->resize(pixmap.deviceIndependentSize().toSize().boundedTo(available));
  dialog->show();
}

void QmitkAgentConversationWidget::OnLinkClicked(const QUrl& url)
{
  if (url.scheme() == ImageScheme)
  {
    this->ShowImage(url.path().toInt());
  }
  else if (url.scheme() == StepsScheme)
  {
    bool ok = false;
    const auto index = url.path().toULongLong(&ok);

    if (ok && index < m_StepGroups.size())
    {
      const bool expand = !m_StepGroups[index].Expanded;
      this->SetStepGroupExpanded(index, expand);

      // Steps that unfold above the end grow the conversation below the view
      // without a scroll, so it would still count as being at its end.
      if (expand && index + 1 < m_StepGroups.size())
        m_FollowEnd = false;
    }
  }
  // Links in the agent's answers must not open local files or programs.
  else if (url.scheme() == "https" || url.scheme() == "http")
  {
    QDesktopServices::openUrl(url);
  }
}

void QmitkAgentConversationWidget::ScrollToEnd()
{
  if (m_FollowEnd)
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
}
