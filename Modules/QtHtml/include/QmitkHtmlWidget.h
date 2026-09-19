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
#include <QByteArray>
#include <QColor>
#include <QString>
#include <QUrl>

#include <functional>
#include <memory>

/**
 * \brief A litehtml-based HTML view rendered with QPainter.
 *
 * Lays out and paints static HTML/CSS through the litehtml engine (no
 * JavaScript). It is a pure renderer: there is no text selection or find. The
 * page and its referenced resources (images, style sheets) are fetched through
 * a resource handler set by the owner, so any URL scheme (e.g. qthelp://) can
 * be served without a global URL scheme handler.
 */
class MITKQTHTML_EXPORT QmitkHtmlWidget : public QAbstractScrollArea
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

private:
  // The litehtml document_container implementation lives in Impl so that
  // litehtml stays out of this public header (and out of the module's public
  // dependencies). See QmitkHtmlWidget.cpp.
  class Impl;
  std::unique_ptr<Impl> m_Impl;
};

#endif // QMITKHTMLWIDGET_H
