/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkHtmlWidget.h"

#include <litehtml/el_image.h>

#include <QApplication>
#include <QBuffer>
#include <QConicalGradient>
#include <QFontMetricsF>
#include <QImageReader>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QMovie>
#include <QPainter>
#include <QRadialGradient>
#include <QScrollBar>
#include <QSvgRenderer>

#include <cmath>
#include <cstring>

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
}

QmitkHtmlWidget::QmitkHtmlWidget(QWidget *parent)
  : QAbstractScrollArea(parent),
    m_Zoom(1.0),
    m_FitToContent(false),
    m_PageColor(Qt::white),
    m_Painter(nullptr)
{
  m_DefaultFontName = QApplication::font().family().toStdString();

  this->setFrameShape(QFrame::NoFrame);
  this->viewport()->setMouseTracking(true);
  this->viewport()->setBackgroundRole(QPalette::Base);
}

QmitkHtmlWidget::~QmitkHtmlWidget()
{
}

void QmitkHtmlWidget::SetResourceHandler(const ResourceHandler &handler)
{
  m_ResourceHandler = handler;
}

void QmitkHtmlWidget::SetHtml(const QString &html, const QUrl &baseUrl)
{
  m_BaseUrl = baseUrl;
  m_Images.clear();
  this->ClearAnimations();
  this->ClearSvgImages();

  m_Document = litehtml::document::createFromString(html.toStdString(), this);

  this->horizontalScrollBar()->setValue(0);
  this->verticalScrollBar()->setValue(0);

  this->Render();
  this->viewport()->update();
  this->updateGeometry();
}

QString QmitkHtmlWidget::DocumentTitle() const
{
  return m_Caption;
}

void QmitkHtmlWidget::SetZoomFactor(qreal factor)
{
  if (factor <= 0.0 || qFuzzyCompare(factor, m_Zoom))
    return;

  m_Zoom = factor;
  this->Render();
  this->viewport()->update();
}

qreal QmitkHtmlWidget::ZoomFactor() const
{
  return m_Zoom;
}

void QmitkHtmlWidget::SetFitToContent(bool enabled)
{
  if (m_FitToContent == enabled)
    return;

  m_FitToContent = enabled;

  const Qt::ScrollBarPolicy policy = enabled ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded;
  this->setHorizontalScrollBarPolicy(policy);
  this->setVerticalScrollBarPolicy(policy);

  this->updateGeometry();
}

void QmitkHtmlWidget::SetPageColor(const QColor &color)
{
  m_PageColor = color;
  this->viewport()->update();
}

QSize QmitkHtmlWidget::sizeHint() const
{
  if (m_FitToContent && m_Document)
    return QSize(qRound(m_Document->width() * m_Zoom), qRound(m_Document->height() * m_Zoom));

  return QAbstractScrollArea::sizeHint();
}

bool QmitkHtmlWidget::hasHeightForWidth() const
{
  return m_FitToContent;
}

int QmitkHtmlWidget::heightForWidth(int width) const
{
  if (!m_FitToContent || !m_Document)
    return QAbstractScrollArea::heightForWidth(width);

  // Lay the document out at the requested width to obtain the content height.
  m_Document->render(static_cast<litehtml::pixel_t>(width / m_Zoom));
  return qRound(m_Document->height() * m_Zoom);
}

void QmitkHtmlWidget::ScrollToAnchor(const QString &name)
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
  this->verticalScrollBar()->setValue(qRound(placement.y * m_Zoom));
}

QByteArray QmitkHtmlWidget::Fetch(const QUrl &url) const
{
  // Inline "data:" URIs carry their own payload (base64 or percent-encoded),
  // so decode them here instead of routing them to the resource handler.
  if (url.scheme() == QLatin1String("data"))
  {
    const QString path = url.path(QUrl::FullyDecoded);
    const int comma = path.indexOf(QLatin1Char(','));
    if (comma < 0)
      return QByteArray();

    const QString meta = path.left(comma);
    const QString payload = path.mid(comma + 1);

    return meta.contains(QLatin1String(";base64"))
      ? QByteArray::fromBase64(payload.toUtf8())
      : payload.toUtf8();
  }

  return m_ResourceHandler ? m_ResourceHandler(url) : QByteArray();
}

