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

#include "QmitkApplicationCursor.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qpixmap.h>

#include <iostream>

QmitkApplicationCursor::QmitkApplicationCursor()
{
  mitk::ApplicationCursor::RegisterImplementation(this);
}

void QmitkApplicationCursor::PushCursor(const char* XPM[], int hotspotX, int hotspotY) 
{
  QPixmap pixmap( XPM );
  QCursor cursor( pixmap, hotspotX, hotspotY ); // no test for validity in QPixmap(xpm)!
  QApplication::setOverrideCursor( cursor );
}

void QmitkApplicationCursor::PopCursor()
{
  QApplication::restoreOverrideCursor();
}

