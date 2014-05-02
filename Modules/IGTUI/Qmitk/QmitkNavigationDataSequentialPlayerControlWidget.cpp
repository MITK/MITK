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

#include "QmitkNavigationDataSequentialPlayerControlWidget.h"
#include "ui_QmitkNavigationDataSequentialPlayerControlWidget.h"

#include <QTimer>

QmitkNavigationDataSequentialPlayerControlWidget::QmitkNavigationDataSequentialPlayerControlWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::QmitkNavigationDataSequentialPlayerControlWidget)
{
  ui->setupUi(this);
  m_UpdateTimer = new QTimer();
  connect( m_UpdateTimer, SIGNAL(timeout()), this, SLOT(OnUpdate()) );
}

QmitkNavigationDataSequentialPlayerControlWidget::~QmitkNavigationDataSequentialPlayerControlWidget()
{
  delete ui;
}

void QmitkNavigationDataSequentialPlayerControlWidget::SetPlayer(mitk::NavigationDataSequentialPlayer::Pointer player)
{
  m_Player = player;

  ui->samplePositionHorizontalSlider->setMaximum(player->GetNumberOfSnapshots());
}

void QmitkNavigationDataSequentialPlayerControlWidget::OnStop()
{
  m_UpdateTimer->stop();
  m_Player->GoToSnapshot(0);

  this->UpdatePlayerDisplay();

  // make sure that the play/pause button is not checked after stopping
  ui->playPushButton->setChecked(false);
}

void QmitkNavigationDataSequentialPlayerControlWidget::OnPlayPause()
{
  if ( m_UpdateTimer->isActive() )
  {
    m_UpdateTimer->stop();
  }
  else
  {
    if ( m_Player->IsAtEnd() ) { m_Player->GoToSnapshot(0); }

    m_UpdateTimer->start(ui->updateIntervalSpinBox->value());
    if ( ! ui->playPushButton->isChecked() ) { ui->playPushButton->setChecked(true); }
  }
}

void QmitkNavigationDataSequentialPlayerControlWidget::OnRestart()
{
  this->OnStop();
  this->OnPlayPause();
}

void QmitkNavigationDataSequentialPlayerControlWidget::OnUpdate()
{
  // if the last snapshot was reached
  if ( ! m_Player->GoToNextSnapshot() )
  {
    m_UpdateTimer->stop();
    ui->playPushButton->setChecked(false);

    emit SignalEndReached();
  }

  m_Player->Update();

  this->UpdatePlayerDisplay();

  emit SignalUpdate();
}

void QmitkNavigationDataSequentialPlayerControlWidget::OnUpdateIntervalChanged(int value)
{
  m_UpdateTimer->setInterval(value);
}

void QmitkNavigationDataSequentialPlayerControlWidget::UpdatePlayerDisplay()
{
  int currentSnapshotNumber = static_cast<int>(m_Player->GetCurrentSnapshotNumber());

  ui->sampleLCDNumber->display(currentSnapshotNumber);

  ui->samplePositionHorizontalSlider->setValue(currentSnapshotNumber);
}
