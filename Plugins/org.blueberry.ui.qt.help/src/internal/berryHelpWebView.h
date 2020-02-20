/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYHELPWEBVIEW_H
#define BERRYHELPWEBVIEW_H

#include <QFont>
#include <QAction>

#include <QWebEnginePage>
#include <QWebEngineView>

#include <berryIEditorSite.h>

class QWebEngineUrlSchemeHandler;

namespace berry {

class QHelpEngineWrapper;

class HelpWebView : public QWebEngineView
{
  Q_OBJECT

public:
  explicit HelpWebView(IEditorSite::Pointer editorSite, QWidget *parent, qreal zoom = 0.0);
  ~HelpWebView() override;

  QFont viewerFont() const;
  void setViewerFont(const QFont &font);

  qreal scale() const { return this->zoomFactor(); }

  bool handleForwardBackwardMouseButtons(QMouseEvent *e);

  void setSource(const QUrl &url);

  inline QString documentTitle() const
  { return title(); }

  inline bool hasSelection() const
  { return !selectedText().isEmpty(); } // ### this is suboptimal

  inline void copy()
  { return triggerPageAction(QWebEnginePage::Copy); }

  inline bool isForwardAvailable() const
  { return pageAction(QWebEnginePage::Forward)->isEnabled(); }
  inline bool isBackwardAvailable() const
  { return pageAction(QWebEnginePage::Back)->isEnabled(); }
  inline bool hasLoadFinished() const
  { return m_LoadFinished; }

  static QString mimeFromUrl(const QUrl &url);
  static bool canOpenPage(const QString &url);
  static bool isLocalUrl(const QUrl &url);
  static bool launchWithExternalApp(const QUrl &url);
  static const QString m_MissingContextMessage;
  static const QString m_PageNotFoundMessage;

public Q_SLOTS:

  void backward() { back(); }
  void home();

  void scaleUp();
  void scaleDown();
  void resetScale();

Q_SIGNALS:
  void copyAvailable(bool enabled);
  void forwardAvailable(bool enabled);
  void backwardAvailable(bool enabled);
  void highlighted(const QString &);
  void sourceChanged(const QUrl &);

protected:
  void wheelEvent(QWheelEvent *) override;

private Q_SLOTS:
  void actionChanged();
  void setLoadStarted();
  void setLoadFinished(bool ok);

private:

  bool m_LoadFinished;
  QHelpEngineWrapper& m_HelpEngine;
  QWebEngineUrlSchemeHandler* m_HelpSchemeHandler;
};

}

#endif // BERRYHELPWEBVIEW_H
