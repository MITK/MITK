/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkApplicationCursor_h
#define QmitkApplicationCursor_h

#include <mitkApplicationCursor.h>
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
  /** \brief Constructs and registers this cursor implementation. */
  QmitkApplicationCursor();

  /** \copydoc mitk::ApplicationCursorImplementation::PushCursor(const char*[], int, int) */
  void PushCursor(const char *XPM[], int hotspotX, int hotspotY) override;
  /** \brief Push a cursor from an input stream onto the cursor stack. */
  void PushCursor(std::istream &, int hotspotX, int hotspotY) override;
  /** \brief Pop the most recently pushed cursor from the cursor stack. */
  void PopCursor() override;
  /** \brief Get the current cursor position in display coordinates. */
  const mitk::Point2I GetCursorPosition() override;
  /** \brief Set the cursor position in display coordinates. */
  void SetCursorPosition(const mitk::Point2I &) override;

protected:
private:
};

#endif
