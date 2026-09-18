/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkIconTheme.h>

#include <mitkLog.h>

#include <QApplication>
#include <QFile>
#include <QIconEngine>
#include <QImage>
#include <QPixmap>
#include <QRegularExpression>

namespace
{
  /* Icons are baked on the GUI thread only, so plain statics suffice. The
   * generation counter starts at 1 because 0 marks an engine that never baked.
   */
  unsigned s_Generation = 1;
  bool s_Parsed = false;
  QString s_Color;
  QString s_AccentColor;

  QString ParseColor(const QString &subject, const QString &colorName, const QString &fallback)
  {
    const QString pattern = QString("%1\\s*[=:]\\s*(#[0-9a-f]{6})").arg(colorName);

    QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
    auto match = re.match(subject);

    return match.hasMatch()
      ? match.captured(1)
      : fallback;
  }

  void EnsureParsed()
  {
    if (s_Parsed)
      return;

    const auto styleSheet = qApp != nullptr
      ? qApp->styleSheet()
      : QString();

    s_Color = ParseColor(styleSheet, QStringLiteral("iconColor"), QStringLiteral("#000000"));
    s_AccentColor = ParseColor(styleSheet, QStringLiteral("iconAccentColor"), QStringLiteral("#ffffff"));
    s_Parsed = true;
  }

  /* SVG allows both the six-digit and the equivalent three-digit notation of a
   * hexadecimal color code, hence both spellings of a magic color must be
   * replaced. The negative lookahead keeps the three-digit notation from
   * matching the leading half of an unrelated six-digit color code.
   */
  QString ReplaceMagicColor(const QString &svg, const QString &magicColors, const QString &themeColor)
  {
    const QRegularExpression re(QString("#(?:%1)(?![0-9a-f])").arg(magicColors),
      QRegularExpression::CaseInsensitiveOption);

    return QString(svg).replace(re, themeColor);
  }

  QIcon BakeIcon(const QByteArray &originalSVG)
  {
    auto themedSVG = QmitkIconTheme::ReplaceColor(QString(originalSVG), QmitkIconTheme::GetColor());
    themedSVG = ReplaceMagicColor(themedSVG, QStringLiteral("ff00ff|f0f"), QmitkIconTheme::GetAccentColor());

    return QPixmap::fromImage(QImage::fromData(themedSVG.toUtf8()));
  }

  /* Keeps the SVG and re-bakes it lazily whenever the theme generation moved,
   * forwarding everything else to the baked QIcon. Delegating to a regular
   * pixmap-backed QIcon keeps Qt's mode handling (disabled, active, selected
   * variants via QStyle::generatedIconPixmap), its shrink-only actualSize()
   * and its pixmap caching identical to a statically baked icon.
   */
  class ThemedIconEngine : public QIconEngine
  {
  public:
    explicit ThemedIconEngine(const QByteArray &originalSVG)
      // Deep copy: QmitkToolSelectionBox passes QByteArray::fromRawData() over
      // a buffer it frees right after GetIcon() returns.
      : m_SVG(originalSVG.constData(), originalSVG.size())
    {
    }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override
    {
      this->EnsureCurrent();
      m_Icon.paint(painter, rect, Qt::AlignCenter, mode, state);
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
      this->EnsureCurrent();
      return m_Icon.pixmap(size, 1.0, mode, state);
    }

    // Since Qt 6.8, QIcon::pixmap() hands the device-independent size and the
    // device pixel ratio straight through, so both are forwarded unchanged.
    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override
    {
      this->EnsureCurrent();
      return m_Icon.pixmap(size, scale > 0.0 ? scale : 1.0, mode, state);
    }

    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
      this->EnsureCurrent();
      return m_Icon.actualSize(size, mode, state);
    }

    QList<QSize> availableSizes(QIcon::Mode mode, QIcon::State state) override
    {
      this->EnsureCurrent();
      return m_Icon.availableSizes(mode, state);
    }

    bool isNull() override
    {
      this->EnsureCurrent();
      return m_Icon.isNull();
    }

    QIconEngine *clone() const override
    {
      return new ThemedIconEngine(*this);
    }

  private:
    void EnsureCurrent()
    {
      if (m_Generation == s_Generation)
        return;

      m_Icon = BakeIcon(m_SVG);
      m_Generation = s_Generation;
    }

    QByteArray m_SVG;
    QIcon m_Icon;
    unsigned m_Generation = 0;
  };
}

QmitkIconTheme::QmitkIconTheme() = default;

QmitkIconTheme::~QmitkIconTheme() = default;

QmitkIconTheme *QmitkIconTheme::GetInstance()
{
  static QmitkIconTheme instance;
  return &instance;
}

QIcon QmitkIconTheme::GetIcon(const QByteArray &originalSVG)
{
  return QIcon(new ThemedIconEngine(originalSVG));
}

QIcon QmitkIconTheme::GetIcon(const QString &resourcePath)
{
  QFile resourceFile(resourcePath);

  if (resourceFile.open(QIODevice::ReadOnly))
  {
    auto originalSVG = resourceFile.readAll();
    return GetIcon(originalSVG);
  }

  MITK_WARN << "Could not read " << resourcePath.toStdString();
  return QIcon();
}

QString QmitkIconTheme::ReplaceColor(const QString &svg, const QString &color)
{
  return ReplaceMagicColor(svg, QStringLiteral("00ff00|0f0"), color);
}

QString QmitkIconTheme::GetColor()
{
  EnsureParsed();
  return s_Color;
}

QString QmitkIconTheme::GetAccentColor()
{
  EnsureParsed();
  return s_AccentColor;
}

void QmitkIconTheme::Refresh()
{
  s_Parsed = false;
  ++s_Generation;

  emit GetInstance()->Changed();
}
