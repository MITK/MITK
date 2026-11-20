/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <QmitkHtmlWidget.h>

#include <litehtml.h>
#include <litehtml/el_image.h>

#include <QApplication>
#include <QBuffer>
#include <QConicalGradient>
#include <QFontMetricsF>
#include <QHash>
#include <QImage>
#include <QImageReader>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QMovie>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QRadialGradient>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSvgRenderer>
#include <QXmlStreamReader>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <string>

namespace
{
  // A litehtml font handle: the resolved Qt font plus its ascent, so draw_text
  // can place the baseline (litehtml passes the glyph box top, not the baseline).
  struct FontHandle
  {
    QFont font;
    qreal ascent = 0;
  };

  QColor toQColor(const litehtml::web_color &color)
  {
    return QColor(color.red, color.green, color.blue, color.alpha);
  }

  QRectF toRectF(const litehtml::position &pos)
  {
    return QRectF(pos.x, pos.y, pos.width, pos.height);
  }

  void applyStops(QGradient &gradient, const litehtml::background_layer::gradient_base &src)
  {
    for (const auto &point : src.color_points)
      gradient.setColorAt(point.offset, toQColor(point.color));
  }

  bool hasRadius(const litehtml::border_radiuses &r)
  {
    return r.top_left_x > 0 || r.top_left_y > 0 || r.top_right_x > 0 || r.top_right_y > 0 || r.bottom_right_x > 0 ||
           r.bottom_right_y > 0 || r.bottom_left_x > 0 || r.bottom_left_y > 0;
  }

  /** \brief The radii shrunk evenly where adjacent corners would overlap, as CSS does (border-radius: 999px makes a capsule). */
  litehtml::border_radiuses fitRadii(litehtml::border_radiuses r, const QRectF &rect)
  {
    const std::array<litehtml::pixel_t *, 8> values = {
      &r.top_left_x, &r.top_left_y, &r.top_right_x, &r.top_right_y, &r.bottom_right_x, &r.bottom_right_y, &r.bottom_left_x, &r.bottom_left_y };

    for (auto *value : values)
      *value = std::max<litehtml::pixel_t>(0, *value);

    // A corner with one radius of zero is square, as in CSS.
    for (std::size_t i = 0; i < values.size(); i += 2)
    {
      if (*values[i] <= 0 || *values[i + 1] <= 0)
        *values[i] = *values[i + 1] = 0;
    }

    qreal factor = 1.0;

    const auto limit = [&factor](qreal length, qreal sum) {
      if (sum > length && sum > 0)
        factor = std::min(factor, length / sum);
    };

    limit(rect.width(), r.top_left_x + r.top_right_x);
    limit(rect.width(), r.bottom_left_x + r.bottom_right_x);
    limit(rect.height(), r.top_left_y + r.bottom_left_y);
    limit(rect.height(), r.top_right_y + r.bottom_right_y);

    if (factor < 1.0)
    {
      for (auto *value : values)
        *value = static_cast<litehtml::pixel_t>(*value * factor);
    }

    return r;
  }

  /** \brief The rectangle with elliptic corners of the given radii, which fitRadii() has made fit. */
  QPainterPath roundedRect(const QRectF &rect, const litehtml::border_radiuses &r)
  {
    QPainterPath path;
    path.moveTo(rect.left() + r.top_left_x, rect.top());
    path.lineTo(rect.right() - r.top_right_x, rect.top());

    if (r.top_right_x > 0 && r.top_right_y > 0)
      path.arcTo(QRectF(rect.right() - 2 * r.top_right_x, rect.top(), 2 * r.top_right_x, 2 * r.top_right_y), 90, -90);

    path.lineTo(rect.right(), rect.bottom() - r.bottom_right_y);

    if (r.bottom_right_x > 0 && r.bottom_right_y > 0)
      path.arcTo(QRectF(rect.right() - 2 * r.bottom_right_x, rect.bottom() - 2 * r.bottom_right_y, 2 * r.bottom_right_x, 2 * r.bottom_right_y), 0, -90);

    path.lineTo(rect.left() + r.bottom_left_x, rect.bottom());

    if (r.bottom_left_x > 0 && r.bottom_left_y > 0)
      path.arcTo(QRectF(rect.left(), rect.bottom() - 2 * r.bottom_left_y, 2 * r.bottom_left_x, 2 * r.bottom_left_y), 270, -90);

    path.lineTo(rect.left(), rect.top() + r.top_left_y);

    if (r.top_left_x > 0 && r.top_left_y > 0)
      path.arcTo(QRectF(rect.left(), rect.top(), 2 * r.top_left_x, 2 * r.top_left_y), 180, -90);

    path.closeSubpath();
    return path;
  }

  /** \brief Fills the background box, with its rounded corners if it has any. */
  void fillBackground(QPainter *painter, const litehtml::background_layer &layer, const QBrush &brush)
  {
    const QRectF box = toRectF(layer.border_box);

    if (hasRadius(layer.border_radius))
      painter->fillPath(roundedRect(box, fitRadii(layer.border_radius, box)), brush);
    else
      painter->fillRect(box, brush);
  }

  bool isDrawn(const litehtml::border &border)
  {
    return border.width > 0 && border.style != litehtml::border_style_none && border.style != litehtml::border_style_hidden;
  }

