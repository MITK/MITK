/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkReport.h"

#include "QmitkReportSource.h"

#include <mitkExceptionMacro.h>

#include <QFile>
#include <QRegularExpression>
#include <QSaveFile>

namespace
{
  constexpr qsizetype MaxTitleLength = 200;

  QString LoadStyleSheet()
  {
    QFile file(QStringLiteral(":/MitkReports/report.css"));
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
  }

  void WriteFile(const QString& path, const QByteArray& content)
  {
    QSaveFile file(path);

    if (!file.open(QIODevice::WriteOnly) || file.write(content) != content.size() || !file.commit())
      mitkThrow() << "Cannot write " << path.toStdString() << ": " << file.errorString().toStdString();
  }
}

QString QmitkReport::NormalizeTitle(const QString& title)
{
  auto normalized = title.simplified().left(MaxTitleLength);

  if (!normalized.isEmpty() && normalized.back().isHighSurrogate())
    normalized.chop(1);

  return normalized.trimmed();
}

QString QmitkReport::GetFileBaseName() const
{
  static const QRegularExpression forbidden(QStringLiteral("[\\\\/:*?\"<>|\\x00-\\x1f]"));

  // Names of devices on Windows, also with a suffix like .html.
  static const QRegularExpression device(QStringLiteral("^(con|prn|aux|nul|com[1-9]|lpt[1-9])(\\.|$)"),
    QRegularExpression::CaseInsensitiveOption);

  auto name = this->Title;
  name.replace(forbidden, QStringLiteral(" "));
  name = name.simplified().left(80);

  if (!name.isEmpty() && name.back().isHighSurrogate())
    name.chop(1);

  // Windows drops dots and spaces at the end of a file name.
  while (name.endsWith(QLatin1Char('.')) || name.endsWith(QLatin1Char(' ')))
    name.chop(1);

  if (name.isEmpty())
    return QStringLiteral("Report %1").arg(this->Id);

  return device.match(name).hasMatch()
    ? QStringLiteral("Report ") + name
    : name;
}

QString QmitkReport::GetDocument(bool forExport, bool darkTheme) const
{
  static const QString styleSheet = LoadStyleSheet();
  auto body = this->Html;

  if (forExport)
  {
    QString inlined;
    qsizetype copied = 0;
    auto sources = QmitkReportSource::GetAttribute().globalMatch(body);

    while (sources.hasNext())
    {
      const auto source = sources.next();
      const auto image = this->Images.constFind(source.captured(2));

      if (image == this->Images.constEnd())
        continue;

      inlined += QStringView(body).mid(copied, source.capturedStart(2) - copied);
      inlined += QStringLiteral("data:image/png;base64,") + QString::fromLatin1(image->toBase64());
      copied = source.capturedEnd(2);
    }

    inlined += QStringView(body).mid(copied);
    body = inlined;

    // No policy stops the redirect of <meta http-equiv="refresh">, which a browser
    // follows wherever it stands, nor the resource hints of <link>, like
    // dns-prefetch, and a report needs neither <base> nor <form>, and brings its
    // styles inline. The srcdoc of an <iframe> would bring all of them back,
    // decoded, and the editor draws no frames. With the opening bracket
    // escaped, they show as text.
    static const QRegularExpression tagOpeners(QStringLiteral("<(?=(?:meta|base|form|link|iframe)[\\s/>])"),
                                               QRegularExpression::CaseInsensitiveOption);
    body.replace(tagOpeners, QStringLiteral("&lt;"));
  }

  const auto title = this->Title.toHtmlEscaped();

  // Saved and opened pages look as the editor shows them, in the Workbench's theme.
  const auto theme = darkTheme ? QStringLiteral(" class=\"dark\"") : QString();

  QString page = QStringLiteral("<!DOCTYPE html>\n<html lang=\"en\"") + theme + QStringLiteral(">\n<head>\n<meta charset=\"utf-8\">\n");

  if (forExport)
  {
    // form-action and base-uri do not fall back to default-src. Browsers
    // resolve the host names of links ahead outside of the policy.
    page += QStringLiteral("<meta http-equiv=\"Content-Security-Policy\" "
                           "content=\"default-src 'none'; img-src data:; style-src 'unsafe-inline'; "
                           "form-action 'none'; base-uri 'none'\">\n"
                           "<meta http-equiv=\"x-dns-prefetch-control\" content=\"off\">\n");
  }

  page += QStringLiteral("<title>") + title + QStringLiteral("</title>\n<style>\n") + styleSheet +
          QStringLiteral("</style>\n</head>\n<body>\n<header class=\"report-header\">\n<h1>") + title +
          QStringLiteral("</h1>\n<p class=\"report-meta\">MITK Workbench, ") +
          this->Modified.toString(QStringLiteral("yyyy-MM-dd HH:mm")) +
          QStringLiteral("</p>\n</header>\n<main>\n") + body + QStringLiteral("\n</main>\n</body>\n</html>\n");

  return page;
}

void QmitkReport::WriteDocument(const QString& path, bool darkTheme) const
{
  WriteFile(path, this->GetDocument(true, darkTheme).toUtf8());
}
