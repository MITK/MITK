/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkNodeDetailsDialog.h"

#include "QmitkDataStorageComboBox.h"

#include <sstream>

#include <QGridLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QLineEdit>
#include <QEvent>
#include <QKeyEvent>

QmitkNodeDetailsDialog::QmitkNodeDetailsDialog(const QList<mitk::DataNode::Pointer>& nodes, QWidget* parent /*= nullptr*/, Qt::WindowFlags flags /*= nullptr */)
  : QDialog(parent, flags)
{
  QList<mitk::DataNode::ConstPointer> constNodes;

  for (auto& node : nodes)
  {
    constNodes.append(node.GetPointer());
  }

  InitWidgets(constNodes);
}

QmitkNodeDetailsDialog::QmitkNodeDetailsDialog(const QList<mitk::DataNode::ConstPointer>& nodes, QWidget* parent /*= nullptr*/, Qt::WindowFlags flags /*= nullptr */)
  : QDialog(parent, flags)
{
  InitWidgets(nodes);
}

void QmitkNodeDetailsDialog::InitWidgets(const QList<mitk::DataNode::ConstPointer>& nodes)
{
  auto parentLayout = new QGridLayout;
  auto dataStorageComboBox = new QmitkDataStorageComboBox(this, true);
  m_KeyWord = new QLineEdit;
  m_KeyWord->installEventFilter(this);
  m_SearchButton = new QPushButton("Search (F3)", this);
  m_SearchButton->installEventFilter(this);
  m_TextBrowser = new QTextBrowser(this);
  QPushButton* cancelButton = new QPushButton("Cancel", this);

  setMinimumSize(512, 512);
  setLayout(parentLayout);
  setSizeGripEnabled(true);
  setModal(true);

  parentLayout->addWidget(dataStorageComboBox, 0, 0, 1, 2);
  parentLayout->addWidget(m_KeyWord, 1, 0);
  parentLayout->addWidget(m_SearchButton, 1, 1);
  parentLayout->addWidget(m_TextBrowser, 2, 0, 1, 2);
  parentLayout->addWidget(cancelButton, 3, 0, 1, 2);

  connect(dataStorageComboBox, &QmitkDataStorageComboBox::OnSelectionChanged, this, &QmitkNodeDetailsDialog::OnSelectionChanged);

  for (auto& node : nodes)
  {
    dataStorageComboBox->AddNode(node);
  }

  connect(m_KeyWord, &QLineEdit::textChanged, this, &QmitkNodeDetailsDialog::KeyWordTextChanged);
  connect(m_SearchButton, &QPushButton::clicked, this, &QmitkNodeDetailsDialog::OnSearchButtonClicked);
  connect(cancelButton, &QPushButton::clicked, this, &QmitkNodeDetailsDialog::OnCancelButtonClicked);

  cancelButton->setDefault(true);
};

void QmitkNodeDetailsDialog::OnSelectionChanged(const mitk::DataNode* node)
{
  if (nullptr == node)
  {
    return;
  }

  std::ostringstream s;
  itk::Indent i(2);
  mitk::BaseData* baseData = node->GetData();
  if (nullptr != baseData)
  {
    baseData->Print(s, i);
  }

  m_TextBrowser->setPlainText(QString::fromStdString(s.str()));
}

void QmitkNodeDetailsDialog::OnSearchButtonClicked(bool /*checked*/ /*= false */)
{
  QString keyWord = m_KeyWord->text();
  QString text = m_TextBrowser->toPlainText();

  if (keyWord.isEmpty() || text.isEmpty())
  {
    return;
  }

  m_TextBrowser->find(keyWord);
  m_SearchButton->setText("Search Next(F3)");
}

void QmitkNodeDetailsDialog::OnCancelButtonClicked(bool /*checked*/ /*= false */)
{
  done(0);
}

bool QmitkNodeDetailsDialog::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::KeyPress)
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_F3 || keyEvent->key() == Qt::Key_Return)
    {
      // trigger deletion of selected node(s)
      OnSearchButtonClicked(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter(obj, event);
}

void QmitkNodeDetailsDialog::KeyWordTextChanged(const QString& /*text*/)
{
  QTextCursor textCursor = m_TextBrowser->textCursor();
  textCursor.setPosition(0);
  m_TextBrowser->setTextCursor(textCursor);
  m_SearchButton->setText("Search (F3)");
}