  /**
   * \brief Draws the ring between the rounded outer edge and the inner edge.
   *
   * Sides of different colors meet on the diagonals from the outer to the
   * inner corners, as in browsers.
   */
  void drawRoundedBorders(QPainter *painter, const litehtml::borders &borders, const QRectF &rect)
  {
    const auto outerRadii = fitRadii(borders.radius, rect);
    const QRectF inner = rect.adjusted(borders.left.width, borders.top.width, -borders.right.width, -borders.bottom.width);

    litehtml::border_radiuses innerRadii;
    innerRadii.top_left_x = outerRadii.top_left_x - borders.left.width;
    innerRadii.top_left_y = outerRadii.top_left_y - borders.top.width;
    innerRadii.top_right_x = outerRadii.top_right_x - borders.right.width;
    innerRadii.top_right_y = outerRadii.top_right_y - borders.top.width;
    innerRadii.bottom_right_x = outerRadii.bottom_right_x - borders.right.width;
    innerRadii.bottom_right_y = outerRadii.bottom_right_y - borders.bottom.width;
    innerRadii.bottom_left_x = outerRadii.bottom_left_x - borders.left.width;
    innerRadii.bottom_left_y = outerRadii.bottom_left_y - borders.bottom.width;

    QPainterPath ring = roundedRect(rect, outerRadii);

    if (inner.width() > 0 && inner.height() > 0)
      ring = ring.subtracted(roundedRect(inner, fitRadii(innerRadii, inner)));

    const std::array<const litehtml::border *, 4> sides = { &borders.top, &borders.right, &borders.bottom, &borders.left };
    const bool uniform = std::all_of(sides.begin(), sides.end(), [&borders](const litehtml::border *side) {
      return isDrawn(*side) && side->color == borders.top.color;
    });

    if (uniform)
    {
      painter->fillPath(ring, toQColor(borders.top.color));
      return;
    }

    const std::array<QPolygonF, 4> trapezoids = {
      QPolygonF({ rect.topLeft(), rect.topRight(), inner.topRight(), inner.topLeft() }),
      QPolygonF({ rect.topRight(), rect.bottomRight(), inner.bottomRight(), inner.topRight() }),
      QPolygonF({ rect.bottomRight(), rect.bottomLeft(), inner.bottomLeft(), inner.bottomRight() }),
      QPolygonF({ rect.bottomLeft(), rect.topLeft(), inner.topLeft(), inner.bottomLeft() })
    };

    for (std::size_t i = 0; i < sides.size(); ++i)
    {
      if (!isDrawn(*sides[i]))
        continue;

      QPainterPath trapezoid;
      trapezoid.addPolygon(trapezoids[i]);
      painter->fillPath(ring.intersected(trapezoid), toQColor(sides[i]->color));
    }
  }

  /**
   * \brief Whether an SVG refers to nothing outside of itself.
   *
   * Qt SVG opens what an &lt;image&gt; names, unless it is a "data:" URI, and
   * the style sheet that an xml-stylesheet instruction names, while it parses
   * and outside of the resource handler, so an SVG of an untrusted page could
   * read local files or reach network shares. A "data:" image is decoded by
   * its content, which may be an SVG with references of its own. So only
   * references to the SVG's own elements are accepted.
   */
  bool IsSelfContainedSvg(const QByteArray &svg)
  {
    QXmlStreamReader reader(svg);

    while (!reader.atEnd())
    {
      switch (reader.readNext())
      {
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::ProcessingInstruction:
          return false;

        case QXmlStreamReader::StartElement:
          for (const auto &attribute : reader.attributes())
          {
            if (attribute.name() == QLatin1String("href") && !attribute.value().startsWith(QLatin1Char('#')))
              return false;
          }
          break;

        default:
          break;
      }
    }

    return !reader.hasError();
  }

  /** \brief An &lt;img&gt; showing the SVG markup, with the attributes that size and style its box. */
  QString svgAsImage(const QString &openTag, QString svg)
  {
    // A standalone SVG document needs the namespaces that HTML implies.
    if (!openTag.contains(QStringLiteral("xmlns=")))
      svg.insert(4, QStringLiteral(" xmlns=\"http://www.w3.org/2000/svg\""));

    if (svg.contains(QStringLiteral("xlink:")) && !openTag.contains(QStringLiteral("xmlns:xlink=")))
      svg.insert(4, QStringLiteral(" xmlns:xlink=\"http://www.w3.org/1999/xlink\""));

    QString image = QStringLiteral("<img src=\"data:image/svg+xml;base64,") + QString::fromLatin1(svg.toUtf8().toBase64()) + QLatin1Char('"');

    static const QRegularExpression attribute(QStringLiteral("([\\w:-]+)\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"));
    static const QStringList keptAttributes = { "width", "height", "class", "style", "id" };

    auto attributes = attribute.globalMatch(openTag);

    while (attributes.hasNext())
    {
      const auto match = attributes.next();
      const auto name = match.captured(1).toLower();

      if (!keptAttributes.contains(name))
        continue;

      auto value = match.hasCaptured(2) ? match.captured(2) : match.captured(3);
      image += QLatin1Char(' ') + name + QStringLiteral("=\"") + value.replace(QLatin1Char('"'), QStringLiteral("&quot;")) + QLatin1Char('"');
    }

    return image + QLatin1Char('>');
  }

