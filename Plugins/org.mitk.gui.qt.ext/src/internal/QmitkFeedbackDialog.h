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
#include <map>
#include <vector>

class QAbstractButton;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QTextBrowser;
class QToolButton;
class QVBoxLayout;
class QmitkCrashDumpListWidget;

/**
 * \brief Lets the user assemble, review, and hand off a feedback report.
 *
 * Every item that may go into the report is listed with its own include
 * choice, and the preview shows the report text that a hand-off emits. A
 * hand-off writes the bundle to a temporary folder and opens the user's
 * mail program, web browser, or file manager; the dialog itself sends
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

    /** The crash dump whose session wrote this log, empty for the log of
     *  this session. Such a log is offered only while its dump is included. */
    std::filesystem::path dump;
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
    /** Offered for inclusion; their session logs are among the log sources. */
    std::vector<mitk::CrashDumpInfo> dumps;

    /** Whether the dumps start included, as when the report was filed for
     *  them, or are left for the user to pick. */
    bool dumpsIncluded = true;
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

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  struct LogRow
  {
    QWidget* row = nullptr;
    std::filesystem::path dump;
    QCheckBox* include = nullptr;
    QPlainTextEdit* text = nullptr;
    QLabel* summary = nullptr;
    QString originalText;
    QString originalSummary;
  };

  QWidget* CreateHeader();
  QWidget* CreateForm();
  QWidget* CreatePreview();
  void SetPreviewVisible(bool visible);
  QWidget* CreateFooter();

  /** A row of the list of included items: \p title (a check box or a
   *  label) with \p summary below it, an optional \p trailing widget, and,
   *  if given, \p details that a Details button shows and hides. */
  QWidget* CreateItemRow(QWidget* title, QWidget* summary, QWidget* trailing, QWidget* details,
    bool detailsShown = false);

  /** A framed panel with a check box for \p statement and an explanatory
   *  \p note below it. Clicking the statement toggles the check box. */
  QWidget* CreateConfirmation(QCheckBox* checkBox, const QString& statement, const QString& note);

  /** \p checkBox (without text of its own) followed by \p text, which wraps
   *  where a check box text would widen the form. */
  QWidget* CreateWrappingCheckBox(QCheckBox* checkBox, const QString& text);

  /** A word-wrapped label that clicks \p button when clicked. */
  QLabel* CreateClickableLabel(const QString& text, QAbstractButton* button);

  void AddEnvironmentRow(QVBoxLayout* layout);
  void AddLogRows(QVBoxLayout* layout);
  void AddScreenshotRow(QVBoxLayout* layout);
  void AddDumpsRow(QVBoxLayout* layout);
  void AddImagesRow(QVBoxLayout* layout);

  /** Exactly what the preview shows and a hand-off emits. */
  mitk::FeedbackReport AssembleReport() const;

  void OnReportChanged();
  void OnPhiItemIncluded(bool included);
  void OnPublicItemChanged();
  void OnDumpsChanged();
  void OnEnvironmentClicked();
  void OnAddImages();
  void OnRemoveImage(const QString& path);

  /** The dumps the report includes: checked in the list while the dumps
   *  row is checked. */
  std::vector<std::filesystem::path> GetIncludedDumps() const;

  /** Shows the dump list and the logs of the included dumps only. */
  void UpdateDumpRows();

  void UpdateHeaderIcon();
  void UpdateImageList();

  void UpdateSummaries();
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
  void OnShowBundle();

  /** Tells the user that \p application could not be started and offers to
   *  show the bundle instead. */
  void OfferShowBundle(const QString& application, const QString& zipPath);

  Input m_Input;

  QLabel* m_HeaderIcon = nullptr;
  QButtonGroup* m_CategoryGroup = nullptr;
  QPlainTextEdit* m_Description = nullptr;

  QCheckBox* m_EnvironmentInclude = nullptr;
  QLabel* m_EnvironmentSummary = nullptr;
  std::vector<std::pair<QCheckBox*, mitk::FeedbackItem>> m_EnvironmentItems;

  std::vector<LogRow> m_LogRows;

  struct UserImage
  {
    QString path;
    QPixmap thumbnail;
  };

  /** Image files the user added, all included. */
  std::vector<UserImage> m_Images;
  QLabel* m_ImagesSummary = nullptr;
  QVBoxLayout* m_ImageList = nullptr;
  QString m_ImageDirectory;

  QCheckBox* m_ScreenshotInclude = nullptr;
  QComboBox* m_ScreenshotTarget = nullptr;
  QLabel* m_ScreenshotPreview = nullptr;
  mutable std::vector<unsigned char> m_WindowPng;
  mutable std::vector<unsigned char> m_EditorPng;

  QCheckBox* m_DumpsInclude = nullptr;
  QWidget* m_DumpDetails = nullptr;
  QmitkCrashDumpListWidget* m_DumpList = nullptr;
  QLabel* m_DumpSummary = nullptr;
  QCheckBox* m_DeleteDumps = nullptr;
  std::size_t m_IncludedDumpCount = 0;

  QWidget* m_SharingPanel = nullptr;
  QCheckBox* m_SharingConfirmation = nullptr;
  QWidget* m_PublicPanel = nullptr;
  QCheckBox* m_PublicConfirmation = nullptr;

  /** Labels that toggle a check box when clicked. */
  std::map<QObject*, QAbstractButton*> m_ClickableLabels;

  QWidget* m_PreviewPane = nullptr;
  QTextBrowser* m_Preview = nullptr;
  QToolButton* m_PreviewToggle = nullptr;

  QProgressBar* m_BusyIndicator = nullptr;
  QLabel* m_Status = nullptr;
  QLabel* m_GitHubReason = nullptr;
  QPushButton* m_EmailButton = nullptr;
  QPushButton* m_IssueButton = nullptr;
  QPushButton* m_ShowBundleButton = nullptr;
  QPushButton* m_CloseButton = nullptr;

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
};

#endif
