/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonPackageUpdatePrompt.h>

#include <mitkException.h>
#include <mitkLog.h>
#include <mitkPythonWorker.h>

#include <QmitkPipInstallDialog.h>

#include <QCoreApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QString>

#include <set>

namespace
{
  constexpr auto LINE_HEIGHT_STYLE = "style='line-height: 1.25'";
}

QString mitk::PythonPackage::DescribeHowToFree(VirtualEnvUse use, const std::string& packageName)
{
  const auto name = QString::fromStdString(packageName);

  if (use == VirtualEnvUse::Worker)
    return QString("Uninitialize %1, or switch to another tool, and try again.").arg(name);

  const auto appName = QCoreApplication::applicationName();
  const auto restartTarget = appName.isEmpty() ? QStringLiteral("the application") : appName;

  return QString("Modules of %1 are loaded into %2. Restart %2 and try again.").arg(name, restartTarget);
}

mitk::PythonPackage::UpdatePromptChoice mitk::PythonPackage::ShowUpdatePrompt(QWidget* parent, const std::string& packageName, const VersionRange& supportedVersions, const VersionCheckResult& result, VirtualEnvUse use, bool inInitFlow)
{
  const QString style = LINE_HEIGHT_STYLE;
  const auto name = QString::fromStdString(packageName);
  const auto installed = QString::fromStdString(result.Installed);
  const auto latest = QString::fromStdString(result.Latest);
  const auto minimum = QString::fromStdString(supportedVersions.Minimum);

  const bool inUse = use != VirtualEnvUse::None;
  const auto blocked = inUse
    ? QString("%1 cannot be updated while it is in use. %2").arg(name, DescribeHowToFree(use, packageName))
    : QString();

  if (result.Status == VersionStatus::BelowMinimum)
  {
    const auto outdated = QString(
      "<h3 %1>%2 is outdated</h3>"
      "<p %1>The installed %2 %3 is older than the version this "
      "application requires (%4 or newer) and may not work correctly.</p>")
      .arg(style, name, installed, minimum);

    if (inUse)
    {
      // An in-place update is impossible while the environment is in use.
      // Nothing actionable here.
      QMessageBox::warning(parent, name, outdated + QString("<p %1>%2</p>").arg(style, blocked));
      return UpdatePromptChoice::Cancel;
    }

    QMessageBox messageBox(QMessageBox::Warning, name, outdated + QString(
      "<p %1>Click <em>Update now</em> to update to a compatible version.</p>")
      .arg(style), QMessageBox::NoButton, parent);
    auto* updateButton = messageBox.addButton("Update now", QMessageBox::AcceptRole);
    messageBox.addButton(QMessageBox::Cancel);
    messageBox.setDefaultButton(updateButton);
    messageBox.exec();

    return messageBox.clickedButton() == updateButton
      ? UpdatePromptChoice::Update
      : UpdatePromptChoice::Cancel;
  }

  // UpdateAvailable: the installed version still works, so updating is optional.
  const auto available = QString(
    "<h3 %1>A newer %2 is available</h3>"
    "<p %1>%2 %3 is installed; %4 is available.</p>")
    .arg(style, name, installed, latest);

  if (inUse)
  {
    if (inInitFlow)
    {
      QMessageBox messageBox(QMessageBox::Information, name, available + QString(
        "<p %1>%2 Or click <em>Continue</em> to keep using the installed version.</p>")
        .arg(style, blocked), QMessageBox::NoButton, parent);
      auto* continueButton = messageBox.addButton("Continue", QMessageBox::AcceptRole);
      messageBox.addButton(QMessageBox::Cancel);
      messageBox.setDefaultButton(continueButton);
      // Dismissing (Esc or the window close button) keeps the working installed
      // version rather than aborting initialization.
      messageBox.setEscapeButton(continueButton);
      messageBox.exec();

      return messageBox.clickedButton() == continueButton
        ? UpdatePromptChoice::ContinueInstalled
        : UpdatePromptChoice::Cancel;
    }

    // Preferences page: nothing to do but acknowledge.
    QMessageBox::information(parent, name, available + QString("<p %1>%2</p>").arg(style, blocked));
    return UpdatePromptChoice::ContinueInstalled;
  }

  // UpdateAvailable and not in use: an in-place update is possible.
  if (inInitFlow)
  {
    QMessageBox messageBox(QMessageBox::Information, name, available + QString(
      "<p %1>Click <em>Update now</em> to update, or <em>Continue with installed</em> "
      "to keep using %2.</p>")
      .arg(style, installed), QMessageBox::NoButton, parent);
    auto* updateButton = messageBox.addButton("Update now", QMessageBox::AcceptRole);
    auto* continueButton = messageBox.addButton("Continue with installed", QMessageBox::AcceptRole);
    messageBox.addButton(QMessageBox::Cancel);
    messageBox.setDefaultButton(continueButton);
    // Dismissing continues with the installed version rather than aborting init.
    messageBox.setEscapeButton(continueButton);
    messageBox.exec();

    if (messageBox.clickedButton() == updateButton)
      return UpdatePromptChoice::Update;

    return messageBox.clickedButton() == continueButton
      ? UpdatePromptChoice::ContinueInstalled
      : UpdatePromptChoice::Cancel;
  }

  // Preferences page: offer the update or just close.
  QMessageBox messageBox(QMessageBox::Information, name, available + QString(
    "<p %1>Click <em>Update now</em> to update.</p>")
    .arg(style), QMessageBox::NoButton, parent);
  auto* updateButton = messageBox.addButton("Update now", QMessageBox::AcceptRole);
  messageBox.addButton(QMessageBox::Close);
  messageBox.setDefaultButton(updateButton);
  messageBox.exec();

  return messageBox.clickedButton() == updateButton
    ? UpdatePromptChoice::Update
    : UpdatePromptChoice::Cancel;
}

