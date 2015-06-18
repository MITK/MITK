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

#include "QmitkUpdateTimerWidget.h"

#include <QTimer>
#include <math.h>


static unsigned int DEFAULTUPDATEVALUE = 50; // default update value (in msec) for the timer

static unsigned int MINIMUMUPDATEVALUE = 10; // smallest value for the update rate spinbox
static unsigned int MAXIMUMUPDATEVALUE = 1000; // greatest value for the update rate spinbox
static unsigned int UPDATEVALUESTEP = 10; // step size for the update rate spinbox

QmitkUpdateTimerWidget::QmitkUpdateTimerWidget(QWidget* parent)
: QWidget(parent), m_Controls(nullptr)
{
  this->m_UpdateTimer = new QTimer( this );
  this->CreateQtPartControl( this );

  this->m_Controls->m_StopNavigationBtn->setEnabled( false );
  this->SetupUpdateRateSB( MINIMUMUPDATEVALUE, MAXIMUMUPDATEVALUE, UPDATEVALUESTEP );

  this->m_UpdateTimer->setInterval( DEFAULTUPDATEVALUE );
  this->m_Controls->m_UpdateRateSB->setValue( DEFAULTUPDATEVALUE );

  this->DisableWidget();
}

QmitkUpdateTimerWidget::~QmitkUpdateTimerWidget()
{
  m_UpdateTimer->stop();
  m_UpdateTimer = nullptr;
  m_Controls = nullptr;
}


void QmitkUpdateTimerWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkUpdateTimerWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}


void QmitkUpdateTimerWidget::CreateConnections()
{
  connect( (QObject*)(m_Controls->m_StartNavigationBtn), SIGNAL(clicked()), this, SLOT(OnStartTimer()) );
  connect( (QObject*)(m_Controls->m_StopNavigationBtn), SIGNAL(clicked()), this, SLOT(OnStopTimer()) );
  connect(  m_Controls->m_UpdateRateSB, SIGNAL(valueChanged(int)), this, SLOT(OnChangeTimerInterval(int)) );
}

unsigned int QmitkUpdateTimerWidget::GetTimerInterval()
{
  return this->m_UpdateTimer->interval();
}

void QmitkUpdateTimerWidget::OnChangeTimerInterval( int interval )
{
  this->SetTimerInterval(interval);
  this->SetFrameRateLabel();
}

void QmitkUpdateTimerWidget::SetTimerInterval( unsigned int msec )
{
  this->m_UpdateTimer->setInterval( msec );
  this->m_Controls->m_UpdateRateSB->setValue( msec );
}

void QmitkUpdateTimerWidget::StartTimer()
{
  if(!m_UpdateTimer->isActive())
  {
    this->m_UpdateTimer->start();
    this->m_Controls->m_StartNavigationBtn->setEnabled( false );
    this->m_Controls->m_StopNavigationBtn->setEnabled( true );
    this->m_Controls->m_NavigationStateLbl->setStyleSheet( "QLabel{background-color: #96e066 }" );
    this->m_Controls->m_NavigationStateLbl->setText( "Started ... " );

    emit Started();
  }
}

void QmitkUpdateTimerWidget::StopTimer()
{
  if(m_UpdateTimer->isActive())
  {
    m_UpdateTimer->stop();
    this->m_Controls->m_StopNavigationBtn->setEnabled( false );
    this->m_Controls->m_StartNavigationBtn->setEnabled( true );
    this->m_Controls->m_NavigationStateLbl->setStyleSheet( "QLabel{background-color: #ffcccc }" );
    this->m_Controls->m_NavigationStateLbl->setText( "Stopped ... " );

    emit Stopped();
  }
}

QTimer* QmitkUpdateTimerWidget::GetUpdateTimer()
{
  return this->m_UpdateTimer;
}

void QmitkUpdateTimerWidget::OnStartTimer()
{
  this->StartTimer();
}

void QmitkUpdateTimerWidget::OnStopTimer()
{
  this->StopTimer();
}


void QmitkUpdateTimerWidget::SetPurposeLabelText( QString text )
{
  m_Controls->m_StartNavigationBtn->setText( " Start " + text );
  m_Controls->m_StopNavigationBtn->setText( " Stop " + text );
}


void QmitkUpdateTimerWidget::SetupUpdateRateSB( int min, int max, int step )
{
  this->m_Controls->m_UpdateRateSB->setRange( min , max );
  this->m_Controls->m_UpdateRateSB->setSingleStep( step );
}


void QmitkUpdateTimerWidget::SetFrameRateLabel()
{
  float frameRate = floor(1000 / (float) this->GetTimerInterval() + 0.5);  // floor rounding can be used because there are no negative values
  QString frameRateString = QString::number( frameRate, 'g', 4 );
  this->m_Controls->m_FrameRateLbl->setText("msec   (" + frameRateString + " Hz)");
}

void QmitkUpdateTimerWidget::HideFramerateSettings( bool hidden )
{
  this->m_Controls->m_UpdatesInMsecLbl->setVisible( !hidden );
  this->m_Controls->m_UpdateRateSB->setVisible ( !hidden );
  this->m_Controls->m_FrameRateLbl->setVisible ( !hidden );
}


void QmitkUpdateTimerWidget::EnableWidget()
{
  this->setEnabled( true );
}

void QmitkUpdateTimerWidget::DisableWidget()
{
  this->StopTimer();
  this->setEnabled( false );
}



void QmitkUpdateTimerWidget::SetIcon( WidgetButtons button, const QIcon& icon )
{
  switch( button )
  {
  case StartButton:
    m_Controls->m_StartNavigationBtn->setIcon(icon);
    break;

  case StopButton: m_Controls->m_StopNavigationBtn->setIcon(icon);
    break;

  default:
    break;
  }
}
