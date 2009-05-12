#include "QmitkCopyToClipBoardDialog.h"

#include <qtextedit.h>
#include <qlayout.h>

QmitkCopyToClipBoardDialog::QmitkCopyToClipBoardDialog(const QString& text, QWidget* parent, const char* name)
: QDialog(parent, name)
{
  QBoxLayout * layout = new QVBoxLayout( this );
  //!mmueller
  //QTextEdit* textedit = new QTextEdit(text, "", this);
  //#changed to:
  QTextEdit* textedit = new QTextEdit(this);
  // with setPlainText() line breaks are displayed correctly
  textedit->setPlainText(text);
  //!
  textedit->setReadOnly(true);
  layout->addWidget( textedit );

  QDialog::resize(500,400);
}

QmitkCopyToClipBoardDialog::~QmitkCopyToClipBoardDialog()
{
}

