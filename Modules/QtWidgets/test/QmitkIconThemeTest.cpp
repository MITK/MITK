/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkIconTheme.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QApplication>
#include <QByteArray>
#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QString>

#include <algorithm>
#include <string>
#include <vector>

namespace
{
  /* The colors are declared inside a comment like in the workbench style
   * sheets, so the Qt style sheet parser does not see unknown properties.
   */
  void ApplyTheme(const QString &color, const QString &accentColor)
  {
    qApp->setStyleSheet(QStringLiteral("/*\n  iconColor = %1\n  iconAccentColor = %2\n*/\n").arg(color, accentColor));
    QmitkIconTheme::Refresh();
  }

  /* A filled rectangle of the given size. Without explicit root attributes the
   * SVG declares that size as width and height.
   */
  QByteArray Svg(const QString &fill, int width, int height, QString rootAttributes = QString())
  {
    if (rootAttributes.isEmpty())
      rootAttributes = QStringLiteral("width=\"%1\" height=\"%2\"").arg(width).arg(height);

    return QStringLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" %1><rect width=\"%2\" height=\"%3\" fill=\"%4\"/></svg>")
      .arg(rootAttributes, QString::number(width), QString::number(height), fill)
      .toUtf8();
  }

  QByteArray Svg(const QString &fill, int size)
  {
    return Svg(fill, size, size);
  }

  std::string ColorAt(const QPixmap &pixmap, int x, int y)
  {
    return pixmap.toImage().pixelColor(x, y).name().toStdString();
  }

  std::string CenterColor(const QIcon &icon, int size)
  {
    return ColorAt(icon.pixmap(size, size), size / 2, size / 2);
  }

  std::string ToString(const QSize &size)
  {
    return std::to_string(size.width()) + "x" + std::to_string(size.height());
  }
}

class QmitkIconThemeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkIconThemeTestSuite);
  MITK_TEST(ThemeSwitchRecolorsExistingIcon);
  MITK_TEST(AccentColorFollowsTheme);
  MITK_TEST(CustomColorOverridesThemeColor);
  MITK_TEST(RefreshEmitsChanged);
  MITK_TEST(ColorsFollowRefresh);
  MITK_TEST(RendersAtTheRequestedSize);
  MITK_TEST(RendersInDevicePixels);
  MITK_TEST(KeepsTheAspectRatio);
  MITK_TEST(GeneratesModeVariants);
  MITK_TEST(SvgBytesAreCopied);
  MITK_TEST(MissingResourceYieldsNullIcon);
  MITK_TEST(InvalidSvgYieldsNullIcon);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    EnsureQApplication();
  }

  void tearDown() override
  {
    qApp->setStyleSheet(QString());
    QmitkIconTheme::Refresh();
  }

  void ThemeSwitchRecolorsExistingIcon()
  {
    ApplyTheme("#ff0000", "#ffffff");

    const QIcon icon = QmitkIconTheme::GetIcon(Svg("#00ff00", 16));
    CPPUNIT_ASSERT_EQUAL(std::string("#ff0000"), CenterColor(icon, 16));

    ApplyTheme("#0000ff", "#ffffff");
    CPPUNIT_ASSERT_EQUAL(std::string("#0000ff"), CenterColor(icon, 16));
  }

  void AccentColorFollowsTheme()
  {
    ApplyTheme("#000000", "#ff0000");

    const QIcon icon = QmitkIconTheme::GetIcon(Svg("#ff00ff", 16));
    CPPUNIT_ASSERT_EQUAL(std::string("#ff0000"), CenterColor(icon, 16));

    ApplyTheme("#000000", "#0000ff");
    CPPUNIT_ASSERT_EQUAL(std::string("#0000ff"), CenterColor(icon, 16));
  }

  void CustomColorOverridesThemeColor()
  {
    ApplyTheme("#ff0000", "#ffffff");

    const QIcon icon = QmitkIconTheme::GetIcon(Svg("#00ff00", 16), "#123456");
    CPPUNIT_ASSERT_EQUAL(std::string("#123456"), CenterColor(icon, 16));

    ApplyTheme("#0000ff", "#ffffff");
    CPPUNIT_ASSERT_EQUAL(std::string("#123456"), CenterColor(icon, 16));

    // A custom color equal to either magic color must not be themed away
    const QIcon greenIcon = QmitkIconTheme::GetIcon(Svg("#00ff00", 16), "#00ff00");
    CPPUNIT_ASSERT_EQUAL(std::string("#00ff00"), CenterColor(greenIcon, 16));

    const QIcon magentaIcon = QmitkIconTheme::GetIcon(Svg("#00ff00", 16), "#ff00ff");
    CPPUNIT_ASSERT_EQUAL(std::string("#ff00ff"), CenterColor(magentaIcon, 16));
  }

  void RefreshEmitsChanged()
  {
    int count = 0;
    const auto connection = QObject::connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, [&count] { ++count; });

    QmitkIconTheme::Refresh();
    QmitkIconTheme::Refresh();

    QObject::disconnect(connection);
    CPPUNIT_ASSERT_EQUAL(2, count);
  }

  void ColorsFollowRefresh()
  {
    ApplyTheme("#123456", "#abcdef");

    CPPUNIT_ASSERT_EQUAL(std::string("#123456"), QmitkIconTheme::GetColor().toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("#abcdef"), QmitkIconTheme::GetAccentColor().toStdString());
  }

  void RendersAtTheRequestedSize()
  {
    ApplyTheme("#ff0000", "#ffffff");

    // A declared size, a viewBox only, and the relative size some icons in the tree use
    const std::vector<QByteArray> svgs = {
      Svg("#00ff00", 48),
      Svg("#00ff00", 128, 128, QStringLiteral("viewBox=\"0 0 128 128\"")),
      Svg("#00ff00", 1792, 1792, QStringLiteral("width=\"10%\" height=\"10%\" viewBox=\"0 0 1792 1792\""))
    };

    for (const auto &svg : svgs)
    {
      const QIcon icon = QmitkIconTheme::GetIcon(svg);

      CPPUNIT_ASSERT(!icon.isNull());
      CPPUNIT_ASSERT_EQUAL(std::string("16x16"), ToString(icon.actualSize(QSize(16, 16))));
      CPPUNIT_ASSERT_EQUAL(std::string("256x256"), ToString(icon.actualSize(QSize(256, 256))));
      CPPUNIT_ASSERT_EQUAL(std::string("256x256"), ToString(icon.pixmap(256, 256).size()));
      CPPUNIT_ASSERT_EQUAL(std::string("#ff0000"), CenterColor(icon, 256));
    }
  }

  void RendersInDevicePixels()
  {
    ApplyTheme("#ff0000", "#ffffff");

    const QIcon icon = QmitkIconTheme::GetIcon(Svg("#00ff00", 48));
    const auto pixmap = icon.pixmap(QSize(16, 16), 2.0);

    CPPUNIT_ASSERT_EQUAL(std::string("32x32"), ToString(pixmap.size()));
    CPPUNIT_ASSERT_EQUAL(2.0, pixmap.devicePixelRatio());
    CPPUNIT_ASSERT_EQUAL(std::string("#ff0000"), ColorAt(pixmap, 16, 16));
  }

  void KeepsTheAspectRatio()
  {
    ApplyTheme("#ff0000", "#ffffff");

    const QIcon icon = QmitkIconTheme::GetIcon(Svg("#00ff00", 48, 24));

    CPPUNIT_ASSERT_EQUAL(std::string("32x16"), ToString(icon.actualSize(QSize(32, 32))));
    CPPUNIT_ASSERT_EQUAL(std::string("32x16"), ToString(icon.pixmap(32, 32).size()));
  }

  void GeneratesModeVariants()
  {
    ApplyTheme("#ff0000", "#ffffff");

    const QIcon icon = QmitkIconTheme::GetIcon(Svg("#00ff00", 48));
    const auto disabled = icon.pixmap(QSize(16, 16), 1.0, QIcon::Disabled);

    CPPUNIT_ASSERT_EQUAL(std::string("16x16"), ToString(disabled.size()));
    CPPUNIT_ASSERT(ColorAt(disabled, 8, 8) != std::string("#ff0000"));
  }

  void SvgBytesAreCopied()
  {
    ApplyTheme("#ff0000", "#ffffff");

    const auto svg = Svg("#00ff00", 16);
    std::vector<char> buffer(svg.begin(), svg.end());

    const QIcon icon = QmitkIconTheme::GetIcon(QByteArray::fromRawData(buffer.data(), static_cast<qsizetype>(buffer.size())));

    // The buffer stays alive but no longer holds an SVG
    std::fill(buffer.begin(), buffer.end(), 'x');

    CPPUNIT_ASSERT_EQUAL(std::string("#ff0000"), CenterColor(icon, 16));
  }

  void MissingResourceYieldsNullIcon()
  {
    CPPUNIT_ASSERT(QmitkIconTheme::GetIcon(QStringLiteral(":/QmitkIconThemeTest/missing.svg")).isNull());
  }

  void InvalidSvgYieldsNullIcon()
  {
    const QIcon icon = QmitkIconTheme::GetIcon(QByteArrayLiteral("not an svg"));

    CPPUNIT_ASSERT(icon.isNull());
    CPPUNIT_ASSERT(icon.pixmap(16, 16).isNull());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkIconTheme)
