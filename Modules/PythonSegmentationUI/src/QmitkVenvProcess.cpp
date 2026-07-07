/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkVenvProcess.h>

#include <QProcess>

QProcessEnvironment QmitkVenvProcess::CleanEnvironment()
{
  // The host sets PYTHONHOME / PYTHONPATH for its embedded interpreter; leaving
  // them in place would make the venv interpreter import from the host instead.
  auto env = QProcessEnvironment::systemEnvironment();
  env.remove(QStringLiteral("PYTHONHOME"));
  env.remove(QStringLiteral("PYTHONPATH"));
  return env;
}

QString QmitkVenvProcess::ToQString(const fs::path& path)
{
#if defined(_WIN32)
  return QString::fromStdWString(path.wstring());
#else
  return QString::fromStdString(path.string());
#endif
}

QmitkVenvProcess::Result QmitkVenvProcess::Run(const QString& executable, const QStringList& args, int finishedTimeoutMs)
{
  Result result;

  QProcess process;
  process.setProcessEnvironment(CleanEnvironment());
  process.start(executable, args);

  if (!process.waitForStarted(15000))
  {
    result.standardError = QStringLiteral("Could not start %1.").arg(executable);
    return result;
  }

  if (!process.waitForFinished(finishedTimeoutMs))
  {
    process.kill();
    process.waitForFinished(2000);
    result.standardError = QStringLiteral("%1 did not finish in time.").arg(executable);
    return result;
  }

  result.standardOutput = QString::fromUtf8(process.readAllStandardOutput());
  result.standardError = QString::fromUtf8(process.readAllStandardError());
  result.success = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;

  return result;
}
