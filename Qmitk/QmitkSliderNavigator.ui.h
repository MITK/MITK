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
    
    slider->setMaxValue(m_Stepper->GetSteps()-1);
    slider->setValue(m_Stepper->GetPos());
    
    spinBox->setMinValue(m_Stepper->ConvertPosToUnit(0));
    spinBox->setMaxValue(m_Stepper->ConvertPosToUnit(m_Stepper->GetSteps()));
    spinBox->setValue(m_Stepper->ConvertPosToUnit(m_Stepper->GetPos()));
    
    m_InRefetch=false;
  }
}


void QmitkSliderNavigator::SetStepper( mitk::Stepper * stepper)
{
    m_Stepper = stepper;
}


void QmitkSliderNavigator::slider_valueChanged( int )
{
  if(!m_InRefetch)
    m_Stepper->SetPos(slider->value());
}

void QmitkSliderNavigator::init()
{
    m_InRefetch = false;
}


void QmitkSliderNavigator::spinBox_valueChanged( int )
{
  if(!m_InRefetch)
    m_Stepper->SetPos(m_Stepper->ConvertPosToUnit(spinBox->value()));
}