  /**
   * \brief The HTML with every outermost &lt;svg&gt; element replaced by an image of it.
   *
   * litehtml has no SVG element: inline SVG would be laid out as unknown HTML
   * and draw nothing. As an image it takes the path of SVG files, drawn as
   * vectors. Style sheets of the page no longer reach into it.
   */
  QString replaceInlineSvg(const QString &html)
  {
    // Case-sensitive like the XML that the markup becomes. A quoted attribute
    // value may contain '>', and a name like svg-icon is another element.
    static const QRegularExpression svgTag(QStringLiteral("<(/?)svg(?=[\\s/>])(?:[^>\"']|\"[^\"]*\"|'[^']*')*>"));

    QString result;
    qsizetype copied = 0;
    qsizetype start = 0;
    QString openTag;
    int depth = 0;

    auto tags = svgTag.globalMatch(html);

    while (tags.hasNext())
    {
      const auto tag = tags.next();
      const bool closing = !tag.captured(1).isEmpty();
      const bool selfClosing = !closing && tag.captured(0).endsWith(QStringLiteral("/>"));

      if (depth == 0)
      {
        if (closing)
          continue;

        start = tag.capturedStart();
        openTag = tag.captured(0);

        if (!selfClosing)
        {
          depth = 1;
          continue;
        }
      }
      else if (closing)
      {
        if (--depth > 0)
          continue;
      }
      else
      {
        if (!selfClosing)
          ++depth;

        continue;
      }

      const auto end = tag.capturedEnd();
      result += QStringView(html).mid(copied, start - copied);
      result += svgAsImage(openTag, html.mid(start, end - start));
      copied = end;
    }

    if (copied == 0)
      return html;

    result += QStringView(html).mid(copied);
    return result;
  }
}

// The document_container callbacks are deeply tied to the owning widget (they
// paint onto it, size against its viewport, and emit its signals), so Impl
// keeps a back-pointer q and reaches the few widget services it needs through
// it. Everything litehtml-specific stays here, off the public header.
class QmitkHtmlWidget::Impl : public litehtml::document_container
{
public:
  explicit Impl(QmitkHtmlWidget *widget)
    : q(widget)
  {
    m_DefaultFontName = QApplication::font().family().toStdString();
  }

  void SetResourceHandler(const ResourceHandler &handler)
  {
    m_ResourceHandler = handler;
  }

  void SetHtml(const QString &html, const QUrl &baseUrl)
  {
    m_BaseUrl = baseUrl;
    // litehtml only calls set_caption when the document has a <title>, so clear
    // the previous title up front or a title-less page keeps reporting the old one.
    m_Caption.clear();
    m_Images.clear();
    this->ClearAnimations();
    this->ClearSvgImages();

    m_Document = litehtml::document::createFromString(replaceInlineSvg(html).toStdString(), this);

    q->horizontalScrollBar()->setValue(0);
    q->verticalScrollBar()->setValue(0);

    this->Render();
    q->viewport()->update();
    q->updateGeometry();
  }

  QString DocumentTitle() const
  {
    return m_Caption;
  }

  void SetZoomFactor(qreal factor)
  {
    if (factor <= 0.0 || qFuzzyCompare(factor, m_Zoom))
      return;

    m_Zoom = factor;
    this->Render();
    q->viewport()->update();
  }

  qreal ZoomFactor() const
  {
    return m_Zoom;
  }

  void ScrollToAnchor(const QString &name)
  {
    if (!m_Document || name.isEmpty())
      return;

    const litehtml::element::ptr root = m_Document->root();
    if (!root)
      return;

    // Match an element by id, then fall back to a named anchor (<a name="...">).
    litehtml::element::ptr anchor = root->select_one(("#" + name).toStdString());
    if (!anchor)
      anchor = root->select_one(("a[name=\"" + name + "\"]").toStdString());
    if (!anchor)
      return;

    const litehtml::position placement = anchor->get_placement();
    q->verticalScrollBar()->setValue(qRound(placement.y * m_Zoom));
  }

  void SetFitToContent(bool enabled)
  {
    if (m_FitToContent == enabled)
      return;

    m_FitToContent = enabled;

    const Qt::ScrollBarPolicy policy = enabled ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded;
    q->setHorizontalScrollBarPolicy(policy);
    q->setVerticalScrollBarPolicy(policy);

    q->updateGeometry();
  }

  bool FitToContent() const
  {
    return m_FitToContent;
  }

  void SetPageColor(const QColor &color)
  {
    m_PageColor = color;
    q->viewport()->update();
  }

  /** \brief Scaled content size when fitting to content, else an invalid size. */
  QSize ContentSize() const
  {
    if (m_FitToContent && m_Document)
      return QSize(qRound(m_Document->width() * m_Zoom), qRound(m_Document->height() * m_Zoom));

    return QSize();
  }

  bool HasDocument() const
  {
    return static_cast<bool>(m_Document);
  }

  int HeightForWidth(int width) const
  {
    // Lay the document out at the requested width to obtain the content height.
    m_Document->render(static_cast<litehtml::pixel_t>(width / m_Zoom));
    const int height = qRound(m_Document->height() * m_Zoom);

    // Layouts also query widths the widget never gets, so restore the layout
    // for the current viewport, which painting and hit testing rely on.
    const int viewportWidth = q->viewport()->width();

    if (width != viewportWidth)
      m_Document->render(static_cast<litehtml::pixel_t>(viewportWidth / m_Zoom));

    return height;
  }

  void Render()
  {
    if (!m_Document)
      return;

    const auto width = static_cast<litehtml::pixel_t>(q->viewport()->width() / m_Zoom);
    m_Document->render(width);
    this->UpdateScrollBars();
  }

  void Paint()
  {
    QPainter painter(q->viewport());
    painter.fillRect(q->viewport()->rect(), m_PageColor);

    if (!m_Document)
      return;

    const int hscroll = q->horizontalScrollBar()->value();
    const int vscroll = q->verticalScrollBar()->value();

    painter.save();
    painter.translate(-hscroll, -vscroll);
    painter.scale(m_Zoom, m_Zoom);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    litehtml::position clip;
    clip.x = static_cast<litehtml::pixel_t>(hscroll / m_Zoom);
    clip.y = static_cast<litehtml::pixel_t>(vscroll / m_Zoom);
    clip.width = static_cast<litehtml::pixel_t>(q->viewport()->width() / m_Zoom);
    clip.height = static_cast<litehtml::pixel_t>(q->viewport()->height() / m_Zoom);
    m_VisibleArea = toRectF(clip);

    m_Painter = &painter;
    m_ClipDepth = 0;
    m_Document->draw(reinterpret_cast<litehtml::uint_ptr>(&painter), 0, 0, &clip);
    m_Painter = nullptr;

    painter.restore();
  }

