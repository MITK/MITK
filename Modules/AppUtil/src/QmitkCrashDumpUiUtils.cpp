/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCrashDumpUiUtils.h"

#include <mitkICrashReportService.h>

#include <QApplication>
#include <QDesktopServices>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPalette>
#include <QRegularExpression>
#include <QUrl>
#include <QVBoxLayout>

#include <set>

namespace
{
  constexpr int kIconSize = 48;
  // The badge reaches this far past the icon's lower-right edge, so it can
  // be clearly visible while covering only a corner of the icon.
  constexpr int kBadgeOverhang = 8;
  constexpr int kBadgedIconSize = kIconSize + kBadgeOverhang;

  /** The color of the stylesheet class "font.<name>", which the MITK light
   *  and dark stylesheets define for rich text; invalid if there is none. */
  QColor StyleSheetColor(const QString& name)
  {
    const QRegularExpression pattern(
      QString(R"(font\.%1\s*\{[^}]*?\bcolor\s*:\s*([^;}]+))").arg(QRegularExpression::escape(name)));

    const auto match = pattern.match(qApp->styleSheet());
    return match.hasMatch() ? QColor(match.captured(1).trimmed()) : QColor();
  }

  bool IsDarkTheme()
  {
    return QApplication::palette().color(QPalette::Window).lightness() < 128;
  }

  /** The application's icon. BlueBerry applications set it on the main
   *  window only, not application-wide. */
  QIcon ApplicationIcon()
  {
    if (!QApplication::windowIcon().isNull())
      return QApplication::windowIcon();

    for (auto* widget : QApplication::topLevelWidgets())
    {
      if (qobject_cast<QMainWindow*>(widget) != nullptr && !widget->windowIcon().isNull())
        return widget->windowIcon();
    }

    return {};
  }

