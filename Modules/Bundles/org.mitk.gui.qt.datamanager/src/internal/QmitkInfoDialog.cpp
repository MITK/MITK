#include "QmitkInfoDialog.h"

#include "QmitkDataStorageComboBox.h"

#include <sstream>

#include <QGridLayout>
#include <QTextBrowser>
#include <QPushButton>

QmitkInfoDialog::QmitkInfoDialog( std::vector<mitk::DataTreeNode*> _Nodes, QWidget * parent /*= 0*/, Qt::WindowFlags f /*= 0 */ )
: QDialog(parent, f)
{
  // DIM
  QGridLayout* parentLayout = new QGridLayout;
  QmitkDataStorageComboBox* _QmitkDataStorageComboBox = new QmitkDataStorageComboBox(this, true);
  m_TextBrowser = new QTextBrowser(this);
  QPushButton* m_CancelButton = new QPushButton("Cancel", this);

  // SET
  this->setLayout(parentLayout);
  this->setSizeGripEnabled(true);
  this->setModal(true);
  
  parentLayout->addWidget(_QmitkDataStorageComboBox, 0, 0);
  parentLayout->addWidget(m_TextBrowser, 1, 0);
  parentLayout->addWidget(m_CancelButton, 2, 0);

  QObject::connect( _QmitkDataStorageComboBox, SIGNAL( OnSelectionChanged( const mitk::DataTreeNode* ) )
    , this, SLOT( OnSelectionChanged( const mitk::DataTreeNode* ) ) );

  for (std::vector<mitk::DataTreeNode*>::iterator it = _Nodes.begin()
    ; it != _Nodes.end(); it++)
  {
    _QmitkDataStorageComboBox->AddNode(*it);
  }

  QObject::connect( _QmitkDataStorageComboBox, SIGNAL( OnCancelButtonClicked( bool checked ) )
    , this, SLOT( OnCancelButtonClicked( bool checked ) ) );

  m_CancelButton->setDefault(true);
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

void QmitkInfoDialog::OnCancelButtonClicked( bool checked /*= false */ )
{
  this->reject();
}