QUrl QmitkHtmlWidget::Resolve(const QString &src, const QString &baseUrl) const
{
  const QUrl url(src);
  if (!url.isRelative())
    return url;

  const QUrl base = baseUrl.isEmpty() ? m_BaseUrl : QUrl(baseUrl);
  return base.resolved(url);
}

void QmitkHtmlWidget::EnsureImage(const QString &key, const QUrl &url)
{
  if (m_Images.contains(key) || m_Animations.contains(key) || m_SvgImages.contains(key))
    return;

  const QByteArray data = this->Fetch(url);

  QBuffer probe;
  probe.setData(data);
  probe.open(QIODevice::ReadOnly);
  QImageReader reader(&probe);

  // SVGs are kept as a vector renderer so they stay crisp at any display size.
  // A GIF with more than one frame (imageCount != 1, treating an unknown 0 as
  // animated) is played as a QMovie; everything else is a static image.
  if (reader.format() == "svg")
  {
    auto *renderer = new QSvgRenderer(data, this);

    // Fall back to raster handling if the SVG cannot be parsed.
    if (renderer->isValid())
    {
      m_SvgImages.insert(key, renderer);
      return;
    }

    renderer->deleteLater();
  }

  if (reader.supportsAnimation() && reader.imageCount() != 1)
  {
    auto *movie = new QMovie(this);
    auto *buffer = new QBuffer(movie);
    buffer->setData(data);
    buffer->open(QIODevice::ReadOnly);
    movie->setDevice(buffer);

    connect(movie, &QMovie::frameChanged, this, [this]() { this->viewport()->update(); });
    movie->start();

    m_Animations.insert(key, AnimatedImage{ movie, reader.size() });
  }
  else
  {
    m_Images.insert(key, QImage::fromData(data));
  }
}

QImage QmitkHtmlWidget::CurrentFrame(const QString &key) const
{
  const auto animation = m_Animations.constFind(key);
  if (animation != m_Animations.constEnd())
    return animation->movie->currentImage();

  return m_Images.value(key);
}

void QmitkHtmlWidget::ClearAnimations()
{
  for (const AnimatedImage &animation : m_Animations)
  {
    animation.movie->stop();
    animation.movie->deleteLater();
  }
  m_Animations.clear();
}

void QmitkHtmlWidget::ClearSvgImages()
{
  for (QSvgRenderer *renderer : m_SvgImages)
    renderer->deleteLater();

  m_SvgImages.clear();
}

void QmitkHtmlWidget::Render()
{
  if (!m_Document)
    return;

  const auto width = static_cast<litehtml::pixel_t>(this->viewport()->width() / m_Zoom);
  m_Document->render(width);
  this->UpdateScrollBars();
}

void QmitkHtmlWidget::UpdateScrollBars()
{
  if (!m_Document)
    return;

  const int contentWidth = qRound(m_Document->width() * m_Zoom);
  const int contentHeight = qRound(m_Document->height() * m_Zoom);
  const int viewWidth = this->viewport()->width();
  const int viewHeight = this->viewport()->height();

  this->horizontalScrollBar()->setPageStep(viewWidth);
  this->horizontalScrollBar()->setSingleStep(20);
  this->horizontalScrollBar()->setRange(0, qMax(0, contentWidth - viewWidth));

  this->verticalScrollBar()->setPageStep(viewHeight);
  this->verticalScrollBar()->setSingleStep(20);
  this->verticalScrollBar()->setRange(0, qMax(0, contentHeight - viewHeight));
}

void QmitkHtmlWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this->viewport());
  painter.fillRect(this->viewport()->rect(), m_PageColor);

  if (!m_Document)
    return;

  const int hscroll = this->horizontalScrollBar()->value();
  const int vscroll = this->verticalScrollBar()->value();

  painter.save();
  painter.translate(-hscroll, -vscroll);
  painter.scale(m_Zoom, m_Zoom);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  litehtml::position clip;
  clip.x = static_cast<litehtml::pixel_t>(hscroll / m_Zoom);
  clip.y = static_cast<litehtml::pixel_t>(vscroll / m_Zoom);
  clip.width = static_cast<litehtml::pixel_t>(this->viewport()->width() / m_Zoom);
  clip.height = static_cast<litehtml::pixel_t>(this->viewport()->height() / m_Zoom);

  m_Painter = &painter;
  m_Document->draw(reinterpret_cast<litehtml::uint_ptr>(&painter), 0, 0, &clip);
  m_Painter = nullptr;

  painter.restore();
}

void QmitkHtmlWidget::resizeEvent(QResizeEvent *event)
{
  QAbstractScrollArea::resizeEvent(event);
  this->Render();
  this->viewport()->update();
}

void QmitkHtmlWidget::scrollContentsBy(int, int)
{
  this->viewport()->update();
}

void QmitkHtmlWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_Document)
    return;

  const litehtml::pixel_t x = (event->position().x() + this->horizontalScrollBar()->value()) / m_Zoom;
  const litehtml::pixel_t y = (event->position().y() + this->verticalScrollBar()->value()) / m_Zoom;

  litehtml::position::vector redraw;
  if (m_Document->on_mouse_over(x, y, event->position().x() / m_Zoom, event->position().y() / m_Zoom, redraw))
    this->viewport()->update();
}

void QmitkHtmlWidget::mousePressEvent(QMouseEvent *event)
{
  if (!m_Document || event->button() != Qt::LeftButton)
  {
    QAbstractScrollArea::mousePressEvent(event);
    return;
  }

  const litehtml::pixel_t x = (event->position().x() + this->horizontalScrollBar()->value()) / m_Zoom;
  const litehtml::pixel_t y = (event->position().y() + this->verticalScrollBar()->value()) / m_Zoom;

  litehtml::position::vector redraw;
  if (m_Document->on_lbutton_down(x, y, event->position().x() / m_Zoom, event->position().y() / m_Zoom, redraw))
    this->viewport()->update();
}

void QmitkHtmlWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if (!m_Document || event->button() != Qt::LeftButton)
  {
    QAbstractScrollArea::mouseReleaseEvent(event);
    return;
  }

  const litehtml::pixel_t x = (event->position().x() + this->horizontalScrollBar()->value()) / m_Zoom;
  const litehtml::pixel_t y = (event->position().y() + this->verticalScrollBar()->value()) / m_Zoom;

  // Triggers document_container::on_anchor_click for <a href> under the cursor.
  litehtml::position::vector redraw;
  if (m_Document->on_lbutton_up(x, y, event->position().x() / m_Zoom, event->position().y() / m_Zoom, redraw))
    this->viewport()->update();
}

void QmitkHtmlWidget::leaveEvent(QEvent *)
{
  if (!m_Document)
    return;

  litehtml::position::vector redraw;
  if (m_Document->on_mouse_leave(redraw))
    this->viewport()->update();
}

litehtml::uint_ptr QmitkHtmlWidget::create_font(const litehtml::font_description &descr, const litehtml::document *, litehtml::font_metrics *fm)
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

void QmitkHtmlWidget::delete_font(litehtml::uint_ptr hFont)
{
  delete reinterpret_cast<FontHandle *>(hFont);
}

litehtml::pixel_t QmitkHtmlWidget::text_width(const char *text, litehtml::uint_ptr hFont)
{
  const auto *handle = reinterpret_cast<FontHandle *>(hFont);
  const QFontMetricsF metrics(handle->font);
  return static_cast<litehtml::pixel_t>(metrics.horizontalAdvance(QString::fromUtf8(text)));
}

void QmitkHtmlWidget::draw_text(litehtml::uint_ptr hdc, const char *text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position &pos)
{
  auto *painter = reinterpret_cast<QPainter *>(hdc);
  const auto *handle = reinterpret_cast<FontHandle *>(hFont);

  painter->setFont(handle->font);
  painter->setPen(toQColor(color));
  painter->drawText(QPointF(pos.x, pos.y + handle->ascent), QString::fromUtf8(text));
}

