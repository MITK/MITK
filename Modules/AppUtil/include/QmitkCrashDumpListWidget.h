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

// Forward-declared only: AUTOMOC processes this header in every build,
// including those without MitkCrashHandling.
namespace mitk
{
  struct CrashDumpInfo;
}

/**
 * \brief Table of crash dumps with kind, time, size, version, log and status.
 *
 * Status is "new" for a dump that has not triggered the next-start dialog
 * yet and "kept" otherwise. The log column says whether the session log
 * recorded with the dump still exists; it is evaluated by SetDumps().
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

signals:
  void SelectionChanged();

private:
  QTreeWidget* m_Tree;
  std::vector<mitk::CrashDumpInfo> m_Dumps;
};

#endif
