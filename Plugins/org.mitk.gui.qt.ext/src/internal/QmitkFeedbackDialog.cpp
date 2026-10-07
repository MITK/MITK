/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFeedbackDialog.h"

#include "QmitkFeedbackInput.h"

#ifdef MITK_HAS_CRASHHANDLING
#include <QmitkCrashDumpListWidget.h>
#endif

#include <QmitkIconTheme.h>

#include <mitkException.h>
#include <mitkFeedbackBundle.h>
#include <mitkFeedbackHandoff.h>

#include <QBuffer>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFrame>
#include <QGuiApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QScreen>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrentRun>

#include <algorithm>

namespace
{
  // GitHub's attachment limit for issues; many mail servers reject larger
  // messages as well.
  constexpr qint64 LargeBundleSize = 25 * 1024 * 1024;

  constexpr int IconSize = 40;

  // Below this dialog width, form and preview side by side are both cramped.
  constexpr int MinimumWidthWithPreview = 1100;

  // Check boxes are indented by this much in rows, so that summaries line
  // up with the check box text.
  constexpr int SummaryIndent = 24;

  // The themes style widgets through the application style sheet only, so
  // the palette cannot tell light from dark. These match the themes' text
  // colors at reduced emphasis and their "font.warning" class.
  QString MutedColor()
  {
    return QmitkIconTheme::IsDarkTheme() ? "#a6a6a6" : "#5f5f5f";
  }

  QString WarningColor()
  {
    return QmitkIconTheme::IsDarkTheme() ? "#ff5c33" : "#d00000";
  }

  QLabel* CreateMutedLabel(const QString& text = QString())
  {
    auto* label = new QLabel(text);
    label->setWordWrap(true);
    label->setStyleSheet(QString("color: %1;").arg(MutedColor()));
    return label;
  }

  /** A heading in the text rather than a larger widget font, which the
   *  application's style sheet would override. */
  QLabel* CreateHeading(const QString& text)
  {
    auto* label = new QLabel(QString("<b>%1</b>").arg(text.toHtmlEscaped()));
    label->setTextFormat(Qt::RichText);
    return label;
  }

  QFrame* CreateDivider()
  {
    auto* divider = new QFrame;
    divider->setFixedHeight(1);
    divider->setStyleSheet("background-color: rgba(128, 128, 128, 70); border: none;");
    return divider;
  }

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

  /** Markdown headings come out several sizes larger than the text, which
   *  in a narrow pane leaves little room for the content; a slight
   *  increase over the body text keeps them distinct. */
  void ScaleHeadings(QTextDocument* document, qreal bodyPointSize)
  {
    for (auto block = document->begin(); block.isValid(); block = block.next())
    {
      const auto level = block.blockFormat().headingLevel();

      if (level == 0)
        continue;

      QTextCursor cursor(block);
      cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

      QTextCharFormat format;
      format.setProperty(QTextFormat::FontSizeAdjustment, 0);
      format.setFontPointSize(bodyPointSize * (level == 1 ? 1.3 : 1.1));
      format.setFontWeight(QFont::Bold);
      cursor.mergeCharFormat(format);

      auto blockFormat = block.blockFormat();
      blockFormat.setTopMargin(level == 1 ? 0 : 10);
      blockFormat.setBottomMargin(4);
      cursor.setBlockFormat(blockFormat);
    }
  }

  /** "The last 1000 lines" or "All 345 lines", taken from the first line
   *  that QmitkReadFeedbackLog() puts in front of every log excerpt. */
  QString LogSummary(const QString& text)
  {
    const auto firstLine = text.section('\n', 0, 0);

    if (!firstLine.startsWith('['))
      return {};

    auto summary = firstLine.mid(1).section(" of the log", 0, 0);
    return summary.isEmpty() ? summary : summary.at(0).toUpper() + summary.mid(1);
  }
}