mitk::PythonPackage::VersionCheckOutcome mitk::PythonPackage::CheckVersionAndOfferUpdate(QWidget* parent, PackageProbe& probe, const std::string& packageName, const std::string& distributionName, const VersionRange& supportedVersions, const PipInstallSpec& upgradeSpec)
{
  // The distributions that were looked up online in this run, whether the
  // lookup succeeded or not. Process-wide on purpose: the tool GUIs that call
  // this are created anew with every activation of their tool, and a user
  // without network should wait for the timeout only once per run.
  static std::set<std::string> onlineChecksDone;

  const bool checkForUpdate = onlineChecksDone.insert(distributionName).second;
  const auto versionCheck = probe.CheckVersion(distributionName, supportedVersions, checkForUpdate);

  // Its worker would otherwise count as using the virtual environment.
  probe.End();

  if (versionCheck.Status != VersionStatus::BelowMinimum && versionCheck.Status != VersionStatus::UpdateAvailable)
    return VersionCheckOutcome::KeptInstalled;

  switch (ShowUpdatePrompt(parent, packageName, supportedVersions, versionCheck, GetVirtualEnvUse(upgradeSpec.venvName), true))
  {
    case UpdatePromptChoice::Update:
    {
      QmitkPipInstallDialog dialog(upgradeSpec, parent, QmitkPipInstallDialog::Mode::Update);

      return dialog.exec() == QDialog::Accepted
        ? VersionCheckOutcome::Updated
        : VersionCheckOutcome::Aborted;
    }

    case UpdatePromptChoice::ContinueInstalled:
      return VersionCheckOutcome::KeptInstalled;

    case UpdatePromptChoice::Cancel:
      break;
  }

  return VersionCheckOutcome::Aborted;
}

void mitk::PythonPackage::ShowInitializationError(QWidget* parent, const std::string& packageName, const Exception& e)
{
  const auto name = QString::fromStdString(packageName);

  // mitk::Exception::GetDescription() may return nullptr.
  const char* rawDescription = e.GetDescription();
  const auto description = QString::fromLocal8Bit(rawDescription != nullptr ? rawDescription : "");

  // An error thrown from C++ carries a message for the user. A Python error
  // carries a traceback and a worker that ended carries its last output; both
  // go behind a generic headline into the details.
  auto headline = description;
  QString details;

  if (const auto* error = dynamic_cast<const PythonWorkerError*>(&e))
  {
    headline = QString("%1 reported an error during initialization (see details).").arg(name);
    details = QString::fromStdString(error->GetTraceback());
  }
  else if (dynamic_cast<const PythonWorkerExitedException*>(&e) != nullptr)
  {
    headline = QString("%1 stopped unexpectedly during initialization (see details).").arg(name);
    details = QString::fromStdString(rawDescription != nullptr ? rawDescription : "");
  }
  else if (description.contains("An error occurred while executing Python code:"))
  {
    headline = QString("%1 reported an error during initialization (see details).").arg(name);
    details = description;
  }

  MITK_ERROR << packageName << " initialization failed:\n" << (details.isEmpty() ? description : details).toStdString();

  // Escaped, since the description of a C++ error can embed characters that
  // the message box would read as HTML.
  QMessageBox messageBox(QMessageBox::Critical, name,
    QString("<p %1>%2</p>").arg(LINE_HEIGHT_STYLE).arg(headline.toHtmlEscaped()), QMessageBox::Ok, parent);

  if (!details.isEmpty())
    messageBox.setDetailedText(details);

  messageBox.setTextInteractionFlags(Qt::TextSelectableByMouse);
  messageBox.exec();
}
