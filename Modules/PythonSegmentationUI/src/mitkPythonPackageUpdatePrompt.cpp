/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonPackageUpdatePrompt.h>

#include <QCoreApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QString>

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
