#include "mitkStatusBar.h"
#include <itkObjectFactory.h>
#include <itkOutputWindow.h>


namespace mitk {

StatusBar::Pointer StatusBar::m_Instance = NULL;

/**
 * Display the text in the statusbar of the applikation
 */
void StatusBar::DisplayText(const char* t)
{
    if (m_Instance.IsNotNull())
        m_Instance->DisplayText(t);
}

/**
 * Display the text in the statusbar of the applikation for ms seconds
 */
void StatusBar::DisplayText(const char* t, int ms)
{
  if (m_Instance.IsNotNull())
    m_Instance->DisplayText(t, ms);
}

void StatusBar::DisplayErrorText(const char *t)
{
  if (m_Instance.IsNotNull())
    m_Instance->DisplayErrorText(t);
}
void StatusBar::DisplayWarningText(const char *t)
{
  if (m_Instance.IsNotNull())
    m_Instance->DisplayWarningText(t);
}
void StatusBar::DisplayWarningText(const char *t, int ms)
{
  if (m_Instance.IsNotNull())
    m_Instance->DisplayWarningText(t, ms);
}
void StatusBar::DisplayGenericOutputText(const char *t)
{
  if (m_Instance.IsNotNull())
    m_Instance->DisplayGenericOutputText(t);
}
void StatusBar::DisplayDebugText(const char *t)
{
  if (m_Instance.IsNotNull())
    m_Instance->DisplayDebugText(t);
}

void StatusBar::Clear()
{
  if ( m_Instance.IsNotNull())
    m_Instance->Clear();
}
void StatusBar::SetSizeGripEnabled(bool enable)
{
  if (m_Instance.IsNotNull())
  {
    m_Instance->SetSizeGripEnabled(enable);
  }
}
/**
 * Get the instance of this StatusBar
 */
StatusBar* StatusBar::GetInstance()
{
  if (m_Instance.IsNull())//if not set, then send a errormessage on OutputWindow
  {
    (itk::OutputWindow::GetInstance())->DisplayErrorText("No instance of mitk::StatusBar! Check Applikation!");

  }
  
  return m_Instance;
}

/**
 * Set an instance of this; application must do this!See Header!
 */
void StatusBar::SetInstance(StatusBar* instance)
{
  if ( m_Instance == instance )
  {
    return;
  }
  m_Instance = instance;
}

StatusBar::StatusBar()
{
}

StatusBar::~StatusBar()
{
}

}//end namespace mitk