/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkStatusBar.h"

#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qapplication.h>
#include <qdesktopwidget.h>

#include <mitkStatusBar.h>

#include <itkObjectFactory.h>


/**
 * Display the text in the statusbar of the applikation
 */
void QmitkStatusBar::DisplayText(const char* t)
{
  m_StatusBar->showMessage(t);
  // TODO bug #1357
  //qApp->processEvents(); // produces crashes!
}

/**
 * Display the text in the statusbar of the applikation for ms seconds
 */
void QmitkStatusBar::DisplayText(const char* t, int ms)
{
  m_StatusBar->showMessage(t, ms);
  // TODO bug #1357
  //qApp->processEvents(); // produces crashes!
}
/**
 * Show the grey value text in the statusbar
 */
void QmitkStatusBar::DisplayGreyValueText(const char* t)
{
  QString text(t);
  m_GreyValueLabel->setText(text);
}
/**
 * Clear the text in the StatusBar
 */
void QmitkStatusBar::Clear()
{
  if (m_StatusBar != nullptr)
    m_StatusBar->clearMessage();
  // TODO bug #1357
  //qApp->processEvents(); // produces crashes!
}

/**
 * enable or disable the QSizeGrip
 */
void QmitkStatusBar::SetSizeGripEnabled(bool enable)
{
  if (m_StatusBar != nullptr)
    m_StatusBar->setSizeGripEnabled(enable);
}


QmitkStatusBar::QmitkStatusBar(QStatusBar* instance)
:StatusBarImplementation()
{
    m_StatusBar = instance;
    m_GreyValueLabel = new QLabel(m_StatusBar,nullptr);
    int xResolution = QApplication::desktop()->screenGeometry(0).width()-100;
    m_GreyValueLabel->setMaximumSize(QSize(xResolution,50));
    m_GreyValueLabel->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
    m_StatusBar->addPermanentWidget(m_GreyValueLabel);
    mitk::StatusBar::SetImplementation(this);
}

QmitkStatusBar::~QmitkStatusBar()
{
}

