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

#ifndef QMITK_APPLICATION_CURSOR_H_INCLUDED
#define QMITK_APPLICATION_CURSOR_H_INCLUDED

#include <QmitkExports.h>
#include "mitkApplicationCursor.h"

/*!
  \brief Qt specific implementation of ApplicationCursorImplementation

  This class very simply calls the QApplication's methods setOverrideCursor() and
  restoreOverrideCursor().
*/
class QMITK_EXPORT QmitkApplicationCursor : public mitk::ApplicationCursorImplementation
{
  public:

    // Will be instantiated automatically from QmitkApplicationCursor.cpp once
    QmitkApplicationCursor();

    virtual void PushCursor(const char* XPM[], int hotspotX, int hotspotY);
    virtual void PopCursor();
    virtual const mitk::Point2I GetCursorPosition();
    virtual void SetCursorPosition(const mitk::Point2I&);


  protected:
  private:
};

#endif

