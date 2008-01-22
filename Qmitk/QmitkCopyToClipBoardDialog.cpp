#include "QmitkCopyToClipBoardDialog.h"

#include <qtextedit.h>
#include <qlayout.h>

QmitkCopyToClipBoardDialog::QmitkCopyToClipBoardDialog(const QString& text, QObject* parent, const char* name)
{
  QBoxLayout * layout = new QVBoxLayout( this );
  QTextEdit* textedit = new QTextEdit(text, "", this);
  textedit->setReadOnly(true);
  layout->addWidget( textedit );

  QDialog::resize(500,400);
}

QmitkCopyToClipBoardDialog::~QmitkCopyToClipBoardDialog()
{
}

