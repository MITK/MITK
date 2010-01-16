/*=========================================================================
 
Program:   BlueBerry Platform
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

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include "berryLogView.h"

#include "berryQtLogView.h"

#include <QHBoxLayout>

namespace berry {

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
