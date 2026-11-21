/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkHtmlWidget.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QString>

#include <vector>

namespace
{
  QString Svg(const QString &content)
  {
    return QStringLiteral("<svg width=\"20\" height=\"10\">%1</svg>").arg(content);
  }

  QString Page(const QString &body)
  {
    return QStringLiteral("<html><head><title>t</title></head><body><p>text</p>%1</body></html>").arg(body);
  }
}

class QmitkHtmlWidgetSvgCheckTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkHtmlWidgetSvgCheckTestSuite);
  MITK_TEST(TestHtmlWithoutSvgHasNoProblem);
  MITK_TEST(TestSelfContainedChartIsDrawable);
  MITK_TEST(TestReferencesToOwnElementsAreDrawable);
  MITK_TEST(TestCharacterReferencesAreDrawable);
  MITK_TEST(TestProcessingInstructionIsRefused);
  MITK_TEST(TestDoctypeIsRefused);
  MITK_TEST(TestReferencesToOtherPlacesAreRefused);
  MITK_TEST(TestMalformedXmlIsRefused);
  MITK_TEST(TestAttributeOutsideOfSvgIsNotChecked);
  MITK_TEST(TestMessageNamesTheSvgThatIsBad);
  MITK_TEST(TestNestedSvgCountsAsPartOfTheOutermost);
  MITK_TEST(TestOtherElementsAreNoSvg);
  MITK_TEST(TestSvgOutOfLowerCaseIsRefused);
  MITK_TEST(TestUnclosedSvgIsRefused);
  CPPUNIT_TEST_SUITE_END();

  static void AssertDrawable(const QString &html)
  {
    const auto problem = QmitkHtmlWidget::FindUndrawableSvg(html);
    CPPUNIT_ASSERT_MESSAGE("Expected to be drawable: " + html.toStdString() + "\nbut: " + problem.toStdString(), problem.isEmpty());
  }

  static void AssertRefused(const QString &html, int index, const QString &reason)
  {
    const auto problem = QmitkHtmlWidget::FindUndrawableSvg(html);
    const auto prefix = QStringLiteral("Inline svg %1 cannot be drawn, as ").arg(index);

    CPPUNIT_ASSERT_MESSAGE("Expected a problem for: " + html.toStdString(), !problem.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Prefix expected in: " + problem.toStdString(), problem.startsWith(prefix));
    CPPUNIT_ASSERT_MESSAGE("Expected reason \"" + reason.toStdString() + "\" in: " + problem.toStdString(), problem.contains(reason));
    CPPUNIT_ASSERT_MESSAGE("A period should end: " + problem.toStdString(), problem.endsWith(QLatin1Char('.')));
  }

public:
  void TestHtmlWithoutSvgHasNoProblem()
  {
    AssertDrawable(QString());
    AssertDrawable(Page(QString()));
    AssertDrawable(Page(QStringLiteral("<a href=\"http://example.com\">a &plusmn; b & c</a><img src=\"x.png\">")));
  }

  void TestSelfContainedChartIsDrawable()
  {
    AssertDrawable(Page(Svg(QStringLiteral(
      "<rect x=\"0\" y=\"0\" width=\"20\" height=\"10\" fill=\"red\"/>"
      "<text x=\"1\" y=\"8\">Dose &amp; &lt;volume&gt;</text>"
      "<!-- a comment, which is no processing instruction -->"
      "<![CDATA[ & < ]]>"))));

    AssertDrawable(Page(QStringLiteral("<svg viewBox=\"0 0 1 1\"/>")));
    AssertDrawable(Page(QStringLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1\" height=\"1\"></svg>")));
  }

  void TestReferencesToOwnElementsAreDrawable()
  {
    const std::vector<QString> contents = {
      QStringLiteral("<defs><linearGradient id=\"g\"><stop offset=\"0\"/></linearGradient><path id=\"p\" d=\"M0 0\"/></defs>"
                     "<rect width=\"1\" height=\"1\" fill=\"url(#g)\"/><use href=\"#p\"/>"),
      QStringLiteral("<path id=\"p\" d=\"M0 0\"/><use xlink:href=\"#p\"/>"),
      QStringLiteral("<path id=\"p\" d=\"M0 0\"/><use xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"#p\"/>"),
      QStringLiteral("<a href=\"#top\"><text>to top</text></a>")
    };

    for (const auto &content : contents)
      AssertDrawable(Page(Svg(content)));
  }

  void TestCharacterReferencesAreDrawable()
  {
    AssertDrawable(Page(Svg(QStringLiteral("<text>45&#176; 45&#xB0; &#x41;</text>"))));
  }

  void TestProcessingInstructionIsRefused()
  {
    AssertRefused(Page(Svg(QStringLiteral("<?xml-stylesheet href=\"file:///C:/secret.css\" type=\"text/css\"?><rect/>"))),
                  1, QStringLiteral("it has a processing instruction"));
    AssertRefused(Page(Svg(QStringLiteral("<?xml-stylesheet href=\"http://example.com/a.css\"?>"))),
                  1, QStringLiteral("it has a processing instruction"));
  }

  void TestDoctypeIsRefused()
  {
    // Inline, the markup starts with the svg element, so a DOCTYPE cannot be recognized
    // as one: the XML reader gives up on it before.
    AssertRefused(Page(Svg(QStringLiteral("<!DOCTYPE svg [<!ENTITY e SYSTEM \"file:///C:/secret.txt\">]><text>&e;</text>"))),
                  1, QStringLiteral("it is not well-formed XML"));
  }

  void TestReferencesToOtherPlacesAreRefused()
  {
    const std::vector<QString> references = {
      QStringLiteral("C:\\data\\image.png"),
      QStringLiteral("/etc/passwd"),
      QStringLiteral("file:///C:/data/image.svg"),
      QStringLiteral("\\\\server\\share\\image.svg"),
      QStringLiteral("//server/share/image.svg"),
      QStringLiteral("http://example.com/image.png"),
      QStringLiteral("https://example.com/image.png"),
      QStringLiteral("data:image/svg+xml;base64,AAAA"),
      QStringLiteral("image.png"),
      QStringLiteral("./#fragment"),
      QStringLiteral(" #fragment"),
      QStringLiteral("")
    };

    for (const auto &reference : references)
    {
      const auto outside = QStringLiteral("it refers to something outside of itself (href=\"%1\")").arg(reference);
      AssertRefused(Page(Svg(QStringLiteral("<use href='%1'/>").arg(reference))), 1, outside);

      // An image is refused as such, as no reference helps it.
      const auto image = QStringLiteral("it has an image element (href=\"%1\")").arg(reference);
      AssertRefused(Page(Svg(QStringLiteral("<image href=\"%1\" width=\"1\" height=\"1\"/>").arg(reference))), 1, image);
      AssertRefused(Page(Svg(QStringLiteral("<image xlink:href=\"%1\" width=\"1\" height=\"1\"/>").arg(reference))), 1, image);
    }

    // The declaration of the prefix does not make the reference one to the svg's own elements.
    AssertRefused(Page(Svg(QStringLiteral("<image xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"http://example.com/a.png\"/>"))),
                  1, QStringLiteral("href=\"http://example.com/a.png\""));

    // An image takes a "#" reference as the path of a file, which Windows resolves without a folder "#".
    for (const auto &reference : { QStringLiteral("#p"), QStringLiteral("#/../../Users/Public/Pictures/x.png") })
    {
      const auto expected = QStringLiteral("it has an image element (href=\"%1\")").arg(reference);

      AssertRefused(Page(Svg(QStringLiteral("<path id=\"p\" d=\"M0 0\"/><image href=\"%1\" width=\"1\" height=\"1\"/>").arg(reference))), 1, expected);
      AssertRefused(Page(Svg(QStringLiteral("<image xlink:href=\"%1\" width=\"1\" height=\"1\"/>").arg(reference))), 1, expected);
    }
  }

  void TestMalformedXmlIsRefused()
  {
    const std::vector<QString> contents = {
      QStringLiteral("<text>1 &plusmn; 2</text>"),
      QStringLiteral("<text>&nbsp;</text>"),
      QStringLiteral("<text>this & that</text>"),
      QStringLiteral("<rect width=10 height=\"1\"/>"),
      QStringLiteral("<g><rect/>"),
      QStringLiteral("<g><rect></g>"),
      QStringLiteral("<rect></g>"),
      QStringLiteral("<rect width=\"1\" width=\"2\"/>"),
      QStringLiteral("<rect width=\"1\"height=\"2\"/>")
    };

    for (const auto &content : contents)
    {
      const auto problem = QmitkHtmlWidget::FindUndrawableSvg(Page(Svg(content)));

      CPPUNIT_ASSERT_MESSAGE("Expected a problem for: " + content.toStdString(), !problem.isEmpty());
      CPPUNIT_ASSERT_MESSAGE("Reason expected in: " + problem.toStdString(),
                             problem.startsWith(QStringLiteral("Inline svg 1 cannot be drawn, as it is not well-formed XML: ")));
      CPPUNIT_ASSERT_MESSAGE("Position expected in: " + problem.toStdString(), problem.contains(QStringLiteral(" (line ")) && problem.contains(QStringLiteral(", column ")));
      CPPUNIT_ASSERT_MESSAGE("A single period should end: " + problem.toStdString(), problem.endsWith(QStringLiteral(").")));
    }
  }

  void TestAttributeOutsideOfSvgIsNotChecked()
  {
    AssertDrawable(Page(Svg(QStringLiteral("<rect/>")) + QStringLiteral("<a href=\"http://example.com\">link</a><link href=\"style.css\">")));
  }

  void TestMessageNamesTheSvgThatIsBad()
  {
    const auto good = Svg(QStringLiteral("<rect/>"));
    const auto badReference = Svg(QStringLiteral("<use href=\"http://example.com/a.png\"/>"));
    const auto badXml = Svg(QStringLiteral("<text>&plusmn;</text>"));

    AssertRefused(Page(good + QStringLiteral("<p>between</p>") + badReference), 2, QStringLiteral("href=\"http://example.com/a.png\""));
    AssertRefused(Page(good + good + badXml + good), 3, QStringLiteral("it is not well-formed XML"));
    AssertRefused(Page(good + good + good + badXml), 4, QStringLiteral("it is not well-formed XML"));

    // The first one that cannot be drawn is named.
    AssertRefused(Page(good + badXml + badReference), 2, QStringLiteral("it is not well-formed XML"));
    AssertRefused(Page(badReference + badXml), 1, QStringLiteral("it refers to something outside of itself"));
  }

  void TestNestedSvgCountsAsPartOfTheOutermost()
  {
    // Qt SVG skips the content of a nested svg, so the outermost one is refused, and counted once.
    const auto nested = QStringLiteral("<svg><svg><rect/></svg><rect/></svg>");

    AssertRefused(Page(nested), 1, QStringLiteral("it has a nested svg"));
    AssertRefused(Page(Svg(QStringLiteral("<rect/>")) + nested), 2, QStringLiteral("it has a nested svg"));
  }

  void TestOtherElementsAreNoSvg()
  {
    // Other element names, and an attribute value that holds a '>'.
    AssertDrawable(Page(QStringLiteral("<svg-icon><image href=\"C:\\x.png\"></svg-icon>")));
    AssertDrawable(Page(QStringLiteral("<svgx>&plusmn;</svgx>")));

    AssertDrawable(Page(QStringLiteral("<svg data-note=\"a > b\"><rect/></svg>")));
    AssertRefused(Page(QStringLiteral("<svg data-note=\"a > b\"><image href=\"C:\\x.png\"/></svg>")), 1, QStringLiteral("href=\"C:\\x.png\""));
  }

  void TestSvgOutOfLowerCaseIsRefused()
  {
    // HTML takes these for svg, the XML they become does not, so they would draw nothing.
    for (const auto &html : { QStringLiteral("<SVG><rect/></SVG>"), QStringLiteral("<Svg width=\"1\"><rect/></Svg>"),
                              Svg(QStringLiteral("<rect/>")) + QStringLiteral("<SVG/>") })
    {
      const auto problem = QmitkHtmlWidget::FindUndrawableSvg(Page(html));
      CPPUNIT_ASSERT_MESSAGE("Expected a problem for: " + html.toStdString(),
                             problem.startsWith(QStringLiteral("An inline svg cannot be drawn, as its element is written ")));
    }
  }

  void TestUnclosedSvgIsRefused()
  {
    const auto good = Svg(QStringLiteral("<rect/>"));

    AssertRefused(Page(QStringLiteral("<svg width=\"1\"><rect/>")), 1, QStringLiteral("it is not closed with </svg>"));
    AssertRefused(Page(good + QStringLiteral("<p>between</p><svg><rect/>")), 2, QStringLiteral("it is not closed with </svg>"));

    // An inner svg that is closed does not close the outer one.
    AssertRefused(Page(QStringLiteral("<svg><svg><rect/></svg>")), 1, QStringLiteral("it is not closed with </svg>"));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkHtmlWidgetSvgCheck)
