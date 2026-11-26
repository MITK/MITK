/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFeedback_h
#define QmitkFeedback_h

#include <org_mitk_gui_qt_ext_Export.h>

class QWidget;

/** \brief Entry point of the in-application feedback and problem report. */
namespace QmitkFeedback
{
  /** \brief Run the feedback dialog modally and return when it has closed.
   *
   *  The user reviews everything the report contains and hands it off
   *  through their browser, mail program, or file manager; the application
   *  itself sends nothing. \p parent may be null, in which case the active
   *  workbench window is used. */
  MITK_QT_COMMON_EXT_EXPORT void OpenDialog(QWidget* parent = nullptr);
}

#endif
