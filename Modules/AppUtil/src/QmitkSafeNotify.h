/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSafeNotify_h
#define QmitkSafeNotify_h

#include <mitkException.h>
#include <mitkLog.h>

#include <QMessageBox>
#include <QPushButton>

#ifdef MITK_HAS_CRASHHANDLING
#include <mitkCrashDumpFacility.h>
#include <mitkICrashReportService.h>

#include <QmitkCrashDumpManagerDialog.h>

#include "QmitkCrashDumpUiUtils.h"
#endif

#include <cstdlib>

/**
 * \brief Safely delivers a Qt event, catching and displaying any exceptions.
 *
 * Wraps a call to QApplication::notify() in a try/catch block. If an
 * exception is thrown during event delivery, a critical QMessageBox is
 * shown to the user with the option to exit the application or ignore
 * the error.
 *
 * \tparam A A QApplication-derived type whose notify() method is to be called.
 * \param app Pointer to the application instance.
 * \param receiver The QObject that should receive the event.
 * \param event The QEvent to deliver.
 * \return true if the event was handled successfully, false if an exception occurred.
 */
template <class A>
bool QmitkSafeNotify(A *app, QObject *receiver, QEvent *event)
{
  QString msg;
  try
  {
    return app->A::notify(receiver, event);
  }
  catch (mitk::Exception &e)
  {
    msg = QString("MITK Exception:\n\n") + QString("Description: ") + QString(e.GetDescription()) + QString("\n\n") +
          QString("Filename: ") + QString(e.GetFile()) + QString("\n\n") + QString("Line: ") +
          QString::number(e.GetLine());
  }
  catch (std::exception &e)
  {
    msg = e.what();
  }
  catch (...)
  {
    msg = "Unknown exception";
  }
  MITK_ERROR << "An error occurred: " << msg.toStdString();

  QMessageBox msgBox;
  msgBox.setText("An error occurred. You should save all data and quit the program to prevent possible data loss.");
  msgBox.setDetailedText(msg);
  msgBox.setIcon(QMessageBox::Critical);
  auto *exitButton = msgBox.addButton("Exit immediately", QMessageBox::YesRole);
  auto *ignoreButton = msgBox.addButton("Ignore", QMessageBox::NoRole);
#ifdef MITK_HAS_CRASHHANDLING
  // Offered only where a capture can succeed.
  QPushButton *captureButton = nullptr;
  if (mitk::CrashDumpFacility::IsActive() && mitk::CrashDumpFacility::SupportsSnapshots())
    captureButton = msgBox.addButton("Capture diagnostics", QMessageBox::ActionRole);
#endif

  msgBox.exec();
  auto *clicked = msgBox.clickedButton();

  if (clicked == exitButton)
  {
    MITK_ERROR << "The program was closed.";
    std::exit(EXIT_FAILURE);
  }
  else if (clicked == ignoreButton)
  {
    MITK_ERROR
      << "The error was ignored by the user. The program may be in a corrupt state and don't behave like expected!";
  }
#ifdef MITK_HAS_CRASHHANDLING
  else if (captureButton != nullptr && clicked == captureButton)
  {
    const auto snapshot = mitk::CrashDumpFacility::CaptureSnapshot(mitk::SnapshotKind::OnDemand);

    QMessageBox resultBox;
    if (snapshot.has_value())
    {
      resultBox.setIcon(QMessageBox::Information);
      resultBox.setTextFormat(Qt::RichText);
      resultBox.setText(QmitkCrashDumpUi::Paragraph("A diagnostic snapshot was saved.") +
        QmitkCrashDumpUi::PrivacyNote());
      resultBox.setDetailedText(QString::fromStdWString(snapshot->wstring()));
      auto *managerButton = resultBox.addButton("Open Diagnostic Data...", QMessageBox::ActionRole);
      QPushButton *reportButton = nullptr;
      if (mitk::IsCrashReportServiceAvailable())
        reportButton = resultBox.addButton("File report...", QMessageBox::ActionRole);
      resultBox.addButton(QMessageBox::Ok);
      resultBox.exec();

      if (resultBox.clickedButton() == managerButton)
      {
        QmitkCrashDumpManagerDialog::ShowManager();
      }
      else if (reportButton != nullptr && resultBox.clickedButton() == reportButton)
      {
        for (const auto &dump : mitk::CrashDumpFacility::ListAllDumps())
        {
          if (dump.Path == *snapshot)
          {
            mitk::FileCrashReport({ dump }, nullptr);
            break;
          }
        }
      }
    }
    else
    {
      resultBox.setIcon(QMessageBox::Warning);
      resultBox.setText("Could not capture a diagnostic snapshot.");
      resultBox.exec();
    }
  }
#endif

  return false;
}

#endif
