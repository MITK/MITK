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
    
    m_MinValue = 0;
    m_MaxValue = m_Stepper->GetSteps()-1;
    m_SliceNumber = m_Stepper->GetPos();
       
    m_InRefetch=false;
  }
}


void QmitkTwoButtonNavigator::SetStepper( mitk::Stepper * stepper)
{
    m_Stepper = stepper;
    m_InRefetch = ( m_Stepper.IsNull() ); // this avoids trying to use m_Stepper until it is set to something != NULL (additionally to the avoiding recursions during refetching)
}


/*void QmitkTwoButtonNavigator::slider_valueChanged( int )
{
  if(!m_InRefetch)
    m_Stepper->SetPos(slider->value());
}
*/

void QmitkTwoButtonNavigator::init()
{
  m_InRefetch = true; // this avoids trying to use m_Stepper until it is set to something != NULL (additionally to the avoiding recursions during refetching)
}


/*void QmitkSliderNavigator::spinBox_valueChanged( int )
{
  if(!m_InRefetch)
    m_Stepper->SetPos(m_Stepper->ConvertPosToUnit(spinBox->value()));
}
*/


void QmitkTwoButtonNavigator::prevButton_clicked()
  {
  if(!m_InRefetch) 
    {
    if (m_SliceNumber > m_MinValue) m_SliceNumber--;
    m_Stepper->SetPos( m_SliceNumber );
    m_SliceNumberLabel->setText( QString("%1").arg(m_SliceNumber) );
    }
  }


void QmitkTwoButtonNavigator::nextButton_clicked()
  {
  if(!m_InRefetch) 
    {
    if (m_SliceNumber < m_MaxValue) m_SliceNumber++;
    m_Stepper->SetPos( m_SliceNumber );
    m_SliceNumberLabel->setText( QString("%1").arg( m_SliceNumber ) );
    }
  }
