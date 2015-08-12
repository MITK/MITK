#include "QmitkCommentTextView.h"

QmitkCommentTextView::QmitkCommentTextView()
{
  m_gui.setupUi(this);

  connect(m_gui.done, SIGNAL(clicked()), this, SLOT(done()));

  setModal(true);

  setFixedSize(size());
}

QmitkCommentTextView::~QmitkCommentTextView()
{
}

void QmitkCommentTextView::done()
{
  m_commentText = m_gui.CommentText->toPlainText();

  setHidden(true);
  close();
}

QString QmitkCommentTextView::getText()
{
  return m_commentText;
}

void QmitkCommentTextView::show()
{
  m_gui.CommentText->clear();

  exec();
}