litehtml::pixel_t QmitkHtmlWidget::pt_to_px(float pt) const
{
  return static_cast<litehtml::pixel_t>(pt * this->logicalDpiY() / 72.0);
}

litehtml::pixel_t QmitkHtmlWidget::get_default_font_size() const
{
  return 16;
}

const char *QmitkHtmlWidget::get_default_font_name() const
{
  return m_DefaultFontName.c_str();
}

void QmitkHtmlWidget::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker &marker)
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

void QmitkHtmlWidget::load_image(const char *src, const char *baseurl, bool)
{
  const QUrl url = this->Resolve(QString::fromUtf8(src), QString::fromUtf8(baseurl != nullptr ? baseurl : ""));
  this->EnsureImage(url.toString(), url);
}

void QmitkHtmlWidget::get_image_size(const char *src, const char *baseurl, litehtml::size &sz)
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

void QmitkHtmlWidget::draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const std::string &url, const std::string &base_url)
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
  const qreal firstX = repeatX ? tile.x() - std::ceil((tile.x() - clip.left()) / tile.width()) * tile.width() : tile.x();
  const qreal firstY = repeatY ? tile.y() - std::ceil((tile.y() - clip.top()) / tile.height()) * tile.height() : tile.y();
  const qreal lastX = repeatX ? clip.right() : tile.x();
  const qreal lastY = repeatY ? clip.bottom() : tile.y();

  painter->save();
  painter->setClipRect(clip, Qt::IntersectClip);

  for (qreal y = firstY; y <= lastY; y += tile.height())
  {
    for (qreal x = firstX; x <= lastX; x += tile.width())
    {
      const QRectF target(x, y, tile.width(), tile.height());
      if (svg != nullptr)
        svg->render(painter, target);
      else
        painter->drawImage(target, image);
    }
  }

  painter->restore();
}

void QmitkHtmlWidget::draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::web_color &color)
{
  if (color.alpha == 0)
    return;

  auto *painter = reinterpret_cast<QPainter *>(hdc);
  painter->fillRect(toRectF(layer.border_box), toQColor(color));
}

void QmitkHtmlWidget::draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::background_layer::linear_gradient &gradient)
{
  QLinearGradient brush(gradient.start.x, gradient.start.y, gradient.end.x, gradient.end.y);
  applyStops(brush, gradient);
  reinterpret_cast<QPainter *>(hdc)->fillRect(toRectF(layer.border_box), brush);
}

void QmitkHtmlWidget::draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::background_layer::radial_gradient &gradient)
{
  QRadialGradient brush(QPointF(gradient.position.x, gradient.position.y), qMax(gradient.radius.x, gradient.radius.y));
  applyStops(brush, gradient);
  reinterpret_cast<QPainter *>(hdc)->fillRect(toRectF(layer.border_box), brush);
}

void QmitkHtmlWidget::draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::background_layer::conic_gradient &gradient)
{
  QConicalGradient brush(QPointF(gradient.position.x, gradient.position.y), gradient.angle);
  applyStops(brush, gradient);
  reinterpret_cast<QPainter *>(hdc)->fillRect(toRectF(layer.border_box), brush);
}

void QmitkHtmlWidget::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders &borders, const litehtml::position &draw_pos, bool)
{
  auto *painter = reinterpret_cast<QPainter *>(hdc);
  const QRectF rect = toRectF(draw_pos);

  auto drawSide = [painter](const litehtml::border &border, const QRectF &sideRect)
  {
    if (border.width <= 0 || border.style == litehtml::border_style_none || border.style == litehtml::border_style_hidden)
      return;
    painter->fillRect(sideRect, toQColor(border.color));
  };

  drawSide(borders.top, QRectF(rect.left(), rect.top(), rect.width(), borders.top.width));
  drawSide(borders.bottom, QRectF(rect.left(), rect.bottom() - borders.bottom.width, rect.width(), borders.bottom.width));
  drawSide(borders.left, QRectF(rect.left(), rect.top(), borders.left.width, rect.height()));
  drawSide(borders.right, QRectF(rect.right() - borders.right.width, rect.top(), borders.right.width, rect.height()));
}

