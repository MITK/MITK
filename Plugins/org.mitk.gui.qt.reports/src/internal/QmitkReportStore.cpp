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

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStringList>
#include <QTemporaryDir>

#include <algorithm>

namespace
{
  constexpr std::size_t MaxPoolSize = 100;

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
}

QmitkReportStore::QmitkReportStore(QObject* parent)
  : QObject(parent)
{
}

QmitkReportStore::~QmitkReportStore() = default;

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
  report.Title = title;
  report.Html = html;
  report.Modified = QDateTime::currentDateTime();

  const auto id = report.Id;
  m_Reports.emplace(id, std::move(report));

  emit Changed(id);
  return id;
}

void QmitkReportStore::Update(int id, const QString& title, const QString& html)
{
  const auto report = m_Reports.find(id);

  if (report == m_Reports.end())
    mitkThrow() << "There is no report " << id << ".";

  report->second.Images = this->CollectImages(html, &report->second);
  report->second.Title = title;
  report->second.Html = html;
  report->second.Modified = QDateTime::currentDateTime();

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
    m_TemporaryDir = std::make_unique<QTemporaryDir>(QDir(QDir::tempPath()).filePath(QStringLiteral("MITK-Reports-XXXXXX")));

  if (!m_TemporaryDir->isValid())
    mitkThrow() << "Cannot create a temporary folder: " << m_TemporaryDir->errorString().toStdString();

  const auto path = m_TemporaryDir->filePath(GetFileBaseName(*report) + QStringLiteral(" (%1).html").arg(id));
  this->WriteDocument(id, path);

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
