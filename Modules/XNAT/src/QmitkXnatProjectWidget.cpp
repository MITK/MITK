/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkXnatProjectWidget.h"

#include <ctkXnatProject.h>

QmitkXnatProjectWidget::QmitkXnatProjectWidget(QWidget *parent) : QWidget(parent)
{
  m_Mode = INFO;
  Init();
}

QmitkXnatProjectWidget::QmitkXnatProjectWidget(Mode mode, QWidget *parent) : QWidget(parent), m_Mode(mode)
{
  Init();
}

QmitkXnatProjectWidget::~QmitkXnatProjectWidget()
{
}

void QmitkXnatProjectWidget::Init()
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  if (m_Mode == Mode::INFO)
  {
    // make not needed row invisible
    m_Controls.idLabel->setVisible(false);
    m_Controls.idLineEdit->setVisible(false);
    m_Controls.mandatoryLabel->setVisible(false);
  }
}

void QmitkXnatProjectWidget::SetProject(ctkXnatProject *project)
{
  m_Project = project;

  // Set the UI labels
  m_Controls.nameLineEdit->setText(m_Project->property("name"));
  m_Controls.descriptionLineEdit->setText(m_Project->property("description"));
  if (!m_Project->property("pi_lastname").isEmpty())
  {
    m_Controls.piLineEdit->setText(m_Project->property("pi_lastname") + ", " + m_Project->property("pi_firstname"));
  }
}

ctkXnatProject *QmitkXnatProjectWidget::GetProject() const
{
  if (m_Mode == Mode::CREATE)
  {
    if (!m_Controls.idLineEdit->text().isEmpty())
    {
      m_Project->setProperty("ID", m_Controls.idLineEdit->text());
    }
    if (!m_Controls.nameLineEdit->text().isEmpty())
    {
      m_Project->setProperty("name", m_Controls.nameLineEdit->text());
    }
    if (!m_Controls.descriptionLineEdit->text().isEmpty())
    {
      m_Project->setProperty("description", m_Controls.descriptionLineEdit->text());
    }
    // pi name needs to be inserted separated
  }
  return m_Project;
}