  void OnMouseMove(QMouseEvent *event)
  {
    if (!m_Document)
      return;

    const litehtml::pixel_t x = (event->position().x() + q->horizontalScrollBar()->value()) / m_Zoom;
    const litehtml::pixel_t y = (event->position().y() + q->verticalScrollBar()->value()) / m_Zoom;

    litehtml::position::vector redraw;
    if (m_Document->on_mouse_over(x, y, event->position().x() / m_Zoom, event->position().y() / m_Zoom, redraw))
      q->viewport()->update();

    this->ShowLinkTarget(this->GetLinkTarget());
  }

  /** \brief The target of the link under the mouse, resolved as a click would open it; empty if there is none. */
  QString GetLinkTarget() const
  {
    // A click is handled by the nearest anchor around the element under the mouse.
    for (std::shared_ptr<const litehtml::element> element = m_Document->get_over_element(); element != nullptr;
         element = element->parent())
    {
      const char *href = element->get_attr("href");

      if (href != nullptr && std::strcmp(element->get_tagName(), "a") == 0)
        return m_BaseUrl.resolved(QUrl(QString::fromUtf8(href))).toString();
    }

    return QString();
  }

  void ShowLinkTarget(const QString &target)
  {
    // The scroll area's own tool tip is the one shown, as it receives the viewport's events.
    if (q->toolTip() != target)
      q->setToolTip(target);
  }

  /** \brief Returns false when there is nothing to handle (no document or non-left button). */
  bool OnMousePress(QMouseEvent *event)
  {
    if (!m_Document || event->button() != Qt::LeftButton)
      return false;

    const litehtml::pixel_t x = (event->position().x() + q->horizontalScrollBar()->value()) / m_Zoom;
    const litehtml::pixel_t y = (event->position().y() + q->verticalScrollBar()->value()) / m_Zoom;

    litehtml::position::vector redraw;
    if (m_Document->on_lbutton_down(x, y, event->position().x() / m_Zoom, event->position().y() / m_Zoom, redraw))
      q->viewport()->update();

    return true;
  }

  /** \brief Returns false when there is nothing to handle (no document or non-left button). */
  bool OnMouseRelease(QMouseEvent *event)
  {
    if (!m_Document || event->button() != Qt::LeftButton)
      return false;

    const litehtml::pixel_t x = (event->position().x() + q->horizontalScrollBar()->value()) / m_Zoom;
    const litehtml::pixel_t y = (event->position().y() + q->verticalScrollBar()->value()) / m_Zoom;

    // Triggers document_container::on_anchor_click for <a href> under the cursor.
    litehtml::position::vector redraw;
    if (m_Document->on_lbutton_up(x, y, event->position().x() / m_Zoom, event->position().y() / m_Zoom, redraw))
      q->viewport()->update();

    return true;
  }

  void OnMouseLeave()
  {
    this->ShowLinkTarget(QString());

    if (!m_Document)
      return;

    litehtml::position::vector redraw;
    if (m_Document->on_mouse_leave(redraw))
      q->viewport()->update();
  }

  // litehtml::document_container

  litehtml::uint_ptr create_font(const litehtml::font_description &descr, const litehtml::document *, litehtml::font_metrics *fm) override
  {
    auto *handle = new FontHandle;
    QFont &font = handle->font;

    QStringList families;
    const auto requested = QString::fromStdString(descr.family).split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString &family : requested)
      families << family.trimmed().remove(QLatin1Char('"')).remove(QLatin1Char('\''));
    if (!families.isEmpty())
      font.setFamilies(families);

    font.setPixelSize(qMax(1, qRound(descr.size)));
    font.setWeight(QFont::Weight(descr.weight > 0 ? descr.weight : static_cast<int>(QFont::Normal)));
    font.setItalic(descr.style == litehtml::font_style_italic);
    font.setUnderline((descr.decoration_line & litehtml::text_decoration_line_underline) != 0);
    font.setStrikeOut((descr.decoration_line & litehtml::text_decoration_line_line_through) != 0);

    const QFontMetricsF metrics(font);
    handle->ascent = metrics.ascent();

    if (fm != nullptr)
    {
      fm->font_size = descr.size;
      fm->ascent = metrics.ascent();
      fm->descent = metrics.descent();
      fm->height = metrics.height();
      fm->x_height = metrics.xHeight();
      fm->ch_width = metrics.horizontalAdvance(QLatin1Char('0'));
      fm->draw_spaces = true;
      fm->sub_shift = metrics.descent();
      fm->super_shift = metrics.ascent() / 2;
    }

