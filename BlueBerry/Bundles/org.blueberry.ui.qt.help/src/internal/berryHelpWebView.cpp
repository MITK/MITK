/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "berryHelpWebView.h"

#include "berryHelpPluginActivator.h"
#include "berryHelpEditor.h"
#include "berryHelpEditorInput.h"
#include "berryQHelpEngineWrapper.h"

#include <berryIWorkbenchPage.h>

#include <QCoreApplication>
#include <QTimer>
#include <QStringBuilder>
#include <QTemporaryFile>
#include <QDesktopServices>
#include <QWheelEvent>

#include <QWebPage>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace berry {

struct ExtensionMap {
  const char *extension;
  const char *mimeType;
} extensionMap[] = {
    { ".bmp", "image/bmp" },
    { ".css", "text/css" },
    { ".gif", "image/gif" },
    { ".html", "text/html" },
    { ".htm", "text/html" },
    { ".ico", "image/x-icon" },
    { ".jpeg", "image/jpeg" },
    { ".jpg", "image/jpeg" },
    { ".js", "application/x-javascript" },
    { ".mng", "video/x-mng" },
    { ".pbm", "image/x-portable-bitmap" },
    { ".pgm", "image/x-portable-graymap" },
    { ".pdf", "application/pdf" },
    { ".png", "image/png" },
    { ".ppm", "image/x-portable-pixmap" },
    { ".rss", "application/rss+xml" },
    { ".svg", "image/svg+xml" },
    { ".svgz", "image/svg+xml" },
    { ".text", "text/plain" },
    { ".tif", "image/tiff" },
    { ".tiff", "image/tiff" },
    { ".txt", "text/plain" },
    { ".xbm", "image/x-xbitmap" },
    { ".xml", "text/xml" },
    { ".xpm", "image/x-xpm" },
    { ".xsl", "text/xsl" },
    { ".xhtml", "application/xhtml+xml" },
    { ".wml", "text/vnd.wap.wml" },
    { ".wmlc", "application/vnd.wap.wmlc" },
    { "about:blank", 0 },
    { 0, 0 }
};


const QString HelpWebView::m_PageNotFoundMessage =
    QCoreApplication::translate("org.blueberry.ui.qt.help", "<title>Error 404...</title><div "
                                "align=\"center\"><br><br><h1>The page could not be found</h1><br><h3>'%1'"
                                "</h3></div>");

class HelpNetworkReply : public QNetworkReply
{
public:
  HelpNetworkReply(const QNetworkRequest &request, const QByteArray &fileData,
                   const QString &mimeType);

  virtual void abort();

  virtual qint64 bytesAvailable() const
  { return data.length() + QNetworkReply::bytesAvailable(); }

protected:
  virtual qint64 readData(char *data, qint64 maxlen);

private:
  QByteArray data;
  qint64 origLen;
};

HelpNetworkReply::HelpNetworkReply(const QNetworkRequest &request,
                                   const QByteArray &fileData, const QString& mimeType)
  : data(fileData), origLen(fileData.length())
{
  setRequest(request);
  setOpenMode(QIODevice::ReadOnly);

  setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
  setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(origLen));
  QTimer::singleShot(0, this, SIGNAL(metaDataChanged()));
  QTimer::singleShot(0, this, SIGNAL(readyRead()));
}

void HelpNetworkReply::abort()
{
}

qint64 HelpNetworkReply::readData(char *buffer, qint64 maxlen)
{
  qint64 len = qMin(qint64(data.length()), maxlen);
  if (len) {
    memcpy(buffer, data.constData(), len);
    data.remove(0, len);
  }
  if (!data.length())
    QTimer::singleShot(0, this, SIGNAL(finished()));
  return len;
}

class HelpNetworkAccessManager : public QNetworkAccessManager
{
public:
  HelpNetworkAccessManager(QObject *parent);

protected:
    virtual QNetworkReply *createRequest(Operation op,
                                         const QNetworkRequest &request,
                                         QIODevice *outgoingData = 0);
};

HelpNetworkAccessManager::HelpNetworkAccessManager(QObject *parent)
  : QNetworkAccessManager(parent)
{
}

