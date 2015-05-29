/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological rmatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkXnatExperimentWidget.h"

#include <ctkXnatExperiment.h>

QmitkXnatExperimentWidget::QmitkXnatExperimentWidget(Mode mode, QWidget* parent)
  : QWidget(parent)
  , m_Mode(mode)
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

QmitkXnatExperimentWidget::~QmitkXnatExperimentWidget()
{
}

void QmitkXnatExperimentWidget::SetExperiment(ctkXnatExperiment* experiment)
{
  m_Experiment = experiment;

  // Set the UI labels
  if (m_Experiment->parent())
  {
    m_Controls.breadcrumbLabel->setText("Project:" + m_Experiment->parent()->parent()->property("name") +
      " > Subject:" + m_Experiment->parent()->property("label"));
  }
  m_Controls.labelLineEdit->setText(m_Experiment->property("label"));
  m_Controls.modalityLineEdit->setText(m_Experiment->property("modality"));
  m_Controls.dateLineEdit->setText(m_Experiment->property("date"));
  m_Controls.timeLineEdit->setText(m_Experiment->property("time"));
  m_Controls.scannerLineEdit->setText(m_Experiment->property("scanner"));
}

ctkXnatExperiment* QmitkXnatExperimentWidget::GetExperiment() const
{
  if (m_Mode == Mode::CREATE)
  {
    if (!m_Controls.labelLineEdit->text().isEmpty())
    {
      m_Experiment->setProperty("label", m_Controls.labelLineEdit->text());
    }
    if (!m_Controls.modalityLineEdit->text().isEmpty())
    {
      m_Experiment->setProperty("label", m_Controls.modalityLineEdit->text());
    }
    if (!m_Controls.dateLineEdit->text().isEmpty())
    {
      m_Experiment->setProperty("label", m_Controls.dateLineEdit->text());
    }
    if (!m_Controls.dateLineEdit->text().isEmpty())
    {
      m_Experiment->setProperty("label", m_Controls.dateLineEdit->text());
    }
    if (!m_Controls.scannerLineEdit->text().isEmpty())
    {
      m_Experiment->setProperty("label", m_Controls.scannerLineEdit->text());
    }
  }

  return m_Experiment;
}
