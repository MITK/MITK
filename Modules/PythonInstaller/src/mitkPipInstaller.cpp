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
#include <QRegularExpression>

namespace
{
  // Quote arguments containing whitespace so the logged command line is
  // copy-pasteable into a shell for reproduction.
  QString FormatCommand(const QString& program, const QStringList& args)
  {
    auto quote = [](const QString& token) {
      return token.contains(QChar::Space) ? '"' + token + '"' : token;
    };

    QString result = quote(program);
    for (const auto& arg : args)
    {
      result += ' ';
      result += quote(arg);
    }
    // Redact credentials from URLs: "://user:password@" -> "://****:****@"
    static QRegularExpression credentialPattern("://[^/@]+:[^/@]+@");
    result.replace(credentialPattern, "://****:****@");

    return result;
  }

  // Canonicalize a package name per PEP 503: lowercase, and treat runs of
  // '-', '_', '.' as a single '-'. Used to match resolved package names
  // against the original requirement specifiers.
  std::string CanonicalizePackageName(const std::string& name)
  {
    std::string result;
    result.reserve(name.size());

    for (char c : name)
    {
      if (c == '_' || c == '.' || c == '-')
      {
        if (!result.empty() && result.back() != '-')
          result.push_back('-');
      }
      else
      {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
      }
    }

    while (!result.empty() && result.back() == '-')
      result.pop_back();

    return result;
  }

  // Extract the package-name prefix of a requirement specifier.
  // Stops at the first character that is not part of a PEP 508 distribution name.
  std::string ExtractPackageName(const std::string& requirement)
  {
    std::size_t end = 0;
    while (end < requirement.size())
    {
      char c = requirement[end];
      if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.' || c == '-')
        ++end;
      else
        break;
    }
    return requirement.substr(0, end);
  }

  // Check whether a requirement is a direct reference (VCS URL or PEP 508
  // "name @ url" form) rather than a regular PyPI specifier.
  bool IsDirectReference(const std::string& req)
  {
    // Bare VCS URL: git+https://..., hg+https://..., etc.
    if (req.size() > 4 &&
        (req.compare(0, 4, "git+") == 0 || req.compare(0, 3, "hg+") == 0 ||
         req.compare(0, 4, "svn+") == 0 || req.compare(0, 4, "bzr+") == 0))
      return true;

    // PEP 508 direct reference: "name @ https://..." or "name @ git+https://..."
    auto pos = req.find(" @ ");
    return pos != std::string::npos && pos + 3 < req.size();
  }

  // Format a std::string as a Python single-quoted string literal.
  // Escapes backslashes and single quotes; everything else passes through.
  QString PyQuote(const std::string& value)
  {
    QString result = "'";
    for (char c : value)
    {
      if (c == '\\' || c == '\'')
        result.append('\\');
      result.append(QChar::fromLatin1(c));
    }
    result.append('\'');
    return result;
  }

  // Format a vector<string> as a Python list literal, e.g.
  // {"a", "b"} -> "['a', 'b']". An empty vector returns "None" so the
  // caller can pass "allow_patterns=None" and get the whole-repo default.
  QString PyListLiteralOrNone(const std::vector<std::string>& values)
  {
    if (values.empty())
      return "None";

    QString result = "[";
    for (std::size_t i = 0; i < values.size(); ++i)
    {
      if (i > 0)
        result.append(", ");
      result.append(PyQuote(values[i]));
    }
    result.append(']');
    return result;
  }
}

mitk::PipInstaller::PipInstaller(QObject* parent)
  : QObject(parent),
    m_Process(new QProcess(this))
{
  connect(m_Process, &QProcess::readyReadStandardOutput, this, &PipInstaller::OnStandardOutputReady);
  connect(m_Process, &QProcess::readyReadStandardError, this, &PipInstaller::OnStandardErrorReady);
  connect(m_Process, &QProcess::finished, this, &PipInstaller::OnProcessFinished);
  connect(m_Process, &QProcess::errorOccurred, this, [](QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart)
      MITK_ERROR << "Failed to start pip subprocess.";
  });
}

mitk::PipInstaller::~PipInstaller()
{
  if (m_Process->state() != QProcess::NotRunning)
    m_Process->kill();
}

