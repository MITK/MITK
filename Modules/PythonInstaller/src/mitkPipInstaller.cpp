/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPipInstaller.h>

#include <mitkLogMacros.h>
#include <mitkPythonHelper.h>

#include <nlohmann/json.hpp>

#include <QFile>

namespace
{
  QString toQ(const std::string& s) { return QString::fromStdString(s); }
}

mitk::PipInstaller::PipInstaller(QObject* parent)
  : QObject(parent),
    m_Process(new QProcess(this))
{
  connect(m_Process, &QProcess::readyReadStandardOutput, this, &PipInstaller::OnStandardOutputReady);
  connect(m_Process, &QProcess::readyReadStandardError, this, &PipInstaller::OnStandardErrorReady);
  connect(m_Process, &QProcess::finished, this, &PipInstaller::OnProcessFinished);
}

mitk::PipInstaller::~PipInstaller()
{
  if (m_Process->state() != QProcess::NotRunning)
    m_Process->kill();
}

void mitk::PipInstaller::SetInstallSpec(const PipInstallSpec& spec)
{
  m_Spec = spec;
}

void mitk::PipInstaller::StartResolve()
{
  m_AutoInstall = false;
  m_ResolvedPackages.clear();
  m_CurrentGroup = 0;
  m_CurrentPackage = 0;
  m_GroupStartIndex = 0;
  m_AnyFailed = false;

  if (m_Spec.upgradePipFirst)
  {
    StartPipUpgrade();
  }
  else
  {
    emit resolveStarted();
    StartResolveGroup();
  }
}

void mitk::PipInstaller::StartInstall()
{
  if (m_ResolvedPackages.empty())
  {
    emit installFinished(true);
    return;
  }

  m_State = State::Installing;
  m_CurrentPackage = 0;
  m_AnyFailed = false;

  emit progressChanged(0, static_cast<int>(m_ResolvedPackages.size()));
  StartInstallPackage();
}

void mitk::PipInstaller::StartResolveAndInstall()
{
  m_AutoInstall = true;
  m_ResolvedPackages.clear();
  m_CurrentGroup = 0;
  m_CurrentPackage = 0;
  m_GroupStartIndex = 0;
  m_AnyFailed = false;

  if (m_Spec.upgradePipFirst)
  {
    StartPipUpgrade();
  }
  else
  {
    emit resolveStarted();
    StartResolveGroup();
  }
}

void mitk::PipInstaller::Cancel()
{
  if (m_Process->state() != QProcess::NotRunning)
    m_Process->kill();

  auto previousState = m_State;
  m_State = State::Failed;

  if (previousState == State::Resolving)
    emit resolveFinished(false, {});
  else if (previousState == State::Installing)
    emit installFinished(false);
}

bool mitk::PipInstaller::IsRunning() const
{
  return m_State != State::Idle && m_State != State::Done && m_State != State::Failed;
}

std::vector<mitk::PipPackageInfo> mitk::PipInstaller::ResolvedPackages() const
{
  return m_ResolvedPackages;
}

// --- Private slots ---

void mitk::PipInstaller::OnStandardOutputReady()
{
  emit outputReceived(QString::fromLocal8Bit(m_Process->readAllStandardOutput()), false);
}

void mitk::PipInstaller::OnStandardErrorReady()
{
  emit outputReceived(QString::fromLocal8Bit(m_Process->readAllStandardError()), true);
}