void QmitkHtmlWidget::set_caption(const char *caption)
{
  m_Caption = QString::fromUtf8(caption);
  emit titleChanged(m_Caption);
}

void QmitkHtmlWidget::set_base_url(const char *base_url)
{
  const QString base = QString::fromUtf8(base_url != nullptr ? base_url : "");
  if (!base.isEmpty())
    m_BaseUrl = m_BaseUrl.resolved(QUrl(base));
}

void QmitkHtmlWidget::link(const std::shared_ptr<litehtml::document> &, const litehtml::element::ptr &)
{
}

void QmitkHtmlWidget::on_anchor_click(const char *url, const litehtml::element::ptr &)
{
  // on_anchor_click is invoked from inside the litehtml document's mouse
  // handler. Emitting synchronously would let a slot replace (and destroy) the
  // document while we are still executing inside it, so defer to the event loop.
  const QUrl resolved = m_BaseUrl.resolved(QUrl(QString::fromUtf8(url)));
  QMetaObject::invokeMethod(this, [this, resolved]() { emit linkClicked(resolved); }, Qt::QueuedConnection);
}

void QmitkHtmlWidget::on_mouse_event(const litehtml::element::ptr &, litehtml::mouse_event)
{
}

void QmitkHtmlWidget::set_cursor(const char *cursor)
{
  const QString name = QString::fromUtf8(cursor != nullptr ? cursor : "");
  this->viewport()->setCursor(name == QLatin1String("pointer") ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void QmitkHtmlWidget::transform_text(std::string &text, litehtml::text_transform tt)
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

void QmitkHtmlWidget::import_css(std::string &text, const std::string &url, std::string &baseurl)
{
  const QUrl resolved = this->Resolve(QString::fromStdString(url), QString::fromStdString(baseurl));
  const QByteArray data = this->Fetch(resolved);
  text.assign(data.constData(), static_cast<std::size_t>(data.size()));
}

void QmitkHtmlWidget::set_clip(const litehtml::position &pos, const litehtml::border_radiuses &)
{
  if (m_Painter == nullptr)
    return;

  m_Painter->save();
  m_Painter->setClipRect(toRectF(pos), Qt::IntersectClip);
}

void QmitkHtmlWidget::del_clip()
{
  if (m_Painter != nullptr)
    m_Painter->restore();
}

void QmitkHtmlWidget::get_viewport(litehtml::position &viewport) const
{
  viewport.x = static_cast<litehtml::pixel_t>(this->horizontalScrollBar()->value() / m_Zoom);
  viewport.y = static_cast<litehtml::pixel_t>(this->verticalScrollBar()->value() / m_Zoom);
  viewport.width = static_cast<litehtml::pixel_t>(this->viewport()->width() / m_Zoom);
  viewport.height = static_cast<litehtml::pixel_t>(this->viewport()->height() / m_Zoom);
}

litehtml::element::ptr QmitkHtmlWidget::create_element(const char *tag_name, const litehtml::string_map &attributes, const std::shared_ptr<litehtml::document> &doc)
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

void QmitkHtmlWidget::get_media_features(litehtml::media_features &media) const
{
  media.type = litehtml::media_type_screen;
  media.width = static_cast<litehtml::pixel_t>(this->viewport()->width() / m_Zoom);
  media.height = static_cast<litehtml::pixel_t>(this->viewport()->height() / m_Zoom);
  media.device_width = media.width;
  media.device_height = media.height;
  media.color = 8;
  media.color_index = 0;
  media.monochrome = 0;
  media.resolution = static_cast<litehtml::pixel_t>(this->logicalDpiY());
}

void QmitkHtmlWidget::get_language(std::string &language, std::string &culture) const
{
  language = "en";
  culture = "";
}