QNetworkReply *HelpNetworkAccessManager::createRequest(Operation /*op*/,
                                                       const QNetworkRequest &request,
                                                       QIODevice* /*outgoingData*/)
{
  QString url = request.url().toString();
  QHelpEngine& helpEngine = HelpPluginActivator::getInstance()->getQHelpEngine();

  // TODO: For some reason the url to load is already wrong (passed from webkit)
  // though the css file and the references inside should work that way. One
  // possible problem might be that the css is loaded at the same level as the
  // html, thus a path inside the css like (../images/foo.png) might cd out of
  // the virtual folder
  // if (!helpEngine.findFile(url).isValid()) {
  //   if (url.startsWith(AbstractHelpWebView::DocPath)) {
  //     QUrl newUrl = request.url();
  //     if (!newUrl.path().startsWith(QLatin1String("/qdoc/"))) {
  //       newUrl.setPath(QLatin1String("qdoc") + newUrl.path());
  //       url = newUrl.toString();
  //     }
  //   }
  // }

  const QString &mimeType = HelpWebView::mimeFromUrl(url);
  const QByteArray &data = helpEngine.findFile(url).isValid()
                           ? helpEngine.fileData(url)
                           : HelpWebView::m_PageNotFoundMessage.arg(url).toUtf8();
  return new HelpNetworkReply(request, data, mimeType.isEmpty()
                              ? QLatin1String("application/octet-stream") : mimeType);
}

class HelpPage : public QWebPage
{

public:

  HelpPage(IEditorSite::Pointer editorSite, QObject *parent);

protected:

  virtual QWebPage *createWindow(QWebPage::WebWindowType);
  virtual void triggerAction(WebAction action, bool checked = false);

  virtual bool acceptNavigationRequest(QWebFrame *frame,
                                       const QNetworkRequest &request,
                                       NavigationType type);

private:

  IEditorSite::Pointer m_EditorSite;
  bool m_CloseNewTabIfNeeded;

  friend class HelpWebView;
  QUrl m_loadingUrl;
  Qt::MouseButtons m_pressedButtons;
  Qt::KeyboardModifiers m_keyboardModifiers;

};

HelpPage::HelpPage(IEditorSite::Pointer editorSite, QObject *parent)
  : QWebPage(parent)
  , m_EditorSite(editorSite)
  , m_CloseNewTabIfNeeded(false)
  , m_pressedButtons(Qt::NoButton)
  , m_keyboardModifiers(Qt::NoModifier)
{

}

QWebPage *HelpPage::createWindow(QWebPage::WebWindowType type)
{
  IEditorInput::Pointer input(new HelpEditorInput(QUrl()));
  IEditorPart::Pointer editorPart = m_EditorSite->GetPage()->OpenEditor(input, HelpEditor::EDITOR_ID);
  HelpEditor::Pointer helpEditor = editorPart.Cast<HelpEditor>();
  HelpPage* newPage = static_cast<HelpPage*>(helpEditor->GetQWebPage());
  if (newPage)
    newPage->m_CloseNewTabIfNeeded = m_CloseNewTabIfNeeded;
  m_CloseNewTabIfNeeded = false;
  return newPage;
}

void HelpPage::triggerAction(WebAction action, bool checked)
{
  switch (action)
  {
  case OpenLinkInNewWindow:
    m_CloseNewTabIfNeeded = true;
  default:        // fall through
    QWebPage::triggerAction(action, checked);
    break;
  }
}

bool HelpPage::acceptNavigationRequest(QWebFrame *,
                                       const QNetworkRequest &request,
                                       QWebPage::NavigationType type)
{
  const bool closeNewTab = m_CloseNewTabIfNeeded;
  m_CloseNewTabIfNeeded = false;

//  const QUrl &url = request.url();
//  if (AbstractHelpWebView::launchWithExternalApp(url))
//  {
//    if (closeNewTab)
//      QMetaObject::invokeMethod(centralWidget, "closeTab");
//    return false;
//  }

//  if (type == QWebPage::NavigationTypeLinkClicked
//      && (m_keyboardModifiers & Qt::ControlModifier
//          || m_pressedButtons == Qt::MidButton))
//  {
//    if (centralWidget->newEmptyTab())
//      centralWidget->setSource(url);
//    m_pressedButtons = Qt::NoButton;
//    m_keyboardModifiers = Qt::NoModifier;
//    return false;
//  }

//  m_loadingUrl = url; // because of async page loading, we will hit some kind
  // of race condition while using a remote command, like a combination of
  // SetSource; SyncContent. SetSource would be called and SyncContents shortly
  // afterwards, but the page might not have finished loading and the old url
  // would be returned.
  return true;
}

