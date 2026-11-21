/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkReport.h>

#include <mitkException.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QString>
#include <QTemporaryDir>
#include <QTime>

#include <string>
#include <utility>
#include <vector>

namespace
{
  std::string ToString(const QString& text)
  {
    return text.toStdString();
  }

  QmitkReport MakeReport(const QString& title, const QString& html)
  {
    QmitkReport report;
    report.Id = 5;
    report.Title = title;
    report.Html = html;
    report.Modified = QDateTime(QDate(2026, 11, 20), QTime(9, 5));
    return report;
  }

  /** The part of a page between <main> and </main>. */
  QString GetBody(const QString& document)
  {
    const auto start = document.indexOf(QStringLiteral("<main>\n")) + 7;
    const auto end = document.lastIndexOf(QStringLiteral("\n</main>"));
    return document.mid(start, end - start);
  }

  const QString Policy = QStringLiteral(
    "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; img-src data:; "
    "style-src 'unsafe-inline'; form-action 'none'; base-uri 'none'\">");
}

class QmitkReportTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkReportTestSuite);
  MITK_TEST(TestTitleIsNormalized);
  MITK_TEST(TestFileBaseNames);
  MITK_TEST(TestDocumentForTheEditorIsTheBodyInAPage);
  MITK_TEST(TestDarkThemeIsAClassOfTheRoot);
  MITK_TEST(TestExportedDocumentHasThePolicyBeforeAnyContent);
  MITK_TEST(TestExportInlinesQuotedImages);
  MITK_TEST(TestExportLeavesOtherReferencesAlone);
  MITK_TEST(TestExportShowsMetaBaseAndFormAsText);
  MITK_TEST(TestExportLeavesScriptsToThePolicy);
  MITK_TEST(TestTitleMarkupIsEscaped);
  MITK_TEST(TestWriteDocument);
  MITK_TEST(TestWriteDocumentFailure);
  CPPUNIT_TEST_SUITE_END();

  static void AssertContains(const QString& text, const QString& part)
  {
    CPPUNIT_ASSERT_MESSAGE("Expected to contain: " + ToString(part) + "\nin: " + ToString(text), text.contains(part));
  }

  static void AssertContainsNot(const QString& text, const QString& part)
  {
    CPPUNIT_ASSERT_MESSAGE("Expected not to contain: " + ToString(part) + "\nin: " + ToString(text), !text.contains(part));
  }