    return reinterpret_cast<litehtml::uint_ptr>(handle);
  }

  void delete_font(litehtml::uint_ptr hFont) override
  {
    delete reinterpret_cast<FontHandle *>(hFont);
  }

  litehtml::pixel_t text_width(const char *text, litehtml::uint_ptr hFont) override
  {
    const auto *handle = reinterpret_cast<FontHandle *>(hFont);
    const QFontMetricsF metrics(handle->font);
    return static_cast<litehtml::pixel_t>(metrics.horizontalAdvance(QString::fromUtf8(text)));
  }

  void draw_text(litehtml::uint_ptr hdc, const char *text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position &pos) override
  {
    auto *painter = reinterpret_cast<QPainter *>(hdc);
    const auto *handle = reinterpret_cast<FontHandle *>(hFont);

    painter->setFont(handle->font);
    painter->setPen(toQColor(color));
    painter->drawText(QPointF(pos.x, pos.y + handle->ascent), QString::fromUtf8(text));
  }

  litehtml::pixel_t pt_to_px(float pt) const override
  {
    return static_cast<litehtml::pixel_t>(pt * q->logicalDpiY() / 72.0);
  }

  litehtml::pixel_t get_default_font_size() const override
  {
    return 16;
  }

  const char *get_default_font_name() const override
  {
    return m_DefaultFontName.c_str();
  }

  void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker &marker) override
  {
    auto *painter = reinterpret_cast<QPainter *>(hdc);
    const QRectF rect = toRectF(marker.pos);
    const QColor color = toQColor(marker.color);

    if (!marker.image.empty())
    {
      const QUrl url = this->Resolve(QString::fromStdString(marker.image), QString::fromUtf8(marker.baseurl != nullptr ? marker.baseurl : ""));
      const QString key = url.toString();
      this->EnsureImage(key, url);

      const auto svg = m_SvgImages.constFind(key);
      if (svg != m_SvgImages.constEnd())
      {
        svg.value()->render(painter, rect);
      }
      else
      {
        const QImage image = this->CurrentFrame(key);
        if (!image.isNull())
          painter->drawImage(rect, image);
      }
      return;
    }

    painter->setPen(color);

    switch (marker.marker_type)
    {
      case litehtml::list_style_type_disc:
        painter->setBrush(color);
        painter->drawEllipse(rect);
        break;
      case litehtml::list_style_type_circle:
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(rect);
        break;
      case litehtml::list_style_type_square:
        painter->setBrush(color);
        painter->drawRect(rect);
        break;
      default:
        if (marker.font != 0)
        {
          const auto *handle = reinterpret_cast<FontHandle *>(marker.font);
          painter->setFont(handle->font);
          painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, QString::number(marker.index) + QLatin1Char('.'));
        }
        break;
    }
  }

  void load_image(const char *src, const char *baseurl, bool) override
  {
    const QUrl url = this->Resolve(QString::fromUtf8(src), QString::fromUtf8(baseurl != nullptr ? baseurl : ""));
    this->EnsureImage(url.toString(), url);
  }

  void get_image_size(const char *src, const char *baseurl, litehtml::size &sz) override
  {
    const QUrl url = this->Resolve(QString::fromUtf8(src), QString::fromUtf8(baseurl != nullptr ? baseurl : ""));
    const QString key = url.toString();
    this->EnsureImage(key, url);

    QSize size;
    const auto svg = m_SvgImages.constFind(key);
    if (svg != m_SvgImages.constEnd())
    {
      size = svg.value()->defaultSize();

      // Icons declared with only a viewBox (no width/height) report an empty
      // default size; fall back to the viewBox so the layout box is non-zero.
      if (size.isEmpty())
        size = svg.value()->viewBox().size();
    }
    else
    {
      const auto animation = m_Animations.constFind(key);
      size = animation != m_Animations.constEnd() ? animation->size : m_Images.value(key).size();
    }

    sz.width = size.width();
    sz.height = size.height();
  }

  void draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const std::string &url, const std::string &base_url) override
  {
    const QUrl resolvedUrl = this->Resolve(QString::fromStdString(url), QString::fromStdString(base_url));
    const QString key = resolvedUrl.toString();
    this->EnsureImage(key, resolvedUrl);

    auto *painter = reinterpret_cast<QPainter *>(hdc);

    // litehtml reports a single tile's placement in origin_box (position and size
    // after background-size/position) and the paintable area in clip_box;
    // border_box carries only the border radius. Draw the tile at its own size and
    // repeat it across clip_box, instead of stretching one copy over the box.
    const QRectF tile = toRectF(layer.origin_box);
    const QRectF clip = toRectF(layer.clip_box);
    if (tile.width() <= 0.0 || tile.height() <= 0.0)
      return;

    // Clamp the tile step to a full pixel. A degenerate sub-pixel tile (e.g. a
    // zero-intrinsic-size SVG used as a repeating background) would otherwise
    // make the repeat loop below run for millions of iterations and freeze the UI.
    const qreal stepX = qMax(tile.width(), 1.0);
    const qreal stepY = qMax(tile.height(), 1.0);

    QSvgRenderer *svg = m_SvgImages.value(key, nullptr);
    QImage image;
    if (svg == nullptr)
    {
      image = this->CurrentFrame(key);
      if (image.isNull())
        return;
    }

    const bool repeatX = layer.repeat == litehtml::background_repeat_repeat || layer.repeat == litehtml::background_repeat_repeat_x;
    const bool repeatY = layer.repeat == litehtml::background_repeat_repeat || layer.repeat == litehtml::background_repeat_repeat_y;

    // When repeating, back the first tile up to just before the clip box so the
    // pattern stays anchored on origin_box; otherwise draw the single tile only.
    const qreal firstX = repeatX ? tile.x() - std::ceil((tile.x() - clip.left()) / stepX) * stepX : tile.x();
    const qreal firstY = repeatY ? tile.y() - std::ceil((tile.y() - clip.top()) / stepY) * stepY : tile.y();
    const qreal lastX = repeatX ? clip.right() : tile.x();
    const qreal lastY = repeatY ? clip.bottom() : tile.y();

    painter->save();
    painter->setClipRect(clip, Qt::IntersectClip);

    if (hasRadius(layer.border_radius))
    {
      const QRectF box = toRectF(layer.border_box);
      painter->setClipPath(roundedRect(box, fitRadii(layer.border_radius, box)), Qt::IntersectClip);
    }

    for (qreal y = firstY; y <= lastY; y += stepY)
    {
      for (qreal x = firstX; x <= lastX; x += stepX)
      {
        const QRectF target(x, y, stepX, stepY);
        if (svg != nullptr)
        {
          // Qt SVG draws text without a font attribute in the painter's font, which is
          // whatever the last callback left there.
          QFont font = QApplication::font();
          font.setPixelSize(this->get_default_font_size());
          painter->setFont(font);
          svg->render(painter, target);
        }
        else
          painter->drawImage(target, image);
      }
    }

    painter->restore();
  }

  void draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::web_color &color) override
  {
    if (color.alpha == 0)
      return;

    // CSS paints the root element's background over the whole canvas, below a short document too.
    if (layer.is_root)
    {
      reinterpret_cast<QPainter *>(hdc)->fillRect(toRectF(layer.border_box).united(m_VisibleArea), toQColor(color));
      return;
    }

    fillBackground(reinterpret_cast<QPainter *>(hdc), layer, toQColor(color));
  }

  void draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::background_layer::linear_gradient &gradient) override
  {
    QLinearGradient brush(gradient.start.x, gradient.start.y, gradient.end.x, gradient.end.y);
    applyStops(brush, gradient);
    fillBackground(reinterpret_cast<QPainter *>(hdc), layer, brush);
  }

  void draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::background_layer::radial_gradient &gradient) override
  {
    QRadialGradient brush(QPointF(gradient.position.x, gradient.position.y), qMax(gradient.radius.x, gradient.radius.y));
    applyStops(brush, gradient);
    fillBackground(reinterpret_cast<QPainter *>(hdc), layer, brush);
  }

  void draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::background_layer::conic_gradient &gradient) override
  {
    QConicalGradient brush(QPointF(gradient.position.x, gradient.position.y), gradient.angle);
    applyStops(brush, gradient);
    fillBackground(reinterpret_cast<QPainter *>(hdc), layer, brush);
  }

  void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders &borders, const litehtml::position &draw_pos, bool) override
  {
    auto *painter = reinterpret_cast<QPainter *>(hdc);
    const QRectF rect = toRectF(draw_pos);

    if (hasRadius(borders.radius))
    {
      drawRoundedBorders(painter, borders, rect);
      return;
    }

    auto drawSide = [painter](const litehtml::border &border, const QRectF &sideRect)
    {
      if (!isDrawn(border))
        return;
      painter->fillRect(sideRect, toQColor(border.color));
    };

    drawSide(borders.top, QRectF(rect.left(), rect.top(), rect.width(), borders.top.width));
    drawSide(borders.bottom, QRectF(rect.left(), rect.bottom() - borders.bottom.width, rect.width(), borders.bottom.width));
    drawSide(borders.left, QRectF(rect.left(), rect.top(), borders.left.width, rect.height()));
    drawSide(borders.right, QRectF(rect.right() - borders.right.width, rect.top(), borders.right.width, rect.height()));
  }

  void set_caption(const char *caption) override
  {
    m_Caption = QString::fromUtf8(caption);
    emit q->titleChanged(m_Caption);
  }

  void set_base_url(const char *base_url) override
  {
    const QString base = QString::fromUtf8(base_url != nullptr ? base_url : "");
    if (!base.isEmpty())
      m_BaseUrl = m_BaseUrl.resolved(QUrl(base));
  }

  void link(const std::shared_ptr<litehtml::document> &, const litehtml::element::ptr &) override
  {
  }

  void on_anchor_click(const char *url, const litehtml::element::ptr &) override
  {
    // on_anchor_click is invoked from inside the litehtml document's mouse
    // handler. Emitting synchronously would let a slot replace (and destroy) the
    // document while we are still executing inside it, so defer to the event loop.
    const QUrl resolved = m_BaseUrl.resolved(QUrl(QString::fromUtf8(url)));
    QMetaObject::invokeMethod(q, [this, resolved]() { emit q->linkClicked(resolved); }, Qt::QueuedConnection);
  }

  void on_mouse_event(const litehtml::element::ptr &, litehtml::mouse_event) override
  {
  }

  void set_cursor(const char *cursor) override
  {
    const QString name = QString::fromUtf8(cursor != nullptr ? cursor : "");
    q->viewport()->setCursor(name == QLatin1String("pointer") ? Qt::PointingHandCursor : Qt::ArrowCursor);
  }

  void transform_text(std::string &text, litehtml::text_transform tt) override
  {
    QString value = QString::fromStdString(text);

    switch (tt)
    {
      case litehtml::text_transform_uppercase:
        value = value.toUpper();
        break;
      case litehtml::text_transform_lowercase:
        value = value.toLower();
        break;
      case litehtml::text_transform_capitalize:
      {
        bool atWordStart = true;
        for (QChar &character : value)
        {
          if (atWordStart && character.isLetter())
            character = character.toUpper();
          atWordStart = character.isSpace();
        }
        break;
      }
      default:
        return;
    }

    text = value.toStdString();
  }

  void import_css(std::string &text, const std::string &url, std::string &baseurl) override
  {
    const QUrl resolved = this->Resolve(QString::fromStdString(url), QString::fromStdString(baseurl));
    const QByteArray data = this->Fetch(resolved);
    text.assign(data.constData(), static_cast<std::size_t>(data.size()));

    // baseurl is an in/out parameter: litehtml resolves url()/@import inside this
    // sheet against it, so hand back the sheet's own location (empty on input for
    // a <link>) instead of leaving relative resources to resolve against the page.
    baseurl = resolved.toString().toStdString();
  }

  void set_clip(const litehtml::position &pos, const litehtml::border_radiuses &radius) override
  {
    if (m_Painter == nullptr)
      return;

    m_Painter->save();
    ++m_ClipDepth;
    const QRectF rect = toRectF(pos);

    if (hasRadius(radius))
      m_Painter->setClipPath(roundedRect(rect, fitRadii(radius, rect)), Qt::IntersectClip);
    else
      m_Painter->setClipRect(rect, Qt::IntersectClip);
  }

  void del_clip() override
  {
    // litehtml removes the clip of an inline element with overflow without
    // having set one; restoring for it would take the state of Paint() away.
    if (m_Painter == nullptr || m_ClipDepth == 0)
      return;

    --m_ClipDepth;
    m_Painter->restore();
  }

  void get_viewport(litehtml::position &viewport) const override
  {
    viewport.x = static_cast<litehtml::pixel_t>(q->horizontalScrollBar()->value() / m_Zoom);
    viewport.y = static_cast<litehtml::pixel_t>(q->verticalScrollBar()->value() / m_Zoom);
    viewport.width = static_cast<litehtml::pixel_t>(q->viewport()->width() / m_Zoom);
    viewport.height = static_cast<litehtml::pixel_t>(q->viewport()->height() / m_Zoom);
  }

  litehtml::element::ptr create_element(const char *tag_name, const litehtml::string_map &attributes, const std::shared_ptr<litehtml::document> &doc) override
  {
    // Doxygen embeds SVG figures as <object type="image/svg+xml" data="...">, and
    // litehtml has no <object> element (it would fall back to a plain container
    // that reserves no space and draws nothing). Map an image object onto the
    // built-in image element, copying the data attribute to src.
    if (std::strcmp(tag_name, "object") == 0)
    {
      const auto type = attributes.find("type");
      const auto data = attributes.find("data");

      if (type != attributes.end() && data != attributes.end() && type->second.starts_with("image/"))
      {
        auto image = std::make_shared<litehtml::el_image>(doc);
        image->set_attr("src", data->second.c_str());
        return image;
      }
    }

    // Returning nullptr makes litehtml use its built-in element implementations.
    return nullptr;
  }

  void get_media_features(litehtml::media_features &media) const override
  {
    media.type = litehtml::media_type_screen;
    media.width = static_cast<litehtml::pixel_t>(q->viewport()->width() / m_Zoom);
    media.height = static_cast<litehtml::pixel_t>(q->viewport()->height() / m_Zoom);
    media.device_width = media.width;
    media.device_height = media.height;
    media.color = 8;
    media.color_index = 0;
    media.monochrome = 0;
    media.resolution = static_cast<litehtml::pixel_t>(q->logicalDpiY());
  }

  void get_language(std::string &language, std::string &culture) const override
  {
    language = "en";
    culture = "";
  }

