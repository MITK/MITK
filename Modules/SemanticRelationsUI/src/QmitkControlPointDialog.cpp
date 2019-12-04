/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkControlPointDialog.h"

#include <QAbstractItemModel>
#include <QStringListModel>
#include <QLabel>
#include <QLayout>
#include <qpushbutton.h>

QmitkControlPointDialog::QmitkControlPointDialog(QWidget* parent)
  : QDialog(parent)
{
  QBoxLayout* verticalLayout = new QVBoxLayout(this);
  verticalLayout->setMargin(5);
  verticalLayout->setSpacing(5);

  QLabel* dateLabel = new QLabel(tr("Set date"), this);
  verticalLayout->addWidget(dateLabel);

  m_DateEdit = new QDateEdit(this);
  m_DateEdit->setDisplayFormat("yyyy-MM-dd");
  m_DateEdit->setFocus();
  verticalLayout->addWidget(m_DateEdit);

  QPushButton* acceptButton = new QPushButton(tr("Ok"), this);
  QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
  acceptButton->setDefault(true);

  connect(acceptButton, &QPushButton::clicked, this, &QmitkControlPointDialog::accept);
  connect(cancelButton, &QPushButton::clicked, this, &QmitkControlPointDialog::reject);

  QBoxLayout* horizontalLayout = new QHBoxLayout();
  horizontalLayout->setSpacing(5);
  horizontalLayout->addStretch();
  horizontalLayout->addWidget(acceptButton);
  horizontalLayout->addWidget(cancelButton);
  verticalLayout->addLayout(horizontalLayout);

  setMinimumSize(250, 100);
}

void QmitkControlPointDialog::SetCurrentDate(mitk::SemanticTypes::ControlPoint currentControlPoint)
{
  m_DateEdit->setDate(QDate(currentControlPoint.date.year(), currentControlPoint.date.month(), currentControlPoint.date.day()));
}

QDate QmitkControlPointDialog::GetCurrentDate() const
{
  return m_DateEdit->date();
}
