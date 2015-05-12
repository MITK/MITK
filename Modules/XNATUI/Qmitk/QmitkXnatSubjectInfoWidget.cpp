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
  for (int i = 0; i < m_Subject->properties().count(); i++)
  {
    std::cout << "KEY: " << m_Subject->properties().keys().at(i).toStdString() << " --- " << "VALUE: " << m_Subject->properties().values().at(i).toStdString() << "\n";
  }
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