private:
  struct AnimatedImage
  {
    QMovie *movie;
    QSize size;
  };

  void UpdateScrollBars()
  {
    if (!m_Document)
      return;

    const int contentWidth = qRound(m_Document->width() * m_Zoom);
    const int contentHeight = qRound(m_Document->height() * m_Zoom);
    const int viewWidth = q->viewport()->width();
    const int viewHeight = q->viewport()->height();

    q->horizontalScrollBar()->setPageStep(viewWidth);
    q->horizontalScrollBar()->setSingleStep(20);
    q->horizontalScrollBar()->setRange(0, qMax(0, contentWidth - viewWidth));

    q->verticalScrollBar()->setPageStep(viewHeight);
    q->verticalScrollBar()->setSingleStep(20);
    q->verticalScrollBar()->setRange(0, qMax(0, contentHeight - viewHeight));
  }

  QByteArray Fetch(const QUrl &url) const
  {
    // Inline "data:" URIs carry their own payload (base64 or percent-encoded),
    // so decode them here instead of routing them to the resource handler.
    if (url.scheme() == QLatin1String("data"))
    {
      // QUrl parses a data: payload like a normal URL, so an unencoded '?' or '#'
      // inside it (e.g. an inline SVG referencing url(#gradient)) is split off as
      // query/fragment. Reassemble the parts to recover the original payload.
      QString content = url.path(QUrl::FullyDecoded);
      if (url.hasQuery())
        content += QLatin1Char('?') + url.query(QUrl::FullyDecoded);
      if (url.hasFragment())
        content += QLatin1Char('#') + url.fragment(QUrl::FullyDecoded);

      const int comma = content.indexOf(QLatin1Char(','));
      if (comma < 0)
        return QByteArray();

      const QString meta = content.left(comma);
      const QString payload = content.mid(comma + 1);

      return meta.contains(QLatin1String(";base64"))
        ? QByteArray::fromBase64(payload.toUtf8())
        : payload.toUtf8();
    }

    return m_ResourceHandler ? m_ResourceHandler(url) : QByteArray();
  }

  QUrl Resolve(const QString &src, const QString &baseUrl) const
  {
    const QUrl url(src);
    if (!url.isRelative())
      return url;

    const QUrl base = baseUrl.isEmpty() ? m_BaseUrl : QUrl(baseUrl);
    return base.resolved(url);
  }

  void EnsureImage(const QString &key, const QUrl &url)
  {
    if (m_Images.contains(key) || m_Animations.contains(key) || m_SvgImages.contains(key))
      return;

    const QByteArray data = this->Fetch(url);

    QBuffer probe;
    probe.setData(data);
    probe.open(QIODevice::ReadOnly);
    QImageReader reader(&probe);

    // An SVG that refers to anything outside of itself is not drawn at all, not
    // even as a raster image, which Qt's SVG image plugin would parse alike.
    // The same holds for compressed SVG, which is not checked.
    if (reader.format().startsWith("svg") && (reader.format() != "svg" || !IsSelfContainedSvg(data)))
    {
      qWarning("QmitkHtmlWidget: an SVG image that refers to files or other documents is not shown.");
      m_Images.insert(key, QImage());
      return;
    }

    // SVGs are kept as a vector renderer so they stay crisp at any display size.
    // A GIF with more than one frame (imageCount != 1, treating an unknown 0 as
    // animated) is played as a QMovie; everything else is a static image.
    if (reader.format() == "svg")
    {
      auto *renderer = new QSvgRenderer(data, q);

      // Fall back to raster handling if the SVG cannot be parsed.
      if (renderer->isValid())
      {
        // As browsers fit a viewBox into a box of another ratio, instead of stretching it.
        renderer->setAspectRatioMode(Qt::KeepAspectRatio);
        m_SvgImages.insert(key, renderer);
        return;
      }

      renderer->deleteLater();
    }

    if (reader.supportsAnimation() && reader.imageCount() != 1)
    {
      auto *movie = new QMovie(q);
      auto *buffer = new QBuffer(movie);
      buffer->setData(data);
      buffer->open(QIODevice::ReadOnly);
      movie->setDevice(buffer);

      QObject::connect(movie, &QMovie::frameChanged, q, [this]() { q->viewport()->update(); });
      movie->start();

      m_Animations.insert(key, AnimatedImage{ movie, reader.size() });
    }
    else
    {
      m_Images.insert(key, QImage::fromData(data));
    }
  }

  QImage CurrentFrame(const QString &key) const
  {
    const auto animation = m_Animations.constFind(key);
    if (animation != m_Animations.constEnd())
      return animation->movie->currentImage();

    return m_Images.value(key);
  }

  void ClearAnimations()
  {
    for (const AnimatedImage &animation : m_Animations)
    {
      animation.movie->stop();
      animation.movie->deleteLater();
    }
    m_Animations.clear();
  }

  void ClearSvgImages()
  {
    for (QSvgRenderer *renderer : m_SvgImages)
      renderer->deleteLater();

    m_SvgImages.clear();
  }

  QmitkHtmlWidget *const q;
  ResourceHandler m_ResourceHandler;
  litehtml::document::ptr m_Document;
  QUrl m_BaseUrl;
  QString m_Caption;
  std::string m_DefaultFontName;
  qreal m_Zoom = 1.0;
  bool m_FitToContent = false;
  QColor m_PageColor = Qt::white;

  QHash<QString, QImage> m_Images;
  QHash<QString, AnimatedImage> m_Animations;
  QHash<QString, QSvgRenderer *> m_SvgImages;
  QPainter *m_Painter = nullptr;

  /** \brief The clips that set_clip() has pushed onto the painter and del_clip() has not removed yet. */
  int m_ClipDepth = 0;

  /** \brief The part of the document the current paint shows, in document coordinates. */
  QRectF m_VisibleArea;
};

