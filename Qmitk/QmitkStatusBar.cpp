#include "QmitkStatusBar.h"
#include <qstatusbar.h>
#include <qmainwindow.h>
#include <itkObjectFactory.h>


QStatusBar* QmitkStatusBar::m_StatusBar=NULL;

/**
 * Display the text in the statusbar of the applikation
 */
void QmitkStatusBar::DisplayText(const char* t)
{
  m_StatusBar->message(t);
}

/**
 * Display the text in the statusbar of the applikation for ms seconds
 */
void QmitkStatusBar::DisplayText(const char* t, int ms)
{
  m_StatusBar->message(t, ms);
}

/**
 * Clear the text in the StatusBar
 */
void QmitkStatusBar::Clear()
{
  if (m_StatusBar != NULL)
    m_StatusBar->clear();
}

/**
 * enable or disable the QSizeGrip
 */
void QmitkStatusBar::SetSizeGripEnabled(bool enable)
{
  if (m_StatusBar != NULL)
    m_StatusBar->setSizeGripEnabled(enable);
}

mitk::StatusBar::Pointer QmitkStatusBar::New()
{ 
  if ( ! QmitkStatusBar::m_Instance )
  {
    // Try the factory first
    QmitkStatusBar::m_Instance = itk::ObjectFactory<Self>::Create();
    // if the factory did not provide one, then create it here
    if( ! QmitkStatusBar::m_Instance )
    {
      QmitkStatusBar::m_Instance = new QmitkStatusBar;
      // Remove extra reference from construction.
      QmitkStatusBar::m_Instance->UnRegister();
    }  
    //mitk::StatusBar::SetInstance(m_Instance);//not needed, because same static m_Instance
  }
  
return m_Instance;
}

void QmitkStatusBar::SetStatusBar(QStatusBar* statusBar)
{
  //if not created yet, then create it!
  if ( ! QmitkStatusBar::m_Instance )
  {
    // Try the factory first
    QmitkStatusBar::m_Instance = itk::ObjectFactory<Self>::Create();
    // if the factory did not provide one, then create it here
    if( ! QmitkStatusBar::m_Instance )
    {
      QmitkStatusBar::m_Instance = new QmitkStatusBar;
      // Remove extra reference from construction.
      QmitkStatusBar::m_Instance->UnRegister();
    }
  }

  QStatusBar * tempStatusBar = dynamic_cast<QStatusBar *>(statusBar);
  if ( tempStatusBar == NULL )
    return;
  
  m_StatusBar = statusBar;
}

QmitkStatusBar::QmitkStatusBar()
:StatusBar()
{
}

QmitkStatusBar::~QmitkStatusBar()
{
}
