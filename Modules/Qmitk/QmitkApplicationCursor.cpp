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

#include "QmitkApplicationCursor.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qpixmap.h>

#include <iostream>

// us
#include "mitkModuleResource.h"
#include "mitkModuleResourceStream.h"

QmitkApplicationCursor::QmitkApplicationCursor()
{
  mitk::ApplicationCursor::RegisterImplementation(this);
}

void QmitkApplicationCursor::PushCursor(const mitk::ModuleResource resource, int hotspotX, int hotspotY)
{
  if (resource.IsValid())
  {
    mitk::ModuleResourceStream resourceStream(resource, std::ios::binary);
    resourceStream.seekg(0, std::ios::end);
    std::ios::pos_type length = resourceStream.tellg();
    resourceStream.seekg(0, std::ios::beg);

    char* data = new char[length];
    resourceStream.read(data, length);
    QPixmap pixmap;
    pixmap.loadFromData(QByteArray::fromRawData(data, length));
    QCursor cursor( pixmap, hotspotX, hotspotY ); // no test for validity in QPixmap(xpm)!
    QApplication::setOverrideCursor( cursor );
    delete[] data;

  }
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

const mitk::Point2I QmitkApplicationCursor::GetCursorPosition()
{
  mitk::Point2I mp;
  QPoint qp = QCursor::pos();
  mp[0] = qp.x();
  mp[1] = qp.y();
  return mp;
}

void QmitkApplicationCursor::SetCursorPosition(const mitk::Point2I& p)
{
  static bool selfCall = false;
  if (selfCall) return;  // this is to avoid recursive calls
  selfCall = true;
  QCursor::setPos( p[0], p[1] );
  selfCall = false;
}


