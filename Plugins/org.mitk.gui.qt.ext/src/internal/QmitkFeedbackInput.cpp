/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFeedbackInput.h"

#include <QmitkAbstractMultiWidget.h>
#include <QmitkRenderWindow.h>

#ifdef MITK_HAS_CRASHHANDLING
#include <QmitkCrashDumpListWidget.h>
#endif

#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryPlatformUI.h>
#include <berryShell.h>

#include <mitkEnvironmentInfo.h>
#include <mitkException.h>
#include <mitkIRenderWindowPart.h>
#include <mitkLogBackend.h>
#include <mitkLogTail.h>

#include <vtkRenderWindow.h>

#include <QDateTime>
#include <QFileInfo>
#include <QWidget>

#include <algorithm>
#include <sstream>

namespace
{
  constexpr std::size_t MaxLogLines = 1000;

  const std::string CurrentSessionLogName = "logs/current-session.log";
  const std::string CurrentSessionLabel = "This session";

  berry::IWorkbenchWindow::Pointer GetActiveWorkbenchWindow()
  {
    if (!berry::PlatformUI::IsWorkbenchRunning())
      return {};

    return berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow();
  }

  QWidget* GetWindowWidget(const berry::IWorkbenchWindow::Pointer& window)
  {
    return window.IsNull() ? nullptr : static_cast<QWidget*>(window->GetShell()->GetControl());
  }

  /** The render windows of the active editor, the active one first; empty
   *  if the active editor has none. */
  std::vector<QmitkRenderWindow*> GetActiveEditorRenderWindows(const berry::IWorkbenchWindow::Pointer& window)
  {
    if (window.IsNull() || window->GetActivePage().IsNull())
      return {};

    const auto editor = window->GetActivePage()->GetActiveEditor();
    const auto* part = dynamic_cast<mitk::IRenderWindowPart*>(editor.GetPointer());

    if (part == nullptr)
      return {};

    std::vector<QmitkRenderWindow*> renderWindows;

    if (auto* active = part->GetActiveQmitkRenderWindow(); active != nullptr)
      renderWindows.push_back(active);

    for (auto* renderWindow : part->GetQmitkRenderWindows())
    {
      if (renderWindow != nullptr && std::find(renderWindows.begin(), renderWindows.end(), renderWindow) == renderWindows.end())
        renderWindows.push_back(renderWindow);
    }

    return renderWindows;
  }

  /** The multi-widget hosting \p renderWindow, i.e. the editor area of the
   *  standard and the MxN editor. Its render windows sit further down the
   *  widget tree (in render window widgets, and splitters for MxN), so the
   *  search covers all ancestors rather than the direct parent. */
  QmitkAbstractMultiWidget* FindMultiWidget(QmitkRenderWindow* renderWindow)
  {
    for (auto* widget = renderWindow->parentWidget(); widget != nullptr; widget = widget->parentWidget())
    {
      if (auto* multiWidget = dynamic_cast<QmitkAbstractMultiWidget*>(widget); multiWidget != nullptr)
        return multiWidget;
    }

    return nullptr;
  }

  /** The renderer of the context VTK actually draws with. An offscreen GL
   *  context could be bound to a different GPU on multi-GPU hosts. Empty if
   *  there is no initialized render window. */
  std::string GetGpuRenderer(const std::vector<QmitkRenderWindow*>& renderWindows)
  {
    if (renderWindows.empty())
      return {};

    auto* vtkWindow = renderWindows.front()->GetVtkRenderWindow();

    if (vtkWindow == nullptr || !vtkWindow->GetInitialized())
      return {};

    const std::string prefix = "OpenGL renderer string:";
    std::istringstream capabilities(vtkWindow->ReportCapabilities());
    std::string line;

    while (std::getline(capabilities, line))
    {
      if (line.compare(0, prefix.size(), prefix) != 0)
        continue;

      const auto first = line.find_first_not_of(' ', prefix.size());
      return first != std::string::npos ? line.substr(first) : std::string();
    }

    return {};
  }
}

