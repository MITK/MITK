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
#include <mitkPythonHelper.h>

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

mitk::PythonPackage::UpdatePromptChoice mitk::PythonPackage::ShowUpdatePrompt(QWidget* parent, const std::string& packageName, const VersionRange& supportedVersions, const VersionCheckResult& result, bool modulesLoaded, bool inInitFlow)
{
  const QString style = LINE_HEIGHT_STYLE;
  const auto name = QString::fromStdString(packageName);
  const auto installed = QString::fromStdString(result.Installed);
  const auto latest = QString::fromStdString(result.Latest);
  const auto minimum = QString::fromStdString(supportedVersions.Minimum);

  const auto appName = QCoreApplication::applicationName();
  const auto restartTarget = appName.isEmpty() ? QStringLiteral("the application") : appName;

  if (result.Status == VersionStatus::BelowMinimum)
  {
    const auto outdated = QString(
      "<h3 %1>%2 is outdated</h3>"
      "<p %1>The installed %2 %3 is older than the version this "
      "application requires (%4 or newer) and may not work correctly.</p>")
      .arg(style, name, installed, minimum);

    if (modulesLoaded)
    {
      // An in-place update is impossible while the package is loaded; the user
      // must restart first. Nothing actionable here.
      QMessageBox::warning(parent, name, outdated + QString(
        "<p %1>%2 is currently loaded, so it cannot be updated right now. "
        "Restart %3 and try again.</p>")
        .arg(style, name, restartTarget));
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

  if (modulesLoaded)
  {
    if (inInitFlow)
    {
      QMessageBox messageBox(QMessageBox::Information, name, available + QString(
        "<p %1>%2 is currently loaded, so it cannot be updated right now. "
        "Restart %3 and try again, or click <em>Continue</em> to keep using the "
        "installed version.</p>")
        .arg(style, name, restartTarget), QMessageBox::NoButton, parent);
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
    QMessageBox::information(parent, name, available + QString(
      "<p %1>%2 is currently loaded, so it cannot be updated right now. "
      "Restart %3 and try again.</p>")
      .arg(style, name, restartTarget));
    return UpdatePromptChoice::ContinueInstalled;
  }

  // UpdateAvailable and not loaded: an in-place update is possible.
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

mitk::PythonPackage::VersionCheckOutcome mitk::PythonPackage::CheckVersionAndOfferUpdate(QWidget* parent, PythonContext& context, const std::string& packageName, const std::string& distributionName, const VersionRange& supportedVersions, const PipInstallSpec& upgradeSpec)
{
  // The distributions that were looked up online in this run, whether the
  // lookup succeeded or not. Process-wide on purpose: the tool GUIs that call
  // this are created anew with every activation of their tool, and once the
  // modules of a virtual environment are loaded, an in-place update is blocked
  // until the application restarts anyway.
  static std::set<std::string> onlineChecksDone;

  const bool checkForUpdate = onlineChecksDone.insert(distributionName).second;
  const auto versionCheck = CheckInstalledVersion(context, distributionName, supportedVersions, checkForUpdate);

  if (versionCheck.Status != VersionStatus::BelowMinimum && versionCheck.Status != VersionStatus::UpdateAvailable)
    return VersionCheckOutcome::KeptInstalled;

  const bool modulesLoaded = PythonHelper::IsAnyVirtualEnvModuleLoaded(upgradeSpec.venvName);

  switch (ShowUpdatePrompt(parent, packageName, supportedVersions, versionCheck, modulesLoaded, true))
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

  MITK_ERROR << packageName << " initialization failed:\n" << description.toStdString();

  // An error thrown from C++ carries a message for the user. An error from the
  // embedded Python interpreter carries a traceback, which goes behind a
  // generic headline into the details.
  const bool isPythonError = description.contains("An error occurred while executing Python code:");

  const auto headline = isPythonError
    ? QString("%1 reported an error during initialization (see details).").arg(name)
    : description;

  // Escaped, since the description of a C++ error can embed characters that
  // the message box would read as HTML.
  QMessageBox messageBox(QMessageBox::Critical, name,
    QString("<p %1>%2</p>").arg(LINE_HEIGHT_STYLE).arg(headline.toHtmlEscaped()), QMessageBox::Ok, parent);

  if (isPythonError)
    messageBox.setDetailedText(description);

  messageBox.setTextInteractionFlags(Qt::TextSelectableByMouse);
  messageBox.exec();
}
