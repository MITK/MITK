#include "QmitkInfoDialog.h"

#include "QmitkDataStorageComboBox.h"

#include <sstream>

#include <QGridLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QLineEdit>
#include <QEvent>
#include <QKeyEvent>

QmitkInfoDialog::QmitkInfoDialog( std::vector<mitk::DataTreeNode*> _Nodes, QWidget * parent /*= 0*/, Qt::WindowFlags f /*= 0 */ )
: QDialog(parent, f)
{
  // DIM
  QGridLayout* parentLayout = new QGridLayout;
  QmitkDataStorageComboBox* _QmitkDataStorageComboBox = new QmitkDataStorageComboBox(this, true);
  m_KeyWord = new QLineEdit;
  m_KeyWord->installEventFilter(this);
  m_SearchButton = new QPushButton("Search (F3)", this);
  m_SearchButton->installEventFilter(this);
  m_TextBrowser = new QTextBrowser(this);
  QPushButton* _CancelButton = new QPushButton("Cancel", this);

  // SET
  this->setMinimumSize(512, 512);
  this->setLayout(parentLayout);
  this->setSizeGripEnabled(true);
  this->setModal(true);
  
  parentLayout->addWidget(_QmitkDataStorageComboBox, 0, 0, 1, 2);
  parentLayout->addWidget(m_KeyWord, 1, 0);
  parentLayout->addWidget(m_SearchButton, 1, 1);
  parentLayout->addWidget(m_TextBrowser, 2, 0, 1, 2);
  parentLayout->addWidget(_CancelButton, 3, 0, 1, 2);

  QObject::connect( _QmitkDataStorageComboBox, SIGNAL( OnSelectionChanged( const mitk::DataTreeNode* ) )
    , this, SLOT( OnSelectionChanged( const mitk::DataTreeNode* ) ) );

  for (std::vector<mitk::DataTreeNode*>::iterator it = _Nodes.begin()
    ; it != _Nodes.end(); it++)
  {
    _QmitkDataStorageComboBox->AddNode(*it);
  }

  QObject::connect( m_KeyWord, SIGNAL( textChanged ( const QString & )  )
    , this, SLOT( KeyWordTextChanged(const QString &) ) );

  QObject::connect( m_SearchButton, SIGNAL( clicked ( bool ) )
    , this, SLOT( OnSearchButtonClicked( bool ) ) );

  QObject::connect( _CancelButton, SIGNAL( clicked ( bool ) )
    , this, SLOT( OnCancelButtonClicked( bool ) ) );

  _CancelButton->setDefault(true);
  
}

void QmitkInfoDialog::OnSelectionChanged( const mitk::DataTreeNode* node )
{
  std::ostringstream s;
  itk::Indent i(2);
  mitk::BaseData* _BaseData = node->GetData();
  if(_BaseData)
    _BaseData->Print(s, i);
  m_TextBrowser->setPlainText(QString::fromStdString(s.str()));
}

void QmitkInfoDialog::OnSearchButtonClicked( bool  /*checked*/ /*= false */ )
{
  QString keyWord = m_KeyWord->text();
  QString text = m_TextBrowser->toPlainText();
  
  if(keyWord.isEmpty() || text.isEmpty())
    return;

  m_TextBrowser->find(keyWord);
  m_SearchButton->setText("Search Next(F3)");
}

void QmitkInfoDialog::OnCancelButtonClicked( bool  /*checked*/ /*= false */ )
{
  this->done(0);
}

bool QmitkInfoDialog::eventFilter( QObject *obj, QEvent *event )
{
  if (event->type() == QEvent::KeyPress) 
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if(keyEvent->key() == Qt::Key_F3  || keyEvent->key() == Qt::Key_Return)
    {
      // trigger deletion of selected node(s)
      this->OnSearchButtonClicked(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter(obj, event);
}

void QmitkInfoDialog::KeyWordTextChanged(const QString &  /*text*/)
{
  QTextCursor textCursor = m_TextBrowser->textCursor();
  textCursor.setPosition(0);
  m_TextBrowser->setTextCursor(textCursor);
  m_SearchButton->setText("Search (F3)");
}
