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

#include <MitkQtWidgetsExports.h>
#include "mitkApplicationCursor.h"

/*!
  \ingroup QmitkModule
  \brief Qt specific implementation of ApplicationCursorImplementation

  This class very simply calls the QApplication's methods setOverrideCursor() and
  restoreOverrideCursor().
*/
class MITKQTWIDGETS_EXPORT QmitkApplicationCursor : public mitk::ApplicationCursorImplementation
{
  public:

    // Will be instantiated automatically from QmitkApplicationCursor.cpp once
    QmitkApplicationCursor();

    virtual void PushCursor(const char* XPM[], int hotspotX, int hotspotY) override;
    virtual void PushCursor(std::istream&, int hotspotX, int hotspotY) override;
    virtual void PopCursor() override;
    virtual const mitk::Point2I GetCursorPosition() override;
    virtual void SetCursorPosition(const mitk::Point2I&) override;


  protected:
  private:
};

#endif

