/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFeedbackDialog.h"

#include <ui_QmitkFeedbackDialog.h>

#include <mitkException.h>
#include <mitkFeedbackBundle.h>
#include <mitkFeedbackHandoff.h>

#include <algorithm>

#include <QBuffer>
#include <QCheckBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrentRun>

namespace
{
  // GitHub's attachment limit for issues; many mail servers reject larger
  // messages as well.
  constexpr qint64 LargeBundleSize = 25 * 1024 * 1024;

  std::vector<unsigned char> EncodePng(const QPixmap& pixmap)
  {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");

    return std::vector<unsigned char>(bytes.begin(), bytes.end());
  }

  /** \p value percent-encoded for a URL query. QUrlQuery leaves "+" as is,
   *  which GitHub reads as a space. */
  QByteArray EncodeQueryValue(const std::string& value)
  {
    return QUrl::toPercentEncoding(QString::fromStdString(value));
  }

  QString WriteBundle(const mitk::FeedbackReport& report, const QString& zipPath)
  {
    // Errors are returned as values: QtConcurrent rethrows only QException
    // types faithfully.
    try
    {
      mitk::WriteFeedbackBundle(report, zipPath.toStdU16String());
      return {};
    }
    catch (const mitk::Exception& e)
    {
      return QString::fromStdString(e.GetDescription());
    }
    catch (const std::exception& e)
    {
      return QString::fromStdString(e.what());
    }
  }
}

QmitkFeedbackDialog::QmitkFeedbackDialog(const Input& input, QWidget* parent)
  : QDialog(parent),
    m_Controls(std::make_unique<Ui::QmitkFeedbackDialog>()),
    m_Input(input)
{
  m_Controls->setupUi(this);
  m_Controls->splitter->setStretchFactor(0, 3);
  m_Controls->splitter->setStretchFactor(1, 2);

  m_Controls->sharingConfirmationLabel->setText(QString::fromStdString(mitk::GetSharingConfirmationStatement()));
  m_Controls->publicSharingConfirmationLabel->setText(
    QString::fromStdString(mitk::GetPublicSharingConfirmationStatement()));

  for (const auto category : { mitk::FeedbackCategory::Bug, mitk::FeedbackCategory::Feature, mitk::FeedbackCategory::Idea })
    m_Controls->categoryComboBox->addItem(QString::fromStdString(mitk::ToString(category)), static_cast<int>(category));

  this->AddEnvironmentItems(m_Input.environment);
  this->AddLogSources();
  this->SetUpScreenshot();

  connect(m_Controls->categoryComboBox, &QComboBox::currentIndexChanged, this, &QmitkFeedbackDialog::OnReportChanged);
  connect(m_Controls->descriptionEdit, &QPlainTextEdit::textChanged, this, &QmitkFeedbackDialog::OnReportChanged);
  connect(m_Controls->sharingConfirmationCheckBox, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnReportChanged);
  connect(m_Controls->publicSharingConfirmationCheckBox, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnReportChanged);

  connect(m_Controls->logToggleButton, &QToolButton::toggled, this, [this](bool expanded) {
    m_Controls->logTabs->setVisible(expanded);
    m_Controls->logToggleButton->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
  });

  connect(m_Controls->openIssueButton, &QPushButton::clicked, this, &QmitkFeedbackDialog::OnOpenIssue);
  connect(m_Controls->openEmailButton, &QPushButton::clicked, this, &QmitkFeedbackDialog::OnOpenEmail);
  connect(m_Controls->revealBundleButton, &QPushButton::clicked, this, &QmitkFeedbackDialog::OnRevealBundle);
  connect(m_Controls->closeButton, &QPushButton::clicked, this, &QDialog::reject);
  connect(&m_BundleWatcher, &QFutureWatcher<QString>::finished, this, &QmitkFeedbackDialog::OnBundleWritten);

  this->OnReportChanged();
}

QmitkFeedbackDialog::~QmitkFeedbackDialog() = default;

void QmitkFeedbackDialog::done(int result)
{
  // The bundle being written may be the one a later hand-off or the caller
  // relies on, so the dialog stays open until it is complete.
  if (m_BundleWatcher.isRunning())
    return;

  QDialog::done(result);
}

