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

QmitkXnatProjectInfoWidget::QmitkXnatProjectInfoWidget(QWidget* parent, ctkXnatProject* project)
  : QWidget(parent)
  , m_Project(project)
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
}

QmitkXnatProjectInfoWidget::~QmitkXnatProjectInfoWidget()
{
  delete m_Project;
}

void QmitkXnatProjectInfoWidget::SetProject(ctkXnatProject* project)
{
  m_Project = project;
}

ctkXnatProject* QmitkXnatProjectInfoWidget::GetProject() const
{
  return m_Project;
}