public:
  void TestTitleIsNormalized()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("a b c"), ToString(QmitkReport::NormalizeTitle(QStringLiteral("  a \t b\n c  "))));
    CPPUNIT_ASSERT_EQUAL(std::string(""), ToString(QmitkReport::NormalizeTitle(QString())));
    CPPUNIT_ASSERT_EQUAL(std::string(""), ToString(QmitkReport::NormalizeTitle(QStringLiteral(" \t\n "))));

    // Markup stays; the page escapes it.
    CPPUNIT_ASSERT_EQUAL(std::string("<b>x</b>"), ToString(QmitkReport::NormalizeTitle(QStringLiteral(" <b>x</b> "))));

    CPPUNIT_ASSERT_EQUAL(200, static_cast<int>(QmitkReport::NormalizeTitle(QString(250, 'a')).size()));
    CPPUNIT_ASSERT_EQUAL(200, static_cast<int>(QmitkReport::NormalizeTitle(QString(200, 'a') + 'b').size()));

    // The cut does not leave a space at the end, or half of a character outside of the basic plane.
    CPPUNIT_ASSERT_EQUAL(199, static_cast<int>(QmitkReport::NormalizeTitle(QString(199, 'a') + QStringLiteral(" bbb")).size()));

    const auto emoji = QString::fromUcs4(U"\U0001F600");
    CPPUNIT_ASSERT_EQUAL(199, static_cast<int>(QmitkReport::NormalizeTitle(QString(199, 'a') + emoji + 'b').size()));
    CPPUNIT_ASSERT_EQUAL(200, static_cast<int>(QmitkReport::NormalizeTitle(QString(198, 'a') + emoji + 'b').size()));
  }

  void TestFileBaseNames()
  {
    const std::vector<std::pair<QString, QString>> cases = {
      { QStringLiteral("Liver CT"), QStringLiteral("Liver CT") },
      { QStringLiteral("a/b\\c:d*e?f\"g<h>i|j"), QStringLiteral("a b c d e f g h i j") },
      { QStringLiteral("a\x01 b\x1f""c"), QStringLiteral("a b c") },
      { QStringLiteral("  spaced \t out  "), QStringLiteral("spaced out") },
      { QStringLiteral("Größe ✓"), QStringLiteral("Größe ✓") },
      { QString(), QStringLiteral("Report 5") },
      { QStringLiteral("   "), QStringLiteral("Report 5") },
      { QStringLiteral("///"), QStringLiteral("Report 5") },
      { QStringLiteral("<?>"), QStringLiteral("Report 5") },
      { QString(100, 'x'), QString(80, 'x') },
      { QString(79, 'x') + QStringLiteral("/y"), QString(79, 'x') },
      // A character outside the BMP across the cut is dropped whole.
      { QString(79, 'x') + QString::fromUtf8("\xF0\x9F\x99\x82"), QString(79, 'x') },
      { QStringLiteral("Results..."), QStringLiteral("Results") },
      { QStringLiteral("CON"), QStringLiteral("Report CON") },
      { QStringLiteral("lpt1.final"), QStringLiteral("Report lpt1.final") },
      { QStringLiteral("Console"), QStringLiteral("Console") },
    };

    for (const auto& [title, expected] : cases)
    {
      const auto report = MakeReport(title, QString());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Title: " + ToString(title), ToString(expected), ToString(report.GetFileBaseName()));
    }
  }

  void TestDocumentForTheEditorIsTheBodyInAPage()
  {
    const auto html = QStringLiteral("<p>x</p><meta name=\"a\"><form></form><script>1</script><img src=\"image-1.png\">");
    auto report = MakeReport(QStringLiteral("My report"), html);
    report.Images.insert(QStringLiteral("image-1.png"), "PNG");

    const auto document = report.GetDocument(false, false);

    CPPUNIT_ASSERT(document.startsWith(QStringLiteral("<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n<title>My report</title>\n<style>\n")));
    AssertContains(document, QStringLiteral("<h1>My report</h1>\n<p class=\"report-meta\">MITK Workbench, 2026-11-20 09:05</p>"));
    CPPUNIT_ASSERT(document.endsWith(QStringLiteral("\n</main>\n</body>\n</html>\n")));
    AssertContainsNot(document, QStringLiteral("Content-Security-Policy"));

    // Nothing of the body is touched: the editor resolves the images itself.
    CPPUNIT_ASSERT_EQUAL(ToString(html), ToString(GetBody(document)));

    // The style sheet of the module is in the page.
    const auto styleStart = document.indexOf(QStringLiteral("<style>\n")) + 8;
    const auto styleEnd = document.indexOf(QStringLiteral("</style>"));
    CPPUNIT_ASSERT(styleEnd - styleStart > 1000);
  }

  void TestDarkThemeIsAClassOfTheRoot()
  {
    const auto report = MakeReport(QStringLiteral("t"), QStringLiteral("<p>x</p>"));

    AssertContains(report.GetDocument(false, true), QStringLiteral("<html lang=\"en\" class=\"dark\">"));
    AssertContains(report.GetDocument(true, true), QStringLiteral("<html lang=\"en\" class=\"dark\">"));
    AssertContainsNot(report.GetDocument(true, false), QStringLiteral("dark\""));
  }

  void TestExportedDocumentHasThePolicyBeforeAnyContent()
  {
    const auto report = MakeReport(QStringLiteral("t"), QStringLiteral("<p>x</p>"));
    const auto document = report.GetDocument(true, false);

    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(document.count(Policy)));
    CPPUNIT_ASSERT(document.indexOf(Policy) < document.indexOf(QStringLiteral("<title>")));
    CPPUNIT_ASSERT(document.indexOf(Policy) > document.indexOf(QStringLiteral("<meta charset")));

    const auto noPrefetch = QStringLiteral("<meta http-equiv=\"x-dns-prefetch-control\" content=\"off\">");
    CPPUNIT_ASSERT(document.indexOf(noPrefetch) > 0 && document.indexOf(noPrefetch) < document.indexOf(QStringLiteral("<title>")));
  }

  void TestExportInlinesQuotedImages()
  {
    auto report = MakeReport(QStringLiteral("image-1.png"),
      QStringLiteral("<img src=\"image-1.png\"><img SRC = 'image-1.png' alt=\"image-1.png\"><img src=\"image-2.png\">"
                     "<img src=\"data:image/png;base64,QQ==\">"));
    report.Images.insert(QStringLiteral("image-1.png"), "PNG");

    const auto body = GetBody(report.GetDocument(true, false));
    const auto inlined = QStringLiteral("data:image/png;base64,UE5H");

    CPPUNIT_ASSERT_EQUAL(
      ToString(QStringLiteral("<img src=\"%1\"><img SRC = '%1' alt=\"image-1.png\"><img src=\"image-2.png\">"
                              "<img src=\"data:image/png;base64,QQ==\">").arg(inlined)),
      ToString(body));

    // The title is no source.
    AssertContains(report.GetDocument(true, false), QStringLiteral("<title>image-1.png</title>"));
  }

  void TestExportLeavesOtherReferencesAlone()
  {
    auto report = MakeReport(QStringLiteral("t"),
      QStringLiteral("<img src=image-1.png><div style=\"background:url(image-1.png)\"></div>"
                     "<a href=\"image-1.png\">x</a><img srcset=\"image-1.png 2x\">"));
    report.Images.insert(QStringLiteral("image-1.png"), "PNG");

    CPPUNIT_ASSERT_EQUAL(ToString(report.Html), ToString(GetBody(report.GetDocument(true, false))));
  }

  void TestExportShowsMetaBaseAndFormAsText()
  {
    const auto report = MakeReport(QStringLiteral("t"),
      QStringLiteral("<META http-equiv=\"refresh\" content=\"0;url=http://x\"><base href=\"http://x\"><form action=\"x\"></form>"
                     "<FORM\n><meta/><Base/><p>"
                     "<link rel=\"dns-prefetch\" href=\"//x\"><LINK/><linkage>l</linkage>"
                     "<iframe srcdoc=\"&lt;link rel=preconnect href=https://x&gt;\"></iframe><IFRAME/><iframes>i</iframes>"
                     "<metadata>m</metadata><formatted>f</formatted><basefont><meta"));

    CPPUNIT_ASSERT_EQUAL(
      std::string("&lt;META http-equiv=\"refresh\" content=\"0;url=http://x\">&lt;base href=\"http://x\">&lt;form action=\"x\"></form>"
                  "&lt;FORM\n>&lt;meta/>&lt;Base/><p>"
                  "&lt;link rel=\"dns-prefetch\" href=\"//x\">&lt;LINK/><linkage>l</linkage>"
                  "&lt;iframe srcdoc=\"&lt;link rel=preconnect href=https://x&gt;\"></iframe>&lt;IFRAME/><iframes>i</iframes>"
                  "<metadata>m</metadata><formatted>f</formatted><basefont><meta"),
      ToString(GetBody(report.GetDocument(true, false))));

    // The editor's page is for MITK itself and keeps them.
    CPPUNIT_ASSERT_EQUAL(ToString(report.Html), ToString(GetBody(report.GetDocument(false, false))));
  }

  void TestExportLeavesScriptsToThePolicy()
  {
    const auto report = MakeReport(QStringLiteral("t"), QStringLiteral("<script>alert(1)</script><p onclick=\"x()\">y</p>"));
    const auto document = report.GetDocument(true, false);

    CPPUNIT_ASSERT_EQUAL(ToString(report.Html), ToString(GetBody(document)));

    // The policy names no script source, so default-src 'none' applies to scripts.
    AssertContains(document, QStringLiteral("default-src 'none'"));
    AssertContainsNot(document, QStringLiteral("script-src"));
  }

  void TestTitleMarkupIsEscaped()
  {
    const auto report = MakeReport(QStringLiteral("<b>\"x\" & </title><script>"), QStringLiteral("<p>x</p>"));
    const auto escaped = QStringLiteral("&lt;b&gt;&quot;x&quot; &amp; &lt;/title&gt;&lt;script&gt;");

    for (const auto forExport : { false, true })
    {
      const auto document = report.GetDocument(forExport, false);

      AssertContains(document, QStringLiteral("<title>") + escaped + QStringLiteral("</title>"));
      AssertContains(document, QStringLiteral("<h1>") + escaped + QStringLiteral("</h1>"));
      AssertContainsNot(document, QStringLiteral("<script>"));
    }
  }

  void TestWriteDocument()
  {
    QTemporaryDir folder;
    CPPUNIT_ASSERT(folder.isValid());

    auto report = MakeReport(QStringLiteral("Größe"), QStringLiteral("<p>ä</p><img src=\"image-1.png\">"));
    report.Images.insert(QStringLiteral("image-1.png"), "PNG");

    const auto path = QDir(folder.path()).filePath(QStringLiteral("report.html"));

    {
      QFile old(path);
      CPPUNIT_ASSERT(old.open(QIODevice::WriteOnly));
      old.write(QByteArray(100000, 'o'));
    }

    report.WriteDocument(path, true);

    QFile file(path);
    CPPUNIT_ASSERT(file.open(QIODevice::ReadOnly));
    const auto content = file.readAll();

    CPPUNIT_ASSERT(content == report.GetDocument(true, true).toUtf8());
    AssertContains(QString::fromUtf8(content), QStringLiteral("data:image/png;base64,UE5H"));
    AssertContains(QString::fromUtf8(content), QStringLiteral("<title>Größe</title>"));
  }

  void TestWriteDocumentFailure()
  {
    QTemporaryDir folder;
    CPPUNIT_ASSERT(folder.isValid());

    const auto report = MakeReport(QStringLiteral("t"), QStringLiteral("<p>x</p>"));
    const auto path = QDir(folder.path()).filePath(QStringLiteral("missing/report.html"));

    try
    {
      report.WriteDocument(path, false);
      CPPUNIT_FAIL("Expected an exception.");
    }
    catch (const mitk::Exception& e)
    {
      CPPUNIT_ASSERT_MESSAGE(e.GetDescription(), std::string(e.GetDescription()).starts_with("Cannot write "));
    }

    CPPUNIT_ASSERT(!QFile::exists(path));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkReport)
