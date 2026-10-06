/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrashDumpManagerDialog_h
#define QmitkCrashDumpManagerDialog_h

#include <MitkAppUtilExports.h>

#include <QDialog>

#include <vector>

class QLabel;
class QPushButton;
class QmitkCrashDumpListWidget;

namespace mitk
{
  struct CrashDumpInfo;
}

/**
 * \brief Non-modal manager for all diagnostic data (crash dumps and snapshots).
 *
 * Deleting is immediate, which is why this is a dialog of its own rather
 * than part of the transactional (OK/Cancel) preferences. This session's
 * provisional watchdog snapshots are not listed; they are deleted
 * automatically.
 */
class MITKAPPUTIL_EXPORT QmitkCrashDumpManagerDialog : public QDialog
{
  Q_OBJECT

public:
  /** \brief Show the application-wide instance, creating it on first use and
   *  raising it otherwise. \p parent is used only when no modal dialog is
   *  active; otherwise the manager is parented to the modal dialog, so that
   *  it can be used from it (e.g. from the preferences). An already open
   *  manager returns to its previous parent when that modal dialog finishes. */
  static void ShowManager(QWidget* parent = nullptr);

  /** \brief Re-read the dumps from disk. */
  void Refresh();

private:
  explicit QmitkCrashDumpManagerDialog(QWidget* parent);

  void OnSelectionChanged();
  void OnFileReport();
  void OnShowInFolder();
  void OnShowLog();
  void OnCopyPath();
  void OnDelete();
  void OnDeleteAll();

  void DeleteDumps(const std::vector<mitk::CrashDumpInfo>& dumps, const QString& question);

  QmitkCrashDumpListWidget* m_List;
  QLabel* m_StateLabel;
  QPushButton* m_FileReportButton;
  QPushButton* m_ShowInFolderButton;
  QPushButton* m_ShowLogButton;
  QPushButton* m_CopyPathButton;
  QPushButton* m_DeleteButton;
  QPushButton* m_DeleteAllButton;
};

#endif
