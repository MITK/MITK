/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKHTMLWIDGET_H
#define QMITKHTMLWIDGET_H

#include <MitkQtHtmlExports.h>

#include <QAbstractScrollArea>
#include <QColor>
#include <QHash>
#include <QImage>
#include <QUrl>

#include <functional>
#include <string>

#include <litehtml.h>

class QMovie;
class QPainter;
class QSvgRenderer;

/**
 * \brief A litehtml-based HTML view rendered with QPainter.
 *
 * Lays out and paints static HTML/CSS through the litehtml engine (no
 * JavaScript). It is a pure renderer: there is no text selection or find. The
 * page and its referenced resources (images, style sheets) are fetched through
 * a resource handler set by the owner, so any URL scheme (e.g. qthelp://) can
 * be served without a global URL scheme handler.
 */
class MITKQTHTML_EXPORT QmitkHtmlWidget : public QAbstractScrollArea, public litehtml::document_container
{
  Q_OBJECT

public:
  explicit QmitkHtmlWidget(QWidget *parent = nullptr);
  ~QmitkHtmlWidget() override;

  /** \brief Bytes for a referenced URL (page, image, css), or empty if unresolved. */
  using ResourceHandler = std::function<QByteArray(const QUrl &)>;
  void SetResourceHandler(const ResourceHandler &handler);

  /** \brief Parse, lay out and display an HTML document with the given base URL. */
  void SetHtml(const QString &html, const QUrl &baseUrl);

  /** \brief The current document's title (from its &lt;title&gt; element). */
  QString DocumentTitle() const;

  void SetZoomFactor(qreal factor);
  qreal ZoomFactor() const;

  /** \brief Scroll so that the element with the given anchor name is visible. */
  void ScrollToAnchor(const QString &name);

  /**
   * \brief When enabled, the view sizes to its content height (via
   * heightForWidth) and hides its scroll bars, so it can flow inside a larger
   * layout instead of scrolling internally. Disabled by default.
   */
  void SetFitToContent(bool enabled);

  /** \brief Colour painted behind the document (default white). */
  void SetPageColor(const QColor &color);

  QSize sizeHint() const override;
  bool hasHeightForWidth() const override;
  int heightForWidth(int width) const override;

Q_SIGNALS:
  void linkClicked(const QUrl &url);
  void titleChanged(const QString &title);

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void scrollContentsBy(int dx, int dy) override;

  // litehtml::document_container
  litehtml::uint_ptr create_font(const litehtml::font_description &descr, const litehtml::document *doc, litehtml::font_metrics *fm) override;
  void delete_font(litehtml::uint_ptr hFont) override;
  litehtml::pixel_t text_width(const char *text, litehtml::uint_ptr hFont) override;
  void draw_text(litehtml::uint_ptr hdc, const char *text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position &pos) override;
  litehtml::pixel_t pt_to_px(float pt) const override;
  litehtml::pixel_t get_default_font_size() const override;
  const char *get_default_font_name() const override;
  void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker &marker) override;
  void load_image(const char *src, const char *baseurl, bool redraw_on_ready) override;
  void get_image_size(const char *src, const char *baseurl, litehtml::size &sz) override;
  void draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const std::string &url, const std::string &base_url) override;
  void draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::web_color &color) override;
  void draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::background_layer::linear_gradient &gradient) override;
  void draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::background_layer::radial_gradient &gradient) override;
  void draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer &layer, const litehtml::background_layer::conic_gradient &gradient) override;
  void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders &borders, const litehtml::position &draw_pos, bool root) override;
  void set_caption(const char *caption) override;
  void set_base_url(const char *base_url) override;
  void link(const std::shared_ptr<litehtml::document> &doc, const litehtml::element::ptr &el) override;
  void on_anchor_click(const char *url, const litehtml::element::ptr &el) override;
  void on_mouse_event(const litehtml::element::ptr &el, litehtml::mouse_event event) override;
  void set_cursor(const char *cursor) override;
  void transform_text(std::string &text, litehtml::text_transform tt) override;
  void import_css(std::string &text, const std::string &url, std::string &baseurl) override;
  void set_clip(const litehtml::position &pos, const litehtml::border_radiuses &bdr_radius) override;
  void del_clip() override;
  void get_viewport(litehtml::position &viewport) const override;
  litehtml::element::ptr create_element(const char *tag_name, const litehtml::string_map &attributes, const std::shared_ptr<litehtml::document> &doc) override;
  void get_media_features(litehtml::media_features &media) const override;
  void get_language(std::string &language, std::string &culture) const override;

private:
  struct AnimatedImage
  {
    QMovie *movie;
    QSize size;
  };

  QByteArray Fetch(const QUrl &url) const;
  QUrl Resolve(const QString &src, const QString &baseUrl) const;
  /** \brief Fetch and cache an image, as a QMovie if it is an animated GIF. */
  void EnsureImage(const QString &key, const QUrl &url);
  /** \brief The current frame of an animated image, or the static image. */
  QImage CurrentFrame(const QString &key) const;
  void ClearAnimations();
  void ClearSvgImages();
  void Render();
  void UpdateScrollBars();

  ResourceHandler m_ResourceHandler;
  litehtml::document::ptr m_Document;
  QUrl m_BaseUrl;
  QString m_Caption;
  std::string m_DefaultFontName;
  qreal m_Zoom;
  bool m_FitToContent;
  QColor m_PageColor;

  QHash<QString, QImage> m_Images;
  QHash<QString, AnimatedImage> m_Animations;
  QHash<QString, QSvgRenderer *> m_SvgImages;
  QPainter *m_Painter;
};

#endif // QMITKHTMLWIDGET_H
