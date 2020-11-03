/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkXnatSubjectWidget.h"

#include <ctkXnatSubject.h>
#include <iostream>

QmitkXnatSubjectWidget::QmitkXnatSubjectWidget(QWidget *parent) : QWidget(parent)
{
  m_Mode = INFO;
  Init();
}

QmitkXnatSubjectWidget::QmitkXnatSubjectWidget(Mode mode, QWidget *parent) : QWidget(parent), m_Mode(mode)
{
  Init();
}

QmitkXnatSubjectWidget::~QmitkXnatSubjectWidget()
{
}

void QmitkXnatSubjectWidget::Init()
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  if (m_Mode == Mode::INFO)
  {
    // make not needed row invisible
    m_Controls.labelLabel->setText("ID:");
    m_Controls.mandatoryLabel->setVisible(false);
  }
}

void QmitkXnatSubjectWidget::SetSubject(ctkXnatSubject *subject)
{
  m_Subject = subject;

  // Set the UI labels
  if (m_Subject->parent())
  {
    m_Controls.breadcrumbLabel->setText("Project:" + m_Subject->parent()->property("name"));
  }
  m_Controls.labelLineEdit->setText(m_Subject->property("label"));
  m_Controls.birthdateLineEdit->setText(m_Subject->property("dob"));
  m_Controls.genderLineEdit->setText(m_Subject->property("gender"));
  m_Controls.handednessLineEdit->setText(m_Subject->property("handedness"));
  m_Controls.heightLineEdit->setText(m_Subject->property("height"));
  m_Controls.weightLineEdit->setText(m_Subject->property("weight"));
}

ctkXnatSubject *QmitkXnatSubjectWidget::GetSubject() const
{
  if (m_Mode == CREATE)
  {
    if (!m_Controls.labelLineEdit->text().isEmpty())
    {
      m_Subject->setProperty("label", m_Controls.labelLineEdit->text());
    }
    if (!m_Controls.birthdateLineEdit->text().isEmpty())
    {
      m_Subject->setProperty("dob", m_Controls.birthdateLineEdit->text());
    }
    if (!m_Controls.genderLineEdit->text().isEmpty())
    {
      m_Subject->setProperty("gender", m_Controls.genderLineEdit->text());
    }
    if (!m_Controls.handednessLineEdit->text().isEmpty())
    {
      m_Subject->setProperty("handedness", m_Controls.handednessLineEdit->text());
    }
    if (!m_Controls.heightLineEdit->text().isEmpty())
    {
      m_Subject->setProperty("height", m_Controls.heightLineEdit->text());
    }
    if (!m_Controls.weightLineEdit->text().isEmpty())
    {
      m_Subject->setProperty("weight", m_Controls.weightLineEdit->text());
    }
  }

  return m_Subject;
}
