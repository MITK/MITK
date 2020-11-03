/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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

#include <QWebEngineSettings>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineProfile>

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
    { "about:blank", nullptr },
    { nullptr, nullptr }
};

class HelpDeviceReply final : public QIODevice
{
public:
  HelpDeviceReply(const QUrl& request, const QByteArray& fileData);
  ~HelpDeviceReply() override;

  qint64 bytesAvailable() const override;
  void close() override;

private:
  qint64 readData(char* data, qint64 maxlen) override;
  qint64 writeData(const char* data, qint64 maxlen) override;

  QByteArray m_Data;
  const qint64 m_OrigLen;
};

HelpDeviceReply::HelpDeviceReply(const QUrl&, const QByteArray& fileData)
  : m_Data(fileData),
    m_OrigLen(fileData.length())
{
  this->setOpenMode(QIODevice::ReadOnly);

  QTimer::singleShot(0, this, &QIODevice::readyRead);
  QTimer::singleShot(0, this, &QIODevice::readChannelFinished);
}

HelpDeviceReply::~HelpDeviceReply()
{
}

qint64 HelpDeviceReply::bytesAvailable() const
{
  return m_Data.length() + QIODevice::bytesAvailable();
}

void HelpDeviceReply::close()
{
  QIODevice::close();
  this->deleteLater();
}

qint64 HelpDeviceReply::readData(char* data, qint64 maxlen)
{
  qint64 len = qMin(qint64(m_Data.length()), maxlen);

  if (len)
  {
    memcpy(data, m_Data.constData(), len);
    m_Data.remove(0, len);
  }

  return len;
}

qint64 HelpDeviceReply::writeData(const char*, qint64)
{
  return 0;
}


class HelpUrlSchemeHandler final : public QWebEngineUrlSchemeHandler
{
public:
  explicit HelpUrlSchemeHandler(QObject* parent = nullptr);
  ~HelpUrlSchemeHandler() override;

  void requestStarted(QWebEngineUrlRequestJob* job) override;
};

HelpUrlSchemeHandler::HelpUrlSchemeHandler(QObject* parent)
  : QWebEngineUrlSchemeHandler(parent)
{
}

HelpUrlSchemeHandler::~HelpUrlSchemeHandler()
{
}

enum class ResolveUrlResult
{
  Error,
  Redirect,
  Data
};

ResolveUrlResult ResolveUrl(const QUrl& url, QUrl& redirectedUrl, QByteArray& data)
{
  auto& helpEngine = HelpPluginActivator::getInstance()->getQHelpEngine();

  const auto targetUrl = helpEngine.findFile(url);

  if (!targetUrl.isValid())
    return ResolveUrlResult::Error;

  if (targetUrl != url)
  {
    redirectedUrl = targetUrl;
    return ResolveUrlResult::Redirect;
  }

  data = helpEngine.fileData(targetUrl);
  return ResolveUrlResult::Data;
}


void HelpUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob* job)
{
  QUrl url = job->requestUrl();
  QUrl redirectedUrl;
  QByteArray data;

  switch (ResolveUrl(url, redirectedUrl, data))
  {
  case ResolveUrlResult::Data:
    job->reply(
      HelpWebView::mimeFromUrl(url).toLatin1(),
      new HelpDeviceReply(url, data));
    break;

  case ResolveUrlResult::Redirect:
    job->redirect(redirectedUrl);
    break;

  case ResolveUrlResult::Error:
    job->reply(
      QByteArrayLiteral("text/html"),
      new HelpDeviceReply(url, HelpWebView::m_PageNotFoundMessage.arg(url.toString()).toUtf8()));
    break;
  }
}

const QString HelpWebView::m_PageNotFoundMessage =
    QCoreApplication::translate("org.blueberry.ui.qt.help", "<title>Context Help</title><div "
                                "align=\"center\"><br><br><h1>No help page found for identifier</h1><br><h3>'%1'"
                                "</h3></div>");

const QString HelpWebView::m_MissingContextMessage =
    QCoreApplication::translate("org.blueberry.ui.qt.help", "<title>Context Help</title><div "
                                "align=\"center\"><br><br><h1>Unknown context..</h1><h1>&nbsp;</h1><h1>Please click inside a view and hit F1 again!</h1></div>");

class HelpPage final : public QWebEnginePage
{
public:
  explicit HelpPage(QObject* parent = nullptr);
  ~HelpPage() override;

private:
  bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;
};

