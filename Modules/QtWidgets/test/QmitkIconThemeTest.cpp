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

  QByteArray Svg(const QString &fill, int size)
  {
    return QStringLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%2\" height=\"%2\">"
                          "<rect width=\"%2\" height=\"%2\" fill=\"%1\"/></svg>")
      .arg(fill).arg(size).toUtf8();
  }

  std::string CenterColor(const QIcon &icon, int size)
  {
    return icon.pixmap(size, size).toImage().pixelColor(size / 2, size / 2).name().toStdString();
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
  MITK_TEST(RefreshEmitsChanged);
  MITK_TEST(ColorsFollowRefresh);
  MITK_TEST(MatchesStaticallyBakedIconGeometry);
  MITK_TEST(SvgBytesAreCopied);
  MITK_TEST(MissingResourceYieldsNullIcon);
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

  void MatchesStaticallyBakedIconGeometry()
  {
    ApplyTheme("#ff0000", "#ffffff");

    const auto svg = Svg("#00ff00", 48);
    const QIcon icon = QmitkIconTheme::GetIcon(svg);
    const QIcon reference(QPixmap::fromImage(QImage::fromData(svg)));

    const auto assertSameGeometry = [&icon, &reference](const QSize &size, qreal devicePixelRatio)
    {
      const auto pixmap = icon.pixmap(size, devicePixelRatio);
      const auto referencePixmap = reference.pixmap(size, devicePixelRatio);

      CPPUNIT_ASSERT_EQUAL(ToString(referencePixmap.size()), ToString(pixmap.size()));
      CPPUNIT_ASSERT_EQUAL(referencePixmap.devicePixelRatio(), pixmap.devicePixelRatio());
    };

    assertSameGeometry(QSize(16, 16), 1.0);
    assertSameGeometry(QSize(16, 16), 2.0);
    assertSameGeometry(QSize(256, 256), 1.0);

    // A request at ratio 2 must come back in device pixels
    CPPUNIT_ASSERT_EQUAL(std::string("32x32"), ToString(icon.pixmap(QSize(16, 16), 2.0).size()));

    // A pixmap-backed icon never grows beyond its natural size
    CPPUNIT_ASSERT_EQUAL(std::string("48x48"), ToString(icon.actualSize(QSize(256, 256))));
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
};

MITK_TEST_SUITE_REGISTRATION(QmitkIconTheme)
