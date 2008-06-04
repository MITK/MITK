/*=========================================================================
 
Program:   openCherry Platform
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

#include "cherryQtErrorView.h"

#include <QDockWidget>

namespace cherry
{

QtErrorView::QtErrorView()
 : m_Label(0)
{
  
}

void* QtErrorView::CreatePartControl(void* parent)
{
  QDockWidget* dock = static_cast<QDockWidget*>(parent);
  m_Label = new QLabel(dock);
  m_Label->setText(m_ErrorMsg);
  dock->setWidget(m_Label);
  
  return m_Label;
}
 
void QtErrorView::SetFocus()
{
  m_Label->setFocus();
}

void QtErrorView::SetErrorMsg(const std::string& msg)
{
  m_ErrorMsg = msg.c_str();
  if (m_Label != 0)
    m_Label->setText(m_ErrorMsg);
}

}