HelpPage::HelpPage(QObject* parent)
  : QWebEnginePage(parent)
{
}

HelpPage::~HelpPage()
{
}

bool HelpPage::acceptNavigationRequest(const QUrl& url, NavigationType, bool)
{
  if (url.scheme().contains("http"))
  {
    QDesktopServices::openUrl(url);
    return false;
  }

  return true;
}

HelpWebView::HelpWebView(IEditorSite::Pointer, QWidget *parent, qreal zoom)
  : QWebEngineView(parent),
    m_LoadFinished(false),
    m_HelpEngine(HelpPluginActivator::getInstance()->getQHelpEngine()),
    m_HelpSchemeHandler(new HelpUrlSchemeHandler(this))
{
  QWebEngineProfile::defaultProfile()->installUrlSchemeHandler("qthelp", m_HelpSchemeHandler);

  auto helpPage = new HelpPage(this);
  this->setPage(helpPage);

  this->setAcceptDrops(false);

  auto action = pageAction(QWebEnginePage::OpenLinkInNewWindow);
  action->setText("Open Link in New Tab");

  if (parent == nullptr)
    action->setVisible(false);

  this->pageAction(QWebEnginePage::DownloadLinkToDisk)->setVisible(false);
  this->pageAction(QWebEnginePage::DownloadImageToDisk)->setVisible(false);

  connect(pageAction(QWebEnginePage::Copy), SIGNAL(changed()), this, SLOT(actionChanged()));
  connect(pageAction(QWebEnginePage::Back), SIGNAL(changed()), this, SLOT(actionChanged()));
  connect(pageAction(QWebEnginePage::Forward), SIGNAL(changed()), this, SLOT(actionChanged()));
  connect(page(), SIGNAL(linkHovered(QString)), this, SIGNAL(highlighted(QString)));
  connect(this, SIGNAL(urlChanged(QUrl)), this, SIGNAL(sourceChanged(QUrl)));
  connect(this, SIGNAL(loadStarted()), this, SLOT(setLoadStarted()));
  connect(this, SIGNAL(loadFinished(bool)), this, SLOT(setLoadFinished(bool)));

  this->setFont(this->viewerFont());
  this->setZoomFactor(zoom == 0.0 ? 1.0 : zoom);
}

HelpWebView::~HelpWebView()
{
}

QFont HelpWebView::viewerFont() const
{
  QWebEngineSettings *webSettings = QWebEngineSettings::globalSettings();
  return QFont(webSettings->fontFamily(QWebEngineSettings::StandardFont),
               webSettings->fontSize(QWebEngineSettings::DefaultFontSize));
}

void HelpWebView::setViewerFont(const QFont &font)
{
  QWebEngineSettings *webSettings = settings();
  webSettings->setFontFamily(QWebEngineSettings::StandardFont, font.family());
  webSettings->setFontSize(QWebEngineSettings::DefaultFontSize, font.pointSize());
}

void HelpWebView::scaleUp()
{
  setZoomFactor(zoomFactor() + 0.1);
}

void HelpWebView::scaleDown()
{
  setZoomFactor(qMax(0.0, zoomFactor() - 0.1));
}

void HelpWebView::resetScale()
{
  setZoomFactor(1.0);
}

bool HelpWebView::handleForwardBackwardMouseButtons(QMouseEvent *e)
{
  if (e->button() == Qt::XButton1)
  {
    triggerPageAction(QWebEnginePage::Back);
    return true;
  }

  if (e->button() == Qt::XButton2)
  {
    triggerPageAction(QWebEnginePage::Forward);
    return true;
  }

  return false;
}

void HelpWebView::setSource(const QUrl &url)
{
  if (url.toString().trimmed().isEmpty()) {
    setHtml(m_MissingContextMessage);
  }
  else if (m_HelpEngine.findFile(url).isValid())
  {
    load(url);
  }
  else
  {
    setHtml(m_PageNotFoundMessage.arg(url.toString()));
  }
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
    QWebEngineView::wheelEvent(e);
  }
}

void HelpWebView::actionChanged()
{
  QAction *a = qobject_cast<QAction *>(sender());
  if (a == pageAction(QWebEnginePage::Copy))
    emit copyAvailable(a->isEnabled());
  else if (a == pageAction(QWebEnginePage::Back))
    emit backwardAvailable(a->isEnabled());
  else if (a == pageAction(QWebEnginePage::Forward))
    emit forwardAvailable(a->isEnabled());
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
