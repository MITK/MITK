/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkLineCompletionTextEdit.h>

#include <QAbstractItemView>
#include <QCompleter>
#include <QKeyEvent>
#include <QStringListModel>
#include <QTextBlock>

namespace
{
  // A single character matches too much to be of help.
  constexpr qsizetype MIN_LINE_LENGTH = 2;

  constexpr qsizetype MAX_COMPLETIONS = 100;
}

QmitkLineCompletionTextEdit::QmitkLineCompletionTextEdit(QWidget* parent)
  : QPlainTextEdit(parent),
    m_Completer(new QCompleter(this)),
    m_Model(new QStringListModel(this))
{
  // The completions are matched here, so the completer lists them as they are.
  m_Completer->setModel(m_Model);
  m_Completer->setWidget(this);
  m_Completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
  m_Completer->setMaxVisibleItems(10);

  this->setTabChangesFocus(true);

  connect(m_Completer, qOverload<const QString&>(&QCompleter::activated), this, &QmitkLineCompletionTextEdit::InsertCompletion);
}

QmitkLineCompletionTextEdit::~QmitkLineCompletionTextEdit() = default;

void QmitkLineCompletionTextEdit::SetCompletions(const QStringList& completions)
{
  m_Completions = completions;
  m_LowerCaseCompletions.clear();
  m_LowerCaseCompletions.reserve(completions.size());

  for (const auto& completion : completions)
    m_LowerCaseCompletions << completion.toLower();

  m_Completer->popup()->hide();
}

void QmitkLineCompletionTextEdit::keyPressEvent(QKeyEvent* event)
{
  auto* popup = m_Completer->popup();

  if (popup->isVisible())
  {
    // The completer handles these on its popup and then passes them on to
    // this edit, where they must not take effect a second time.
    switch (event->key())
    {
      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Escape:
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
        event->ignore();
        return;

      default:
        break;
    }
  }

  QPlainTextEdit::keyPressEvent(event);

  if (event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))
  {
    popup->hide();
    return;
  }

  // Typing opens the list. Moving the cursor only updates a list that is open
  // already, so that walking through the lines does not open it at every end.
  const bool edits = !event->text().isEmpty() || event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete;

  if (edits || popup->isVisible())
    this->UpdateCompletionPopup();
}

void QmitkLineCompletionTextEdit::UpdateCompletionPopup()
{
  auto* popup = m_Completer->popup();
  const auto cursor = this->textCursor();

  // The suggestions are for the line being typed, so only at its end.
  if (cursor.hasSelection() || !cursor.atBlockEnd())
  {
    popup->hide();
    return;
  }

  const auto line = cursor.block().text().trimmed();
  const auto completions = this->FindCompletions(line);

  if (completions.isEmpty() || (completions.size() == 1 && completions.front().compare(line, Qt::CaseInsensitive) == 0))
  {
    popup->hide();
    return;
  }

  m_Model->setStringList(completions);

  // Below the line and as wide as the edit, since a completion replaces the
  // whole line.
  auto lineStart = cursor;
  lineStart.movePosition(QTextCursor::StartOfBlock);

  auto rect = this->cursorRect(lineStart);
  rect.translate(this->viewport()->pos());
  rect.setWidth(this->viewport()->width());

  m_Completer->complete(rect);
  popup->setCurrentIndex(m_Completer->completionModel()->index(0, 0));
}

QStringList QmitkLineCompletionTextEdit::FindCompletions(const QString& line) const
{
  if (line.size() < MIN_LINE_LENGTH)
    return {};

  const auto needle = line.toLower();

  QStringList startingWith;
  QStringList containing;

  for (qsizetype i = 0; i < m_LowerCaseCompletions.size(); ++i)
  {
    const auto& lowerCaseCompletion = m_LowerCaseCompletions[i];

    if (lowerCaseCompletion.startsWith(needle))
      startingWith << m_Completions[i];
    else if (lowerCaseCompletion.contains(needle))
      containing << m_Completions[i];
  }

  startingWith << containing;

  if (startingWith.size() > MAX_COMPLETIONS)
    startingWith.resize(MAX_COMPLETIONS);

  return startingWith;
}

void QmitkLineCompletionTextEdit::InsertCompletion(const QString& completion)
{
  auto cursor = this->textCursor();
  cursor.movePosition(QTextCursor::StartOfBlock);
  cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  cursor.insertText(completion);

  this->setTextCursor(cursor);
}
