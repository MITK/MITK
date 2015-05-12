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

#include "QmitkXnatProjectInfoWidget.h"

#include <ctkXnatProject.h>

QmitkXnatProjectInfoWidget::QmitkXnatProjectInfoWidget(ctkXnatProject* project, QWidget* parent)
  : m_Project(project)
  , QWidget(parent)
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  // Set the UI labels
  m_Controls.idLabel->setText(m_Project->property("ID"));
  m_Controls.descriptionLabel->setText(m_Project->property("description"));
  if (!m_Project->property("pi_lastname").isEmpty())
  {
    m_Controls.piLabel->setText(m_Project->property("pi_lastname") + ", " + m_Project->property("pi_firstname"));
  }
}

QmitkXnatProjectInfoWidget::~QmitkXnatProjectInfoWidget()
{
}

ctkXnatProject* QmitkXnatProjectInfoWidget::GetProject() const
{
  return m_Project;
}