  /** The application icon with a badge on its lower-right corner. */
  QPixmap BadgedApplicationIcon(const QColor& badgeColor, qreal devicePixelRatio)
  {
    QPixmap pixmap(QSize(kBadgedIconSize, kBadgedIconSize) * devicePixelRatio);
    pixmap.setDevicePixelRatio(devicePixelRatio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    ApplicationIcon().paint(&painter, QRect(0, 0, kIconSize, kIconSize));

    const qreal diameter = kIconSize * 0.42;
    const QRectF badge(kBadgedIconSize - diameter, kBadgedIconSize - diameter, diameter, diameter);

    // A transparent ring sets the badge off from the icon below it on any
    // background.
    const qreal ring = kIconSize * 0.045;
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawEllipse(badge);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.setBrush(badgeColor);
    painter.drawEllipse(badge.adjusted(ring, ring, -ring, -ring));

    painter.setBrush(Qt::white);
    const qreal barWidth = diameter * 0.14;
    const QPointF center = badge.center();
    painter.drawRoundedRect(QRectF(center.x() - barWidth / 2, badge.top() + diameter * 0.2, barWidth, diameter * 0.38),
      barWidth / 2, barWidth / 2);
    painter.drawEllipse(QPointF(center.x(), badge.top() + diameter * 0.74), barWidth * 0.62, barWidth * 0.62);

    return pixmap;
  }
}

QString QmitkCrashDumpUi::ToQString(const std::filesystem::path& path)
{
  return QString::fromStdWString(path.wstring());
}

QString QmitkCrashDumpUi::KindLabel(mitk::DumpKind kind)
{
  switch (kind)
  {
    case mitk::DumpKind::UnresponsiveTerminated:
      return "Unresponsive (terminated)";
    case mitk::DumpKind::OnDemand:
      return "Captured on request";
    case mitk::DumpKind::Crash:
    default:
      return "Crash";
  }
}

QColor QmitkCrashDumpUi::WarningColor()
{
  const auto color = StyleSheetColor("warning");
  if (color.isValid())
    return color;

  return IsDarkTheme() ? QColor(0xff, 0x5c, 0x33) : QColor(Qt::red);
}

QColor QmitkCrashDumpUi::AccentColor()
{
  const auto color = StyleSheetColor("highlight");
  return color.isValid() ? color : QApplication::palette().color(QPalette::Highlight);
}

QString QmitkCrashDumpUi::Warning(const QString& text)
{
  return QString("<span style=\"color: %1; font-weight: bold;\">%2</span>").arg(WarningColor().name(), text);
}

QString QmitkCrashDumpUi::PrivacyNote()
{
  return Warning("Before you share a dump:") +
    " a crash dump or diagnostic snapshot contains parts of the application's memory from the "
    "session it was taken in. If at any time during that session you opened, browsed or queried "
    "data about real people that was not fully anonymized (including in the DICOM browser or a "
    "PACS query), it may contain such data and must be handled through your usual process for "
    "patient or study-participant data. Other rules of your organisation may also restrict "
    "sharing it, for example for confidential or unpublished data. Please check before you pass "
    "a dump on. MITK never uploads crash dumps; they stay on this computer.";
}

QWidget* QmitkCrashDumpUi::CreateHeader(const QString& title, const QString& subtitle)
{
  const auto accent = AccentColor();

  auto* header = new QFrame;
  header->setObjectName("crashDumpHeader");
  header->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  // Translucent grey instead of a palette colour: BlueBerry themes through
  // the stylesheet alone, so the palette does not tell light from dark, and a
  // tint reads as a band on either.
  header->setStyleSheet(QString(
    "QFrame#crashDumpHeader { background-color: rgba(128, 128, 128, 28); border: none; "
    "border-left: 4px solid %1; border-bottom: 1px solid rgba(128, 128, 128, 90); }").arg(accent.name()));

  auto* iconLabel = new QLabel;
  iconLabel->setPixmap(BadgedApplicationIcon(accent, header->devicePixelRatioF()));
  iconLabel->setFixedSize(kBadgedIconSize, kBadgedIconSize);

  auto* titleLabel = new QLabel(title);
  titleLabel->setObjectName("crashDumpHeaderTitle");
  auto titleFont = titleLabel->font();
  titleFont.setPointSizeF(titleFont.pointSizeF() * 1.35);
  titleFont.setWeight(QFont::DemiBold);
  titleLabel->setFont(titleFont);
  titleLabel->setWordWrap(true);

  auto* subtitleLabel = new QLabel(subtitle);
  subtitleLabel->setObjectName("crashDumpHeaderSubtitle");
  subtitleLabel->setWordWrap(true);

  auto* textLayout = new QVBoxLayout;
  textLayout->setSpacing(2);
  textLayout->addStretch();
  textLayout->addWidget(titleLabel);
  textLayout->addWidget(subtitleLabel);
  textLayout->addStretch();

  auto* layout = new QHBoxLayout(header);
  layout->setContentsMargins(14, 14, 16, 14);
  layout->setSpacing(14);
  layout->addWidget(iconLabel, 0, Qt::AlignVCenter);
  layout->addLayout(textLayout, 1);

  return header;
}

void QmitkCrashDumpUi::ShowInFolders(const std::vector<std::filesystem::path>& files)
{
  // Dumps of different kinds are filed in different folders, so a single
  // folder need not cover them all.
  std::set<std::filesystem::path> folders;

  for (const auto& file : files)
    folders.insert(file.parent_path());

  for (const auto& folder : folders)
  {
    std::error_code error;
    if (std::filesystem::is_directory(folder, error))
      QDesktopServices::openUrl(QUrl::fromLocalFile(ToQString(folder)));
  }
}

bool QmitkCrashDumpUi::FileReport(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent)
{
  auto* service = mitk::GetCrashReportService();

  if (service == nullptr || dumps.empty())
    return false;

  service->FileReport(dumps, parent);
  return true;
}
