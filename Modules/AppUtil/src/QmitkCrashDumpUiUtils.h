/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrashDumpUiUtils_h
#define QmitkCrashDumpUiUtils_h

#include <mitkCrashDumpFacility.h>

#include <QString>

#include <filesystem>
#include <vector>

class QWidget;

/** Wording and actions shared by the crash-dump dialogs. */
namespace QmitkCrashDumpUi
{
  QString ToQString(const std::filesystem::path& path);

  QString KindLabel(mitk::DumpKind kind);

  /** Why dumps must be handled like patient data. */
  QString PrivacyNote();

  /** Opens each distinct folder containing one of \p files. */
  void ShowInFolders(const std::vector<std::filesystem::path>& files);

  /** Hands \p dumps to the registered report service, if there is one.
   *  Returns whether a service took them. */
  bool FileReport(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent);
}

#endif
