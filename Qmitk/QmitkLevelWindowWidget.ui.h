/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include "QmitkSliderLevelWindowWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include<qvalidator.h>

//for constructor
void QmitkLevelWindowWidget::init()
{
  slw = &(SliderLevelWindowWidget->getLevelWindow());
  
  m_Level = (int)slw->GetLevel();//initial
  m_Window = (int)slw->GetWindow();//initial

  QString initLevel;
  initLevel.setNum(slw->GetLevel());

  
  QString initWindow;
  initWindow.setNum(slw->GetWindow());

  //LevelInput->setText("0");//write initial in LineEdit
  //WindowInput->setText("200");//write initial in LineEdit

  LevelInput->setText(initLevel);//write initial in LineEdit
  WindowInput->setText(initWindow);//write initial in LineEdit

  connect(SliderLevelWindowWidget,SIGNAL(levelWindow(mitk::LevelWindow*)),this,SLOT(levelWindowSliderChanged(mitk::LevelWindow*)) );
  connect(SliderLevelWindowWidget, SIGNAL(newRange(mitk::LevelWindow*)), this, SLOT(rangeChanged(mitk::LevelWindow*)) );

  //Validator for both LineEdit-widgets, to limit the valid input-range to int.
  QValidator* validatorWindowInput = new QIntValidator(1, 4096, this);
  WindowInput->setValidator(validatorWindowInput);

  QValidator* validatorLevelInput = new QIntValidator(-2048, 2048, this);
  LevelInput->setValidator(validatorLevelInput);

}



//SET-METHODS
//read the levelInput and change level and slider when the button "ENTER" was pressed in the windowInput-LineEdit
void QmitkLevelWindowWidget::SetLevelValue()
{
  m_Level = atoi(LevelInput->text().latin1());
  slw = &(SliderLevelWindowWidget->getLevelWindow());
  slw->SetLevel(m_Level);
  SliderLevelWindowWidget->setLevelWindow(*slw);
  SliderRepaint();
  validWindow();

}
//read the windowInput and change window and slider when the button "ENTER" was pressed in the windowInput-LineEdit
void QmitkLevelWindowWidget::SetWindowValue()
{
  m_Window = atoi(WindowInput->text().latin1());
  slw = &(SliderLevelWindowWidget->getLevelWindow());
  slw->SetWindow(m_Window);
  SliderLevelWindowWidget->setLevelWindow(getLevelWindow());

  SliderRepaint();
  validWindow();
}

//check for valid dates in the LineEdits
void QmitkLevelWindowWidget::validWindow()
{
  m_Window = (int)getLevelWindow().GetWindow();
  QString stringMWindow;
  stringMWindow.setNum(m_Window);
  WindowInput->setText(stringMWindow);

  m_Level = (int)getLevelWindow().GetLevel();
  QString stringMLevel;
  stringMLevel.setNum(m_Level);
  LevelInput->setText(stringMLevel);

  //send signal to change Slider
  emit levelWindow( & getLevelWindow() );
} 

//repaint slider after a value was changed
void QmitkLevelWindowWidget::SliderRepaint()
{
  SliderLevelWindowWidget->updateFromLineEdit();
}


//set new LevelWindow in class QSliderLevelWindow
void QmitkLevelWindowWidget::setLevelWindow( const mitk::LevelWindow & slw )
{
  SliderLevelWindowWidget->setLevelWindow(slw);
  SliderRepaint();
  validWindow();
}

//get LevelWindow from class QSliderLevelWindow
mitk::LevelWindow& QmitkLevelWindowWidget::getLevelWindow()
{
  return SliderLevelWindowWidget->getLevelWindow();
}

//updates the LineEdit after slider was changed
//is called up when the slider was changed
void QmitkLevelWindowWidget::levelWindowSliderChanged( mitk::LevelWindow * slw )
{
  emit levelWindow(slw);

  QString l;
  int lint = (int)getLevelWindow().GetLevel();
  l.setNum(lint);

  QString w;
  int wint = (int)getLevelWindow().GetWindow();
  w.setNum(wint);

  LevelInput->setText(l);
  WindowInput->setText(w);
  validWindow();
}

//for destructor
void QmitkLevelWindowWidget::destroy()
{

}


void QmitkLevelWindowWidget::rangeChanged( mitk::LevelWindow *lw )
{
  //Validator for both LineEdit-widgets, to limit the valid input-range to int.
  QValidator* validatorWindowInput = new QIntValidator(1, (int)(lw->GetRange()), this);
  WindowInput->setValidator(validatorWindowInput);

  QValidator* validatorLevelInput = new QIntValidator((int)(lw->GetRangeMin()), (int)(lw->GetRangeMax()), this);
  LevelInput->setValidator(validatorLevelInput);
}
