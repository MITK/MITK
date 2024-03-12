/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkNodeDetailsDialog.h>

#include <QKeyEvent>

#include <sstream>

#include <ui_QmitkNodeDetailsDialog.h>

QmitkNodeDetailsDialog::QmitkNodeDetailsDialog(const QList<mitk::DataNode::Pointer>& nodes, QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags),
    m_Ui(new Ui::QmitkNodeDetailsDialog)
{
  QList<mitk::DataNode::ConstPointer> constNodes;

  for (auto& node : nodes)
    constNodes.append(node.GetPointer());

  this->InitWidgets(constNodes);
}

QmitkNodeDetailsDialog::QmitkNodeDetailsDialog(const QList<mitk::DataNode::ConstPointer>& nodes, QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags),
    m_Ui(new Ui::QmitkNodeDetailsDialog)
{
  this->InitWidgets(nodes);
}

QmitkNodeDetailsDialog::~QmitkNodeDetailsDialog()
{
  delete m_Ui;
}

void QmitkNodeDetailsDialog::InitWidgets(const QList<mitk::DataNode::ConstPointer>& nodes)
{
  m_Ui->setupUi(this);

  m_Ui->dataStorageComboBox->SetAutoSelectNewItems(true);

  m_Ui->keywordLineEdit->installEventFilter(this);
  m_Ui->searchButton->installEventFilter(this);

  connect(m_Ui->dataStorageComboBox, &QmitkDataStorageComboBox::OnSelectionChanged, this, &QmitkNodeDetailsDialog::OnSelectionChanged);

  for (auto& node : nodes)
    m_Ui->dataStorageComboBox->AddNode(node);

  connect(m_Ui->keywordLineEdit, &QLineEdit::textChanged, this, &QmitkNodeDetailsDialog::KeywordTextChanged);
  connect(m_Ui->searchButton, &QPushButton::clicked, this, &QmitkNodeDetailsDialog::OnSearchButtonClicked);
};

void QmitkNodeDetailsDialog::OnSelectionChanged(const mitk::DataNode* node)
{
  if (nullptr == node)
    return;

  std::ostringstream stream;
  auto baseData = node->GetData();

  if (nullptr != baseData)
    baseData->Print(stream, 2);

  m_Ui->textBrowser->setPlainText(QString::fromStdString(stream.str()));
}

void QmitkNodeDetailsDialog::OnSearchButtonClicked()
{
  auto keyword = m_Ui->keywordLineEdit->text();
  auto text = m_Ui->textBrowser->toPlainText();

  if (keyword.isEmpty() || text.isEmpty())
    return;

  m_Ui->textBrowser->find(keyword);
}

bool QmitkNodeDetailsDialog::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::KeyPress)
  {
    auto keyEvent = static_cast<QKeyEvent*>(event);

    if (keyEvent->key() == Qt::Key_F3 || keyEvent->key() == Qt::Key_Return)
    {
      this->OnSearchButtonClicked();
      return true;
    }
  }

  return QObject::eventFilter(obj, event);
}

void QmitkNodeDetailsDialog::KeywordTextChanged(const QString&)
{
  auto cursor = m_Ui->textBrowser->textCursor();
  cursor.setPosition(0);
  m_Ui->textBrowser->setTextCursor(cursor);
}
