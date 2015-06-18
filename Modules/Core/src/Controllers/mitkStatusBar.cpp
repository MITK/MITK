/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkStatusBar.h"
#include <itkObjectFactory.h>
#include <itkOutputWindow.h>


namespace mitk {

StatusBarImplementation* StatusBar::m_Implementation = nullptr;
StatusBar* StatusBar::m_Instance = nullptr;

/**
 * Display the text in the statusbar of the applikation
 */
void StatusBar::DisplayText(const char* t)
{
    if (m_Implementation != nullptr)
        m_Implementation->DisplayText(t);
}

/**
 * Display the text in the statusbar of the applikation for ms seconds
 */
void StatusBar::DisplayText(const char* t, int ms)
{
  if (m_Implementation != nullptr)
    m_Implementation->DisplayText(t, ms);
}

void StatusBar::DisplayErrorText(const char *t)
{
  if (m_Implementation != nullptr)
    m_Implementation->DisplayErrorText(t);
}
void StatusBar::DisplayWarningText(const char *t)
{
  if (m_Implementation != nullptr)
    m_Implementation->DisplayWarningText(t);
}
void StatusBar::DisplayWarningText(const char *t, int ms)
{
  if (m_Implementation != nullptr)
    m_Implementation->DisplayWarningText(t, ms);
}
void StatusBar::DisplayGenericOutputText(const char *t)
{
  if (m_Implementation != nullptr)
    m_Implementation->DisplayGenericOutputText(t);
}
void StatusBar::DisplayDebugText(const char *t)
{
  if (m_Implementation != nullptr)
    m_Implementation->DisplayDebugText(t);
}
void StatusBar::DisplayGreyValueText(const char *t)
{
  if (m_Implementation != nullptr)
    m_Implementation->DisplayGreyValueText(t);
}
void StatusBar::Clear()
{
  if ( m_Implementation != nullptr)
    m_Implementation->Clear();
}
void StatusBar::SetSizeGripEnabled(bool enable)
{
  if (m_Implementation != nullptr)
  {
    m_Implementation->SetSizeGripEnabled(enable);
  }
}
/**
 * Get the instance of this StatusBar
 */
StatusBar* StatusBar::GetInstance()
{
  if (m_Instance == nullptr)//if not set, then send a errormessage on OutputWindow
  {
    m_Instance = new StatusBar();
  }

  return m_Instance;
}

/**
 * Set an instance of this; application must do this!See Header!
 */
void StatusBar::SetImplementation(StatusBarImplementation* implementation)
{
  if ( m_Implementation == implementation )
  {
    return;
  }
  m_Implementation = implementation;
}

StatusBar::StatusBar()
{
}

StatusBar::~StatusBar()
{
}

}//end namespace mitk
