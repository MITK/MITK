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
}

void QmitkNavigationDataSequentialPlayerControlWidget::OnPlayPause()
{
  if ( m_UpdateTimer->isActive() )
  {
    m_UpdateTimer->stop();
  }
  else
  {
    m_UpdateTimer->start(ui->updateIntervalSpinBox->value());
  }
}

void QmitkNavigationDataSequentialPlayerControlWidget::OnRestart()
{
  m_Player->GoToSnapshot(0);
}

void QmitkNavigationDataSequentialPlayerControlWidget::OnUpdate()
{
  m_Player->GoToNextSnapshot();
  m_Player->Update();

  int currentSnapshotNumber = static_cast<int>(m_Player->GetCurrentSnapshotNumber());

  ui->sampleLCDNumber->display(currentSnapshotNumber);

  ui->samplePositionHorizontalSlider->setValue(currentSnapshotNumber);

  emit SignalUpdate();

  if ( m_Player->GetCurrentSnapshotNumber() == m_Player->GetNumberOfSnapshots() - 1 )
  {
    emit SignalEndReached();
  }
}

void QmitkNavigationDataSequentialPlayerControlWidget::OnUpdateIntervalChanged(int value)
{
  m_UpdateTimer->setInterval(value);
}
