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


#include "QmitkStatusBar.h"

#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qapplication.h>

#include <mitkStatusBar.h>

#include <itkObjectFactory.h>



/**
 * Display the text in the status bar of the application
 */
void QmitkStatusBar::DisplayText(const char* t)
{
  m_StatusBar->message(t);
  // bug #1357
  qApp->processEvents();
}

/**
 * Display the text in the status bar of the application for ms seconds
 */
void QmitkStatusBar::DisplayText(const char* t, int ms)
{
  m_StatusBar->message(t, ms);
  // bug #1357
  qApp->processEvents();
}

/**
 * Clear the text in the status bar
 */
void QmitkStatusBar::Clear()
{
  if (m_StatusBar != NULL)
    m_StatusBar->clear();
  // bug #1357
  qApp->processEvents();
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
    mitk::StatusBar::SetImplementation(this);
}

QmitkStatusBar::~QmitkStatusBar()
{
}

