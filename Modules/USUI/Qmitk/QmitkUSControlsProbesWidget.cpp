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

#include "QmitkUSControlsProbesWidget.h"
#include "ui_QmitkUSControlsProbesWidget.h"

QmitkUSControlsProbesWidget::QmitkUSControlsProbesWidget(mitk::USControlInterfaceProbes::Pointer controlInterface, QWidget *parent)
  : QWidget(parent), ui(new Ui::QmitkUSControlsProbesWidget),
    m_ControlInterface(controlInterface)
{
  ui->setupUi(this);

  if ( ! m_ControlInterface )
  {
    // I do not see an instance of this being reenabled. We might have to look at it again, if we start to reallyuse multiple probes simultaneously.
    ui->probesComboBox->setEnabled(false);
    ui->probesLabel->setEnabled(false);
    return;
  }

  if ( ! m_ControlInterface->GetIsActive() ) { m_ControlInterface->SetIsActive(true); }

  // get all probes an put their names into a combo box
  std::vector<mitk::USProbe::Pointer> probes = m_ControlInterface->GetProbeSet();
  for ( auto it = probes.begin();
    it != probes.end(); ++it )
  {
    std::string probeIdentifier = (*it)->GetName();
    ui->probesComboBox->addItem(QString::fromUtf8(probeIdentifier.data(), probeIdentifier.size()));
  }

  // select first probe as default value
  if ( probes.size() > 0 )
  {
    ui->probesComboBox->setCurrentIndex(0);
    m_ControlInterface->SelectProbe(0);
  }

  connect( ui->probesComboBox, SIGNAL(activated(int)), this, SLOT(OnProbeControlActivated(int)) );
}

QmitkUSControlsProbesWidget::~QmitkUSControlsProbesWidget()
{
  delete ui;
}

void QmitkUSControlsProbesWidget::OnProbeControlActivated(int index)
{
  m_ControlInterface->SelectProbe(index);
}