void QmitkFeedbackDialog::AddEnvironmentItems(const mitk::EnvironmentInfo& environment)
{
  auto* layout = m_Controls->environmentLayout;
  layout->setColumnStretch(1, 1);

  int row = 0;

  for (const auto& [label, value] : mitk::ToLabeledFields(environment))
  {
    auto* checkBox = new QCheckBox(QString::fromStdString(label));
    checkBox->setChecked(true);

    auto* valueLabel = new QLabel(QString::fromStdString(value));
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueLabel->setWordWrap(true);

    layout->addWidget(checkBox, row, 0, Qt::AlignTop);
    layout->addWidget(valueLabel, row, 1);
    ++row;

    m_EnvironmentItems.emplace_back(checkBox, mitk::FeedbackItem{ label, value, true });
    connect(checkBox, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnReportChanged);
  }
}

void QmitkFeedbackDialog::AddLogSources()
{
  for (const auto& source : m_Input.logs)
  {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);

    LogSourceWidgets widgets;

    if (!source.unavailableReason.isEmpty())
    {
      auto* reason = new QLabel(source.unavailableReason);
      reason->setWordWrap(true);
      layout->addWidget(reason);
      layout->addStretch(1);
    }
    else
    {
      widgets.include = new QCheckBox("Include this log excerpt");
      widgets.editedLabel = new QLabel("<i>edited</i>");
      widgets.editedLabel->setVisible(false);

      auto* header = new QHBoxLayout;
      header->addWidget(widgets.include);
      header->addStretch(1);
      header->addWidget(widgets.editedLabel);
      layout->addLayout(header);

      widgets.text = new QPlainTextEdit(QString::fromStdString(source.log.text));
      widgets.text->setLineWrapMode(QPlainTextEdit::NoWrap);
      widgets.text->setMinimumHeight(200);
      widgets.text->setToolTip("Remove or change anything you do not want to share.");
      widgets.originalText = widgets.text->toPlainText();
      layout->addWidget(widgets.text);

      connect(widgets.include, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnPhiItemIncluded);
      connect(widgets.include, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnPublicItemChanged);
      connect(widgets.text, &QPlainTextEdit::textChanged, this, &QmitkFeedbackDialog::OnPublicItemChanged);
    }

    m_LogWidgets.push_back(widgets);
    m_Controls->logTabs->addTab(page, QString::fromStdString(source.log.sourceLabel));
  }

  m_Controls->logGroup->setVisible(!m_Input.logs.empty());
}

void QmitkFeedbackDialog::SetUpScreenshot()
{
  m_Controls->screenshotTargetComboBox->addItem(
    QString::fromStdString(mitk::ToString(mitk::ScreenshotTarget::ApplicationWindow)).toLower(),
    static_cast<int>(mitk::ScreenshotTarget::ApplicationWindow));

  if (!m_Input.editorShot.isNull())
  {
    m_Controls->screenshotTargetComboBox->addItem(
      QString::fromStdString(mitk::ToString(mitk::ScreenshotTarget::EditorArea)).toLower(),
      static_cast<int>(mitk::ScreenshotTarget::EditorArea));
  }

  m_Controls->screenshotGroup->setVisible(!m_Input.windowShot.isNull());

  connect(m_Controls->screenshotCheckBox, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnPhiItemIncluded);
  connect(m_Controls->screenshotCheckBox, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnPublicItemChanged);
  connect(m_Controls->screenshotTargetComboBox, &QComboBox::currentIndexChanged, this,
    &QmitkFeedbackDialog::OnPublicItemChanged);
}

mitk::FeedbackReport QmitkFeedbackDialog::AssembleReport() const
{
  mitk::FeedbackReport report;
  report.category = static_cast<mitk::FeedbackCategory>(m_Controls->categoryComboBox->currentData().toInt());
  report.description = m_Controls->descriptionEdit->toPlainText().trimmed().toStdString();

  for (const auto& [checkBox, item] : m_EnvironmentItems)
  {
    report.environment.push_back(item);
    report.environment.back().included = checkBox->isChecked();
  }

  for (std::size_t i = 0; i < m_LogWidgets.size(); ++i)
  {
    const auto& widgets = m_LogWidgets[i];

    if (widgets.text == nullptr)
      continue;

    const auto text = widgets.text->toPlainText();

    auto log = m_Input.logs[i].log;
    log.text = text.toStdString();

    if (!log.text.empty() && log.text.back() != '\n')
      log.text += '\n';

    log.included = widgets.include->isChecked();
    log.edited = text != widgets.originalText;
    report.logs.push_back(log);
  }

  if (!m_Input.windowShot.isNull() && m_Controls->screenshotCheckBox->isChecked())
  {
    report.includeScreenshot = true;
    report.screenshotTarget =
      static_cast<mitk::ScreenshotTarget>(m_Controls->screenshotTargetComboBox->currentData().toInt());

    // Encoded on first use: a full-window PNG takes long enough to be felt
    // on every keystroke.
    if (report.screenshotTarget == mitk::ScreenshotTarget::EditorArea)
    {
      if (m_EditorPng.empty())
        m_EditorPng = EncodePng(m_Input.editorShot);

      report.screenshotPng = m_EditorPng;
    }
    else
    {
      if (m_WindowPng.empty())
        m_WindowPng = EncodePng(m_Input.windowShot);

      report.screenshotPng = m_WindowPng;
    }
  }

  report.sharingConfirmed = m_Controls->sharingConfirmationCheckBox->isChecked();
  report.publicSharingConfirmed = m_Controls->publicSharingConfirmationCheckBox->isChecked();

  return report;
}

