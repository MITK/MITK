/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFeedbackDialog_h
#define QmitkFeedbackDialog_h

#include <mitkEnvironmentInfo.h>
#include <mitkFeedbackReport.h>

#ifdef MITK_HAS_CRASHHANDLING
#include <mitkCrashDumpFacility.h>
#endif

#include <QDialog>
#include <QFutureWatcher>
#include <QPixmap>

#include <filesystem>
#include <functional>
#include <memory>
#include <vector>

class QCheckBox;
class QmitkCrashDumpListWidget;
class QLabel;
class QPlainTextEdit;

namespace Ui
{
  class QmitkFeedbackDialog;
}

/**
 * \brief Lets the user assemble, review, and hand off a feedback report.
 *
 * Every item that may go into the report is shown with its own include
 * choice, and the preview shows the exact report text that a hand-off
 * emits. A hand-off writes the bundle to a temporary folder and opens the
 * user's browser, mail program, or file manager; the dialog itself sends
 * nothing.
 */
class QmitkFeedbackDialog : public QDialog
{
  Q_OBJECT

public:
  /** \brief A log excerpt the user may include. */
  struct LogSource
  {
    /** Initially not included; text starts with a line naming the source. */
    mitk::FeedbackLog log;

    /** Shown instead of the text when there is no log to offer. */
    QString unavailableReason;
  };

  /** \brief Everything gathered before the dialog is shown. */
  struct Input
  {
    mitk::EnvironmentInfo environment;
    std::vector<LogSource> logs;

    /** Captured before the dialog is shown, so it never shows the dialog. */
    QPixmap windowShot;

    /** Null when no multi-widget editor is active. */
    QPixmap editorShot;

#ifdef MITK_HAS_CRASHHANDLING
    /** Offered for inclusion, all included initially; their session logs
     *  are among the log sources. */
    std::vector<mitk::CrashDumpInfo> dumps;
#endif
  };

  explicit QmitkFeedbackDialog(const Input& input, QWidget* parent = nullptr);
  ~QmitkFeedbackDialog() override;

  void done(int result) override;

  /** \brief The crash dumps in the last successfully written bundle if the
   *  user asked to delete them, else none. Deleting is left to the caller
   *  after the dialog has closed, because later hand-offs from the same
   *  dialog may write the bundle again from the same dumps. */
  std::vector<std::filesystem::path> GetDumpsToDelete() const;

private:
  struct LogSourceWidgets
  {
    QCheckBox* include = nullptr;
    QPlainTextEdit* text = nullptr;
    QLabel* editedLabel = nullptr;
    QString originalText;
  };

  void AddEnvironmentItems(const mitk::EnvironmentInfo& environment);
  void AddLogSources();
  void SetUpScreenshot();
  void SetUpDumps();

  /** Exactly what the preview shows and a hand-off emits. */
  mitk::FeedbackReport AssembleReport() const;

  void OnReportChanged();
  void OnPhiItemIncluded(bool included);
  void OnPublicItemChanged();
  void OnDumpsChanged();

  void UpdatePreview();
  void UpdateConfirmations(const mitk::FeedbackReport& report);
  void UpdateHandoffEnabled(const mitk::FeedbackReport& report);
  void UpdateScreenshotPreview();

  using Handoff = std::function<void(const QString& zipPath, const mitk::FeedbackReport& report)>;

  /** Writes the bundle off the UI thread (a long write would look like a
   *  freeze to the UI-freeze watchdog), then runs \p handoff. A bundle
   *  written for the unchanged report is reused. */
  void WriteBundleThen(Handoff handoff);
  void OnBundleWritten();

  void OnOpenIssue();
  void OnOpenEmail();
  void OnRevealBundle();

  /** Tells the user that \p application could not be started and offers to
   *  reveal the bundle instead. */
  void OfferReveal(const QString& application, const QString& zipPath);

  std::unique_ptr<Ui::QmitkFeedbackDialog> m_Controls;
  Input m_Input;

  std::vector<std::pair<QCheckBox*, mitk::FeedbackItem>> m_EnvironmentItems;
  std::vector<LogSourceWidgets> m_LogWidgets;
  mutable std::vector<unsigned char> m_WindowPng;
  mutable std::vector<unsigned char> m_EditorPng;

  /** Incremented on every change of the report; a bundle is reusable only
   *  for the generation it was written for. */
  unsigned int m_Generation = 0;
  unsigned int m_BundleGeneration = 0;
  QString m_BundlePath;
  std::vector<std::filesystem::path> m_BundleAttachments;

  QFutureWatcher<QString> m_BundleWatcher;
  QString m_PendingZipPath;
  unsigned int m_PendingGeneration = 0;
  mitk::FeedbackReport m_PendingReport;
  Handoff m_PendingHandoff;

  QmitkCrashDumpListWidget* m_DumpList = nullptr;
  QCheckBox* m_DeleteDumpsCheckBox = nullptr;
  std::size_t m_IncludedDumpCount = 0;
};

#endif
