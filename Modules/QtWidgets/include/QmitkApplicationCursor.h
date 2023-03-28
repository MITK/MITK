/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkApplicationCursor_h
#define QmitkApplicationCursor_h

#include "mitkApplicationCursor.h"
#include <MitkQtWidgetsExports.h>

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

  void PushCursor(const char *XPM[], int hotspotX, int hotspotY) override;
  void PushCursor(std::istream &, int hotspotX, int hotspotY) override;
  void PopCursor() override;
  const mitk::Point2I GetCursorPosition() override;
  void SetCursorPosition(const mitk::Point2I &) override;

protected:
private:
};

#endif
