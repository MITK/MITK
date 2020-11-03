/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkLesionTextDialog.h"

#include <QLabel>
#include <QLayout>
#include <qpushbutton.h>

QmitkLesionTextDialog::QmitkLesionTextDialog(QWidget* parent)
  : QDialog(parent)
{
  QBoxLayout* verticalLayout = new QVBoxLayout(this);
  verticalLayout->setMargin(5);
  verticalLayout->setSpacing(5);

  QLabel* dialogLabel = new QLabel(tr("Set lesion information"), this);
  verticalLayout->addWidget(dialogLabel);

  m_LineEdit = new QLineEdit(this);
  m_LineEdit->setFocus();
  verticalLayout->addWidget(m_LineEdit);

  QPushButton* acceptButton = new QPushButton(tr("Ok"), this);
  QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
  acceptButton->setDefault(true);

  connect(acceptButton, &QPushButton::clicked, this, &QmitkLesionTextDialog::accept);
  connect(cancelButton, &QPushButton::clicked, this, &QmitkLesionTextDialog::reject);

  QBoxLayout* horizontalLayout = new QHBoxLayout();
  horizontalLayout->setSpacing(5);
  horizontalLayout->addStretch();
  horizontalLayout->addWidget(acceptButton);
  horizontalLayout->addWidget(cancelButton);
  verticalLayout->addLayout(horizontalLayout);

  setMinimumSize(250, 100);
}

void QmitkLesionTextDialog::SetLineEditText(const std::string& lineEditText)
{
  m_LineEdit->setText(QString::fromStdString(lineEditText));
}

QString QmitkLesionTextDialog::GetLineEditText() const
{
  return m_LineEdit->text();
}

QLineEdit* QmitkLesionTextDialog::GetLineEdit() const
{
  return m_LineEdit;
}
