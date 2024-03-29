/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkStringPropertyOnDemandEdit.h"
#include <QInputDialog>

QmitkStringPropertyOnDemandEdit::QmitkStringPropertyOnDemandEdit(mitk::StringProperty *property, QWidget *parent)
  : QFrame(parent), PropertyEditor(property), m_StringProperty(property)
{
  setFrameStyle(QFrame::NoFrame);
  setLineWidth(0);

  // create HBoxLayout with two buttons
  m_layout = new QHBoxLayout(this);
  m_layout->setContentsMargins({});

  m_label = new QLabel(this);
  m_layout->addWidget(m_label);

  m_toolbutton = new QClickableLabel2(this);
  m_toolbutton->setText("...");
  m_layout->addWidget(m_toolbutton);

  m_layout->addStretch(10);

  connect(m_toolbutton, SIGNAL(clicked()), this, SLOT(onToolButtonClicked()));

  ensurePolished();
  adjustSize();

  PropertyChanged();
}

QmitkStringPropertyOnDemandEdit::~QmitkStringPropertyOnDemandEdit()
{
}

void QmitkStringPropertyOnDemandEdit::PropertyChanged()
{
  if (m_Property)
    m_label->setText(m_StringProperty->GetValue());
}

void QmitkStringPropertyOnDemandEdit::PropertyRemoved()
{
  m_Property = nullptr;
  m_StringProperty = nullptr;
  m_label->setText("n/a");
}

void QmitkStringPropertyOnDemandEdit::onToolButtonClicked()
{
  bool ok(false);
  QString newText = QInputDialog::getText(
    this, "Change text", "You can change the displayed text here", QLineEdit::Normal, m_label->text(), &ok);

  if (ok)
  {
    BeginModifyProperty(); // deregister from events

    m_StringProperty->SetValue(newText.toStdString());
    m_label->setText(newText);

    EndModifyProperty(); // again register for events
  }
}
