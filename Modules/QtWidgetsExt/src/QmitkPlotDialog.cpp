/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <qlayout.h>
#include <qpushbutton.h>

#include "QmitkPlotDialog.h"

QmitkPlotDialog::QmitkPlotDialog(const char *title, QWidget *parent, const char *) : QDialog(parent)
{
  auto boxLayout = new QVBoxLayout(this);
  m_Plot = new QmitkPlotWidget(this, title);
  m_CloseDialogButton = new QPushButton("close plot window", this);
  boxLayout->addWidget(m_Plot);
  boxLayout->addWidget(m_CloseDialogButton);
  connect(m_CloseDialogButton, SIGNAL(clicked()), this, SLOT(accept()));
}

QmitkPlotDialog::~QmitkPlotDialog()
{
  delete m_Plot;
}

QmitkPlotWidget *QmitkPlotDialog::GetPlot()
{
  return m_Plot;
}

QwtPlot *QmitkPlotDialog::GetQwtPlot()
{
  return m_Plot->GetPlot();
}
