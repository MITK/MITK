/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFeedbackInput_h
#define QmitkFeedbackInput_h

#include "QmitkFeedbackDialog.h"

#include <filesystem>
#include <string>

class QWidget;

/** \brief The main widget of the active workbench window, or null. */
QWidget* QmitkGetActiveWorkbenchWindowWidget();

/** \brief Everything a general report offers: the environment, the tail of
 *  this session's log, and screenshots of the active workbench window and
 *  of its editor area. Must run before the dialog is shown, so that the
 *  screenshots do not show it. */
QmitkFeedbackDialog::Input QmitkCollectFeedbackInput();

/** \brief The tail of the log file at \p path as a log source, starting
 *  with a line that says how much of the log of \p session it holds.
 *  Unavailable, with the reason, if the file cannot be read. */
QmitkFeedbackDialog::LogSource QmitkReadFeedbackLog(const std::filesystem::path& path, const std::string& archiveName,
  const std::string& sourceLabel, const std::string& session);

/** \brief Run the feedback dialog for \p input modally, then delete the
 *  crash dumps the user asked to delete, telling the user about any that
 *  could not be deleted. \p parent may be null, in which case the active
 *  workbench window is used. */
void QmitkRunFeedbackDialog(const QmitkFeedbackDialog::Input& input, QWidget* parent);

#ifdef MITK_HAS_CRASHHANDLING
/** \brief QmitkCollectFeedbackInput() plus \p dumps and, for each dump with
 *  a kept session log, the tail of that log. */
QmitkFeedbackDialog::Input QmitkCollectCrashReportInput(const std::vector<mitk::CrashDumpInfo>& dumps);

/** \brief When \p dump was written, as a report shows it to readers in any
 *  locale. */
QString QmitkFeedbackDumpTime(const mitk::CrashDumpInfo& dump);
#endif

#endif