QmitkHtmlWidget::QmitkHtmlWidget(QWidget *parent)
  : QAbstractScrollArea(parent),
    m_Impl(std::make_unique<Impl>(this))
{
  this->setFrameShape(QFrame::NoFrame);
  this->viewport()->setMouseTracking(true);
  this->viewport()->setBackgroundRole(QPalette::Base);
}

QmitkHtmlWidget::~QmitkHtmlWidget() = default;

void QmitkHtmlWidget::SetResourceHandler(const ResourceHandler &handler)
{
  m_Impl->SetResourceHandler(handler);
}

void QmitkHtmlWidget::SetHtml(const QString &html, const QUrl &baseUrl)
{
  m_Impl->SetHtml(html, baseUrl);
}

QString QmitkHtmlWidget::DocumentTitle() const
{
  return m_Impl->DocumentTitle();
}

void QmitkHtmlWidget::SetZoomFactor(qreal factor)
{
  m_Impl->SetZoomFactor(factor);
}

qreal QmitkHtmlWidget::ZoomFactor() const
{
  return m_Impl->ZoomFactor();
}

void QmitkHtmlWidget::SetFitToContent(bool enabled)
{
  m_Impl->SetFitToContent(enabled);
}

void QmitkHtmlWidget::SetPageColor(const QColor &color)
{
  m_Impl->SetPageColor(color);
}

