/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "QmitkSliderLevelWindowWidget.h"
#include <itkCommand.h>

//for constructor 

void QmitkLevelWindowWidget::init() 
{
  m_Lw = NULL;
  m_Manager = new mitk::LevelWindowManager();
  itk::ReceptorMemberCommand<QmitkLevelWindowWidget>::Pointer command = itk::ReceptorMemberCommand<QmitkLevelWindowWidget>::New();
  command->SetCallbackFunction(this, &QmitkLevelWindowWidget::OnPropertyModified);
  m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
  SliderLevelWindowWidget->setLevelWindowManager(m_Manager);
  LineEditLevelWindowWidget->setLevelWindowManager(m_Manager);
} 

//for destructor 
void QmitkLevelWindowWidget::destroy() 
{ 
} 

void QmitkLevelWindowWidget::setDataTreeIteratorClone( mitk::DataTreeIteratorClone& it ) 
{ 
    m_Manager->SetDataTreeIteratorClone(it); 
} 


void QmitkLevelWindowWidget::OnPropertyModified( const itk::EventObject & )
{
  try
  {
    m_Lw = m_Manager->GetLevelWindow();
  }
  catch(...)
  {
    m_Lw = NULL;
  }
}


const mitk::LevelWindow QmitkLevelWindowWidget::GetLevelWindow()
{
  return m_Lw;
}


void QmitkLevelWindowWidget::SetLevelWindow( const mitk::LevelWindow & levWin )
{
  m_Manager->SetLevelWindow(levWin);
}
