/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrashDumpListWidget_h
#define QmitkCrashDumpListWidget_h

#include <MitkAppUtilExports.h>

#include <QWidget>

#include <filesystem>
#include <set>
#include <vector>

class QTreeWidget;

namespace mitk
{
  struct CrashDumpInfo;
}

/**
 * \brief Table of crash dumps with kind, time, size, version, log and status.
 *
 * Status is "provisional (this session)" for the dumps passed to SetDumps()
 * in provisionalPaths, "new" for a dump that has not triggered the
 * next-start dialog yet, and "kept" otherwise. The log column reflects
 * mitk::CrashDumpInfo::SessionLog as the facility's listings filled it in:
 * "available" when a log copy is kept with the dump, "not available" when the
 * dump has run info but no log copy, and "unknown" when it has no run info.
 */
class MITKAPPUTIL_EXPORT QmitkCrashDumpListWidget : public QWidget
{
  Q_OBJECT

public:
  enum Column
  {
    KindColumn,
    TimeColumn,
    SizeColumn,
    VersionColumn,
    LogColumn,
    StatusColumn
  };

  explicit QmitkCrashDumpListWidget(QWidget* parent = nullptr);
  ~QmitkCrashDumpListWidget() override;

  /** \brief Show \p dumps in the given order. Dumps listed in
   *  \p provisionalPaths are this session's provisional snapshots and are
   *  marked as such instead of new or kept. Clears the selection. */
  void SetDumps(const std::vector<mitk::CrashDumpInfo>& dumps,
    const std::set<std::filesystem::path>& provisionalPaths = {});

  std::vector<mitk::CrashDumpInfo> GetDumps() const;
  std::vector<mitk::CrashDumpInfo> GetSelectedDumps() const;

  /** \brief Multi-selection (default) or a read-only list. */
  void SetSelectionEnabled(bool enabled);

  /** \brief Size the list to the rows and columns currently shown, without
   *  scroll bars, for a short list that should neither take up space it does
   *  not need nor hide columns. */
  void FitToContents();

signals:
  void SelectionChanged();

private:
  QTreeWidget* m_Tree;
  std::vector<mitk::CrashDumpInfo> m_Dumps;
};

#endif