void QmitkFeedbackDialog::OnReportChanged()
{
  ++m_Generation;

  for (const auto& widgets : m_LogWidgets)
  {
    if (widgets.editedLabel != nullptr)
      widgets.editedLabel->setVisible(widgets.text->toPlainText() != widgets.originalText);
  }

  const auto report = this->AssembleReport();

  m_Controls->previewEdit->setPlainText(QString::fromStdString(mitk::BuildReportText(report)));
  this->UpdateConfirmations(report);
  this->UpdateHandoffEnabled(report);
  this->UpdateScreenshotPreview();
}

void QmitkFeedbackDialog::OnPhiItemIncluded(bool included)
{
  // A confirmation covers what was included when it was given.
  if (included)
    m_Controls->sharingConfirmationCheckBox->setChecked(false);

  this->OnReportChanged();
}

void QmitkFeedbackDialog::OnPublicItemChanged()
{
  m_Controls->publicSharingConfirmationCheckBox->setChecked(false);
  this->OnReportChanged();
}

void QmitkFeedbackDialog::UpdateConfirmations(const mitk::FeedbackReport& report)
{
  m_Controls->sharingConfirmationWidget->setVisible(report.NeedsSharingConfirmation());

  const bool hasAttachment = std::any_of(report.attachments.begin(), report.attachments.end(),
    [](const auto& attachment) { return attachment.included; });
  const bool hasLog = std::any_of(report.logs.begin(), report.logs.end(), [](const auto& log) { return log.included; });

  m_Controls->publicSharingConfirmationWidget->setVisible(!hasAttachment && (hasLog || report.includeScreenshot));
}

void QmitkFeedbackDialog::UpdateHandoffEnabled(const mitk::FeedbackReport& report)
{
  const bool writing = m_BundleWatcher.isRunning();
  const bool ready = !writing && !report.description.empty() &&
    (!report.NeedsSharingConfirmation() || report.sharingConfirmed);

  m_Controls->openEmailButton->setEnabled(ready);
  m_Controls->revealBundleButton->setEnabled(ready);
  m_Controls->openIssueButton->setEnabled(ready && report.CanUsePublicTracker());
  m_Controls->closeButton->setEnabled(!writing);

  QString reason;

  if (!report.CanUsePublicTracker())
  {
    const bool hasAttachment = std::any_of(report.attachments.begin(), report.attachments.end(),
      [](const auto& attachment) { return attachment.included; });

    reason = hasAttachment
      ? QString("Reports with crash dumps are sent by email.")
      : QString("Confirm that the included log or screenshot may be published, or send by email.");
  }

  m_Controls->gitHubReasonLabel->setText(reason);
}