QWidget* QmitkGetActiveWorkbenchWindowWidget()
{
  return GetWindowWidget(GetActiveWorkbenchWindow());
}

QmitkFeedbackDialog::Input QmitkCollectFeedbackInput()
{
  const auto window = GetActiveWorkbenchWindow();

  QmitkFeedbackDialog::Input input;

  if (auto* windowWidget = GetWindowWidget(window); windowWidget != nullptr)
    input.windowShot = windowWidget->grab();

  const auto renderWindows = GetActiveEditorRenderWindows(window);

  if (auto* multiWidget = renderWindows.empty() ? nullptr : FindMultiWidget(renderWindows.front()); multiWidget != nullptr)
  {
    // Render windows of an editor in a background tab have their updates
    // suspended and would be captured outdated.
    multiWidget->ForceImmediateUpdateAll();
    input.editorShot = multiWidget->grab();
  }

  const auto logFile = mitk::LogBackend::GetLogFile();

  if (logFile.empty())
  {
    QmitkFeedbackDialog::LogSource source;
    source.log.archiveName = CurrentSessionLogName;
    source.log.sourceLabel = CurrentSessionLabel;
    source.unavailableReason = "This session writes no log file.";
    input.logs.push_back(source);
  }
  else
  {
    // Narrow conversion, as the log backend uses when opening the file.
    input.logs.push_back(QmitkReadFeedbackLog(std::filesystem::path(logFile), CurrentSessionLogName,
      CurrentSessionLabel, "this session"));
  }

  input.environment = mitk::CollectEnvironmentInfo();
  input.environment.gpuRenderer = GetGpuRenderer(renderWindows);

  return input;
}

QmitkFeedbackDialog::LogSource QmitkReadFeedbackLog(const std::filesystem::path& path, const std::string& archiveName,
  const std::string& sourceLabel, const std::string& session)
{
  QmitkFeedbackDialog::LogSource source;
  source.log.archiveName = archiveName;
  source.log.sourceLabel = sourceLabel;

  try
  {
    const auto tail = mitk::ReadLogTail(path, MaxLogLines);

    std::ostringstream text;

    if (tail.truncated)
      text << "[The last " << tail.linesTaken << " lines of the log of " << session << "; earlier lines are omitted.]\n";
    else
      text << "[All " << tail.linesTaken << " lines of the log of " << session << ".]\n";

    text << tail.text;
    source.log.text = text.str();
  }
  catch (const mitk::Exception&)
  {
    source.unavailableReason = QString("The log file of %1 could not be read.").arg(QString::fromStdString(session));
  }

  return source;
}

#ifdef MITK_HAS_CRASHHANDLING
QmitkFeedbackDialog::Input QmitkCollectCrashReportInput(const std::vector<mitk::CrashDumpInfo>& dumps)
{
  auto input = QmitkCollectFeedbackInput();
  input.dumps = dumps;

  for (const auto& dump : dumps)
  {
    if (dump.SessionLog.empty())
      continue;

    const auto time = QmitkFeedbackDumpTime(dump);
    const auto label = QString("Session log (%1, %2)").arg(QmitkCrashDumpListWidget::KindLabel(dump.Kind), time);
    const auto stem = QString::fromStdU16String(dump.Path.stem().u16string());

    input.logs.push_back(QmitkReadFeedbackLog(dump.SessionLog, ("logs/" + stem + ".log").toStdString(),
      label.toStdString(), ("the session of the dump of " + time).toStdString()));
  }

  return input;
}

QString QmitkFeedbackDumpTime(const mitk::CrashDumpInfo& dump)
{
  const auto path = QString::fromStdU16String(dump.Path.u16string());
  return QFileInfo(path).lastModified().toString("yyyy-MM-dd HH:mm");
}
#endif
