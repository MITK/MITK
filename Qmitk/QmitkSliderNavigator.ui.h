/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


void QmitkSliderNavigator::Refetch()
{
  if(!m_InRefetch)
  {
    m_InRefetch=true;
    
    slider->setMinValue(0);
    slider->setMaxValue(m_Stepper->GetSteps()-1);
    slider->setValue(m_Stepper->GetPos());
    
    spinBox->setMinValue( (int) m_Stepper->ConvertPosToUnit(0) );
    spinBox->setMaxValue( (int) m_Stepper->ConvertPosToUnit(m_Stepper->GetSteps()));
    spinBox->setValue((int) m_Stepper->ConvertPosToUnit(m_Stepper->GetPos()) );
    
    m_InRefetch=false;
  }
}


void QmitkSliderNavigator::SetStepper( mitk::Stepper * stepper)
{
    m_Stepper = stepper;
    m_InRefetch = (stepper==NULL); // this avoids trying to use m_Stepper until it is set to something != NULL (additionally to the avoiding recursions during refetching)
}


void QmitkSliderNavigator::slider_valueChanged( int )
{
  if(!m_InRefetch)
    m_Stepper->SetPos(slider->value());
}

void QmitkSliderNavigator::init()
{
  m_InRefetch = true; // this avoids trying to use m_Stepper until it is set to something != NULL (additionally to the avoiding recursions during refetching)
}


void QmitkSliderNavigator::spinBox_valueChanged( int )
{
  if(!m_InRefetch)
    m_Stepper->SetPos( (int) m_Stepper->ConvertPosToUnit(spinBox->value()) );
}

int QmitkSliderNavigator::GetPos()
{
    return m_Stepper->GetPos();
}

void QmitkSliderNavigator::SetPos(int val)
{
    if(!m_InRefetch)    m_Stepper->SetPos( val );
}
