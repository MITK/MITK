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
  m_Manager = new mitk::LevelWindowManager();
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
