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

#include "cherryLogView.h"

#include "cherryQtLogView.h"

#include <QHBoxLayout>

namespace cherry {

void LogView::CreateQtPartControl(QWidget* parent)
{
  QHBoxLayout* layout = new QHBoxLayout(parent);
  layout->setContentsMargins(0,0,0,0);
  QtLogView* logView = new QtLogView(parent);
  layout->addWidget(logView);
}

void LogView::SetFocus()
{
  
}

}