// -- HelpWebView

HelpWebView::HelpWebView(IEditorSite::Pointer editorSite, QWidget *parent, qreal zoom)
  : QWebView(parent)
  //, parentWidget(parent)
  , m_LoadFinished(false)
  , m_HelpEngine(HelpPluginActivator::getInstance()->getQHelpEngine())
{
  setAcceptDrops(false);

  setPage(new HelpPage(editorSite, parent));

  page()->setNetworkAccessManager(new HelpNetworkAccessManager(this));

  QAction* action = pageAction(QWebPage::OpenLinkInNewWindow);
  action->setText(tr("Open Link in New Tab"));
  if (!parent)
    action->setVisible(false);

  pageAction(QWebPage::DownloadLinkToDisk)->setVisible(false);
  pageAction(QWebPage::DownloadImageToDisk)->setVisible(false);
  pageAction(QWebPage::OpenImageInNewWindow)->setVisible(false);

  connect(pageAction(QWebPage::Copy), SIGNAL(changed()), this,
          SLOT(actionChanged()));
  connect(pageAction(QWebPage::Back), SIGNAL(changed()), this,
          SLOT(actionChanged()));
  connect(pageAction(QWebPage::Forward), SIGNAL(changed()), this,
          SLOT(actionChanged()));
  connect(page(), SIGNAL(linkHovered(QString,QString,QString)), this,
          SIGNAL(highlighted(QString)));
  connect(this, SIGNAL(urlChanged(QUrl)), this, SIGNAL(sourceChanged(QUrl)));
  connect(this, SIGNAL(loadStarted()), this, SLOT(setLoadStarted()));
  connect(this, SIGNAL(loadFinished(bool)), this, SLOT(setLoadFinished(bool)));
  connect(page(), SIGNAL(printRequested(QWebFrame*)), this, SIGNAL(printRequested()));

  setFont(viewerFont());
  setTextSizeMultiplier(zoom == 0.0 ? 1.0 : zoom);
}

HelpWebView::~HelpWebView()
{
}

QFont HelpWebView::viewerFont() const
{
  //if (m_HelpEngine.usesBrowserFont())
  //  return m_HelpEngine.browserFont();

  QWebSettings *webSettings = QWebSettings::globalSettings();
  return QFont(webSettings->fontFamily(QWebSettings::StandardFont),
               webSettings->fontSize(QWebSettings::DefaultFontSize));
}

void HelpWebView::setViewerFont(const QFont &font)
{
  QWebSettings *webSettings = settings();
  webSettings->setFontFamily(QWebSettings::StandardFont, font.family());
  webSettings->setFontSize(QWebSettings::DefaultFontSize, font.pointSize());
}

void HelpWebView::scaleUp()
{
  setTextSizeMultiplier(textSizeMultiplier() + 0.1);
}

void HelpWebView::scaleDown()
{
  setTextSizeMultiplier(qMax(0.0, textSizeMultiplier() - 0.1));
}

void HelpWebView::resetScale()
{
  setTextSizeMultiplier(1.0);
}

bool HelpWebView::handleForwardBackwardMouseButtons(QMouseEvent *e)
{
  if (e->button() == Qt::XButton1)
  {
    triggerPageAction(QWebPage::Back);
    return true;
  }

  if (e->button() == Qt::XButton2)
  {
    triggerPageAction(QWebPage::Forward);
    return true;
  }

  return false;
}