void QmitkFeedbackDialog::UpdateScreenshotPreview()
{
  const bool show = m_Controls->screenshotCheckBox->isChecked();
  m_Controls->screenshotPreviewLabel->setVisible(show);

  if (!show)
    return;

  const auto target = static_cast<mitk::ScreenshotTarget>(m_Controls->screenshotTargetComboBox->currentData().toInt());
  const auto& shot = target == mitk::ScreenshotTarget::EditorArea ? m_Input.editorShot : m_Input.windowShot;

  m_Controls->screenshotPreviewLabel->setPixmap(
    shot.scaled(QSize(360, 240), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void QmitkFeedbackDialog::WriteBundleThen(Handoff handoff)
{
  if (!m_BundlePath.isEmpty() && m_BundleGeneration == m_Generation && QFileInfo::exists(m_BundlePath))
  {
    handoff(m_BundlePath, this->AssembleReport());
    return;
  }

  // Not removed automatically: the bundle has to survive until the user has
  // attached or sent it.
  QTemporaryDir directory(QDir::temp().filePath("mitk-feedback-XXXXXX"));
  directory.setAutoRemove(false);

  if (!directory.isValid())
  {
    QMessageBox::warning(this, this->windowTitle(),
      "The report could not be saved: no temporary folder could be created.\n\n" + directory.errorString());
    return;
  }

  const auto fileName = "mitk-report-" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".zip";

  m_PendingZipPath = directory.filePath(fileName);
  m_PendingGeneration = m_Generation;
  m_PendingReport = this->AssembleReport();
  m_PendingHandoff = std::move(handoff);

  m_Controls->bundleProgressBar->setVisible(true);
  m_Controls->bundleStatusLabel->setText("Saving the report bundle...");

  m_BundleWatcher.setFuture(QtConcurrent::run(WriteBundle, m_PendingReport, m_PendingZipPath));
  this->UpdateHandoffEnabled(m_PendingReport);
}

void QmitkFeedbackDialog::OnBundleWritten()
{
  m_Controls->bundleProgressBar->setVisible(false);

  const auto error = m_BundleWatcher.result();
  const auto handoff = std::move(m_PendingHandoff);
  m_PendingHandoff = nullptr;

  this->UpdateHandoffEnabled(this->AssembleReport());

  if (!error.isEmpty())
  {
    QDir(QFileInfo(m_PendingZipPath).path()).removeRecursively();
    m_Controls->bundleStatusLabel->setText("The report bundle could not be saved.");
    QMessageBox::warning(this, this->windowTitle(), "The report bundle could not be saved.\n\n" + error);
    return;
  }

  m_BundlePath = m_PendingZipPath;
  m_BundleGeneration = m_PendingGeneration;

  const auto size = QFileInfo(m_BundlePath).size();
  auto status = QString("Saved <b>%1</b> (%2).")
    .arg(QFileInfo(m_BundlePath).fileName().toHtmlEscaped(), QLocale().formattedDataSize(size));

  if (size > LargeBundleSize)
  {
    status += " The bundle is larger than 25 MB: GitHub and many mail servers reject attachments that large."
              " Reveal it to pass it on another way.";
  }

  m_Controls->bundleStatusLabel->setText(status);

  handoff(m_BundlePath, m_PendingReport);
}

void QmitkFeedbackDialog::OnOpenIssue()
{
  this->WriteBundleThen([this](const QString& zipPath, const mitk::FeedbackReport& report) {
    mitk::GitHubIssueFields fields;

    try
    {
      fields = mitk::BuildGitHubIssueFields(report, QFileInfo(zipPath).fileName().toStdString());
    }
    catch (const mitk::Exception& e)
    {
      QMessageBox::warning(this, this->windowTitle(), QString::fromStdString(e.GetDescription()));
      return;
    }

    const auto url = QUrl::fromEncoded("https://github.com/MITK/MITK/issues/new?title=" + EncodeQueryValue(fields.title) +
      "&body=" + EncodeQueryValue(fields.body) + "&labels=" + EncodeQueryValue(fields.labels));

    if (!QDesktopServices::openUrl(url))
      this->OfferReveal("your web browser", zipPath);
  });
}

void QmitkFeedbackDialog::OnOpenEmail()
{
  this->WriteBundleThen([this](const QString& zipPath, const mitk::FeedbackReport& report) {
    const auto fields = mitk::BuildMailFields(report, QFileInfo(zipPath).fileName().toStdString());

    const auto url = QUrl::fromEncoded("mailto:" + EncodeQueryValue(fields.recipient) + "?subject=" +
      EncodeQueryValue(fields.subject) + "&body=" + EncodeQueryValue(fields.body));

    if (!QDesktopServices::openUrl(url))
      this->OfferReveal("your mail program", zipPath);
  });
}

void QmitkFeedbackDialog::OnRevealBundle()
{
  this->WriteBundleThen([](const QString& zipPath, const mitk::FeedbackReport&) {
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(zipPath).path()));
  });
}

void QmitkFeedbackDialog::OfferReveal(const QString& application, const QString& zipPath)
{
  QMessageBox box(QMessageBox::Warning, this->windowTitle(),
    QString("MITK could not start %1. The report bundle %2 has been saved; you can send it in another way.")
      .arg(application, QFileInfo(zipPath).fileName()),
    QMessageBox::Close, this);

  auto* revealButton = box.addButton("Reveal bundle", QMessageBox::AcceptRole);
  box.exec();

  if (box.clickedButton() == revealButton)
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(zipPath).path()));
}
