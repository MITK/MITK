/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveModel.h>

#include <mitkLog.h>
#include <mitkPythonHelper.h>

#include <QEventLoop>
#include <QProcess>
#include <QProcessEnvironment>
#include <QProgressDialog>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QWidget>

#include <optional>

namespace
{
  // Runs a Python snippet with the virtual environment's OWN interpreter as a
  // short-lived subprocess and returns its stdout. Running in a subprocess (rather
  // than the embedded interpreter) keeps nnInteractive's model management and its
  // transitive native dependencies (e.g. PyYAML) out of the host process, so they
  // never map libraries from the virtual environment that would otherwise block a
  // later in-place update on Windows. While the subprocess runs, a modal progress
  // dialog (parented to \p parent, captioned with \p busyMessage) keeps the GUI
  // responsive and lets the user cancel. Returns std::nullopt if the environment
  // or its interpreter is missing, or the run is cancelled or times out.
  std::optional<QString> RunVenvPython(const std::string& venvName, const QString& script, QWidget* parent, const QString& busyMessage)
  {
    const auto venvPath = mitk::PythonHelper::GetVirtualEnvPath(venvName);

    if (venvPath.empty() || !fs::exists(venvPath / "pyvenv.cfg"))
      return std::nullopt;

#if defined(_WIN32)
    const auto python = venvPath / "Scripts" / "python.exe";
#else
    const auto python = venvPath / "bin" / "python3";
#endif

    if (!fs::exists(python))
      return std::nullopt;

    QProcess process;

    // Strip PYTHONHOME / PYTHONPATH (the host sets them for its embedded
    // interpreter) so the venv interpreter resolves its environment from
    // pyvenv.cfg and imports from the venv's site-packages, not the host's.
    auto env = QProcessEnvironment::systemEnvironment();
    env.remove("PYTHONHOME");
    env.remove("PYTHONPATH");
    process.setProcessEnvironment(env);

    process.start(QString::fromStdString(python.string()), { QStringLiteral("-c"), script });

    if (!process.waitForStarted(10000))
    {
      process.kill();
      process.waitForFinished(2000);
      return std::nullopt;
    }

    // The first call refreshes the model manifest from Hugging Face, which can
    // take up to a minute. QProcess is asynchronous, so wait on its completion
    // through a local event loop rather than a blocking waitForFinished(): the
    // loop keeps the GUI repainting while a modal progress dialog shows the busy
    // state and offers Cancel. The subprocess only reads the manifest, so
    // cancelling it (which kills the process) leaves nothing partially written.
    QEventLoop loop;
    QObject::connect(&process, &QProcess::finished, &loop, &QEventLoop::quit);
    QObject::connect(&process, &QProcess::errorOccurred, &loop, &QEventLoop::quit);

    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeoutTimer.start(60000);

    QProgressDialog dialog(busyMessage, QStringLiteral("Cancel"), 0, 0, parent);
    dialog.setWindowTitle(QStringLiteral("nnInteractive"));
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setMinimumDuration(0);
    QObject::connect(&dialog, &QProgressDialog::canceled, &loop, &QEventLoop::quit);
    dialog.show();

    // The process can finish between waitForStarted() above and here; only enter
    // the loop while it is still running, otherwise quit() would never arrive.
    if (process.state() != QProcess::NotRunning)
      loop.exec();

    if (process.state() != QProcess::NotRunning)
    {
      // Cancelled or timed out: the process is still running, so stop it.
      process.kill();
      process.waitForFinished(2000);
      return std::nullopt;
    }

    if (process.exitStatus() != QProcess::NormalExit)
      return std::nullopt;

    return QString::fromLocal8Bit(process.readAllStandardOutput());
  }

  // Splits the subprocess stdout into lines, tolerating the CRLF that text-mode
  // Python stdout emits on Windows.
  QStringList SplitLines(const QString& output)
  {
    QString normalized = output;
    normalized.remove(QLatin1Char('\r'));
    return normalized.split(QLatin1Char('\n'));
  }
}

