/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

StatusBarImplementation *StatusBar::m_Instance = NULL;

/**
 * Display the text in the statusbar of the applikation
 */
void StatusBar::DisplayText(const char* t)
{
    if (m_Instance != NULL)
        m_Instance->DisplayText(t);
}

/**
 * Display the text in the statusbar of the applikation for ms seconds
 */
void StatusBar::DisplayText(const char* t, int ms)
{
  if (m_Instance != NULL)
    m_Instance->DisplayText(t, ms);
}

void StatusBar::DisplayErrorText(const char *t)
{
  if (m_Instance != NULL)
    m_Instance->DisplayErrorText(t);
}
void StatusBar::DisplayWarningText(const char *t)
{
  if (m_Instance != NULL)
    m_Instance->DisplayWarningText(t);
}
void StatusBar::DisplayWarningText(const char *t, int ms)
{
  if (m_Instance != NULL)
    m_Instance->DisplayWarningText(t, ms);
}
void StatusBar::DisplayGenericOutputText(const char *t)
{
  if (m_Instance != NULL)
    m_Instance->DisplayGenericOutputText(t);
}
void StatusBar::DisplayDebugText(const char *t)
{
  if (m_Instance != NULL)
    m_Instance->DisplayDebugText(t);
}

void StatusBar::Clear()
{
  if ( m_Instance != NULL)
    m_Instance->Clear();
}
void StatusBar::SetSizeGripEnabled(bool enable)
{
  if (m_Instance != NULL)
  {
    m_Instance->SetSizeGripEnabled(enable);
  }
}
/**
 * Get the instance of this StatusBar
 */
StatusBarImplementation* StatusBar::GetInstance()
{
  if (m_Instance == NULL)//if not set, then send a errormessage on OutputWindow
  {
    (itk::OutputWindow::GetInstance())->DisplayErrorText("No instance of mitk::StatusBar! Check Applikation!");
    //return NULL; m_Instance then is null, so this is redundant!
  }
  
  return m_Instance;
}

/**
 * Set an instance of this; application must do this!See Header!
 */
void StatusBar::SetInstance(StatusBarImplementation* instance)
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