QUrl HelpWebView::source() const
{
  HelpPage *currentPage = static_cast<HelpPage*> (page());
  if (currentPage && !hasLoadFinished())
  {
    // see HelpPage::acceptNavigationRequest(...)
    return currentPage->m_loadingUrl;
  }
  return url();
}

void HelpWebView::setSource(const QUrl &url)
{
  if (m_HelpEngine.findFile(url).isValid())
    load(url);
  else
    setHtml(m_PageNotFoundMessage.arg(url.toString()));
}

void HelpWebView::wheelEvent(QWheelEvent *e)
{
  if (e->modifiers()& Qt::ControlModifier)
  {
    e->accept();
    e->delta() > 0 ? scaleUp() : scaleDown();
  }
  else
  {
    QWebView::wheelEvent(e);
  }
}

void HelpWebView::mouseReleaseEvent(QMouseEvent *e)
{
#ifndef Q_OS_LINUX
  if (handleForwardBackwardMouseButtons(e))
    return;
#endif

  QWebView::mouseReleaseEvent(e);
}

void HelpWebView::actionChanged()
{
  QAction *a = qobject_cast<QAction *>(sender());
  if (a == pageAction(QWebPage::Copy))
    emit copyAvailable(a->isEnabled());
  else if (a == pageAction(QWebPage::Back))
    emit backwardAvailable(a->isEnabled());
  else if (a == pageAction(QWebPage::Forward))
    emit forwardAvailable(a->isEnabled());
}

void HelpWebView::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_LINUX
  if (handleForwardBackwardMouseButtons(event))
    return;
#endif

  HelpPage *currentPage = static_cast<HelpPage*>(page());
  if (currentPage)
  {
    currentPage->m_pressedButtons = event->buttons();
    currentPage->m_keyboardModifiers = event->modifiers();
  }
  QWebView::mousePressEvent(event);
}

void HelpWebView::setLoadStarted()
{
  m_LoadFinished = false;
}

void HelpWebView::setLoadFinished(bool ok)
{
  m_LoadFinished = ok;
  emit sourceChanged(url());
}

QString HelpWebView::mimeFromUrl(const QUrl &url)
{
  const QString &path = url.path();
  const int index = path.lastIndexOf(QLatin1Char('.'));
  const QByteArray &ext = path.mid(index).toUtf8().toLower();

  const ExtensionMap *e = extensionMap;
  while (e->extension)
  {
    if (ext == e->extension)
      return QLatin1String(e->mimeType);
    ++e;
  }
  return QLatin1String("");
}

bool HelpWebView::canOpenPage(const QString &url)
{
  return !mimeFromUrl(url).isEmpty();
}

bool HelpWebView::isLocalUrl(const QUrl &url)
{
  const QString &scheme = url.scheme();
  return scheme.isEmpty()
      || scheme == QLatin1String("file")
      || scheme == QLatin1String("qrc")
      || scheme == QLatin1String("data")
      || scheme == QLatin1String("qthelp")
      || scheme == QLatin1String("about");
}

bool HelpWebView::launchWithExternalApp(const QUrl &url)
{
  if (isLocalUrl(url))
  {
    const QHelpEngine& helpEngine = HelpPluginActivator::getInstance()->getQHelpEngine();
    const QUrl &resolvedUrl = helpEngine.findFile(url);
    if (!resolvedUrl.isValid())
      return false;

    const QString& path = resolvedUrl.path();
    if (!canOpenPage(path))
    {
      QTemporaryFile tmpTmpFile;
      if (!tmpTmpFile.open())
        return false;

      const QString &extension = QFileInfo(path).completeSuffix();
      QFile actualTmpFile(tmpTmpFile.fileName() % QLatin1String(".")
                          % extension);
      if (!actualTmpFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
        return false;

      actualTmpFile.write(helpEngine.fileData(resolvedUrl));
      actualTmpFile.close();
      return QDesktopServices::openUrl(QUrl(actualTmpFile.fileName()));
    }
  }
  else if (url.scheme() == QLatin1String("http"))
  {
    return QDesktopServices::openUrl(url);
  }
  return false;
}

void HelpWebView::home()
{
  setSource(m_HelpEngine.homePage());
}

}
