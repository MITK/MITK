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
#include <QCryptographicHash>
#include <QFile>
#include <QIconEngine>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QRegularExpression>
#include <QStyle>
#include <QStyleOption>
#include <QSvgRenderer>

#include <memory>
#include <optional>

namespace
{
  /* Icons are rendered on the GUI thread only, so plain statics suffice. The
   * generation counter starts at 1 because 0 marks an engine that never
   * themed its SVG.
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

  /* The accent color goes first, so that a custom icon color equal to the
   * magic accent color is not themed away again.
   */
  QByteArray ThemeSVG(const QByteArray &originalSVG, const std::optional<QString> &color)
  {
    auto themedSVG = ReplaceMagicColor(QString(originalSVG), QStringLiteral("ff00ff|f0f"), QmitkIconTheme::GetAccentColor());
    themedSVG = ReplaceMagicColor(themedSVG, QStringLiteral("00ff00|0f0"), color.value_or(QmitkIconTheme::GetColor()));

    return themedSVG.toUtf8();
  }

  /* Renders the themed SVG at the size and device pixel ratio it is displayed
   * with, so icons stay sharp on high-DPI screens and no icon is rasterized at
   * the arbitrary size declared in its file. Rendered pixmaps are shared
   * through QPixmapCache under a key derived from the themed SVG content, so a
   * theme switch simply leads to new keys and needs no invalidation.
   */
  class ThemedIconEngine : public QIconEngine
  {
  public:
    ThemedIconEngine(const QByteArray &originalSVG, const std::optional<QString> &color)
      // Deep copy: callers may pass a QByteArray::fromRawData() view over a
      // buffer that is gone by the time the icon is first painted.
      : m_SVG(originalSVG.constData(), originalSVG.size()),
        m_Color(color)
    {
    }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override
    {
      const auto *device = painter->device();
      const auto scale = device != nullptr
        ? device->devicePixelRatio()
        : qApp->devicePixelRatio();

      painter->drawPixmap(rect, this->scaledPixmap(rect.size(), mode, state, scale));
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
      return this->scaledPixmap(size, mode, state, 1.0);
    }

    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override
    {
      this->EnsureCurrent();

      if (!m_Renderer->isValid())
        return QPixmap();

      const auto ratio = scale > 0.0 ? scale : 1.0;
      const auto deviceSize = (QSizeF(this->actualSize(size, mode, state)) * ratio).toSize();

      if (deviceSize.isEmpty())
        return QPixmap();

      // The disabled variant is derived from the palette, hence the key includes it.
      const auto key = QStringLiteral("%1/%2x%3@%4/%5/%6")
        .arg(m_CacheKeyPrefix)
        .arg(deviceSize.width())
        .arg(deviceSize.height())
        .arg(qRound(ratio * 1000))
        .arg(static_cast<int>(mode))
        .arg(QApplication::palette().cacheKey());

      QPixmap pixmap;

      if (!QPixmapCache::find(key, &pixmap))
      {
        pixmap = this->Render(deviceSize);

        if (mode != QIcon::Normal)
        {
          QStyleOption option;
          option.palette = QApplication::palette();
          pixmap = QApplication::style()->generatedIconPixmap(mode, pixmap, &option);
        }

        pixmap.setDevicePixelRatio(ratio);
        QPixmapCache::insert(key, pixmap);
      }

      return pixmap;
    }

    QSize actualSize(const QSize &size, QIcon::Mode, QIcon::State) override
    {
      this->EnsureCurrent();

      // Only the aspect ratio of the declared size matters for a vector icon.
      return m_Renderer->defaultSize().scaled(size, Qt::KeepAspectRatio);
    }

    bool isNull() override
    {
      this->EnsureCurrent();
      return !m_Renderer->isValid();
    }

    QIconEngine *clone() const override
    {
      return new ThemedIconEngine(m_SVG, m_Color);
    }

  private:
    void EnsureCurrent()
    {
      if (m_Generation == s_Generation)
        return;

      const auto themedSVG = ThemeSVG(m_SVG, m_Color);

      m_Renderer = std::make_unique<QSvgRenderer>(themedSVG);
      m_Renderer->setAspectRatioMode(Qt::KeepAspectRatio);

      m_CacheKeyPrefix = QStringLiteral("QmitkIconTheme/")
        + QString::fromLatin1(QCryptographicHash::hash(themedSVG, QCryptographicHash::Md5).toHex());

      m_Generation = s_Generation;
    }

    QPixmap Render(const QSize &deviceSize) const
    {
      QImage image(deviceSize, QImage::Format_ARGB32_Premultiplied);
      image.fill(Qt::transparent);

      {
        QPainter painter(&image);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        m_Renderer->render(&painter);
      }

      return QPixmap::fromImage(image);
    }

    QByteArray m_SVG;
    std::optional<QString> m_Color;
    std::unique_ptr<QSvgRenderer> m_Renderer;
    QString m_CacheKeyPrefix;
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
  return QIcon(new ThemedIconEngine(originalSVG, std::nullopt));
}

QIcon QmitkIconTheme::GetIcon(const QByteArray &originalSVG, const QString &color)
{
  return QIcon(new ThemedIconEngine(originalSVG, color));
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
  const auto color = s_Color;
  const auto accentColor = s_AccentColor;

  s_Parsed = false;
  EnsureParsed();

  if (color == s_Color && accentColor == s_AccentColor)
    return;

  ++s_Generation;

  emit GetInstance()->Changed();
}