void mitk::PipInstaller::SetInstallSpec(const PipInstallSpec& spec)
{
  if (this->IsRunning())
  {
    MITK_WARN << "Cannot change install spec while an operation is running.";
    return;
  }

  m_Spec = spec;
}

void mitk::PipInstaller::StartInstall()
{
  if (this->IsRunning())
  {
    MITK_WARN << "PipInstaller is already running.";
    return;
  }

  m_ResolvedPackages.clear();
  m_CurrentGroup = 0;
  m_CurrentPackage = 0;
  m_GroupStartIndex = 0;
  m_CurrentDownload = 0;
  m_AnyFailed = false;

  // m_CreatedVirtualEnv is intentionally not reset here. If a prior attempt
  // created the venv, a retry reuses it (BeginVirtualEnvPhase sees it exists
  // and skips creation), so cleanup responsibility must survive the retry.
  // The flag is cleared only in RemoveCreatedVirtualEnv after the venv has
  // actually been removed.

  BeginVirtualEnvPhase();
}

void mitk::PipInstaller::AbandonInstall()
{
  // Only act after a failed install. After a successful install the venv
  // belongs to the consumer; while an operation is in progress the consumer
  // should call Cancel() instead.
  if (m_State != State::Failed)
    return;

  this->RemoveCreatedVirtualEnv();
}

void mitk::PipInstaller::Cancel()
{
  if (m_State == State::Idle || m_State == State::Done ||
      m_State == State::Failed || m_State == State::Cancelling)
    return;

  m_State = State::Cancelling;

  if (m_Process->state() != QProcess::NotRunning)
  {
    // Async path: kill the process and let OnProcessFinished call FinalizeCancel.
    m_Process->kill();
  }
  else
  {
    // No process running (we were in a synchronous transition between phases).
    // Finalize immediately.
    FinalizeCancel();
  }
}

bool mitk::PipInstaller::IsRunning() const
{
  return m_State != State::Idle && m_State != State::Done && m_State != State::Failed;
}

const std::vector<mitk::PipPackageInfo>& mitk::PipInstaller::GetResolvedPackages() const
{
  return m_ResolvedPackages;
}

void mitk::PipInstaller::OnStandardOutputReady()
{
  auto output = QString::fromLocal8Bit(m_Process->readAllStandardOutput());
  emit OutputReceived(output, false);
}

void mitk::PipInstaller::OnStandardErrorReady()
{
  auto output = QString::fromLocal8Bit(m_Process->readAllStandardError());
  emit OutputReceived(output, true);
}

