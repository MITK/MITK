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

#include "QmitkXnatExperimentInfoWidget.h"

#include <ctkXnatExperiment.h>

QmitkXnatExperimentInfoWidget::QmitkXnatExperimentInfoWidget(ctkXnatExperiment* experiment, QWidget* parent)
  : QWidget(parent)
  , m_Experiment(experiment)
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  // Set the UI labels
  m_Controls.breadcrumbLabel->setText("Project:" + m_Experiment->parent()->parent()->property("name") +
    " > Subject:" + m_Experiment->parent()->property("label"));
  m_Controls.nameLabel->setText(m_Experiment->property("label"));
  m_Controls.modalityLabel->setText(m_Experiment->property("modality"));
  m_Controls.dateLabel->setText(m_Experiment->property("date"));
  m_Controls.timeLabel->setText(m_Experiment->property("time"));
  m_Controls.scannerLabel->setText(m_Experiment->property("scanner"));
}

QmitkXnatExperimentInfoWidget::~QmitkXnatExperimentInfoWidget()
{
}

void QmitkXnatExperimentInfoWidget::SetExperiment(ctkXnatExperiment* experiment)
{
  m_Experiment = experiment;
}

ctkXnatExperiment* QmitkXnatExperimentInfoWidget::GetExperiment() const
{
  return m_Experiment;
}