void QmitkHtmlWidget::ScrollToAnchor(const QString &name)
{
  m_Impl->ScrollToAnchor(name);
}

QSize QmitkHtmlWidget::sizeHint() const
{
  const QSize size = m_Impl->ContentSize();
  return size.isValid() ? size : QAbstractScrollArea::sizeHint();
}

bool QmitkHtmlWidget::hasHeightForWidth() const
{
  return m_Impl->FitToContent();
}

int QmitkHtmlWidget::heightForWidth(int width) const
{
  if (!m_Impl->FitToContent() || !m_Impl->HasDocument())
    return QAbstractScrollArea::heightForWidth(width);

  return m_Impl->HeightForWidth(width);
}

void QmitkHtmlWidget::paintEvent(QPaintEvent *)
{
  m_Impl->Paint();
}

void QmitkHtmlWidget::resizeEvent(QResizeEvent *event)
{
  QAbstractScrollArea::resizeEvent(event);
  m_Impl->Render();
  this->viewport()->update();
}

void QmitkHtmlWidget::scrollContentsBy(int, int)
{
  this->viewport()->update();
}

void QmitkHtmlWidget::mouseMoveEvent(QMouseEvent *event)
{
  m_Impl->OnMouseMove(event);
}

void QmitkHtmlWidget::mousePressEvent(QMouseEvent *event)
{
  if (!m_Impl->OnMousePress(event))
    QAbstractScrollArea::mousePressEvent(event);
}

void QmitkHtmlWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if (!m_Impl->OnMouseRelease(event))
    QAbstractScrollArea::mouseReleaseEvent(event);
}

void QmitkHtmlWidget::leaveEvent(QEvent *)
{
  m_Impl->OnMouseLeave();
}
