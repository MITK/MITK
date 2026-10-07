/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrashDumpDialog_h
#define QmitkCrashDumpDialog_h

#include <MitkAppUtilExports.h>

#include <QDialog>

#include <vector>

namespace mitk
{
  struct CrashDumpInfo;
}

/**
 * \brief Next-start dialog for crash dumps left behind by a previous run.
 *
 * Shown when the crash-dump database contains dumps the user has not seen
 * yet. Disposition is keep-only-if-explicit: the dumps are deleted unless
 * the user actively keeps them or files a report with them (offered only
 * while an mitk::ICrashReportService is registered). Closing the dialog
 * counts as removing. Either way the dumps are acknowledged and never
 * surface again; kept ones remain available in QmitkCrashDumpManagerDialog.
 */
class MITKAPPUTIL_EXPORT QmitkCrashDumpDialog : public QDialog
{
  Q_OBJECT

public:
  /** \brief Show the dialog if the previous run left unacknowledged dumps.
   *  Logs a warning instead when a crash is indicated but no dump exists.
   *  Also records this session's log file with the crash-dump facility, so
   *  call it once the log is open. No-op when the crash-dump facility was
   *  never initialized. */
  static void ShowIfCrashedLastRun(QWidget* parent = nullptr);

private:
  enum Choice
  {
    Remove,
    Keep,
    FileReport
  };

  QmitkCrashDumpDialog(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent);

  Choice m_Choice = Remove;
};

#endif
