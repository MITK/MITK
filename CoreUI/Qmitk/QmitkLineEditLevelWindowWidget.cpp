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

#include "QmitkLineEditLevelWindowWidget.h"
#include "QmitkLevelWindowWidgetContextMenu.h"

#include <mitkRenderingManager.h>

#include <itkCommand.h>
#include <QValidator>
#include <QLayout>
#include <QLineEdit>


/**
* Constructor
*/
QmitkLineEditLevelWindowWidget::QmitkLineEditLevelWindowWidget(QWidget* parent, Qt::WindowFlags f)
 : QWidget(parent, f)
{
  m_Manager = mitk::LevelWindowManager::New();
  
  itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::Pointer command = itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::New();
  command->SetCallbackFunction(this, &QmitkLineEditLevelWindowWidget::OnPropertyModified);
  m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
  m_IsObserverTagSet = true;

  m_Contextmenu = new QmitkLevelWindowWidgetContextMenu(this); // true);
  
  QVBoxLayout* layout = new QVBoxLayout( this );
  layout->setMargin(0);
  layout->setSpacing(0);

  m_LevelInput = new QLineEdit( this );
  m_LevelInput->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred));
  //m_LevelInput->setFrameShape( QLineEdit::LineEditPanel );
  //m_LevelInput->setFrameShadow( QLineEdit::Sunken );

  m_WindowInput = new QLineEdit( this );
  m_WindowInput->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred));
  //m_WindowInput->setFrameShape( QLineEdit::LineEditPanel );
  //m_WindowInput->setFrameShadow( QLineEdit::Sunken );

  layout->addWidget(m_LevelInput);
  layout->addWidget(m_WindowInput);

  // signals and slots connections
  connect( m_LevelInput, SIGNAL(editingFinished()), this, SLOT( SetLevelValue() ) );
  connect( m_WindowInput, SIGNAL(editingFinished()), this, SLOT( SetWindowValue() ) );

  // Validator for both LineEdit-widgets, to limit the valid input-range to int.
  QValidator* validatorWindowInput = new QIntValidator(1, 20000000, this);
  m_WindowInput->setValidator(validatorWindowInput);

  QValidator* validatorLevelInput = new QIntValidator(-10000000, 10000000, this);
  m_LevelInput->setValidator(validatorLevelInput);
  
  this->hide();
}

QmitkLineEditLevelWindowWidget::~QmitkLineEditLevelWindowWidget()
{
  if( m_IsObserverTagSet )
  {
    m_Manager->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
}

void QmitkLineEditLevelWindowWidget::OnPropertyModified(const itk::EventObject& )
{
  try
  {
    m_LevelWindow = m_Manager->GetLevelWindow();
    //setValidator();
    QString level;
    level.setNum((int)(m_LevelWindow.GetLevel()));
    m_LevelInput->setText(level);
    QString window;
    window.setNum((int)(m_LevelWindow.GetWindow()));
    m_WindowInput->setText(window);
    m_LevelInput->setEnabled(!m_LevelWindow.IsFixed());  
    m_WindowInput->setEnabled(!m_LevelWindow.IsFixed());  
    this->show();
  }
  catch(...)
  {
    try
    {
      this->hide();
    }
    catch(...)
    {
    }
  }
}

void QmitkLineEditLevelWindowWidget::setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager)
{
  if( m_IsObserverTagSet )
  {
    m_Manager->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
  m_Manager = levelWindowManager;
  if ( m_Manager.IsNotNull() )
  {
    itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::Pointer command = itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::New();
    command->SetCallbackFunction(this, &QmitkLineEditLevelWindowWidget::OnPropertyModified);
    m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
    m_IsObserverTagSet = true;
  }
}

void QmitkLineEditLevelWindowWidget::SetDataStorage( mitk::DataStorage* ds )
{
  m_Manager->SetDataStorage(ds);
}

//read the levelInput and change level and slider when the button "ENTER" was pressed in the windowInput-LineEdit
void QmitkLineEditLevelWindowWidget::SetLevelValue()
{
  validLevel();
}

//read the windowInput and change window and slider when the button "ENTER" was pressed in the windowInput-LineEdit
void QmitkLineEditLevelWindowWidget::SetWindowValue()
{
  validWindow();
}

void QmitkLineEditLevelWindowWidget::contextMenuEvent( QContextMenuEvent * )
{ 
  m_Contextmenu->setLevelWindowManager(m_Manager.GetPointer());
  m_Contextmenu->getContextMenu();
}

void QmitkLineEditLevelWindowWidget::validLevel()
{
  int level = m_LevelInput->text().toInt();
  if ( level + (int)(m_LevelWindow.GetWindow()/2) > (int)(m_LevelWindow.GetRangeMax()))
  {
    level = (int)(m_LevelWindow.GetRangeMax() - m_LevelWindow.GetWindow()/2);
  }
  if (level - (int)(m_LevelWindow.GetWindow()/2) < (int)(m_LevelWindow.GetRangeMin()))
  {
    level = (int)(m_LevelWindow.GetRangeMin() + m_LevelWindow.GetWindow()/2);
  }
  QString qLevel;
  qLevel.setNum(level);
  m_LevelInput->setText(qLevel);
  m_LevelWindow.SetLevelWindow(level, m_LevelWindow.GetWindow());
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLineEditLevelWindowWidget::validWindow()
{
  int window = m_WindowInput->text().toInt();
  if ( (int)(m_LevelWindow.GetLevel()) + window/2 > (int)(m_LevelWindow.GetRangeMax()))
  {
    window = (int)((m_LevelWindow.GetRangeMax() - m_LevelWindow.GetLevel())*2);
  }
  if ((int)(m_LevelWindow.GetLevel()) - window/2 < (int)(m_LevelWindow.GetRangeMin()))
  {
    window = (int)((m_LevelWindow.GetLevel() - m_LevelWindow.GetRangeMin())*2);
  }
  QString qWindow;
  qWindow.setNum(window);
  m_WindowInput->setText(qWindow);
  m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel(), window);
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::LevelWindowManager* QmitkLineEditLevelWindowWidget::GetManager()
{
  return m_Manager.GetPointer();
}
