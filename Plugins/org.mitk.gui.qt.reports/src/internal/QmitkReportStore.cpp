/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkReportStore.h"

#include <mitkExceptionMacro.h>

#include <QmitkIconTheme.h>

#include <berryIWorkbenchListener.h>
#include <berryPlatformUI.h>

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLockFile>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>
#include <QTemporaryDir>
#include <QTimer>

#include <algorithm>

namespace
{
  constexpr std::size_t MaxPoolSize = 100;
  constexpr qsizetype MaxTitleLength = 200;

  QString NormalizeTitle(const QString& title)
  {
    auto normalized = title.simplified().left(MaxTitleLength);

    if (!normalized.isEmpty() && normalized.back().isHighSurrogate())
      normalized.chop(1);

    return normalized.trimmed();
  }

  /** src attributes, with the quote in group 1 and the value in group 2. */
  const QRegularExpression& GetSourceAttribute()
  {
    static const QRegularExpression expression(QStringLiteral("\\bsrc\\s*=\\s*([\"'])(.*?)\\1"),
      QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

    return expression;
  }

  bool IsDataUri(const QString& source)
  {
    return source.startsWith(QStringLiteral("data:"), Qt::CaseInsensitive);
  }

  QString LoadStyleSheet()
  {
    QFile file(QStringLiteral(":/org.mitk.gui.qt.reports/report.css"));
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
  }

  void WriteFile(const QString& path, const QByteArray& content)
  {
    QSaveFile file(path);

    if (!file.open(QIODevice::WriteOnly) || file.write(content) != content.size() || !file.commit())
      mitkThrow() << "Cannot write " << path.toStdString() << ": " << file.errorString().toStdString();
  }

  const QString LockFileName = QStringLiteral("lock");

  /** The folder the user saved a report to last, for the session. */
  QString s_LastFolder;

  /**
   * Removes the temporary folders of sessions that ended without cleaning up. A folder is
   * abandoned when its lock file can be locked, which only a dead owner permits, or when it has
   * no lock file and is older than a day. A folder without a lock file may be one that a
   * starting Workbench has just created.
   */
  void RemoveAbandonedFolders()
  {
    const QDir temporary(QDir::tempPath());
    const auto names = temporary.entryList({ QStringLiteral("MITK-Reports-*") }, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    for (const auto& name : names)
    {
      const auto folder = temporary.filePath(name);
      const auto lockPath = QDir(folder).filePath(LockFileName);
      bool abandoned = false;

      if (QFileInfo::exists(lockPath))
      {
        QLockFile lock(lockPath);
        lock.setStaleLockTime(0);
        abandoned = lock.tryLock(0);
        lock.unlock();
      }
      else
      {
        abandoned = QFileInfo(folder).lastModified() < QDateTime::currentDateTime().addDays(-1);
      }

      if (abandoned)
        QDir(folder).removeRecursively();
    }
  }
}

class QmitkReportStore::WorkbenchListener : public berry::IWorkbenchListener
{
public:
  explicit WorkbenchListener(QmitkReportStore* store)
    : m_Store(store)
  {
  }

  bool PreShutdown(berry::IWorkbench*, bool forced) override
  {
    return m_Store->ConfirmShutdown(forced);
  }

private:
  QmitkReportStore* m_Store;
};

QmitkReportStore::QmitkReportStore(QObject* parent)
  : QObject(parent)
{
}

QmitkReportStore::~QmitkReportStore()
{
  if (m_WorkbenchListener != nullptr)
  {
    try
    {
      berry::PlatformUI::GetWorkbench()->RemoveWorkbenchListener(m_WorkbenchListener.get());
    }
    catch (...)
    {
      // The Workbench is gone already and with it its listeners.
    }
  }
}

QString QmitkReportStore::AddImage(const QByteArray& png)
{
  const auto name = QStringLiteral("image-%1.png").arg(m_NextImageId++);
  m_Pool.emplace_back(name, png);

  while (m_Pool.size() > MaxPoolSize)
    m_Pool.pop_front();

  return name;
}

int QmitkReportStore::Add(const QString& title, const QString& html)
{
  Report report;
  report.Images = this->CollectImages(html, nullptr);
  report.Id = m_NextReportId++;
  report.Title = NormalizeTitle(title);
  report.Html = html;
  report.Modified = QDateTime::currentDateTime();

  const auto id = report.Id;
  m_Reports.emplace(id, std::move(report));

  this->ListenToWorkbench();

  emit Changed(id);
  return id;
}

void QmitkReportStore::Update(int id, const QString& title, const QString& html)
{
  const auto report = m_Reports.find(id);

  if (report == m_Reports.end())
    mitkThrow() << "There is no report " << id << ".";

  report->second.Images = this->CollectImages(html, &report->second);
  report->second.Title = NormalizeTitle(title);
  report->second.Html = html;
  report->second.Modified = QDateTime::currentDateTime();
  report->second.Saved = false;
  report->second.LossAccepted = false;

  emit Changed(id);
}

const QmitkReportStore::Report* QmitkReportStore::Get(int id) const
{
  const auto report = m_Reports.find(id);
  return report != m_Reports.end() ? &report->second : nullptr;
}

std::vector<const QmitkReportStore::Report*> QmitkReportStore::GetReports() const
{
  std::vector<const Report*> reports;

  for (const auto& [id, report] : m_Reports)
    reports.push_back(&report);

  return reports;
}

void QmitkReportStore::Remove(int id)
{
  m_Reports.erase(id);

  if (const auto files = m_TemporaryFiles.find(id); files != m_TemporaryFiles.end())
  {
    for (const auto& path : files->second)
      QFile::remove(path);

    m_TemporaryFiles.erase(files);
  }
}

bool QmitkReportStore::IsSaved(int id) const
{
  const auto* report = this->Get(id);
  return report != nullptr && report->Saved;
}

void QmitkReportStore::MarkSaved(int id)
{
  if (const auto report = m_Reports.find(id); report != m_Reports.end())
    report->second.Saved = true;
}

void QmitkReportStore::AcceptLoss(int id)
{
  if (const auto report = m_Reports.find(id); report != m_Reports.end())
    report->second.LossAccepted = true;
}

bool QmitkReportStore::SaveReport(int id, QWidget* parent)
{
  const auto* report = this->Get(id);

  if (report == nullptr)
    return false;

  const auto folder = !s_LastFolder.isEmpty()
    ? s_LastFolder
    : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

  const auto path = QFileDialog::getSaveFileName(parent, "Save Report",
    QDir(folder).filePath(GetFileBaseName(*report) + ".html"), "HTML files (*.html *.htm)");

  if (path.isEmpty())
    return false;

  s_LastFolder = QFileInfo(path).absolutePath();

  try
  {
    this->WriteDocument(id, path);
  }
  catch (const mitk::Exception& e)
  {
    QMessageBox::warning(parent, "Save Report", QString::fromStdString(e.GetDescription()));
    return false;
  }

  this->MarkSaved(id);
  return true;
}

void QmitkReportStore::CloseReport(int id)
{
  const auto* report = this->Get(id);

  if (report == nullptr)
    return;

  if (report->Saved || report->LossAccepted)
  {
    this->Remove(id);
    return;
  }

  m_ClosedReports.push_back(id);

  if (!m_AskingQueued)
  {
    m_AskingQueued = true;
    QTimer::singleShot(0, this, &QmitkReportStore::AskAboutClosedReports);
  }
}

void QmitkReportStore::AskAboutClosedReports()
{
  m_AskingQueued = false;

  // The dialogs run event loops of their own. Reports closed meanwhile join the queue.
  if (m_Asking)
    return;

  m_Asking = true;

  while (!m_ClosedReports.empty())
  {
    const auto id = m_ClosedReports.front();
    m_ClosedReports.pop_front();

    while (const auto* report = this->Get(id))
    {
      if (report->Saved || report->LossAccepted)
        break;

      auto* parent = QApplication::activeWindow();

      QMessageBox question(QMessageBox::Question, "Report Closed", QString(), QMessageBox::NoButton, parent);
      question.setTextFormat(Qt::PlainText);
      question.setText(QStringLiteral("The report \"%1\" was closed without being saved.").arg(report->Title));
      auto* save = question.addButton("Save as HTML...", QMessageBox::AcceptRole);
      question.addButton("Discard", QMessageBox::DestructiveRole);
      question.setDefaultButton(save);
      question.exec();

      if (question.clickedButton() == save && !this->SaveReport(id, parent))
        continue;

      break;
    }

    this->Remove(id);
  }

  m_Asking = false;
}

std::vector<int> QmitkReportStore::GetUnsavedReports() const
{
  std::vector<int> ids;

  for (const auto& [id, report] : m_Reports)
  {
    if (!report.Saved && !report.LossAccepted)
      ids.push_back(id);
  }

  return ids;
}

bool QmitkReportStore::ConfirmShutdown(bool forced)
{
  const auto unsaved = this->GetUnsavedReports();

  if (unsaved.empty())
    return true;

  // A forced shutdown cannot be canceled, so there is nothing to ask.
  if (forced)
  {
    for (const auto id : unsaved)
      this->AcceptLoss(id);

    return true;
  }

  auto* parent = QApplication::activeWindow();

  QMessageBox question(QMessageBox::Question, "Reports Not Saved", QString(), QMessageBox::NoButton, parent);
  question.setText(unsaved.size() == 1
    ? QStringLiteral("1 report was not saved.")
    : QStringLiteral("%1 reports were not saved.").arg(unsaved.size()));
  auto* save = question.addButton("Save as HTML...", QMessageBox::AcceptRole);
  question.addButton("Discard", QMessageBox::DestructiveRole);
  auto* cancel = question.addButton(QMessageBox::Cancel);
  question.setDefaultButton(save);
  question.setEscapeButton(cancel);
  question.exec();

  if (question.clickedButton() == cancel)
    return false;

  for (const auto id : unsaved)
  {
    if (question.clickedButton() == save)
    {
      if (this->Get(id) != nullptr && !this->SaveReport(id, parent))
        return false;
    }
    else
    {
      this->AcceptLoss(id);
    }
  }

  return true;
}

void QmitkReportStore::ListenToWorkbench()
{
  if (m_WorkbenchListener != nullptr || !berry::PlatformUI::IsWorkbenchRunning())
    return;

  m_WorkbenchListener = std::make_unique<WorkbenchListener>(this);
  berry::PlatformUI::GetWorkbench()->AddWorkbenchListener(m_WorkbenchListener.get());
}

QString QmitkReportStore::GetDocument(int id, bool forExport) const
{
  const auto* report = this->Get(id);

  if (report == nullptr)
    return QString();

  static const QString styleSheet = LoadStyleSheet();
  auto body = report->Html;

  if (forExport)
  {
    QString inlined;
    qsizetype copied = 0;
    auto sources = GetSourceAttribute().globalMatch(body);

    while (sources.hasNext())
    {
      const auto source = sources.next();
      const auto image = report->Images.constFind(source.captured(2));

      if (image == report->Images.constEnd())
        continue;

      inlined += QStringView(body).mid(copied, source.capturedStart(2) - copied);
      inlined += QStringLiteral("data:image/png;base64,") + QString::fromLatin1(image->toBase64());
      copied = source.capturedEnd(2);
    }

    inlined += QStringView(body).mid(copied);
    body = inlined;
  }

  const auto title = report->Title.toHtmlEscaped();

  // Saved and opened pages look as the editor shows them, in the Workbench's theme.
  const auto theme = QmitkIconTheme::IsDarkTheme() ? QStringLiteral(" class=\"dark\"") : QString();

  QString page = QStringLiteral("<!DOCTYPE html>\n<html lang=\"en\"") + theme + QStringLiteral(">\n<head>\n<meta charset=\"utf-8\">\n");

  if (forExport)
  {
    page += QStringLiteral("<meta http-equiv=\"Content-Security-Policy\" "
                           "content=\"default-src 'none'; img-src data:; style-src 'unsafe-inline'\">\n");
  }

  page += QStringLiteral("<title>") + title + QStringLiteral("</title>\n<style>\n") + styleSheet +
          QStringLiteral("</style>\n</head>\n<body>\n<header class=\"report-header\">\n<h1>") + title +
          QStringLiteral("</h1>\n<p class=\"report-meta\">MITK Workbench, ") +
          report->Modified.toString(QStringLiteral("yyyy-MM-dd HH:mm")) +
          QStringLiteral("</p>\n</header>\n<main>\n") + body + QStringLiteral("\n</main>\n</body>\n</html>\n");

  return page;
}

void QmitkReportStore::WriteDocument(int id, const QString& path) const
{
  const auto document = this->GetDocument(id, true);

  if (document.isEmpty())
    mitkThrow() << "There is no report " << id << ".";

  WriteFile(path, document.toUtf8());
}

QString QmitkReportStore::WriteTemporaryFile(int id)
{
  const auto* report = this->Get(id);

  if (report == nullptr)
    mitkThrow() << "There is no report " << id << ".";

  // A folder that could not be created, or that something else removed since, gets another try.
  if (m_TemporaryDir == nullptr || !m_TemporaryDir->isValid() || !QDir(m_TemporaryDir->path()).exists())
  {
    m_Lock.reset();
    m_TemporaryDir.reset();

    RemoveAbandonedFolders();

    m_TemporaryDir = std::make_unique<QTemporaryDir>(QDir(QDir::tempPath()).filePath(QStringLiteral("MITK-Reports-XXXXXX")));

    if (!m_TemporaryDir->isValid())
      mitkThrow() << "Cannot create a temporary folder: " << m_TemporaryDir->errorString().toStdString();

    m_Lock = std::make_unique<QLockFile>(m_TemporaryDir->filePath(LockFileName));
    m_Lock->setStaleLockTime(0);

    if (!m_Lock->tryLock(0))
      mitkThrow() << "Cannot lock the temporary folder " << m_TemporaryDir->path().toStdString() << ".";
  }

  const auto path = m_TemporaryDir->filePath(GetFileBaseName(*report) + QStringLiteral(" (%1).html").arg(id));
  this->WriteDocument(id, path);

  auto& files = m_TemporaryFiles[id];

  if (!files.contains(path))
    files << path;

  return path;
}

QString QmitkReportStore::GetFileBaseName(const Report& report)
{
  static const QRegularExpression forbidden(QStringLiteral("[\\\\/:*?\"<>|\\x00-\\x1f]"));

  auto name = report.Title;
  name.replace(forbidden, QStringLiteral(" "));
  name = name.simplified().left(80).trimmed();

  return name.isEmpty() ? QStringLiteral("Report %1").arg(report.Id) : name;
}

QMap<QString, QByteArray> QmitkReportStore::CollectImages(const QString& html, const Report* report) const
{
  QMap<QString, QByteArray> images;
  QStringList unknown;
  auto sources = GetSourceAttribute().globalMatch(html);

  while (sources.hasNext())
  {
    const auto source = sources.next().captured(2);

    if (IsDataUri(source) || images.contains(source))
      continue;

    if (report != nullptr && report->Images.contains(source))
    {
      images.insert(source, report->Images.value(source));
      continue;
    }

    const auto pooled = std::find_if(m_Pool.begin(), m_Pool.end(), [&source](const auto& image) { return image.first == source; });

    if (pooled != m_Pool.end())
    {
      images.insert(source, pooled->second);
      continue;
    }

    if (!unknown.contains(source))
      unknown << source;
  }

  if (!unknown.isEmpty())
    mitkThrow() << "Unknown images: " << unknown.join(QStringLiteral(", ")).toStdString() << ".";

  return images;
}
