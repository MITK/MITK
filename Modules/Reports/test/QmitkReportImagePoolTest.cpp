/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkReportImagePool.h>

#include <mitkException.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QByteArray>
#include <QMap>
#include <QString>

#include <string>

namespace
{
  std::string ToString(const QString& text)
  {
    return text.toStdString();
  }

  QString Img(const QString& source)
  {
    return QStringLiteral("<img src=\"%1\">").arg(source);
  }
}

class QmitkReportImagePoolTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkReportImagePoolTestSuite);
  MITK_TEST(TestNamesCountUp);
  MITK_TEST(TestCollectKnownImages);
  MITK_TEST(TestCollectFindsSourcesInTheirNotations);
  MITK_TEST(TestCollectIgnoresWhatIsNoQuotedSource);
  MITK_TEST(TestDataUrisAreNoPoolImages);
  MITK_TEST(TestUnknownImagesAreNamedOnce);
  MITK_TEST(TestOwnImagesComeFirst);
  MITK_TEST(TestPoolKeepsTheLast100);
  MITK_TEST(TestNamesAreNotReusedAfterEviction);
  CPPUNIT_TEST_SUITE_END();

  static std::string Collect(const QmitkReportImagePool& pool, const QString& html)
  {
    QString result;
    const auto images = pool.CollectImages(html);

    for (auto it = images.begin(); it != images.end(); ++it)
      result += it.key() + "=" + QString::fromLatin1(it.value()) + ";";

    return ToString(result);
  }

  static std::string ErrorOf(const QmitkReportImagePool& pool, const QString& html, const QMap<QString, QByteArray>& own = {})
  {
    try
    {
      pool.CollectImages(html, own);
    }
    catch (const mitk::Exception& e)
    {
      return e.GetDescription();
    }

    return "";
  }

public:
  void TestNamesCountUp()
  {
    QmitkReportImagePool pool;

    CPPUNIT_ASSERT_EQUAL(std::string("image-1.png"), ToString(pool.AddImage("a")));
    CPPUNIT_ASSERT_EQUAL(std::string("image-2.png"), ToString(pool.AddImage("b")));
    CPPUNIT_ASSERT_EQUAL(std::string("image-3.png"), ToString(pool.AddImage("a")));
  }

  void TestCollectKnownImages()
  {
    QmitkReportImagePool pool;
    pool.AddImage("one");
    pool.AddImage("two");

    CPPUNIT_ASSERT_EQUAL(std::string(""), Collect(pool, QStringLiteral("<p>no images</p>")));
    CPPUNIT_ASSERT_EQUAL(std::string("image-2.png=two;"), Collect(pool, Img("image-2.png")));

    // Twice the same image is one entry.
    CPPUNIT_ASSERT_EQUAL(std::string("image-1.png=one;image-2.png=two;"),
      Collect(pool, Img("image-2.png") + Img("image-1.png") + Img("image-2.png")));
  }

  void TestCollectFindsSourcesInTheirNotations()
  {
    QmitkReportImagePool pool;
    pool.AddImage("one");

    CPPUNIT_ASSERT_EQUAL(std::string("image-1.png=one;"), Collect(pool, QStringLiteral("<img src='image-1.png'>")));
    CPPUNIT_ASSERT_EQUAL(std::string("image-1.png=one;"), Collect(pool, QStringLiteral("<IMG SRC = \"image-1.png\">")));
    CPPUNIT_ASSERT_EQUAL(std::string("image-1.png=one;"), Collect(pool, QStringLiteral("<img\nsrc\n=\n\"image-1.png\">")));
  }

  void TestCollectIgnoresWhatIsNoQuotedSource()
  {
    const QmitkReportImagePool pool;

    // Only quoted src attributes count: unquoted values, CSS and links are not collected, so they never fail.
    CPPUNIT_ASSERT_EQUAL(std::string(""), ErrorOf(pool, QStringLiteral("<img src=image-9.png>")));
    CPPUNIT_ASSERT_EQUAL(std::string(""), ErrorOf(pool, QStringLiteral("<p style=\"background:url(image-9.png)\">x</p>")));
    CPPUNIT_ASSERT_EQUAL(std::string(""), ErrorOf(pool, QStringLiteral("<a href=\"image-9.png\">x</a>")));
    CPPUNIT_ASSERT_EQUAL(std::string(""), ErrorOf(pool, QStringLiteral("<img srcset=\"image-9.png 2x\">")));
  }

  void TestDataUrisAreNoPoolImages()
  {
    const QmitkReportImagePool pool;

    CPPUNIT_ASSERT_EQUAL(std::string(""), Collect(pool, Img("data:image/png;base64,AAAA")));
    CPPUNIT_ASSERT_EQUAL(std::string(""), Collect(pool, Img("DATA:image/svg+xml;utf8,<svg/>")));
  }

  void TestUnknownImagesAreNamedOnce()
  {
    QmitkReportImagePool pool;
    pool.AddImage("one");

    CPPUNIT_ASSERT_EQUAL(std::string("Unknown images: b.png, a.png."),
      ErrorOf(pool, Img("b.png") + Img("image-1.png") + Img("a.png") + Img("b.png")));

    // Remote and local files are no images the report can show.
    CPPUNIT_ASSERT_EQUAL(std::string("Unknown images: https://example.org/x.png, C:\\x.png."),
      ErrorOf(pool, Img("https://example.org/x.png") + Img("C:\\x.png")));
  }

  void TestOwnImagesComeFirst()
  {
    QmitkReportImagePool pool;
    pool.AddImage("pooled");

    QMap<QString, QByteArray> own;
    own.insert("image-1.png", "kept");
    own.insert("image-7.png", "dropped from the pool");

    const auto images = pool.CollectImages(Img("image-1.png") + Img("image-7.png"), own);

    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(images.size()));
    CPPUNIT_ASSERT_EQUAL(std::string("kept"), images.value("image-1.png").toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("dropped from the pool"), images.value("image-7.png").toStdString());

    // Only what the HTML still uses is kept.
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(pool.CollectImages(Img("image-7.png"), own).size()));
    CPPUNIT_ASSERT_EQUAL(std::string("Unknown images: image-8.png."), ErrorOf(pool, Img("image-8.png"), own));
  }

  void TestPoolKeepsTheLast100()
  {
    QmitkReportImagePool pool;

    for (int i = 0; i < 100; ++i)
      pool.AddImage("x");

    CPPUNIT_ASSERT_EQUAL(std::string(""), ErrorOf(pool, Img("image-1.png") + Img("image-100.png")));

    pool.AddImage("x");

    CPPUNIT_ASSERT_EQUAL(std::string("Unknown images: image-1.png."), ErrorOf(pool, Img("image-1.png")));
    CPPUNIT_ASSERT_EQUAL(std::string(""), ErrorOf(pool, Img("image-2.png") + Img("image-101.png")));

    // A report that kept the dropped image still has it.
    QMap<QString, QByteArray> own;
    own.insert("image-1.png", "kept");
    CPPUNIT_ASSERT_EQUAL(std::string(""), ErrorOf(pool, Img("image-1.png"), own));
  }

  void TestNamesAreNotReusedAfterEviction()
  {
    QmitkReportImagePool pool;

    for (int i = 0; i < 250; ++i)
      pool.AddImage("x");

    CPPUNIT_ASSERT_EQUAL(std::string("image-251.png"), ToString(pool.AddImage("y")));
    CPPUNIT_ASSERT_EQUAL(std::string("image-251.png=y;"), Collect(pool, Img("image-251.png")));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkReportImagePool)
