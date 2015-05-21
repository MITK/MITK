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

#include "QmitkXnatSubjectInfoWidget.h"

#include <ctkXnatSubject.h>
#include <iostream>

QmitkXnatSubjectInfoWidget::QmitkXnatSubjectInfoWidget(ctkXnatSubject* subject, QWidget* parent)
  : m_Subject(subject)
  , QWidget(parent)
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  // Set the UI labels
  m_Controls.breadcrumbLabel->setText("Project:" + m_Subject->parent()->property("name"));
  m_Controls.nameLabel->setText(m_Subject->property("label"));
  m_Controls.birthyearLabel->setText(m_Subject->property("dob"));
  m_Controls.genderLabel->setText(m_Subject->property("gender"));
  m_Controls.handednessLabel->setText(m_Subject->property("handedness"));
  m_Controls.heightLabel->setText(m_Subject->property("height"));
  m_Controls.weightLabel->setText(m_Subject->property("weight"));
}

QmitkXnatSubjectInfoWidget::~QmitkXnatSubjectInfoWidget()
{
}

void QmitkXnatSubjectInfoWidget::SetSubject(ctkXnatSubject* subject)
{
  m_Subject = subject;
}

ctkXnatSubject* QmitkXnatSubjectInfoWidget::GetSubject() const
{
  return m_Subject;
}
