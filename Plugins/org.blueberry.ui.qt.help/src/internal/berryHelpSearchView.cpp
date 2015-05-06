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

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include "berryHelpSearchView.h"

#include "berryHelpPluginActivator.h"
#include "berryQHelpEngineWrapper.h"
#include "berryHelpEditorInput.h"
#include "berryHelpEditor.h"

#include <QLayout>
#include <QMenu>
#include <QEvent>
#include <QMouseEvent>
#include <QMimeData>
#include <QTextBrowser>
#include <QClipboard>
#include <QHelpSearchQueryWidget>
#include <QHelpSearchResultWidget>
#include <QApplication>

namespace berry {

HelpSearchView::HelpSearchView()
  : m_ZoomCount(0)
  , m_Parent(0)
  , m_SearchEngine(HelpPluginActivator::getInstance()->getQHelpEngine().searchEngine())
  , m_ResultWidget(0)
  , m_QueryWidget(0)
{
}

HelpSearchView::~HelpSearchView()
{
  // prevent deletion of the widget
  m_ResultWidget->setParent(0);
}

void HelpSearchView::CreateQtPartControl(QWidget* parent)
{
  if (m_ResultWidget == 0)
  {
    m_Parent = parent;

    QVBoxLayout* vLayout = new QVBoxLayout(parent);

    // This will be lead to strange behavior when using multiple instances of this view
    // because the QHelpSearchResultWidget instance is shared. The new view will
    // reparent the widget.
    m_ResultWidget = m_SearchEngine->resultWidget();
    m_QueryWidget = new QHelpSearchQueryWidget();

    vLayout->addWidget(m_QueryWidget);
    vLayout->addWidget(m_ResultWidget);

    connect(m_QueryWidget, SIGNAL(search()), this, SLOT(search()));
    connect(m_ResultWidget, SIGNAL(requestShowLink(QUrl)), this,
            SLOT(requestShowLink(QUrl)));

    connect(m_SearchEngine, SIGNAL(searchingStarted()), this,
            SLOT(searchingStarted()));
    connect(m_SearchEngine, SIGNAL(searchingFinished(int)), this,
            SLOT(searchingFinished(int)));

    QTextBrowser* browser = m_ResultWidget->findChild<QTextBrowser*>();
    if (browser) // Will be null if QtHelp was configured not to use CLucene.
    {
      browser->viewport()->installEventFilter(this);
      browser->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(browser, SIGNAL(customContextMenuRequested(QPoint)),
              this, SLOT(showContextMenu(QPoint)));
    }
  }
}

void HelpSearchView::SetFocus()
{
  if (!(m_ResultWidget->hasFocus()))
  {
    m_QueryWidget->setFocus();
  }
}

void HelpSearchView::zoomIn()
{
  QTextBrowser* browser = m_ResultWidget->findChild<QTextBrowser*>();
  if (browser && m_ZoomCount != 10)
  {
    m_ZoomCount++;
    browser->zoomIn();
  }
}

void HelpSearchView::zoomOut()
{
  QTextBrowser* browser = m_ResultWidget->findChild<QTextBrowser*>();
  if (browser && m_ZoomCount != -5)
  {
    m_ZoomCount--;
    browser->zoomOut();
  }
}

void HelpSearchView::resetZoom()
{
  if (m_ZoomCount == 0)
    return;

  QTextBrowser* browser = m_ResultWidget->findChild<QTextBrowser*>();
  if (browser)
  {
    browser->zoomOut(m_ZoomCount);
    m_ZoomCount = 0;
  }
}

void HelpSearchView::search() const
{
  QList<QHelpSearchQuery> query = m_QueryWidget->query();
  m_SearchEngine->search(query);
}

void HelpSearchView::searchingStarted()
{
  m_Parent->setCursor(QCursor(Qt::WaitCursor));
}

void HelpSearchView::searchingFinished(int hits)
{
  Q_UNUSED(hits)
  m_Parent->unsetCursor();
  //qApp->restoreOverrideCursor();
}

void HelpSearchView::requestShowLink(const QUrl &link)
{
  HelpPluginActivator::linkActivated(this->GetSite()->GetPage(), link);
}

bool HelpSearchView::eventFilter(QObject* o, QEvent *e)
{
  QTextBrowser* browser = m_ResultWidget->findChild<QTextBrowser*>();
  if (browser && o == browser->viewport()
      && e->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    QUrl link = m_ResultWidget->linkAt(me->pos());
    if (!link.isEmpty() || link.isValid())
    {
      bool controlPressed = me->modifiers() & Qt::ControlModifier;
      if((me->button() == Qt::LeftButton && controlPressed)
         || (me->button() == Qt::MidButton))
      {
        IEditorInput::Pointer input(new HelpEditorInput(link));
        this->GetSite()->GetPage()->OpenEditor(input, HelpEditor::EDITOR_ID);
      }
    }
  }
  return QObject::eventFilter(o,e);
}

void HelpSearchView::showContextMenu(const QPoint& point)
{
  QMenu menu;

  QTextBrowser* browser = m_ResultWidget->findChild<QTextBrowser*>();
  if (!browser)
    return;

//  QPoint point = browser->mapFromGlobal(pos);
//  if (!browser->rect().contains(point, true))
//    return;

  QUrl link = browser->anchorAt(point);

  QKeySequence keySeq(QKeySequence::Copy);
  QAction *copyAction = menu.addAction(tr("&Copy") + QLatin1String("\t") +
                                       keySeq.toString(QKeySequence::NativeText));
  copyAction->setEnabled(QTextCursor(browser->textCursor()).hasSelection());

  QAction *copyAnchorAction = menu.addAction(tr("Copy &Link Location"));
  copyAnchorAction->setEnabled(!link.isEmpty() && link.isValid());

  keySeq = QKeySequence(Qt::CTRL);
  QAction *newTabAction = menu.addAction(tr("Open Link in New Tab") +
                                         QLatin1String("\t") + keySeq.toString(QKeySequence::NativeText) +
                                         QLatin1String("LMB"));
  newTabAction->setEnabled(!link.isEmpty() && link.isValid());

  menu.addSeparator();

  keySeq = QKeySequence::SelectAll;
  QAction *selectAllAction = menu.addAction(tr("Select All") +
                                            QLatin1String("\t") + keySeq.toString(QKeySequence::NativeText));

  QAction *usedAction = menu.exec(browser->mapToGlobal(point));
  if (usedAction == copyAction)
  {
    QTextCursor cursor = browser->textCursor();
    if (!cursor.isNull() && cursor.hasSelection())
    {
      QString selectedText = cursor.selectedText();
      QMimeData *data = new QMimeData();
      data->setText(selectedText);
      QApplication::clipboard()->setMimeData(data);
    }
  }
  else if (usedAction == copyAnchorAction)
  {
    QApplication::clipboard()->setText(link.toString());
  }
  else if (usedAction == newTabAction)
  {
    IEditorInput::Pointer input(new HelpEditorInput(link));
    this->GetSite()->GetPage()->OpenEditor(input, HelpEditor::EDITOR_ID);
  }
  else if (usedAction == selectAllAction)
  {
    browser->selectAll();
  }
}

}
