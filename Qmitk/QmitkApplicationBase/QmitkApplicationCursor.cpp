#include "QmitkApplicationCursor.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qpixmap.h>

#include <iostream>

QmitkApplicationCursor globalQmitkApplicationCursor; // create one instance

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

