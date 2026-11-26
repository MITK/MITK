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
#include <QmitkCrashDumpManagerDialog.h>
#endif

#include <berryIEditorReference.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryPlatformUI.h>
#include <berryShell.h>

#include <mitkEnvironmentInfo.h>
#include <mitkException.h>
#include <mitkIRenderWindowPart.h>
#include <mitkLog.h>
#include <mitkLogBackend.h>
#include <mitkLogTail.h>

#include <vtkRenderWindow.h>

#include <QDateTime>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>
#include <QWidget>

#include <algorithm>
#include <exception>
#include <sstream>

namespace
{
  constexpr std::size_t MaxLogLines = 1000;
  constexpr std::size_t MaxLogBytes = 512 * 1024;

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

  /** Runs \p capture. A report is still worth filing when rendering throws,
   *  so a failure only leaves the screenshot named \p what out of the report. */
  template <class Capture>
  void TryCapture(const char* what, Capture&& capture)
  {
    try
    {
      capture();
    }
    catch (const std::exception& e)
    {
      MITK_WARN << "The screenshot of the " << what << " is left out of the report: " << e.what();
    }
    catch (...)
    {
      MITK_WARN << "The screenshot of the " << what << " is left out of the report: unknown exception";
    }
  }

  /** Whether VTK has set up the context of \p renderWindow, i.e. the window
   *  was shown at least once. */
  bool IsInitialized(QmitkRenderWindow* renderWindow)
  {
    auto* vtkWindow = renderWindow->GetVtkRenderWindow();
    return vtkWindow != nullptr && vtkWindow->GetInitialized();
  }

  /** The render windows of \p part, its active one first. */
  std::vector<QmitkRenderWindow*> GetRenderWindows(const mitk::IRenderWindowPart& part)
  {
    std::vector<QmitkRenderWindow*> renderWindows;

    if (auto* active = part.GetActiveQmitkRenderWindow(); active != nullptr)
      renderWindows.push_back(active);

    for (auto* renderWindow : part.GetQmitkRenderWindows())
    {
      if (renderWindow != nullptr && std::find(renderWindows.begin(), renderWindows.end(), renderWindow) == renderWindows.end())
        renderWindows.push_back(renderWindow);
    }

    return renderWindows;
  }

  /** The render windows of the editor that shows the display, the active one
   *  first: the active editor if it is a render window part, else the first
   *  open render window part with an initialized render window. The active
   *  editor can be one without a display, e.g. the welcome screen. Editors
   *  that are not realized yet stay so, and no editor is activated. Empty if
   *  there is none. */
  std::vector<QmitkRenderWindow*> FindDisplayRenderWindows(const berry::IWorkbenchWindow::Pointer& window)
  {
    if (window.IsNull() || window->GetActivePage().IsNull())
      return {};

    const auto page = window->GetActivePage();
    const auto activeEditor = page->GetActiveEditor();

    if (const auto* activePart = dynamic_cast<mitk::IRenderWindowPart*>(activeEditor.GetPointer()); activePart != nullptr)
      return GetRenderWindows(*activePart);

    for (const auto& editorRef : page->GetEditorReferences())
    {
      const auto editor = editorRef->GetPart(false);
      const auto* part = dynamic_cast<mitk::IRenderWindowPart*>(editor.GetPointer());

      if (part == nullptr)
        continue;

      auto renderWindows = GetRenderWindows(*part);

      if (std::any_of(renderWindows.begin(), renderWindows.end(), IsInitialized))
        return renderWindows;
    }

    return {};
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

  /** The renderer of the context VTK actually draws with, taken from the
   *  first initialized window of \p renderWindows. An offscreen GL context
   *  could be bound to a different GPU on multi-GPU hosts. Empty if there is
   *  no initialized render window. */
  std::string GetGpuRenderer(const std::vector<QmitkRenderWindow*>& renderWindows)
  {
    const auto initialized = std::find_if(renderWindows.begin(), renderWindows.end(), IsInitialized);

    if (initialized == renderWindows.end())
      return {};

    auto* vtkWindow = (*initialized)->GetVtkRenderWindow();

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
    TryCapture("application window", [&] { input.windowShot = windowWidget->grab(); });

  const auto renderWindows = FindDisplayRenderWindows(window);

  if (auto* multiWidget = renderWindows.empty() ? nullptr : FindMultiWidget(renderWindows.front()); multiWidget != nullptr)
  {
    TryCapture("display area", [&] {
      // Render windows of an editor in a background tab have their updates
      // suspended and would be captured outdated.
      multiWidget->ForceImmediateUpdateAll();
      input.editorShot = multiWidget->grab();
    });
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
    const auto tail = mitk::ReadLogTail(path, MaxLogLines, MaxLogBytes);

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

    auto source = QmitkReadFeedbackLog(dump.SessionLog, ("logs/" + stem + ".log").toStdString(),
      label.toStdString(), ("the session of the dump of " + time).toStdString());
    source.dump = dump.Path;
    input.logs.push_back(source);
  }

  return input;
}

QString QmitkFeedbackDumpTime(const mitk::CrashDumpInfo& dump)
{
  const auto path = QString::fromStdU16String(dump.Path.u16string());
  return QFileInfo(path).lastModified().toString("yyyy-MM-dd HH:mm");
}
#endif

void QmitkRunFeedbackDialog(const QmitkFeedbackDialog::Input& input, QWidget* parent)
{
  if (parent == nullptr)
    parent = QmitkGetActiveWorkbenchWindowWidget();

  std::vector<std::filesystem::path> dumpsToDelete;

  {
    QmitkFeedbackDialog dialog(input, parent);
    dialog.exec();
    dumpsToDelete = dialog.GetDumpsToDelete();
  }

#ifdef MITK_HAS_CRASHHANDLING
  QStringList failed;

  for (const auto& dump : dumpsToDelete)
  {
    if (mitk::CrashDumpFacility::DeleteDump(dump))
      continue;

    // Another instance or the dump pruning may have removed it meanwhile.
    std::error_code error;

    if (std::filesystem::exists(dump, error) || error)
      failed << QString::fromStdU16String(dump.filename().u16string());
  }

  if (!dumpsToDelete.empty())
    QmitkCrashDumpManagerDialog::RefreshIfOpen();

  if (!failed.isEmpty())
  {
    QMessageBox::warning(parent, "Report a Problem / Send Feedback",
      "These crash dumps could not be deleted:\n\n" + failed.join('\n') +
      "\n\nYou can delete them later under Help > Diagnostic Data.");
  }
#endif
}
