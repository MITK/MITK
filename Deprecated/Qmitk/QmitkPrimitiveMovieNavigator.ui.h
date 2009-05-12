/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
void QmitkPrimitiveMovieNavigator::Refetch()
{
  if(!m_InRefetch)
  {
    m_InRefetch=true;
    m_SpinBox->setMinValue( 0 );
    m_SpinBox->setMaxValue( m_Stepper->GetSteps()-1 );
    m_SpinBox->setValue( m_Stepper->GetPos() );
    m_InRefetch=false;
  }
}


void QmitkPrimitiveMovieNavigator::SetStepper( mitk::Stepper * stepper)
{
  m_Stepper = stepper;
  m_InRefetch = (stepper==NULL); // this avoids trying to use m_Stepper until it is set to something != NULL (additionally to the avoiding recursions during refetching)
}

void QmitkPrimitiveMovieNavigator::init()
{
  m_InRefetch = true; // this avoids trying to use m_Stepper until it is set to something != NULL (additionally to the avoiding recursions during refetching)
  m_Timer = new QTimer(this);
  m_TimerIntervalInMS = 120;
  connect(m_Timer, SIGNAL(timeout()), SLOT(next()) );
}

void QmitkPrimitiveMovieNavigator::goButton_clicked()
{
  if(!m_InRefetch && m_Stepper->GetSteps() > 0) // this step shall only be used if the dataset is 3D+t. If it is not, nothing happens :-) 
  {
    if(m_Timer->isActive()==false)
    {
      m_Timer->start(m_TimerIntervalInMS);
    }
  }
}


void QmitkPrimitiveMovieNavigator::stopButton_clicked()
{
  m_Timer->stop();
}


void QmitkPrimitiveMovieNavigator::next()
{
  if(!m_InRefetch) 
  {
    if(m_Stepper->GetPos()==m_Stepper->GetSteps()-1)
      m_Stepper->First();
    else
      m_Stepper->Next();
  }
}


void QmitkPrimitiveMovieNavigator::spinBoxValueChanged(int)
{
  if(!m_InRefetch) 
  {
    m_Stepper->SetPos( m_SpinBox->value() );
  }
}

int QmitkPrimitiveMovieNavigator::getTimerInterval()
{
  return m_TimerIntervalInMS;
}


void QmitkPrimitiveMovieNavigator::setTimerInterval( int timerIntervalInMS )
{
  if(timerIntervalInMS!=m_TimerIntervalInMS)
  {
    m_TimerIntervalInMS = timerIntervalInMS;
    if(m_Timer->isActive())
    {
      m_Timer->changeInterval(m_TimerIntervalInMS);
    }
  }
}