void mitk::PipInstaller::OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  bool success = exitStatus == QProcess::NormalExit && exitCode == 0;
  auto numPackages = static_cast<int>(m_ResolvedPackages.size());
  auto numGroups = static_cast<int>(m_Spec.groups.size());

  switch (m_State)
  {
  case State::UpgradingPip:
  {
    emit pipUpgradeFinished(success);

    // pip upgrade failure is non-fatal - proceed to resolve first group.
    m_State = State::Resolving;
    emit resolveStarted();
    StartResolveGroup();
    break;
  }

  case State::Resolving:
  {
    if (!success)
    {
      m_State = State::Failed;
      emit resolveFinished(false, {});
      return;
    }

    if (!ParseResolveReport(m_ReportFile.fileName(), m_CurrentGroup))
    {
      m_State = State::Failed;
      emit resolveFinished(false, {});
      return;
    }

    numPackages = static_cast<int>(m_ResolvedPackages.size());

    // Emit the (growing) package list so the UI can update.
    emit resolveFinished(true, m_ResolvedPackages);

    if (m_AutoInstall)
    {
      // Install this group's packages before resolving the next group.
      // This ensures that when the next group is resolved, pip sees
      // the current group's packages as already installed (e.g. torch
      // from a CUDA index won't be re-resolved from PyPI).
      if (m_GroupStartIndex < numPackages)
      {
        m_State = State::Installing;
        m_CurrentPackage = m_GroupStartIndex;
        emit progressChanged(m_GroupStartIndex, numPackages);
        StartInstallPackage();
      }
      else
      {
        // No new packages in this group. Advance to next group.
        m_CurrentGroup++;
        m_GroupStartIndex = numPackages;

        if (m_CurrentGroup < numGroups)
          StartResolveGroup();
        else
          emit installFinished(!m_AnyFailed);
      }
    }
    else
    {
      // Resolve-only mode: resolve all groups sequentially, then stop.
      m_CurrentGroup++;

      if (m_CurrentGroup < numGroups)
      {
        m_GroupStartIndex = numPackages;
        StartResolveGroup();
      }
      else
      {
        m_State = State::Done;
      }
    }
    break;
  }

  case State::Installing:
  {
    auto status = success ? PackageStatus::Installed : PackageStatus::Failed;
    emit packageStatusChanged(m_CurrentPackage, toQ(m_ResolvedPackages[m_CurrentPackage].name), status);

    if (!success)
      m_AnyFailed = true;

    m_CurrentPackage++;
    emit progressChanged(m_CurrentPackage, numPackages);

    if (m_CurrentPackage < numPackages)
    {
      StartInstallPackage();
    }
    else
    {
      // All packages in the current group are installed.
      // Advance to the next group's resolve phase.
      m_CurrentGroup++;

      if (m_CurrentGroup < numGroups)
      {
        m_State = State::Resolving;
        m_GroupStartIndex = numPackages;
        emit resolveStarted();
        StartResolveGroup();
      }
      else
      {
        m_State = m_AnyFailed ? State::Failed : State::Done;
        emit installFinished(!m_AnyFailed);
      }
    }
    break;
  }

  default:
    break;
  }
}

// --- Private helpers ---

void mitk::PipInstaller::StartPipUpgrade()
{
  auto python = PythonExecutable();

  if (python.isEmpty())
  {
    emit errorOccurred("Python executable not found.");
    return;
  }

  m_State = State::UpgradingPip;
  emit pipUpgradeStarted();

  m_Process->start(python, { "-m", "pip", "install", "--upgrade", "pip" });
}

void mitk::PipInstaller::StartResolveGroup()
{
  auto python = PythonExecutable();

  if (python.isEmpty())
  {
    m_State = State::Failed;
    emit errorOccurred("Python executable not found.");
    emit resolveFinished(false, {});
    return;
  }

  if (m_CurrentGroup >= static_cast<int>(m_Spec.groups.size()))
  {
    m_State = m_AnyFailed ? State::Failed : State::Done;
    emit installFinished(!m_AnyFailed);
    return;
  }

  m_GroupStartIndex = static_cast<int>(m_ResolvedPackages.size());
  const auto& group = m_Spec.groups[m_CurrentGroup];

  // Create a temporary file for the pip report.
  m_ReportFile.close();

  if (!m_ReportFile.open())
  {
    m_State = State::Failed;
    emit errorOccurred("Could not create temporary file for pip report.");
    emit resolveFinished(false, {});
    return;
  }

  // Close immediately so pip can write to it. Keep the file name.
  auto reportPath = m_ReportFile.fileName();
  m_ReportFile.close();

  QStringList args = { "-m", "pip", "install", "--dry-run", "--report", reportPath };
  args = BuildPipArgs(args, group);

  for (const auto& req : group.requirements)
    args.append(toQ(req));

  m_State = State::Resolving;
  m_Process->start(python, args);
}

