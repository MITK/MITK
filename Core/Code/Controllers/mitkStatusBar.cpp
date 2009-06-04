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


#include "mitkStatusBar.h"
#include <itkObjectFactory.h>
#include <itkOutputWindow.h>


namespace mitk {

StatusBarImplementation* StatusBar::m_Implementation = NULL;
StatusBar* StatusBar::m_Instance = NULL;

/**
 * Display the text in the statusbar of the applikation
 */
void StatusBar::DisplayText(const char* t)
{
    if (m_Implementation != NULL)
        m_Implementation->DisplayText(t);
}

/**
 * Display the text in the statusbar of the applikation for ms seconds
 */
void StatusBar::DisplayText(const char* t, int ms)
{
  if (m_Implementation != NULL)
    m_Implementation->DisplayText(t, ms);
}

void StatusBar::DisplayErrorText(const char *t)
{
  if (m_Implementation != NULL)
    m_Implementation->DisplayErrorText(t);
}
void StatusBar::DisplayWarningText(const char *t)
{
  if (m_Implementation != NULL)
    m_Implementation->DisplayWarningText(t);
}
void StatusBar::DisplayWarningText(const char *t, int ms)
{
  if (m_Implementation != NULL)
    m_Implementation->DisplayWarningText(t, ms);
}
void StatusBar::DisplayGenericOutputText(const char *t)
{
  if (m_Implementation != NULL)
    m_Implementation->DisplayGenericOutputText(t);
}
void StatusBar::DisplayDebugText(const char *t)
{
  if (m_Implementation != NULL)
    m_Implementation->DisplayDebugText(t);
}

void StatusBar::Clear()
{
  if ( m_Implementation != NULL)
    m_Implementation->Clear();
}
void StatusBar::SetSizeGripEnabled(bool enable)
{
  if (m_Implementation != NULL)
  {
    m_Implementation->SetSizeGripEnabled(enable);
  }
}
/**
 * Get the instance of this StatusBar
 */
StatusBar* StatusBar::GetInstance()
{
  if (m_Instance == NULL)//if not set, then send a errormessage on OutputWindow
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
