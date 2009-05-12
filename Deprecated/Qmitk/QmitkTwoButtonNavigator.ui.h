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
void QmitkTwoButtonNavigator::Refetch()
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


void QmitkTwoButtonNavigator::SetStepper( mitk::Stepper * stepper)
{
    m_Stepper = stepper;
    m_InRefetch = (stepper==NULL); // this avoids trying to use m_Stepper until it is set to something != NULL (additionally to the avoiding recursions during refetching)
}

void QmitkTwoButtonNavigator::init()
{
  m_InRefetch = true; // this avoids trying to use m_Stepper until it is set to something != NULL (additionally to the avoiding recursions during refetching)
}

void QmitkTwoButtonNavigator::SpinBoxValueChanged(int)
{
  if(!m_InRefetch)
    m_Stepper->SetPos(m_SpinBox->value());
}

void QmitkTwoButtonNavigator::prevButton_clicked()
{
  if(!m_InRefetch) 
  {
    m_Stepper->Previous();
  }
}


void QmitkTwoButtonNavigator::nextButton_clicked()
{
  if(!m_InRefetch) 
  {
    m_Stepper->Next();
  }
}
