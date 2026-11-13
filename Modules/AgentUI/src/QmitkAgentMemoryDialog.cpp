/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkAgentMemoryDialog.h>
#include <ui_QmitkAgentMemoryDialog.h>

#include <QmitkAgentMemory.h>
#include <QmitkSpeechBubbleFrame.h>

#include <QAbstractTextDocumentLayout>
#include <QDir>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStyledItemDelegate>
#include <QTextEdit>
#include <QtMath>

namespace
{
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
}

QmitkAgentMemoryDialog::QmitkAgentMemoryDialog(QmitkAgentMemory& memory, qreal fontSize, QWidget* parent)
  : QDialog(parent),
    m_Ui(std::make_unique<Ui::QmitkAgentMemoryDialog>()),
    m_Memory(&memory),
    m_DeleteButton(nullptr),
    m_Changed(false)
{
  m_Ui->setupUi(this);

  auto* list = m_Ui->noteList;
  list->setItemDelegate(new ParagraphDelegate(list));

  // In a style sheet, as the application sets the font size of every widget
  // in one. The editor needs its own rule for the same reason.
  if (fontSize > 0)
    list->setStyleSheet(QString("QListWidget, QTextEdit { font-size: %1pt; }").arg(fontSize));

  // First, as it reads the file again if it could not be read, which the notes follow.
  const bool readable = m_Memory->IsReadable();

  for (const auto& note : m_Memory->GetNotes())
  {
    auto* item = new QListWidgetItem(note.Text, list);
    item->setData(Qt::UserRole, note.Id);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
  }

  m_DeleteButton = m_Ui->buttonBox->addButton("Delete", QDialogButtonBox::ActionRole);
  m_DeleteButton->setEnabled(false);

  connect(list, &QListWidget::itemSelectionChanged, this, [this]() {
    m_DeleteButton->setEnabled(!m_Ui->noteList->selectedItems().isEmpty());
  });

  connect(list, &QListWidget::itemChanged, this, &QmitkAgentMemoryDialog::OnItemChanged);
  connect(m_DeleteButton, &QPushButton::clicked, this, &QmitkAgentMemoryDialog::DeleteSelected);
  connect(m_Ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  if (!readable)
  {
    m_Ui->descriptionLabel->setTextFormat(Qt::PlainText);
    m_Ui->descriptionLabel->setText(QString("The memory file cannot be read, so the memory is not changed: %1")
      .arg(QDir::toNativeSeparators(m_Memory->GetFilePath())));

    list->setEnabled(false);
  }
}

QmitkAgentMemoryDialog::~QmitkAgentMemoryDialog() = default;

bool QmitkAgentMemoryDialog::HasChanged() const
{
  return m_Changed;
}

void QmitkAgentMemoryDialog::OnItemChanged(QListWidgetItem* item)
{
  const auto id = item->data(Qt::UserRole).toInt();

  try
  {
    m_Memory->SetText(id, item->text());
    m_Changed = true;
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, "Agent Memory", QString::fromStdString(e.what()));
  }

  // Show the note as stored: normalized, or unchanged if it was rejected.
  for (const auto& note : m_Memory->GetNotes())
  {
    if (note.Id == id)
    {
      const QSignalBlocker blocker(m_Ui->noteList);
      item->setText(note.Text);
    }
  }
}

void QmitkAgentMemoryDialog::DeleteSelected()
{
  for (auto* item : m_Ui->noteList->selectedItems())
  {
    try
    {
      m_Memory->Remove(item->data(Qt::UserRole).toInt());
      delete item;
      m_Changed = true;
    }
    catch (const std::exception& e)
    {
      QMessageBox::warning(this, "Agent Memory", QString::fromStdString(e.what()));
      return;
    }
  }
}