void mitk::PipInstaller::OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  // Cancellation: the process was killed by Cancel(). Run cleanup and emit the
  // terminal signal here, off the original Cancel() call stack, so the UI
  // thread is not blocked while we tear down the venv.
  if (m_State == State::Cancelling)
  {
    FinalizeCancel();
    return;
  }

  bool success = exitStatus == QProcess::NormalExit && exitCode == 0;
  auto numPackages = static_cast<int>(m_ResolvedPackages.size());

  switch (m_State)
  {
  case State::CreatingVirtualEnv:
  {
    if (!success)
    {
      m_State = State::Failed;
      emit VirtualEnvCreationFinished(false);
      emit ErrorOccurred("Failed to create virtual environment.");
      emit InstallFinished(false);
      return;
    }

    PythonHelper::ActivateVirtualEnv(m_Spec.venvName);
    emit VirtualEnvCreationFinished(true);

    if (m_Spec.upgradePipFirst)
    {
      StartPipUpgrade();
    }
    else
    {
      emit ResolveStarted();
      StartResolveGroup();
    }
    break;
  }

  case State::UpgradingPip:
  {
    emit PipUpgradeFinished(success);

    // pip upgrade failure is non-fatal - proceed to resolve first group.
    m_State = State::Resolving;
    emit ResolveStarted();
    StartResolveGroup();
    break;
  }

  case State::Resolving:
  {
    if (!success)
    {
      m_State = State::Failed;
      emit ResolveFinished(false, {});
      return;
    }

    if (!ParseResolveReport(m_ReportFile.fileName(), m_CurrentGroup))
    {
      m_State = State::Failed;
      emit ResolveFinished(false, {});
      return;
    }

    numPackages = static_cast<int>(m_ResolvedPackages.size());

    // Per-group success notification with the list as it grows.
    emit ResolveFinished(true, m_ResolvedPackages);

    // Install this group's packages before resolving the next group.
    // This ensures that when the next group is resolved, pip sees
    // the current group's packages as already installed (e.g. torch
    // from a CUDA index won't be re-resolved from PyPI).
    if (m_GroupStartIndex < numPackages)
    {
      m_State = State::Installing;
      m_CurrentPackage = m_GroupStartIndex;
      emit ProgressChanged(m_GroupStartIndex, numPackages);
      StartInstallPackage();
    }
    else
    {
      // No new packages in this group. Advance to next group.
      AdvanceToNextGroup();
    }
    break;
  }

  case State::Installing:
  {
    auto status = success ? PackageStatus::Installed : PackageStatus::Failed;
    emit PackageStatusChanged(m_CurrentPackage, QString::fromStdString(m_ResolvedPackages[m_CurrentPackage].name), status);

    if (!success)
      m_AnyFailed = true;

    m_CurrentPackage++;
    emit ProgressChanged(m_CurrentPackage, numPackages);

    if (m_CurrentPackage < numPackages)
    {
      StartInstallPackage();
    }
    else
    {
      // All packages in the current group are installed.
      AdvanceToNextGroup();
    }
    break;
  }

  case State::DownloadingModels:
  {
    const auto& download = m_Spec.huggingFaceDownloads[m_CurrentDownload];
    auto displayName = QString::fromStdString(
      download.displayName.empty() ? download.repoId : download.displayName);

    emit ModelDownloadFinished(displayName, success);

    if (!success && !download.optional)
      m_AnyFailed = true;

    m_CurrentDownload++;

    if (m_CurrentDownload < static_cast<int>(m_Spec.huggingFaceDownloads.size()))
    {
      StartModelDownload();
    }
    else
    {
      m_State = m_AnyFailed ? State::Failed : State::Done;
      emit InstallFinished(!m_AnyFailed);
    }
    break;
  }

  default:
    break;
  }
}

// Either kicks off a `python -m venv <path>` subprocess (asynchronous) or, if
// the venv is unnecessary or already exists, activates it and dispatches
// synchronously to the next phase.
void mitk::PipInstaller::BeginVirtualEnvPhase()
{
  if (m_Spec.venvName.empty() || PythonHelper::VirtualEnvExists(m_Spec.venvName))
  {
    // No venv needed or already exists - just activate and move on.
    if (!m_Spec.venvName.empty())
      PythonHelper::ActivateVirtualEnv(m_Spec.venvName);

    if (m_Spec.upgradePipFirst)
    {
      StartPipUpgrade();
    }
    else
    {
      emit ResolveStarted();
      StartResolveGroup();
    }
    return;
  }

  auto python = RequirePythonExecutable();

  if (python.isEmpty())
    return;

  m_State = State::CreatingVirtualEnv;
  m_CreatedVirtualEnv = true;
  emit VirtualEnvCreationStarted();

  auto venvPath = PythonHelper::GetVirtualEnvPath(m_Spec.venvName);
  QStringList args = { "-m", "venv", QString::fromStdString(venvPath.string()) };
  MITK_INFO << FormatCommand(python, args).toStdString();
  m_Process->start(python, args);
}

void mitk::PipInstaller::StartPipUpgrade()
{
  auto python = RequirePythonExecutable();

  if (python.isEmpty())
    return;

  m_State = State::UpgradingPip;
  emit PipUpgradeStarted();

  QStringList args = { "-m", "pip", "install", "--upgrade", "pip" };
  MITK_INFO << FormatCommand(python, args).toStdString();
  m_Process->start(python, args);
}

