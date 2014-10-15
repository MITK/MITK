/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYHELPWEBVIEW_H
#define BERRYHELPWEBVIEW_H

#include <QFont>
#include <QAction>

#include <QWebView>

#include <berryIEditorSite.h>


namespace berry {

class QHelpEngineWrapper;

class HelpWebView : public QWebView
{
  Q_OBJECT

public:
  explicit HelpWebView(IEditorSite::Pointer editorSite, QWidget *parent, qreal zoom = 0.0);
  ~HelpWebView();

  QFont viewerFont() const;
  void setViewerFont(const QFont &font);

  qreal scale() const { return textSizeMultiplier(); }

  bool handleForwardBackwardMouseButtons(QMouseEvent *e);

  QUrl source() const;
  void setSource(const QUrl &url);

  inline QString documentTitle() const
  { return title(); }

  inline bool hasSelection() const
  { return !selectedText().isEmpty(); } // ### this is suboptimal

  inline void copy()
  { return triggerPageAction(QWebPage::Copy); }

  inline bool isForwardAvailable() const
  { return pageAction(QWebPage::Forward)->isEnabled(); }
  inline bool isBackwardAvailable() const
  { return pageAction(QWebPage::Back)->isEnabled(); }
  inline bool hasLoadFinished() const
  { return m_LoadFinished; }

  static QString mimeFromUrl(const QUrl &url);
  static bool canOpenPage(const QString &url);
  static bool isLocalUrl(const QUrl &url);
  static bool launchWithExternalApp(const QUrl &url);
  static const QString m_PageNotFoundMessage;

public Q_SLOTS:

  void backward() { back(); }
  void home();
  void print();

  void scaleUp();
  void scaleDown();
  void resetScale();

Q_SIGNALS:
  void copyAvailable(bool enabled);
  void forwardAvailable(bool enabled);
  void backwardAvailable(bool enabled);
  void highlighted(const QString &);
  void sourceChanged(const QUrl &);
  void printRequested();

protected:
  virtual void wheelEvent(QWheelEvent *);
  void mouseReleaseEvent(QMouseEvent *e);
  void mousePressEvent(QMouseEvent *event);

private Q_SLOTS:
  void actionChanged();
  void setLoadStarted();
  void setLoadFinished(bool ok);

private:

  bool m_LoadFinished;
  QHelpEngineWrapper& m_HelpEngine;
};

}

#endif // BERRYHELPWEBVIEW_H
