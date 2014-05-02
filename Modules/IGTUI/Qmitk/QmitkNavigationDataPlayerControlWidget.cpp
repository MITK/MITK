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

#include "QmitkNavigationDataPlayerControlWidget.h"
#include "ui_QmitkNavigationDataPlayerControlWidget.h"

#include <QTimer>

QmitkNavigationDataPlayerControlWidget::QmitkNavigationDataPlayerControlWidget(QWidget *parent) :
  QWidget(parent),
  m_UpdateTimer(new QTimer(this)),
  ui(new Ui::QmitkNavigationDataPlayerControlWidget)
{
  ui->setupUi(this);

  connect(m_UpdateTimer, SIGNAL(timeout()), this, SLOT(OnUpdate()));
}

QmitkNavigationDataPlayerControlWidget::~QmitkNavigationDataPlayerControlWidget()
{
  delete ui;
}

void QmitkNavigationDataPlayerControlWidget::SetPlayer(mitk::NavigationDataPlayer::Pointer player)
{
  m_Player = player;

  ui->samplePositionHorizontalSlider->setMaximum(player->GetNumberOfSnapshots()-1);
}

void QmitkNavigationDataPlayerControlWidget::OnStop()
{
  m_UpdateTimer->stop();
  m_Player->StopPlaying();

  ui->playPushButton->setChecked(false);

  this->ResetPlayerDisplay();
}

void QmitkNavigationDataPlayerControlWidget::OnPlayPause()
{
  switch ( m_Player->GetCurrentPlayerState() )
  {
  case mitk::NavigationDataPlayer::PlayerStopped:
  {
    m_Player->StartPlaying();
    if ( ! m_UpdateTimer->isActive() ) { m_UpdateTimer->start(10); }
    if ( ! ui->playPushButton->isChecked() ) { ui->playPushButton->setChecked(true); }
    break;
  }
  case mitk::NavigationDataPlayer::PlayerPaused:
  {
    m_Player->Resume();
    break;
  }
  case mitk::NavigationDataPlayer::PlayerRunning:
  {
    m_Player->Pause();
    break;
  }
  }
}

void QmitkNavigationDataPlayerControlWidget::OnRestart()
{
  this->OnStop();
  this->OnPlayPause();
}

void QmitkNavigationDataPlayerControlWidget::OnUpdate()
{
  m_Player->Update();

  int msc = static_cast<int>(m_Player->GetTimeStampSinceStart());

  // calculation for playing time display
  int ms = msc % 1000;
  msc = (msc - ms) / 1000;
  int s = msc % 60;
  int min = (msc-s) / 60;

  // set lcd numbers
  ui->msecLCDNumber->display(ms);
  ui->secLCDNumber->display(s);
  ui->minLCDNumber->display(min);

  ui->samplePositionHorizontalSlider->setValue(static_cast<int>(m_Player->GetCurrentSnapshotNumber()));

  emit SignalUpdate();

  if ( m_Player->GetCurrentPlayerState() == mitk::NavigationDataPlayer::PlayerStopped )
  {
    m_UpdateTimer->stop();
    ui->playPushButton->setChecked(false);

    emit SignalEndReached();
  }
}

void QmitkNavigationDataPlayerControlWidget::ResetPlayerDisplay()
{
  // set lcd numbers
  ui->msecLCDNumber->display(0);
  ui->secLCDNumber->display(0);
  ui->minLCDNumber->display(0);

  ui->samplePositionHorizontalSlider->setValue(0);
}
