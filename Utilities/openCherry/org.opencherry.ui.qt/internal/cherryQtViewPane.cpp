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

#include "cherryQtViewPane.h"

namespace cherry {

QtViewPane::QtViewPane(const QString& title, QWidget* parent, Qt::WindowFlags flags )
 : QDockWidget(title, parent, flags)
{
  
}

void* QtViewPane::GetControl()
{
  return this;
}

bool QtViewPane::GetVisible()
{
  return this->QDockWidget::isVisible();
}

void QtViewPane::SetVisible(bool visible)
{
  this->QDockWidget::setVisible(visible);
}

}