void mitk::PipInstaller::StartResolveGroup()
{
  auto python = RequirePythonExecutable();

  if (python.isEmpty())
    return;

  if (m_CurrentGroup >= static_cast<int>(m_Spec.groups.size()))
  {
    BeginModelDownloadPhase();
    return;
  }

  m_GroupStartIndex = static_cast<int>(m_ResolvedPackages.size());
  const auto& group = m_Spec.groups[m_CurrentGroup];

  // m_ReportFile is reused across groups: the underlying temp file path is
  // stable for the lifetime of this PipInstaller, but we close the Qt handle
  // each time so pip can write to the path freely. The file is deleted when
  // the QTemporaryFile object is destroyed.
  m_ReportFile.close();

  if (!m_ReportFile.open())
  {
    m_State = State::Failed;
    emit ErrorOccurred("Could not create temporary file for pip report.");
    emit InstallFinished(false);
    return;
  }

  auto reportPath = m_ReportFile.fileName();
  m_ReportFile.close();

  QStringList args = { "-m", "pip", "install", "--dry-run", "--report", reportPath };
  args = BuildPipArgs(args, group);

  for (const auto& req : group.requirements)
    args.append(QString::fromStdString(req));

  m_State = State::Resolving;
  MITK_INFO << FormatCommand(python, args).toStdString();
  m_Process->start(python, args);
}

void mitk::PipInstaller::StartInstallPackage()
{
  auto python = RequirePythonExecutable();

  if (python.isEmpty())
    return;

  if (m_CurrentPackage >= static_cast<int>(m_ResolvedPackages.size()))
  {
    BeginModelDownloadPhase();
    return;
  }

  const auto& pkg = m_ResolvedPackages[m_CurrentPackage];
  emit PackageStatusChanged(m_CurrentPackage, QString::fromStdString(pkg.name), PackageStatus::Installing);

  const auto& group = m_Spec.groups[pkg.group];

  // Direct references (VCS URLs, PEP 508 "name @ url") must be passed as-is
  // so pip fetches from the URL instead of searching PyPI for name==version.
  QString installArg;
  if (!pkg.specifier.empty() && IsDirectReference(pkg.specifier))
    installArg = QString::fromStdString(pkg.specifier);
  else
    installArg = QString::fromStdString(pkg.name) + "==" + QString::fromStdString(pkg.version);

  QStringList args = { "-m", "pip", "install", "--no-deps", installArg };
  args = BuildPipArgs(args, group);

  MITK_INFO << FormatCommand(python, args).toStdString();
  m_Process->start(python, args);
}

QStringList mitk::PipInstaller::BuildPipArgs(const QStringList& baseArgs, const PipInstallGroup& group) const
{
  QStringList args = baseArgs;

  if (!group.indexUrl.empty())
    args.append({ "--index-url", QString::fromStdString(group.indexUrl) });

  for (const auto& arg : group.extraPipArgs)
    args.append(QString::fromStdString(arg));

  return args;
}

bool mitk::PipInstaller::ParseResolveReport(const QString& reportPath, int groupIndex)
{
  QFile file(reportPath);

  if (!file.open(QIODevice::ReadOnly))
  {
    MITK_ERROR << "Could not open report file: " << reportPath.toStdString();
    emit ErrorOccurred("Could not open pip resolve report.");
    return false;
  }

  auto data = file.readAll();
  file.close();

  try
  {
    auto report = nlohmann::json::parse(data.constData(), data.constData() + data.size());

    if (!report.contains("install") || !report["install"].is_array())
    {
      MITK_ERROR << "Report file has no 'install' array.";
      emit ErrorOccurred("Unexpected pip report format.");
      return false;
    }

    const auto& group = m_Spec.groups[groupIndex];

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

      // Match the resolved package against this group's requirements using
      // PEP 503-canonicalized comparison so e.g. "scikit_learn", "scikit-learn"
      // and "Scikit-Learn" all match the same resolved name.
      auto canonicalName = CanonicalizePackageName(info.name);

      for (const auto& req : group.requirements)
      {
        auto reqName = CanonicalizePackageName(ExtractPackageName(req));
        if (reqName == canonicalName)
        {
          info.specifier = req;
          break;
        }
      }

      // Bare VCS URLs (git+https://...) have no extractable package name, so
      // the loop above cannot match them. Fall back to matching requested
      // packages against unclaimed direct-reference requirements.
      if (info.specifier.empty() && info.requested)
      {
        for (const auto& req : group.requirements)
        {
          if (!IsDirectReference(req))
            continue;

          bool alreadyClaimed = false;
          for (const auto& existing : m_ResolvedPackages)
          {
            if (existing.specifier == req)
            {
              alreadyClaimed = true;
              break;
            }
          }

          if (!alreadyClaimed)
          {
            info.specifier = req;
            break;
          }
        }
      }

      auto index = static_cast<int>(m_ResolvedPackages.size());
      m_ResolvedPackages.push_back(info);
      emit PackageStatusChanged(index, QString::fromStdString(info.name), PackageStatus::Pending);
    }
  }
  catch (const nlohmann::json::exception& e)
  {
    MITK_ERROR << "Failed to parse report JSON: " << e.what();
    emit ErrorOccurred(QString("Failed to parse pip report: %1").arg(e.what()));
    return false;
  }

  return true;
}