QmitkFeedbackDialog::QmitkFeedbackDialog(const Input& input, QWidget* parent)
  : QDialog(parent),
    m_Input(input)
{
  this->setWindowTitle("Report a Problem / Send Feedback");

  // Most of a large screen, but not more than the content can use; the
  // preview starts hidden where the form would otherwise be cramped.
  const auto* screen = parent != nullptr ? parent->screen() : QGuiApplication::primaryScreen();
  const auto available = screen != nullptr ? screen->availableGeometry() : QRect(0, 0, 1280, 800);
  const int width = std::clamp(static_cast<int>(available.width() * 0.8), std::min(720, available.width()), 1500);
  const int height = std::clamp(static_cast<int>(available.height() * 0.85), std::min(560, available.height()), 1050);
  this->resize(width, height);

  auto* splitter = new QSplitter(Qt::Horizontal);
  splitter->setChildrenCollapsible(false);
  splitter->addWidget(this->CreateForm());
  splitter->addWidget(this->CreatePreview());
  splitter->setStretchFactor(0, 3);
  splitter->setStretchFactor(1, 2);
  splitter->setSizes({ width * 3 / 5, width * 2 / 5 });
  this->SetPreviewVisible(width >= MinimumWidthWithPreview);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(this->CreateHeader());
  // A horizontal splitter keeps its preferred height unless it is given the
  // stretch, which would hand the spare height to the footer instead.
  layout->addWidget(splitter, 1);
  layout->addWidget(CreateDivider());
  layout->addWidget(this->CreateFooter());

  connect(&m_BundleWatcher, &QFutureWatcher<QString>::finished, this, &QmitkFeedbackDialog::OnBundleWritten);

  this->UpdateHeaderIcon();
  this->UpdateImageList();
  m_IncludedDumpCount = this->GetIncludedDumps().size();
  this->UpdateDumpRows();
  this->OnReportChanged();
  m_Description->setFocus();
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

std::vector<std::filesystem::path> QmitkFeedbackDialog::GetDumpsToDelete() const
{
  if (m_DeleteDumps == nullptr || !m_DeleteDumps->isChecked())
    return {};

  return m_BundleAttachments;
}

bool QmitkFeedbackDialog::eventFilter(QObject* watched, QEvent* event)
{
  if (event->type() == QEvent::MouseButtonRelease)
  {
    if (const auto clickable = m_ClickTargets.find(watched); clickable != m_ClickTargets.end())
    {
      if (clickable->second->isEnabled())
        clickable->second->click();

      return true;
    }
  }

  return QDialog::eventFilter(watched, event);
}

QWidget* QmitkFeedbackDialog::CreateHeader()
{
  auto* header = new QFrame;
  header->setObjectName("feedbackHeader");
  header->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  // Translucent grey instead of a palette color reads as a band on the
  // light and the dark theme; the accent bar matches the crash dump dialogs.
  header->setStyleSheet(QString(
    "QFrame#feedbackHeader { background-color: rgba(128, 128, 128, 28); border: none; "
    "border-left: 4px solid %1; border-bottom: 1px solid rgba(128, 128, 128, 90); }")
    .arg(QmitkIconTheme::GetAccentColor()));

  m_HeaderIcon = new QLabel;
  m_HeaderIcon->setFixedSize(IconSize, IconSize);

  auto* title = new QLabel("<h3 style=\"margin: 0;\">Report a problem or share an idea</h3>");
  title->setTextFormat(Qt::RichText);

  auto* subtitle = new QLabel(
    "Nothing leaves your computer until you send it yourself. Choose what to include, then how to send it.");
  subtitle->setWordWrap(true);

  auto* text = new QVBoxLayout;
  text->setSpacing(2);
  text->addWidget(title);
  text->addWidget(subtitle);

  auto* layout = new QHBoxLayout(header);
  layout->setContentsMargins(14, 12, 16, 12);
  layout->setSpacing(14);
  layout->addWidget(m_HeaderIcon, 0, Qt::AlignVCenter);
  layout->addLayout(text, 1);

  return header;
}

QWidget* QmitkFeedbackDialog::CreateForm()
{
  auto* form = new QWidget;
  auto* layout = new QVBoxLayout(form);
  layout->setContentsMargins(16, 14, 12, 14);
  layout->setSpacing(10);

  m_CategoryGroup = new QButtonGroup(this);
  auto* categories = new QHBoxLayout;
  categories->setSpacing(6);

  for (const auto category : { mitk::FeedbackCategory::Bug, mitk::FeedbackCategory::Feature, mitk::FeedbackCategory::Idea })
  {
    auto* button = new QPushButton(QString::fromStdString(mitk::ToString(category)));
    button->setCheckable(true);
    button->setChecked(category == mitk::FeedbackCategory::Bug);
    m_CategoryGroup->addButton(button, static_cast<int>(category));
    categories->addWidget(button);
  }

  categories->addStretch(1);

  m_PreviewToggle = new QToolButton;
  m_PreviewToggle->setAutoRaise(true);
  m_PreviewToggle->setCursor(Qt::PointingHandCursor);
  connect(m_PreviewToggle, &QToolButton::clicked, this,
    [this] { this->SetPreviewVisible(m_PreviewPane->isHidden()); });
  categories->addWidget(m_PreviewToggle);

  connect(m_CategoryGroup, &QButtonGroup::idClicked, this, &QmitkFeedbackDialog::UpdateHeaderIcon);
  connect(m_CategoryGroup, &QButtonGroup::idClicked, this, &QmitkFeedbackDialog::OnReportChanged);

  m_Description = new QPlainTextEdit;
  m_Description->setPlaceholderText(
    "What happened, and what did you expect instead? How can we reproduce it? Or: what would you like MITK to do?");
  m_Description->setMinimumHeight(90);
  m_Description->setMaximumHeight(120);
  connect(m_Description, &QPlainTextEdit::textChanged, this, &QmitkFeedbackDialog::OnReportChanged);

  auto* items = new QVBoxLayout;
  items->setSpacing(0);
  // In a report filed for crash dumps, they are what the report is about;
  // otherwise they are one more thing the user may add.
#ifdef MITK_HAS_CRASHHANDLING
  const bool dumpsFirst = m_Input.dumpsIncluded;
#else
  const bool dumpsFirst = false;
#endif

  if (dumpsFirst)
    this->AddDumpsRow(items);

  this->AddEnvironmentRow(items);
  this->AddLogRows(items);
  this->AddScreenshotRow(items);
  this->AddImagesRow(items);

  if (!dumpsFirst)
    this->AddDumpsRow(items);

  items->addWidget(CreateDivider());

  m_SharingConfirmation = new QCheckBox;
  m_SharingPanel = this->CreateConfirmation(m_SharingConfirmation,
    QString::fromStdString(mitk::GetSharingConfirmationStatement()),
    "Logs, screenshots, images, and crash dumps can contain patient-related information, such as file paths, names, "
    "image data, or parts of the application's memory.");
  connect(m_SharingConfirmation, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnReportChanged);

  m_PublicConfirmation = new QCheckBox;
  m_PublicPanel = this->CreateConfirmation(m_PublicConfirmation,
    QString::fromStdString(mitk::GetPublicSharingConfirmationStatement()),
    "Needed only to open a GitHub issue. Email goes to the MITK team alone.");
  connect(m_PublicConfirmation, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnReportChanged);

  layout->addLayout(categories);
  layout->addSpacing(4);
  layout->addWidget(CreateHeading("Description"));
  layout->addWidget(m_Description);
  layout->addSpacing(8);
  layout->addWidget(CreateHeading("Included in the report"));
  layout->addLayout(items);
  layout->addWidget(m_SharingPanel);
  layout->addWidget(m_PublicPanel);
  layout->addStretch(1);

  auto* scrollArea = new QScrollArea;
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setWidgetResizable(true);
  scrollArea->setWidget(form);

  // The form scrolls vertically only. A scroll area claims hardly any width
  // of its own, so without this the splitter would squeeze the form below
  // what its rows need and the preview would keep the space.
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setMinimumWidth(form->minimumSizeHint().width() + scrollArea->verticalScrollBar()->sizeHint().width());

  return scrollArea;
}

QWidget* QmitkFeedbackDialog::CreatePreview()
{
  m_Preview = new QTextBrowser;
  m_Preview->setOpenLinks(false);
  m_Preview->setStyleSheet("QTextBrowser { border: 1px solid rgba(128, 128, 128, 90); }");
  m_Preview->document()->setIndentWidth(18);
  m_Preview->document()->setDocumentMargin(10);

  auto* note = CreateMutedLabel("Attached files are listed, not shown. Their contents are what you included on the left.");

  m_PreviewPane = new QWidget;
  auto* layout = new QVBoxLayout(m_PreviewPane);
  layout->setContentsMargins(12, 14, 16, 14);
  layout->setSpacing(8);
  layout->addWidget(CreateHeading("Report preview"));
  layout->addWidget(m_Preview, 1);
  layout->addWidget(note);

  return m_PreviewPane;
}

void QmitkFeedbackDialog::SetPreviewVisible(bool visible)
{
  m_PreviewPane->setVisible(visible);
  m_PreviewToggle->setText(visible ? "Hide preview" : "Show preview");
}

QWidget* QmitkFeedbackDialog::CreateFooter()
{
  m_BusyIndicator = new QProgressBar;
  m_BusyIndicator->setRange(0, 0);
  m_BusyIndicator->setTextVisible(false);
  m_BusyIndicator->setFixedWidth(80);
  m_BusyIndicator->setVisible(false);

  m_Status = new QLabel;
  m_Status->setTextFormat(Qt::RichText);
  m_Status->setWordWrap(true);

  m_GitHubReason = CreateMutedLabel();

  auto* text = new QVBoxLayout;
  text->setSpacing(2);
  text->addWidget(m_Status);
  text->addWidget(m_GitHubReason);

  m_EmailButton = new QPushButton("Email to the MITK team...");
  m_EmailButton->setDefault(true);
  m_EmailButton->setToolTip("Saves the report as a .zip file and opens your mail program. Attach the file before sending.");

  m_IssueButton = new QPushButton("Open GitHub issue...");
  m_IssueButton->setToolTip("Saves the report as a .zip file and opens a prefilled, public GitHub issue in your browser.");

  m_ShowBundleButton = new QPushButton("Show report file");
  m_ShowBundleButton->setToolTip("Saves the report as a .zip file and shows it in your file manager.");

  m_CloseButton = new QPushButton("Close");
  m_CloseButton->setAutoDefault(false);

  connect(m_EmailButton, &QPushButton::clicked, this, &QmitkFeedbackDialog::OnOpenEmail);
  connect(m_IssueButton, &QPushButton::clicked, this, &QmitkFeedbackDialog::OnOpenIssue);
  connect(m_ShowBundleButton, &QPushButton::clicked, this, &QmitkFeedbackDialog::OnShowBundle);
  connect(m_CloseButton, &QPushButton::clicked, this, &QDialog::reject);

  auto* footer = new QWidget;
  auto* layout = new QHBoxLayout(footer);
  layout->setContentsMargins(16, 10, 16, 12);
  layout->setSpacing(8);
  layout->addWidget(m_BusyIndicator, 0, Qt::AlignVCenter);
  layout->addLayout(text, 1);
  layout->addSpacing(12);
  layout->addWidget(m_EmailButton);
  layout->addWidget(m_IssueButton);
  layout->addWidget(m_ShowBundleButton);
  layout->addSpacing(12);
  layout->addWidget(m_CloseButton);

  return footer;
}

QWidget* QmitkFeedbackDialog::CreateItemRow(QWidget* title, QWidget* summary, QWidget* trailing, QWidget* details,
  bool detailsShown)
{
  auto* top = new QHBoxLayout;
  top->setSpacing(8);
  top->addWidget(title);
  top->addStretch(1);

  if (trailing != nullptr)
    top->addWidget(trailing);

  auto* row = new QWidget;
  auto* layout = new QVBoxLayout(row);
  layout->setContentsMargins(0, 8, 0, 8);
  layout->setSpacing(4);
  layout->addLayout(top);

  if (summary != nullptr)
  {
    summary->setContentsMargins(SummaryIndent, 0, 0, 0);
    layout->addWidget(summary);
  }

  if (details != nullptr)
  {
    auto* toggle = new QToolButton;
    toggle->setText(detailsShown ? "Hide details" : "Details");
    toggle->setAutoRaise(true);
    toggle->setCursor(Qt::PointingHandCursor);
    top->addWidget(toggle);

    details->setContentsMargins(SummaryIndent, 4, 0, 0);
    details->setVisible(detailsShown);
    layout->addWidget(details);

    connect(toggle, &QToolButton::clicked, details, [toggle, details] {
      details->setVisible(!details->isVisible());
      toggle->setText(details->isVisible() ? "Hide details" : "Details");
    });
  }

  auto* container = new QWidget;
  auto* containerLayout = new QVBoxLayout(container);
  containerLayout->setContentsMargins(0, 0, 0, 0);
  containerLayout->setSpacing(0);
  containerLayout->addWidget(CreateDivider());
  containerLayout->addWidget(row);

  return container;
}

QWidget* QmitkFeedbackDialog::CreateConfirmation(QCheckBox* checkBox, const QString& statement, const QString& note)
{
  // No tinted background: on a tint, the theme's outline of an unchecked
  // box loses its contrast. The panel as a whole toggles the box, so the
  // user need not aim at it.
  auto* panel = new QFrame;
  panel->setObjectName("feedbackConfirmation");
  panel->setAttribute(Qt::WA_Hover);
  panel->setCursor(Qt::PointingHandCursor);
  panel->setStyleSheet(QString(
    "QFrame#feedbackConfirmation { border: 1px solid rgba(128, 128, 128, 90); border-left: 3px solid %1; }"
    "QFrame#feedbackConfirmation:hover { background-color: rgba(128, 128, 128, 20); }").arg(WarningColor()));
  panel->installEventFilter(this);
  m_ClickTargets[panel] = checkBox;

  checkBox->setStyleSheet("QCheckBox::indicator { width: 18px; height: 18px; }");

  auto* text = new QVBoxLayout;
  text->setSpacing(4);
  text->addWidget(this->CreateClickableLabel(statement, checkBox));
  text->addWidget(CreateMutedLabel(note));

  auto* layout = new QHBoxLayout(panel);
  layout->setContentsMargins(12, 10, 12, 10);
  layout->setSpacing(8);
  layout->addWidget(checkBox, 0, Qt::AlignTop);
  layout->addLayout(text, 1);

  return panel;
}

QWidget* QmitkFeedbackDialog::CreateWrappingCheckBox(QCheckBox* checkBox, const QString& text)
{
  auto* widget = new QWidget;
  auto* layout = new QHBoxLayout(widget);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(8);
  layout->addWidget(checkBox, 0, Qt::AlignTop);
  layout->addWidget(this->CreateClickableLabel(text, checkBox), 1);

  return widget;
}

QLabel* QmitkFeedbackDialog::CreateClickableLabel(const QString& text, QAbstractButton* button)
{
  auto* label = new QLabel(text);
  label->setWordWrap(true);
  label->setCursor(Qt::PointingHandCursor);
  label->installEventFilter(this);
  m_ClickTargets[label] = button;

  return label;
}

void QmitkFeedbackDialog::AddEnvironmentRow(QVBoxLayout* layout)
{
  auto* details = new QWidget;
  auto* grid = new QGridLayout(details);
  grid->setHorizontalSpacing(16);
  grid->setVerticalSpacing(2);
  grid->setColumnStretch(1, 1);

  int row = 0;

  for (const auto& [field, value] : mitk::ToFields(m_Input.environment))
  {
    auto* checkBox = new QCheckBox(QString::fromStdString(mitk::ToString(field)));
    checkBox->setChecked(true);

    auto* valueLabel = new QLabel(QString::fromStdString(value));
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueLabel->setWordWrap(true);

    grid->addWidget(checkBox, row, 0, Qt::AlignTop);
    grid->addWidget(valueLabel, row, 1);
    ++row;

    m_EnvironmentItems.emplace_back(checkBox, mitk::FeedbackItem{ field, value, true });
    connect(checkBox, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnReportChanged);
  }

  m_EnvironmentInclude = new QCheckBox("System information");
  m_EnvironmentInclude->setChecked(true);
  m_EnvironmentSummary = CreateMutedLabel();
  connect(m_EnvironmentInclude, &QCheckBox::clicked, this, &QmitkFeedbackDialog::OnEnvironmentClicked);

  layout->addWidget(this->CreateItemRow(m_EnvironmentInclude, m_EnvironmentSummary, nullptr, details));
}

void QmitkFeedbackDialog::AddLogRows(QVBoxLayout* layout)
{
  for (const auto& source : m_Input.logs)
  {
    LogRow row;
    row.dump = source.dump;

    const auto title = source.log.sourceLabel == "This session"
      ? QString("Log of this session")
      : QString::fromStdString(source.log.sourceLabel);

    row.include = new QCheckBox(title);
    row.include->setObjectName("logInclude");

    if (!source.unavailableReason.isEmpty())
    {
      row.include->setEnabled(false);
      row.summary = CreateMutedLabel(source.unavailableReason);
      row.row = this->CreateItemRow(row.include, row.summary, nullptr, nullptr);
      layout->addWidget(row.row);
      m_LogRows.push_back(row);
      continue;
    }

    row.text = new QPlainTextEdit(QString::fromStdString(source.log.text));
    row.text->setLineWrapMode(QPlainTextEdit::NoWrap);
    row.text->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    row.text->setMinimumHeight(220);
    row.text->setPlaceholderText("The excerpt is empty.");
    row.originalText = row.text->toPlainText();
    row.originalSummary = LogSummary(row.originalText) + ". Open the details to review it, and remove anything you do not want to share.";
    row.summary = CreateMutedLabel(row.originalSummary);

    connect(row.include, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnPhiItemIncluded);
    connect(row.include, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnPublicItemChanged);
    connect(row.text, &QPlainTextEdit::textChanged, this, &QmitkFeedbackDialog::OnPublicItemChanged);

    row.row = this->CreateItemRow(row.include, row.summary, nullptr, row.text);
    layout->addWidget(row.row);
    m_LogRows.push_back(row);
  }
}

void QmitkFeedbackDialog::AddScreenshotRow(QVBoxLayout* layout)
{
  if (m_Input.windowShot.isNull())
    return;

  m_ScreenshotInclude = new QCheckBox("Screenshot of the");
  m_ScreenshotInclude->setObjectName("screenshotInclude");

  m_ScreenshotTarget = new QComboBox;
  m_ScreenshotTarget->addItem("application window", static_cast<int>(mitk::ScreenshotTarget::ApplicationWindow));

  if (!m_Input.editorShot.isNull())
    m_ScreenshotTarget->addItem("display area", static_cast<int>(mitk::ScreenshotTarget::EditorArea));

  auto* title = new QWidget;
  auto* titleLayout = new QHBoxLayout(title);
  titleLayout->setContentsMargins(0, 0, 0, 0);
  titleLayout->setSpacing(6);
  titleLayout->addWidget(m_ScreenshotInclude);
  titleLayout->addWidget(m_ScreenshotTarget);

  m_ScreenshotPreview = new QLabel;
  m_ScreenshotPreview->setVisible(false);

  auto* summary = new QWidget;
  auto* summaryLayout = new QVBoxLayout(summary);
  summaryLayout->setContentsMargins(0, 0, 0, 0);
  summaryLayout->setSpacing(6);
  summaryLayout->addWidget(CreateMutedLabel("Taken just before this dialog opened."));
  summaryLayout->addWidget(m_ScreenshotPreview, 0, Qt::AlignLeft);

  layout->addWidget(this->CreateItemRow(title, summary, nullptr, nullptr));

  connect(m_ScreenshotInclude, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnPhiItemIncluded);
  connect(m_ScreenshotInclude, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnPublicItemChanged);
  connect(m_ScreenshotTarget, &QComboBox::currentIndexChanged, this, &QmitkFeedbackDialog::OnPublicItemChanged);
}

void QmitkFeedbackDialog::AddDumpsRow(QVBoxLayout* layout)
{
#ifdef MITK_HAS_CRASHHANDLING
  if (m_Input.dumps.empty())
    return;

  m_DumpList = new QmitkCrashDumpListWidget;
  m_DumpList->SetDumps(m_Input.dumps);
  m_DumpList->SetCheckable(true);
  m_DumpList->SetAllChecked(m_Input.dumpsIncluded);
  m_DumpList->SetSelectionEnabled(false);
  m_DumpList->SetColumnHidden(QmitkCrashDumpListWidget::StatusColumn, true);
  m_DumpList->FitHeightToContents();

  m_DumpsInclude = new QCheckBox("Crash dumps");
  m_DumpsInclude->setChecked(m_Input.dumpsIncluded);

  m_DeleteDumps = new QCheckBox;
  m_DeleteDumps->setToolTip("Only dumps that are in a saved report file are deleted. "
    "That file in the temporary folder is then their only copy.");

  m_DumpDetails = new QWidget;
  auto* detailsLayout = new QVBoxLayout(m_DumpDetails);
  detailsLayout->setContentsMargins(0, 4, 0, 0);
  detailsLayout->setSpacing(6);
  detailsLayout->addWidget(m_DumpList);
  detailsLayout->addWidget(this->CreateWrappingCheckBox(m_DeleteDumps,
    "Delete the included crash dumps from this computer when I close this dialog"));
  detailsLayout->addWidget(CreateMutedLabel("You can also delete them later under Help > Diagnostic Data."));

  m_DumpSummary = CreateMutedLabel();

  auto* summary = new QWidget;
  auto* summaryLayout = new QVBoxLayout(summary);
  summaryLayout->setContentsMargins(0, 0, 0, 0);
  summaryLayout->setSpacing(4);
  summaryLayout->addWidget(m_DumpSummary);
  summaryLayout->addWidget(m_DumpDetails);

  layout->addWidget(this->CreateItemRow(m_DumpsInclude, summary, nullptr, nullptr));

  connect(m_DumpsInclude, &QCheckBox::toggled, this, &QmitkFeedbackDialog::OnDumpsChanged);
  connect(m_DumpList, &QmitkCrashDumpListWidget::CheckedChanged, this, &QmitkFeedbackDialog::OnDumpsChanged);
#else
  Q_UNUSED(layout)
#endif
}

void QmitkFeedbackDialog::AddImagesRow(QVBoxLayout* layout)
{
  auto* addButton = new QPushButton("Add images...");
  connect(addButton, &QPushButton::clicked, this, &QmitkFeedbackDialog::OnAddImages);

  m_ImagesSummary = CreateMutedLabel();

  auto* summary = new QWidget;
  auto* summaryLayout = new QVBoxLayout(summary);
  summaryLayout->setContentsMargins(0, 0, 0, 0);
  summaryLayout->setSpacing(6);
  summaryLayout->addWidget(m_ImagesSummary);

  m_ImageList = new QVBoxLayout;
  m_ImageList->setSpacing(6);
  summaryLayout->addLayout(m_ImageList);

  layout->addWidget(this->CreateItemRow(new QLabel("Images from files"), summary, addButton, nullptr));
}

void QmitkFeedbackDialog::OnAddImages()
{
  if (m_ImageDirectory.isEmpty())
    m_ImageDirectory = QDir::homePath();

  const auto paths = QFileDialog::getOpenFileNames(this, "Add images", m_ImageDirectory,
    "Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp)");

  if (paths.isEmpty())
    return;

  m_ImageDirectory = QFileInfo(paths.front()).path();

  QStringList unreadable;
  bool added = false;

  for (const auto& path : paths)
  {
    if (std::any_of(m_Images.begin(), m_Images.end(), [&path](const auto& image) { return image.path == path; }))
      continue;

    const QPixmap pixmap(path);

    if (pixmap.isNull())
    {
      unreadable << QFileInfo(path).fileName();
      continue;
    }

    const auto ratio = this->devicePixelRatioF();
    auto thumbnail = pixmap.scaled(QSize(120, 80) * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    thumbnail.setDevicePixelRatio(ratio);

    m_Images.push_back({ path, thumbnail });
    added = true;
  }

  if (!unreadable.isEmpty())
  {
    QMessageBox::warning(this, this->windowTitle(),
      "These files could not be read as images and were not added:\n\n" + unreadable.join('\n'));
  }

  if (!added)
    return;

  this->UpdateImageList();

  // Like any other image, these may show patient data and may be published
  // only once confirmed.
  m_SharingConfirmation->setChecked(false);
  this->OnPublicItemChanged();
}

void QmitkFeedbackDialog::OnRemoveImage(const QString& path)
{
  m_Images.erase(std::remove_if(m_Images.begin(), m_Images.end(), [&path](const auto& image) { return image.path == path; }),
    m_Images.end());

  this->UpdateImageList();
  this->OnPublicItemChanged();
}

void QmitkFeedbackDialog::UpdateImageList()
{
  // Deferred: a row's Remove button may be the sender of the click that got
  // here.
  while (auto* item = m_ImageList->takeAt(0))
  {
    item->widget()->hide();
    item->widget()->deleteLater();
    delete item;
  }

  m_ImagesSummary->setText(m_Images.empty()
    ? QString("Add images of your own, for example a screenshot of something you saw earlier.")
    : QString(m_Images.size() == 1 ? "1 image" : "%1 images").arg(m_Images.size()));

  for (const auto& image : m_Images)
  {
    auto* thumbnail = new QLabel;
    thumbnail->setPixmap(image.thumbnail);
    thumbnail->setFixedSize(image.thumbnail.deviceIndependentSize().toSize());

    // Elided, since a label is at least as wide as its text.
    auto* name = new QLabel(QFileInfo(image.path).fileName());
    name->setToolTip(image.path);
    name->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    name->setText(name->fontMetrics().elidedText(QFileInfo(image.path).fileName(), Qt::ElideMiddle, 320));

    auto* remove = new QToolButton;
    remove->setText("Remove");
    remove->setAutoRaise(true);
    remove->setCursor(Qt::PointingHandCursor);
    connect(remove, &QToolButton::clicked, this, [this, path = image.path] { this->OnRemoveImage(path); });

    auto* row = new QWidget;
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(10);
    rowLayout->addWidget(thumbnail);
    rowLayout->addWidget(name, 1);
    rowLayout->addWidget(remove);

    m_ImageList->addWidget(row);
  }
}

void QmitkFeedbackDialog::UpdateHeaderIcon()
{
  QString pictogram;

  switch (static_cast<mitk::FeedbackCategory>(m_CategoryGroup->checkedId()))
  {
    case mitk::FeedbackCategory::Feature:
      pictogram = "feature";
      break;

    case mitk::FeedbackCategory::Idea:
      pictogram = "idea";
      break;

    case mitk::FeedbackCategory::Bug:
    default:
      pictogram = "bug";
      break;
  }

  m_HeaderIcon->setPixmap(QmitkIconTheme::GetIcon(":/org.mitk.gui.qt.ext/feedback/" + pictogram + ".svg")
    .pixmap(QSize(IconSize, IconSize), this->devicePixelRatioF()));
}

mitk::FeedbackReport QmitkFeedbackDialog::AssembleReport() const
{
  mitk::FeedbackReport report;
  report.category = static_cast<mitk::FeedbackCategory>(m_CategoryGroup->checkedId());
  report.description = m_Description->toPlainText().trimmed().toStdString();

  for (const auto& [checkBox, item] : m_EnvironmentItems)
  {
    report.environment.push_back(item);
    report.environment.back().included = checkBox->isChecked();
  }

  for (std::size_t i = 0; i < m_LogRows.size(); ++i)
  {
    const auto& row = m_LogRows[i];

    if (row.text == nullptr)
      continue;

    const auto text = row.text->toPlainText();

    auto log = m_Input.logs[i].log;
    log.text = text.toStdString();

    if (!log.text.empty() && log.text.back() != '\n')
      log.text += '\n';

    log.included = row.include->isChecked();
    log.edited = text != row.originalText;
    report.logs.push_back(log);
  }

  if (m_ScreenshotInclude != nullptr && m_ScreenshotInclude->isChecked())
  {
    report.includeScreenshot = true;
    report.screenshotTarget = static_cast<mitk::ScreenshotTarget>(m_ScreenshotTarget->currentData().toInt());

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

#ifdef MITK_HAS_CRASHHANDLING
  if (m_DumpList != nullptr)
  {
    const auto included = this->GetIncludedDumps();

    for (const auto& dump : m_Input.dumps)
    {
      const auto release = dump.RunInfo.has_value() && !dump.RunInfo->Release.empty()
        ? QString::fromStdString(dump.RunInfo->Release)
        : QString("unknown");

      // Neither the path nor the install directory of the run info: both
      // can contain the OS user name.
      const auto description = QString("%1, %2, %3, release of that session: %4")
        .arg(QmitkCrashDumpListWidget::KindLabel(dump.Kind), QmitkFeedbackDumpTime(dump),
          QLocale::c().formattedDataSize(static_cast<qint64>(dump.SizeInBytes)), release);

      mitk::FeedbackAttachment attachment;
      attachment.sourcePath = dump.Path;
      attachment.archiveName = "dumps/" + QString::fromStdU16String(dump.Path.filename().u16string()).toStdString();
      attachment.description = description.toStdString();
      attachment.included = std::find(included.begin(), included.end(), dump.Path) != included.end();

      report.attachments.push_back(attachment);
    }
  }
#endif

  for (std::size_t i = 0; i < m_Images.size(); ++i)
  {
    const auto fileName = QFileInfo(m_Images[i].path).fileName();

    mitk::FeedbackAttachment image;
    image.sourcePath = m_Images[i].path.toStdU16String();
    // Numbered, since files from different folders may share a name.
    image.archiveName = "images/" + std::to_string(i + 1) + "-" + mitk::MakeArchiveElement(fileName.toStdString());
    image.description = fileName.toStdString();
    report.images.push_back(image);
  }

  report.sharingConfirmed = m_SharingConfirmation->isChecked();
  report.publicSharingConfirmed = m_PublicConfirmation->isChecked();

  return report;
}

void QmitkFeedbackDialog::OnReportChanged()
{
  ++m_Generation;

  const auto report = this->AssembleReport();

  const auto scroll = m_Preview->verticalScrollBar()->value();
  m_Preview->setMarkdown(QString::fromStdString(mitk::BuildReportText(report)));
  ScaleHeadings(m_Preview->document(), m_Preview->font().pointSizeF());
  m_Preview->verticalScrollBar()->setValue(scroll);

  this->UpdateSummaries();
  this->UpdateConfirmations(report);
  this->UpdateHandoffEnabled(report);
  this->UpdateScreenshotPreview();
}

void QmitkFeedbackDialog::OnPhiItemIncluded(bool included)
{
  // A confirmation covers what was included when it was given.
  if (included)
    m_SharingConfirmation->setChecked(false);

  this->OnReportChanged();
}

void QmitkFeedbackDialog::OnPublicItemChanged()
{
  m_PublicConfirmation->setChecked(false);
  this->OnReportChanged();
}

void QmitkFeedbackDialog::OnDumpsChanged()
{
  const auto includedDumpCount = this->GetIncludedDumps().size();

  // A confirmation covers what was included when it was given.
  if (includedDumpCount > m_IncludedDumpCount)
    m_SharingConfirmation->setChecked(false);

  m_IncludedDumpCount = includedDumpCount;
  this->UpdateDumpRows();
  this->OnReportChanged();
}

std::vector<std::filesystem::path> QmitkFeedbackDialog::GetIncludedDumps() const
{
  std::vector<std::filesystem::path> included;

#ifdef MITK_HAS_CRASHHANDLING
  if (m_DumpsInclude == nullptr || !m_DumpsInclude->isChecked())
    return included;

  for (const auto& dump : m_DumpList->GetCheckedDumps())
    included.push_back(dump.Path);
#endif

  return included;
}

void QmitkFeedbackDialog::UpdateDumpRows()
{
  if (m_DumpDetails != nullptr)
    m_DumpDetails->setVisible(m_DumpsInclude->isChecked());

  const auto included = this->GetIncludedDumps();

  for (const auto& row : m_LogRows)
  {
    if (row.dump.empty())
      continue;

    const bool offered = std::find(included.begin(), included.end(), row.dump) != included.end();
    row.row->setVisible(offered);

    // A log that is not offered must not stay included unseen.
    if (!offered && row.include->isChecked())
    {
      const QSignalBlocker blocker(row.include);
      row.include->setChecked(false);
    }
  }
}

void QmitkFeedbackDialog::OnEnvironmentClicked()
{
  const bool include = m_EnvironmentInclude->checkState() != Qt::Unchecked;

  for (const auto& [checkBox, item] : m_EnvironmentItems)
  {
    const QSignalBlocker blocker(checkBox);
    checkBox->setChecked(include);
  }

  this->OnReportChanged();
}

void QmitkFeedbackDialog::UpdateSummaries()
{
  std::size_t included = 0;
  QStringList values;

  for (const auto& [checkBox, item] : m_EnvironmentItems)
  {
    if (!checkBox->isChecked())
      continue;

    ++included;

    switch (item.field)
    {
      case mitk::EnvironmentField::MitkVersion:
        values << "MITK " + QString::fromStdString(item.value);
        break;

      case mitk::EnvironmentField::OperatingSystem:
      case mitk::EnvironmentField::GpuRenderer:
        values << QString::fromStdString(item.value);
        break;

      default:
        break;
    }
  }

  {
    const QSignalBlocker blocker(m_EnvironmentInclude);

    if (included == 0)
      m_EnvironmentInclude->setCheckState(Qt::Unchecked);
    else if (included == m_EnvironmentItems.size())
      m_EnvironmentInclude->setCheckState(Qt::Checked);
    else
      m_EnvironmentInclude->setCheckState(Qt::PartiallyChecked);
  }

  if (included == 0)
  {
    m_EnvironmentSummary->setText("None included.");
  }
  else
  {
    auto summary = values.join(", ");

    if (included > static_cast<std::size_t>(values.size()))
      summary += QString(values.isEmpty() ? "%1 items" : " and %1 more").arg(included - values.size());

    if (included < m_EnvironmentItems.size())
      summary += QString(" (%1 of %2 included)").arg(included).arg(m_EnvironmentItems.size());

    m_EnvironmentSummary->setText(summary);
  }

  for (const auto& row : m_LogRows)
  {
    if (row.text == nullptr)
      continue;

    if (row.text->toPlainText() != row.originalText)
    {
      row.summary->setText(QString("%1 <span style=\"color: %2;\">Edited by you.</span>")
        .arg(row.originalSummary.toHtmlEscaped(), QmitkIconTheme::GetAccentColor()));
    }
    else
    {
      row.summary->setText(row.originalSummary.toHtmlEscaped());
    }
  }

#ifdef MITK_HAS_CRASHHANDLING
  if (m_DumpSummary != nullptr)
  {
    const auto saved = m_Input.dumps.size();

    if (!m_DumpsInclude->isChecked())
    {
      m_DumpSummary->setText(saved == 1
        ? QString("One crash dump is saved on this computer. Include it if it belongs to this problem.")
        : QString("%1 crash dumps are saved on this computer. Include those that belong to this problem.").arg(saved));
    }
    else
    {
      const auto included = this->GetIncludedDumps().size();
      const auto selection = included == 0
        ? QString("Check those that belong to this problem.")
        : QString("%1 of %2 included.").arg(included).arg(saved);

      m_DumpSummary->setText(selection + " A report with crash dumps goes by email only, since they cannot be "
        "reviewed before sending.");
    }
  }
#endif
}

void QmitkFeedbackDialog::UpdateConfirmations(const mitk::FeedbackReport& report)
{
  m_SharingPanel->setVisible(report.NeedsSharingConfirmation());

  const bool hasAttachment = std::any_of(report.attachments.begin(), report.attachments.end(),
    [](const auto& attachment) { return attachment.included; });
  const bool hasLog = std::any_of(report.logs.begin(), report.logs.end(), [](const auto& log) { return log.included; });

  m_PublicPanel->setVisible(!hasAttachment && (hasLog || report.includeScreenshot || !report.images.empty()));
}

void QmitkFeedbackDialog::UpdateHandoffEnabled(const mitk::FeedbackReport& report)
{
  const bool writing = m_BundleWatcher.isRunning();
  const bool confirmed = !report.NeedsSharingConfirmation() || report.sharingConfirmed;
  const bool ready = !writing && !report.description.empty() && confirmed;

  m_EmailButton->setEnabled(ready);
  m_ShowBundleButton->setEnabled(ready);
  m_IssueButton->setEnabled(ready && report.CanUsePublicTracker());
  m_CloseButton->setEnabled(!writing);

  QString hint;
  bool blocking = true;

  if (report.description.empty())
  {
    hint = "Describe the problem or idea to continue.";
  }
  else if (!confirmed)
  {
    hint = "Confirm that you may share the included items to continue.";
  }
  else if (!report.CanUsePublicTracker())
  {
    const bool hasAttachment = std::any_of(report.attachments.begin(), report.attachments.end(),
      [](const auto& attachment) { return attachment.included; });

    hint = hasAttachment
      ? QString("Reports with crash dumps are sent by email.")
      : QString("To open a GitHub issue, confirm that the included logs and images may be published.");
    blocking = false;
  }

  // What keeps every hand-off disabled must not be overlooked; a note on
  // the GitHub route alone stays in the background.
  m_GitHubReason->setStyleSheet(blocking
    ? QString("color: %1; font-weight: bold;").arg(WarningColor())
    : QString("color: %1;").arg(MutedColor()));
  m_GitHubReason->setText(hint);
  m_GitHubReason->setVisible(!hint.isEmpty());
}

void QmitkFeedbackDialog::UpdateScreenshotPreview()
{
  if (m_ScreenshotInclude == nullptr)
    return;

  const bool show = m_ScreenshotInclude->isChecked();
  m_ScreenshotPreview->setVisible(show);

  if (!show)
    return;

  const auto target = static_cast<mitk::ScreenshotTarget>(m_ScreenshotTarget->currentData().toInt());
  const auto& shot = target == mitk::ScreenshotTarget::EditorArea ? m_Input.editorShot : m_Input.windowShot;

  const auto ratio = this->devicePixelRatioF();
  auto thumbnail = shot.scaled(QSize(320, 200) * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  thumbnail.setDevicePixelRatio(ratio);
  m_ScreenshotPreview->setPixmap(thumbnail);
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

  m_BusyIndicator->setVisible(true);
  m_Status->setText("Saving the report file...");

  m_BundleWatcher.setFuture(QtConcurrent::run(WriteBundle, m_PendingReport, m_PendingZipPath));
  this->UpdateHandoffEnabled(m_PendingReport);
}

void QmitkFeedbackDialog::OnBundleWritten()
{
  m_BusyIndicator->setVisible(false);

  const auto error = m_BundleWatcher.result();
  const auto handoff = std::move(m_PendingHandoff);
  m_PendingHandoff = nullptr;

  this->UpdateHandoffEnabled(this->AssembleReport());

  if (!error.isEmpty())
  {
    QDir(QFileInfo(m_PendingZipPath).path()).removeRecursively();
    m_Status->setText("The report file could not be saved.");
    QMessageBox::warning(this, this->windowTitle(), "The report file could not be saved.\n\n" + error);
    return;
  }

  const auto supersededPath = m_BundlePath;

  m_BundlePath = m_PendingZipPath;
  m_BundleGeneration = m_PendingGeneration;
  m_BundleAttachments.clear();

  for (const auto& attachment : m_PendingReport.attachments)
  {
    if (attachment.included)
      m_BundleAttachments.push_back(attachment.sourcePath);
  }

  // The superseded bundle may hold content the user has removed since.
  // Best effort: a file still held open elsewhere stays.
  if (!supersededPath.isEmpty())
  {
    const auto supersededDirectory = QFileInfo(supersededPath).path();

    if (supersededDirectory != QFileInfo(m_BundlePath).path())
      QDir(supersededDirectory).removeRecursively();
  }

  const auto size = QFileInfo(m_BundlePath).size();
  auto status = QString("Saved <b>%1</b> (%2).")
    .arg(QFileInfo(m_BundlePath).fileName().toHtmlEscaped(), QLocale().formattedDataSize(size));

  if (size > LargeBundleSize)
  {
    status += QString(" <span style=\"color: %1;\">It is larger than 25 MB, which GitHub and many mail servers "
      "reject. Use Show report file to pass it on another way.</span>").arg(WarningColor());
  }

  m_BundleStatus = status;
  m_Status->setText(status);

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

    if (QDesktopServices::openUrl(url))
      this->ShowBundleForAttaching(zipPath, "Drag it into the issue text before submitting.");
    else
      this->OfferShowBundle("your web browser", zipPath);
  });
}

void QmitkFeedbackDialog::OnOpenEmail()
{
  this->WriteBundleThen([this](const QString& zipPath, const mitk::FeedbackReport& report) {
    const auto fields = mitk::BuildMailFields(report, QFileInfo(zipPath).fileName().toStdString());

    // The address is used as is: mail clients differ in decoding "%40".
    const auto url = QUrl::fromEncoded("mailto:" + QByteArray::fromStdString(fields.recipient) + "?subject=" +
      EncodeQueryValue(fields.subject) + "&body=" + EncodeQueryValue(fields.body));

    if (QDesktopServices::openUrl(url))
      this->ShowBundleForAttaching(zipPath, "Attach it to the mail before sending.");
    else
      this->OfferShowBundle("your mail program", zipPath);
  });
}

void QmitkFeedbackDialog::OnShowBundle()
{
  this->WriteBundleThen([](const QString& zipPath, const mitk::FeedbackReport&) {
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(zipPath).path()));
  });
}

void QmitkFeedbackDialog::ShowBundleForAttaching(const QString& zipPath, const QString& instruction)
{
  // Neither a mailto: nor a new-issue URL can carry a file. The bundle is
  // alone in its folder, so opening the folder shows the file to drag.
  QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(zipPath).path()));
  m_Status->setText(m_BundleStatus + " <b>" + instruction.toHtmlEscaped() + "</b>");
}

void QmitkFeedbackDialog::OfferShowBundle(const QString& application, const QString& zipPath)
{
  QMessageBox box(QMessageBox::Warning, this->windowTitle(),
    QString("MITK could not start %1. The report file %2 has been saved; you can send it in another way.")
      .arg(application, QFileInfo(zipPath).fileName()),
    QMessageBox::Close, this);

  auto* showButton = box.addButton("Show report file", QMessageBox::AcceptRole);
  box.exec();

  if (box.clickedButton() == showButton)
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(zipPath).path()));
}
