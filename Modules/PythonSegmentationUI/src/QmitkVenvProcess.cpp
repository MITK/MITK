/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkVenvProcess.h>

#include <mitkPipPackageInfo.h>
#include <mitkPostInstallStepProgress.h>
#include <mitkProgressTask.h>
#include <mitkPythonHelper.h>

#include <QEventLoop>
#include <QProcess>
#include <QTimer>

#include <algorithm>
#include <optional>

QProcessEnvironment QmitkVenvProcess::CleanEnvironment()
{
  // PYTHONHOME / PYTHONPATH of another Python installation would make the venv
  // interpreter import from there instead of from the venv.
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

QmitkVenvProcess::StepResult QmitkVenvProcess::RunStep(const std::string& venvName, const mitk::PostInstallStep& step, QString* output)
{
  QString log;

  const auto finish = [&log, output](StepResult result)
  {
    if (output != nullptr)
      *output = log.trimmed();

    return result;
  };

  const auto python = mitk::PythonHelper::GetVirtualEnvExecutablePath(venvName);

  if (python.empty())
  {
    log = QStringLiteral("The Python interpreter of the virtual environment %1 was not found.").arg(QString::fromStdString(venvName));
    return finish(StepResult::Failed);
  }

  // Decoded as UTF-8 below, whatever the code page of the system is.
  auto environment = CleanEnvironment();
  environment.insert(QStringLiteral("PYTHONIOENCODING"), QStringLiteral("utf-8"));

  QProcess process;
  process.setProcessEnvironment(environment);

  // Created with the first report, so that a step with nothing to do shows nothing.
  std::optional<mitk::ProgressTask> task;
  constexpr unsigned int PROGRESS_STEPS = 1000;

  const auto readOutput = [&]()
  {
    while (process.canReadLine())
    {
      const auto line = process.readLine();
      const auto progress = mitk::ParsePostInstallStepProgress(line.toStdString());

      if (!progress.has_value())
      {
        log += QString::fromUtf8(line);
        continue;
      }

      if (!task.has_value())
        task.emplace(step.displayName, progress->Total > 0 ? PROGRESS_STEPS : mitk::ProgressTask::Indeterminate, true);

      if (progress->Total > 0 && task->GetStepsToDo() > 0)
        task->SetProgress(static_cast<unsigned int>(std::min(progress->Done, progress->Total) * PROGRESS_STEPS / progress->Total));
    }
  };

  QEventLoop loop;
  bool cancelled = false;

  QObject::connect(&process, &QProcess::readyReadStandardOutput, &loop, readOutput);
  QObject::connect(&process, &QProcess::readyReadStandardError, &loop, [&]() { log += QString::fromUtf8(process.readAllStandardError()); });
  QObject::connect(&process, &QProcess::finished, &loop, &QEventLoop::quit);

  QTimer cancelTimer;
  cancelTimer.setInterval(100);

  QObject::connect(&cancelTimer, &QTimer::timeout, &loop, [&]()
  {
    if (task.has_value() && task->IsCancelRequested())
    {
      cancelled = true;
      loop.quit();
    }
  });

  process.start(ToQString(python), { QStringLiteral("-c"), QString::fromStdString(step.pythonCode) });

  if (!process.waitForStarted(15000))
  {
    log = QStringLiteral("Could not start %1.").arg(ToQString(python));
    return finish(StepResult::Failed);
  }

  cancelTimer.start();

  // A step can end before the loop starts, which then would not see it end.
  if (process.state() != QProcess::NotRunning)
    loop.exec();

  cancelTimer.stop();

  if (cancelled)
  {
    // Whatever the step leaves behind when it is stopped this way is for the
    // step to clean up the next time it runs.
    process.kill();
    process.waitForFinished(5000);
    return finish(StepResult::Cancelled);
  }

  readOutput();

  // A last line without a line break.
  log += QString::fromUtf8(process.readAllStandardOutput());
  log += QString::fromUtf8(process.readAllStandardError());

  const bool succeeded = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
  return finish(succeeded ? StepResult::Succeeded : StepResult::Failed);
}
