/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPrimitiveMovieNavigatorWidget.h"

QmitkPrimitiveMovieNavigatorWidget::QmitkPrimitiveMovieNavigatorWidget( QWidget* parent, Qt::WindowFlags fl )
    : QWidget( parent, fl )
{
  m_Controls.setupUi(this);

  // signals and slots connections
  connect( m_Controls.m_SpinBox, SIGNAL( valueChanged(int) ), this, SLOT( spinBoxValueChanged(int) ) );
  connect( m_Controls.m_StopButton, SIGNAL( clicked() ), this, SLOT( stopButton_clicked() ) );
  connect( m_Controls.m_GoButton, SIGNAL( clicked() ), this, SLOT( goButton_clicked() ) );
  connect( m_Controls.m_TimerInterval, SIGNAL( valueChanged(int) ), this, SLOT( setTimerInterval(int) ) );

  //this->setupUi(parent);
  m_InRefetch = true; // this avoids trying to use m_Stepper until it is set to something != NULL (additionally to the avoiding recursions during refetching)
  m_Timer = new QTimer(this);
  m_TimerIntervalInMS = 120;
  connect(m_Timer, SIGNAL(timeout()), SLOT(next()) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
QmitkPrimitiveMovieNavigatorWidget::~QmitkPrimitiveMovieNavigatorWidget()
{
    // no need to delete child widgets, Qt does it all for us
}

void QmitkPrimitiveMovieNavigatorWidget::Refetch()
{
  if(!m_InRefetch)
  {
    m_InRefetch=true;
    m_Controls.m_SpinBox->setMinimum( 0 );
    m_Controls.m_SpinBox->setMaximum( m_Stepper->GetSteps()-1 );
    m_Controls.m_SpinBox->setValue( m_Stepper->GetPos() );
    m_InRefetch=false;
  }
}


void QmitkPrimitiveMovieNavigatorWidget::SetStepper( mitk::Stepper * stepper)
{
  m_Stepper = stepper;
  m_InRefetch = (stepper==NULL); // this avoids trying to use m_Stepper until it is set to something != NULL (additionally to the avoiding recursions during refetching)
}

void QmitkPrimitiveMovieNavigatorWidget::goButton_clicked()
{
  if(!m_InRefetch && m_Stepper->GetSteps() > 0) // this step shall only be used if the dataset is 3D+t. If it is not, nothing happens :-)
  {
    if(m_Timer->isActive()==false)
    {
      m_Timer->start(m_TimerIntervalInMS);
    }
  }
}


void QmitkPrimitiveMovieNavigatorWidget::stopButton_clicked()
{
  m_Timer->stop();
}


void QmitkPrimitiveMovieNavigatorWidget::next()
{
  if(!m_InRefetch)
  {
    if(m_Stepper->GetPos()==m_Stepper->GetSteps()-1)
      m_Stepper->First();
    else
      m_Stepper->Next();
  }
}


void QmitkPrimitiveMovieNavigatorWidget::spinBoxValueChanged(int)
{
  if(!m_InRefetch)
  {
    m_Stepper->SetPos( m_Controls.m_SpinBox->value() );
  }
}

int QmitkPrimitiveMovieNavigatorWidget::getTimerInterval()
{
  return m_TimerIntervalInMS;
}


void QmitkPrimitiveMovieNavigatorWidget::setTimerInterval( int timerIntervalInMS )
{
  if(timerIntervalInMS!=m_TimerIntervalInMS)
  {
    m_TimerIntervalInMS = timerIntervalInMS;
    if(m_Timer->isActive())
    {
      m_Timer->setInterval(m_TimerIntervalInMS);
    }
  }
}
