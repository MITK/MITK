#include "QmitkStatusBar.h"
#include <qstatusbar.h>
#include <mitkStatusBar.h>
#include <qmainwindow.h>
#include <itkObjectFactory.h>



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


QmitkStatusBar::QmitkStatusBar(QStatusBar* instance)
:StatusBarImplementation()
{
    m_StatusBar = instance;
    mitk::StatusBar::SetInstance(this);
}

QmitkStatusBar::~QmitkStatusBar()
{
}
