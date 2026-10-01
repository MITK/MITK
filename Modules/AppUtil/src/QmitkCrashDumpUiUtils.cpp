/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCrashDumpUiUtils.h"

#include <mitkICrashReportService.h>

#include <QDesktopServices>
#include <QUrl>

#include <set>

QString QmitkCrashDumpUi::ToQString(const std::filesystem::path& path)
{
  return QString::fromStdWString(path.wstring());
}

QString QmitkCrashDumpUi::KindLabel(mitk::DumpKind kind)
{
  switch (kind)
  {
    case mitk::DumpKind::UnresponsiveTerminated:
      return "Unresponsive (terminated)";
    case mitk::DumpKind::OnDemand:
      return "Captured on request";
    case mitk::DumpKind::Crash:
    default:
      return "Crash";
  }
}

QString QmitkCrashDumpUi::PrivacyNote()
{
  return "A crash dump or diagnostic snapshot contains parts of the application's memory from the "
         "session it was taken in. If at any time during that session you opened, browsed or queried "
         "data about real people that was not fully anonymized (including in the DICOM browser or a "
         "PACS query), it may contain such data and must be handled through your usual process for "
         "patient or study-participant data. Other rules of your organisation may also restrict "
         "sharing it, for example for confidential or unpublished data. Please check before you pass "
         "a dump on. MITK never uploads crash dumps; they stay on this computer.";
}

void QmitkCrashDumpUi::ShowInFolders(const std::vector<std::filesystem::path>& files)
{
  // Dumps of different kinds are filed in different folders, so a single
  // folder need not cover them all.
  std::set<std::filesystem::path> folders;

  for (const auto& file : files)
    folders.insert(file.parent_path());

  for (const auto& folder : folders)
  {
    std::error_code error;
    if (std::filesystem::is_directory(folder, error))
      QDesktopServices::openUrl(QUrl::fromLocalFile(ToQString(folder)));
  }
}

bool QmitkCrashDumpUi::FileReport(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent)
{
  auto* service = mitk::GetCrashReportService();

  if (service == nullptr || dumps.empty())
    return false;

  service->FileReport(dumps, parent);
  return true;
}
