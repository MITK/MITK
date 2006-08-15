/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "QmitkSliderLevelWindowWidget.h"

//for constructor 

void QmitkLevelWindowWidget::init() 
{
  m_Manager = mitk::LevelWindowManager::New();

  SliderLevelWindowWidget->setLevelWindowManager(m_Manager.GetPointer());
  LineEditLevelWindowWidget->setLevelWindowManager(m_Manager.GetPointer());
} 

//for destructor 
void QmitkLevelWindowWidget::destroy() 
{ 
} 

void QmitkLevelWindowWidget::setDataTreeIteratorClone( mitk::DataTreeIteratorClone& it ) 
{ 
  m_Manager->SetDataTreeIteratorClone(it); 
} 


/// throws exception if no level window object available
const mitk::LevelWindow& QmitkLevelWindowWidget::GetLevelWindow()
{
  return m_Manager->GetLevelWindow();
}


void QmitkLevelWindowWidget::SetLevelWindow( const mitk::LevelWindow & levWin )
{
  m_Manager->SetLevelWindow(levWin);
}
