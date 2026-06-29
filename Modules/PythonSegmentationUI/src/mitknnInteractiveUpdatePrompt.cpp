/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveUpdatePrompt.h>

#include <QCoreApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QString>

namespace
{
  constexpr auto LINE_HEIGHT_STYLE = "style='line-height: 1.25'";
}

mitk::nnInteractive::UpdatePromptChoice mitk::nnInteractive::ShowUpdatePrompt(QWidget* parent, const VersionCheckResult& result, bool modulesLoaded, bool inInitFlow)
{
  const auto installed = QString::fromStdString(result.Installed);
  const auto latest = QString::fromStdString(result.Latest);
  const auto minimum = QString(MINIMUM_VERSION);

  const auto appName = QCoreApplication::applicationName();
  const auto restartTarget = appName.isEmpty() ? QStringLiteral("the application") : appName;

  if (result.Status == VersionStatus::BelowMinimum)
  {
    if (modulesLoaded)
    {
      // An in-place update is impossible while the package is loaded; the user
      // must restart first. Nothing actionable here.
      QMessageBox::warning(parent, "nnInteractive",
        QString(
          "<h3 %1>nnInteractive is outdated</h3>"
          "<p %1>The installed nnInteractive %2 is older than the version this "
          "application requires (%3 or newer) and may not work correctly.</p>"
          "<p %1>nnInteractive is currently loaded, so it cannot be updated right now. "
          "Restart %4 and try again.</p>")
          .arg(LINE_HEIGHT_STYLE).arg(installed).arg(minimum).arg(restartTarget));
      return UpdatePromptChoice::Cancel;
    }

    QMessageBox messageBox(QMessageBox::Warning, "nnInteractive",
      QString(
        "<h3 %1>nnInteractive is outdated</h3>"
        "<p %1>The installed nnInteractive %2 is older than the version this "
        "application requires (%3 or newer) and may not work correctly.</p>"
        "<p %1>Click <em>Update now</em> to update to a compatible version.</p>")
        .arg(LINE_HEIGHT_STYLE).arg(installed).arg(minimum), QMessageBox::NoButton, parent);
    auto* updateButton = messageBox.addButton("Update now", QMessageBox::AcceptRole);
    messageBox.addButton(QMessageBox::Cancel);
    messageBox.setDefaultButton(updateButton);
    messageBox.exec();

    return messageBox.clickedButton() == updateButton
      ? UpdatePromptChoice::Update
      : UpdatePromptChoice::Cancel;
  }

  // UpdateAvailable: the installed version still works, so updating is optional.
  if (modulesLoaded)
  {
    if (inInitFlow)
    {
      QMessageBox messageBox(QMessageBox::Information, "nnInteractive",
        QString(
          "<h3 %1>A newer nnInteractive is available</h3>"
          "<p %1>nnInteractive %2 is installed; %3 is available.</p>"
          "<p %1>nnInteractive is currently loaded, so it cannot be updated right now. "
          "Restart %4 and try again, or click <em>Continue</em> to keep using the "
          "installed version.</p>")
          .arg(LINE_HEIGHT_STYLE).arg(installed).arg(latest).arg(restartTarget), QMessageBox::NoButton, parent);
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
    QMessageBox::information(parent, "nnInteractive",
      QString(
        "<h3 %1>A newer nnInteractive is available</h3>"
        "<p %1>nnInteractive %2 is installed; %3 is available.</p>"
        "<p %1>nnInteractive is currently loaded, so it cannot be updated right now. "
        "Restart %4 and try again.</p>")
        .arg(LINE_HEIGHT_STYLE).arg(installed).arg(latest).arg(restartTarget));
    return UpdatePromptChoice::ContinueInstalled;
  }

  // UpdateAvailable and not loaded: an in-place update is possible.
  if (inInitFlow)
  {
    QMessageBox messageBox(QMessageBox::Information, "nnInteractive",
      QString(
        "<h3 %1>A newer nnInteractive is available</h3>"
        "<p %1>nnInteractive %2 is installed; %3 is available.</p>"
        "<p %1>Click <em>Update now</em> to update, or <em>Continue with installed</em> "
        "to keep using %2.</p>")
        .arg(LINE_HEIGHT_STYLE).arg(installed).arg(latest), QMessageBox::NoButton, parent);
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
  QMessageBox messageBox(QMessageBox::Information, "nnInteractive",
    QString(
      "<h3 %1>A newer nnInteractive is available</h3>"
      "<p %1>nnInteractive %2 is installed; %3 is available.</p>"
      "<p %1>Click <em>Update now</em> to update.</p>")
      .arg(LINE_HEIGHT_STYLE).arg(installed).arg(latest), QMessageBox::NoButton, parent);
  auto* updateButton = messageBox.addButton("Update now", QMessageBox::AcceptRole);
  messageBox.addButton(QMessageBox::Close);
  messageBox.setDefaultButton(updateButton);
  messageBox.exec();

  return messageBox.clickedButton() == updateButton
    ? UpdatePromptChoice::Update
    : UpdatePromptChoice::Cancel;
}