void mitk::PipInstaller::AdvanceToNextGroup()
{
  m_CurrentGroup++;
  m_GroupStartIndex = static_cast<int>(m_ResolvedPackages.size());

  if (m_CurrentGroup < static_cast<int>(m_Spec.groups.size()))
  {
    m_State = State::Resolving;
    emit ResolveStarted();
    StartResolveGroup();
  }
  else
  {
    BeginModelDownloadPhase();
  }
}

// Called once all pip groups have finished. If no Hugging Face downloads are
// configured, immediately emits the terminal InstallFinished. Otherwise
// enters the DownloadingModels state and kicks off the first download.
void mitk::PipInstaller::BeginModelDownloadPhase()
{
  if (m_Spec.huggingFaceDownloads.empty())
  {
    m_State = m_AnyFailed ? State::Failed : State::Done;
    emit InstallFinished(!m_AnyFailed);
    return;
  }

  m_State = State::DownloadingModels;
  m_CurrentDownload = 0;
  StartModelDownload();
}

void mitk::PipInstaller::StartModelDownload()
{
  auto python = RequirePythonExecutable();

  if (python.isEmpty())
    return;

  if (m_CurrentDownload >= static_cast<int>(m_Spec.huggingFaceDownloads.size()))
  {
    m_State = m_AnyFailed ? State::Failed : State::Done;
    emit InstallFinished(!m_AnyFailed);
    return;
  }

  const auto& download = m_Spec.huggingFaceDownloads[m_CurrentDownload];
  auto displayName = QString::fromStdString(
    download.displayName.empty() ? download.repoId : download.displayName);

  // Inline `python -c "<script>"`. QProcess::start with a QStringList handles
  // argument quoting on Windows, so we only have to format repoId / patterns
  // as Python literals. huggingface_hub writes tqdm progress to stderr, which
  // our OnStandardErrorReady -> OutputReceived pipeline forwards to the UI
  // details view. force_download=False keeps the cache authoritative: a
  // re-run on a populated cache is effectively a no-op.
  auto script = QString(
    "from huggingface_hub import snapshot_download\n"
    "snapshot_download(repo_id=%1, allow_patterns=%2, force_download=False)\n")
    .arg(PyQuote(download.repoId), PyListLiteralOrNone(download.allowPatterns));

  emit ModelDownloadStarted(displayName);

  QStringList args = { "-c", script };
  MITK_INFO << FormatCommand(python, args).toStdString();
  m_Process->start(python, args);
}

QString mitk::PipInstaller::PythonExecutable() const
{
  auto path = PythonHelper::GetExecutablePath();

  if (path.empty())
    return {};

  return QString::fromStdString(path.string());
}

QString mitk::PipInstaller::RequirePythonExecutable()
{
  auto python = PythonExecutable();

  if (python.isEmpty())
  {
    m_State = State::Failed;
    emit ErrorOccurred("Python executable not found.");
    emit InstallFinished(false);
  }

  return python;
}

void mitk::PipInstaller::FinalizeCancel()
{
  m_State = State::Failed;
  RemoveCreatedVirtualEnv();
  emit InstallFinished(false);
}

void mitk::PipInstaller::RemoveCreatedVirtualEnv()
{
  if (m_CreatedVirtualEnv && !m_Spec.venvName.empty())
  {
    PythonHelper::RemoveVirtualEnv(m_Spec.venvName);
    m_CreatedVirtualEnv = false;
  }
}
