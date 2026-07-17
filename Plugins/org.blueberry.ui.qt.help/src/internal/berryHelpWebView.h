/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYHELPWEBVIEW_H
#define BERRYHELPWEBVIEW_H

#include <QmitkHtmlWidget.h>

#include <berryIEditorSite.h>

#include <QList>
#include <QUrl>

class QMouseEvent;
class QWheelEvent;

namespace berry {

class QHelpEngineWrapper;

/**
 * \brief Help page viewer built on the litehtml renderer.
 *
 * Serves qthelp:// pages and their resources out of the QHelpEngine, routes
 * external links to the system browser, and maintains a simple back/forward
 * navigation history (litehtml provides none).
 */
class HelpWebView : public QmitkHtmlWidget
{
  Q_OBJECT

public:
  explicit HelpWebView(IEditorSite::Pointer editorSite, QWidget *parent);
  ~HelpWebView() override;

  bool handleForwardBackwardMouseButtons(QMouseEvent *e);

  void setSource(const QUrl &url);

  QString documentTitle() const
  { return this->DocumentTitle(); }

  bool isForwardAvailable() const
  { return m_HistoryIndex < m_History.size() - 1; }
  bool isBackwardAvailable() const
  { return m_HistoryIndex > 0; }

  static const QString m_MissingContextMessage;
  static const QString m_PageNotFoundMessage;

public Q_SLOTS:

  void backward();
  void forward();
  void home();

  void scaleUp();
  void scaleDown();

Q_SIGNALS:
  void backwardAvailable(bool enabled);
  void forwardAvailable(bool enabled);
  void sourceChanged(const QUrl &);

protected:
  void mousePressEvent(QMouseEvent *) override;
  void wheelEvent(QWheelEvent *) override;

private Q_SLOTS:
  void onLinkClicked(const QUrl &url);

private:
  /** \brief Fetch and render the page at url without touching the history. */
  void load(const QUrl &url);
  void updateHistoryButtons();

  QHelpEngineWrapper& m_HelpEngine;

  QUrl m_CurrentUrl;
  QList<QUrl> m_History;
  int m_HistoryIndex;
};

}

#endif // BERRYHELPWEBVIEW_H
