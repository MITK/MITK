/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryHelpWebView.h"

#include "berryHelpPluginActivator.h"
#include "berryQHelpEngineWrapper.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStringBuilder>
#include <QTemporaryFile>
#include <QWheelEvent>

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

const QString HelpWebView::m_PageNotFoundMessage =
    QCoreApplication::translate("org.blueberry.ui.qt.help", "<title>Context Help</title><div "
                                "align=\"center\"><br><br><h1>No help page found for identifier</h1><br><h3>'%1'"
                                "</h3></div>");

const QString HelpWebView::m_MissingContextMessage =
    QCoreApplication::translate("org.blueberry.ui.qt.help", "<title>Context Help</title><div "
                                "align=\"center\"><br><br><h1>Unknown context..</h1><h1>&nbsp;</h1><h1>Please click inside a view and hit F1 again!</h1></div>");

HelpWebView::HelpWebView(IEditorSite::Pointer, QWidget *parent)
  : QmitkHtmlWidget(parent),
    m_HelpEngine(HelpPluginActivator::getInstance()->getQHelpEngine()),
    m_HistoryIndex(-1)
{
  // Serve the page's resources (images, style sheets) out of the .qch files.
  this->SetResourceHandler([this](const QUrl &url) -> QByteArray {
    const QUrl resolved = m_HelpEngine.findFile(url);
    return resolved.isValid() ? m_HelpEngine.fileData(resolved) : QByteArray();
  });

  connect(this, &QmitkHtmlWidget::linkClicked, this, &HelpWebView::onLinkClicked);
}

HelpWebView::~HelpWebView()
{
}

void HelpWebView::onLinkClicked(const QUrl &url)
{
  if (url.scheme().contains("http"))
  {
    QDesktopServices::openUrl(url);
  }
  else
  {
    this->setSource(url);
  }
}

void HelpWebView::load(const QUrl &url)
{
  if (url.toString().trimmed().isEmpty())
  {
    this->SetHtml(m_MissingContextMessage, QUrl());
    m_CurrentUrl = QUrl();
    return;
  }

  QUrl fileUrl = url;
  fileUrl.setFragment(QString());

  // Only reload when the page (ignoring the fragment) actually changes, so a
  // same-page anchor jump neither re-fetches nor re-renders.
  if (fileUrl != m_CurrentUrl)
  {
    const QUrl resolved = m_HelpEngine.findFile(fileUrl);
    if (resolved.isValid())
    {
      this->SetHtml(QString::fromUtf8(m_HelpEngine.fileData(resolved)), fileUrl);
      m_CurrentUrl = fileUrl;
    }
    else
    {
      this->SetHtml(m_PageNotFoundMessage.arg(url.toString()), QUrl());
      m_CurrentUrl = QUrl();
      return;
    }
  }

  if (url.hasFragment())
    this->ScrollToAnchor(url.fragment());
  else
    this->verticalScrollBar()->setValue(0);
}

void HelpWebView::setSource(const QUrl &url)
{
  this->load(url);

  // Drop any forward history and append the new location.
  while (m_History.size() > m_HistoryIndex + 1)
    m_History.removeLast();

  m_History.append(url);
  m_HistoryIndex = m_History.size() - 1;

  emit sourceChanged(url);
  this->updateHistoryButtons();
}

void HelpWebView::backward()
{
  if (!this->isBackwardAvailable())
    return;

  --m_HistoryIndex;
  this->load(m_History.at(m_HistoryIndex));
  emit sourceChanged(m_History.at(m_HistoryIndex));
  this->updateHistoryButtons();
}

void HelpWebView::forward()
{
  if (!this->isForwardAvailable())
    return;

  ++m_HistoryIndex;
  this->load(m_History.at(m_HistoryIndex));
  emit sourceChanged(m_History.at(m_HistoryIndex));
  this->updateHistoryButtons();
}

void HelpWebView::home()
{
  this->setSource(m_HelpEngine.homePage());
}

void HelpWebView::updateHistoryButtons()
{
  emit backwardAvailable(this->isBackwardAvailable());
  emit forwardAvailable(this->isForwardAvailable());
}

void HelpWebView::scaleUp()
{
  this->SetZoomFactor(this->ZoomFactor() + 0.1);
}

void HelpWebView::scaleDown()
{
  this->SetZoomFactor(qMax(0.1, this->ZoomFactor() - 0.1));
}

bool HelpWebView::handleForwardBackwardMouseButtons(QMouseEvent *e)
{
  if (e->button() == Qt::BackButton)
  {
    this->backward();
    return true;
  }

  if (e->button() == Qt::ForwardButton)
  {
    this->forward();
    return true;
  }

  return false;
}

void HelpWebView::wheelEvent(QWheelEvent *e)
{
  if (e->modifiers()& Qt::ControlModifier)
  {
    e->accept();
    e->angleDelta().y() > 0 ? scaleUp() : scaleDown();
  }
  else
  {
    QmitkHtmlWidget::wheelEvent(e);
  }
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

}