std::vector<mitk::nnInteractive::ModelInfo> mitk::nnInteractive::ListModels(const std::string& venvName, ModelListStatus* status, QWidget* parent)
{
  const auto setStatus = [status](ModelListStatus value)
  {
    if (status != nullptr)
      *status = value;
  };

  setStatus(ModelListStatus::Failed);

  // Each model is printed as a tab-separated NNI_MODEL row; a trailing NNI_OK
  // marks a successful run. The ids and short display names never contain tabs or
  // newlines (sanitized below as a safeguard), so one row round-trips one model.
  static const QString script = QStringLiteral(R"PY(
import sys
try:
    from nnInteractive.model_management import list_models
    for _m in list_models():
        _id = str(_m.get('id', '') or '')
        _name = str(_m.get('display_name', '') or _id)
        _def = '1' if _m.get('default') else '0'
        _dl = '1' if _m.get('downloaded') else '0'
        _id = _id.replace('\t', ' ').replace('\r', ' ').replace('\n', ' ')
        _name = _name.replace('\t', ' ').replace('\r', ' ').replace('\n', ' ')
        sys.stdout.write('NNI_MODEL\t' + _id + '\t' + _name + '\t' + _def + '\t' + _dl + '\n')
    sys.stdout.write('NNI_OK\n')
except Exception as _e:
    sys.stdout.write('NNI_ERR\t' + str(_e).replace('\n', ' ') + '\n')
)PY");

  std::vector<ModelInfo> models;

  const auto output = RunVenvPython(venvName, script, parent, QStringLiteral("Loading the nnInteractive model list..."));

  if (!output.has_value())
    return models;

  bool ok = false;

  for (const auto& line : SplitLines(*output))
  {
    if (line == QLatin1String("NNI_OK"))
    {
      ok = true;
      continue;
    }

    if (!line.startsWith(QLatin1String("NNI_MODEL\t")))
      continue;

    const auto fields = line.split(QLatin1Char('\t'));

    // fields: NNI_MODEL, id, display_name, default, downloaded
    if (fields.size() < 5 || fields[1].isEmpty())
      continue;

    ModelInfo info;
    info.Id = fields[1].toStdString();
    info.DisplayName = fields[2].isEmpty() ? info.Id : fields[2].toStdString();
    info.IsDefault = fields[3] == QLatin1String("1");
    info.Downloaded = fields[4] == QLatin1String("1");
    models.push_back(std::move(info));
  }

  if (!ok)
  {
    MITK_WARN << "nnInteractive: could not list models.";
    return {};
  }

  if (models.empty())
  {
    MITK_WARN << "nnInteractive: model list is empty.";
    setStatus(ModelListStatus::Empty);
    return models;
  }

  setStatus(ModelListStatus::Ok);

  return models;
}

mitk::nnInteractive::ModelCheckResult mitk::nnInteractive::CheckModelUpdate(const std::string& venvName, const std::string& selectedModelId, QWidget* parent)
{
  ModelCheckResult result;
  result.CurrentId = selectedModelId;

  static const QString script = QStringLiteral(R"PY(
import sys
try:
    from nnInteractive.model_management import get_default_model_id
    _default = str(get_default_model_id() or '')
    sys.stdout.write('NNI_DEFAULT\t' + _default + '\n')
    sys.stdout.write('NNI_OK\n')
except Exception as _e:
    sys.stdout.write('NNI_ERR\t' + str(_e).replace('\n', ' ') + '\n')
)PY");

  const auto output = RunVenvPython(venvName, script, parent, QStringLiteral("Checking the nnInteractive model manifest..."));

  if (!output.has_value())
    return result; // Status stays Unknown.

  bool ok = false;
  QString defaultId;

  for (const auto& line : SplitLines(*output))
  {
    if (line == QLatin1String("NNI_OK"))
      ok = true;
    else if (line.startsWith(QLatin1String("NNI_DEFAULT\t")))
      defaultId = line.section(QLatin1Char('\t'), 1);
  }

  if (!ok)
    return result; // Status stays Unknown.

  result.RecommendedId = defaultId.toStdString();

  if (result.RecommendedId.empty())
    return result; // Status stays Unknown.

  // An empty selection means "use the recommended default", so it is by
  // definition up to date with whatever the default currently is.
  if (selectedModelId.empty() || selectedModelId == result.RecommendedId)
    result.Status = ModelUpdateStatus::UpToDate;
  else
    result.Status = ModelUpdateStatus::UpdateAvailable;

  return result;
}