void mitk::PipInstaller::StartInstallPackage()
{
  auto python = PythonExecutable();

  if (python.isEmpty())
  {
    m_State = State::Failed;
    emit errorOccurred("Python executable not found.");
    emit installFinished(false);
    return;
  }

  if (m_CurrentPackage >= static_cast<int>(m_ResolvedPackages.size()))
  {
    m_State = m_AnyFailed ? State::Failed : State::Done;
    emit installFinished(!m_AnyFailed);
    return;
  }

  const auto& pkg = m_ResolvedPackages[m_CurrentPackage];
  emit packageStatusChanged(m_CurrentPackage, toQ(pkg.name), PackageStatus::Installing);

  const auto& group = m_Spec.groups[pkg.group];

  QStringList args = { "-m", "pip", "install", "--no-deps",
                        toQ(pkg.name) + "==" + toQ(pkg.version) };
  args = BuildPipArgs(args, group);

  m_Process->start(python, args);
}

QStringList mitk::PipInstaller::BuildPipArgs(const QStringList& baseArgs, const PipInstallGroup& group) const
{
  QStringList args = baseArgs;

  if (!group.indexUrl.empty())
    args.append({ "--index-url", toQ(group.indexUrl) });

  for (const auto& arg : group.extraPipArgs)
    args.append(toQ(arg));

  return args;
}

bool mitk::PipInstaller::ParseResolveReport(const QString& reportPath, int groupIndex)
{
  QFile file(reportPath);

  if (!file.open(QIODevice::ReadOnly))
  {
    MITK_ERROR << "PipInstaller: Could not open report file: " << reportPath.toStdString();
    emit errorOccurred("Could not open pip resolve report.");
    return false;
  }

  auto data = file.readAll();
  file.close();

  try
  {
    auto report = nlohmann::json::parse(data.constData(), data.constData() + data.size());

    if (!report.contains("install") || !report["install"].is_array())
    {
      MITK_ERROR << "PipInstaller: Report file has no 'install' array.";
      emit errorOccurred("Unexpected pip report format.");
      return false;
    }

    for (const auto& entry : report["install"])
    {
      PipPackageInfo info;

      if (entry.contains("metadata"))
      {
        const auto& metadata = entry["metadata"];

        if (metadata.contains("name"))
          info.name = metadata["name"].get<std::string>();

        if (metadata.contains("version"))
          info.version = metadata["version"].get<std::string>();
      }

      info.requested = entry.value("requested", false);
      info.group = groupIndex;

      // Try to find the original specifier in the group's requirements.
      const auto& group = m_Spec.groups[groupIndex];

      for (const auto& req : group.requirements)
      {
        if (req.size() >= info.name.size() &&
            std::equal(info.name.begin(), info.name.end(), req.begin(),
                       [](char a, char b) { return std::tolower(a) == std::tolower(b); }) &&
            (req.size() == info.name.size() || !std::isalnum(static_cast<unsigned char>(req[info.name.size()]))))
        {
          info.specifier = req;
          break;
        }
      }

      m_ResolvedPackages.push_back(info);
    }
  }
  catch (const nlohmann::json::exception& e)
  {
    MITK_ERROR << "PipInstaller: Failed to parse report JSON: " << e.what();
    emit errorOccurred(QString("Failed to parse pip report: %1").arg(e.what()));
    return false;
  }

  return true;
}

QString mitk::PipInstaller::PythonExecutable() const
{
  auto path = mitk::PythonHelper::GetExecutablePath();

  if (path.empty())
    return {};

  return QString::fromStdString(path.string());
}
