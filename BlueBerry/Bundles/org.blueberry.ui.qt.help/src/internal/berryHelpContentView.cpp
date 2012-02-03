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

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include "berryHelpContentView.h"

#include "berryHelpPluginActivator.h"
#include "berryHelpEditor.h"
#include "berryHelpEditorInput.h"
#include "berryHelpWebView.h"
#include "berryQHelpEngineWrapper.h"

#include <berryIWorkbenchPage.h>

#include <QSortFilterProxyModel>
#include <QHelpContentWidget>
#include <QLayout>
#include <QMenu>
#include <QHeaderView>
#include <QDir>

namespace berry {

HelpContentWidget::HelpContentWidget()
  : QTreeView(0)
  , m_SortModel(new QSortFilterProxyModel(this))
  , m_SourceModel(0)
{
  header()->hide();
  setUniformRowHeights(true);
  connect(this, SIGNAL(activated(QModelIndex)),
          this, SLOT(showLink(QModelIndex)));

  m_SortModel->setDynamicSortFilter(true);
  QTreeView::setModel(m_SortModel);
}

QModelIndex HelpContentWidget::indexOf(const QUrl &link)
{
  QHelpContentModel *contentModel =
      qobject_cast<QHelpContentModel*>(m_SourceModel);
  if (!contentModel || link.scheme() != QLatin1String("qthelp"))
    return QModelIndex();

  m_syncIndex = QModelIndex();
  for (int i=0; i<contentModel->rowCount(); ++i)
  {
    QHelpContentItem *itm =
        contentModel->contentItemAt(contentModel->index(i, 0));
    if (itm && itm->url().host() == link.host())
    {
      QString path = link.path();
      if (path.startsWith(QLatin1Char('/')))
        path = path.mid(1);
      if (searchContentItem(contentModel, contentModel->index(i, 0), path))
      {
        return m_syncIndex;
      }
    }
  }
  return QModelIndex();
}

void HelpContentWidget::setModel(QAbstractItemModel *model)
{
  m_SourceModel = model;
  m_SortModel->setSourceModel(model);
}

bool HelpContentWidget::searchContentItem(QHelpContentModel *model,
                                          const QModelIndex &parent, const QString &path)
{
  QHelpContentItem *parentItem = model->contentItemAt(parent);
  if (!parentItem)
    return false;

  if (QDir::cleanPath(parentItem->url().path()) == path)
  {
    m_syncIndex = m_SortModel->mapFromSource(parent);
    return true;
  }

  for (int i=0; i<parentItem->childCount(); ++i)
  {
    if (searchContentItem(model, model->index(i, 0, parent), path))
      return true;
  }
  return false;
}

void HelpContentWidget::showLink(const QModelIndex &index)
{
  QHelpContentModel *contentModel = qobject_cast<QHelpContentModel*>(m_SourceModel);
  if (!contentModel)
    return;

  QHelpContentItem *item = contentModel->contentItemAt(m_SortModel->mapToSource(index));
  if (!item)
    return;
  QUrl url = item->url();
  if (url.isValid())
    emit linkActivated(url);
}

HelpContentView::HelpContentView()
  : m_ContentWidget(0)
{
}

HelpContentView::~HelpContentView()
{
}

void HelpContentView::CreateQtPartControl(QWidget* parent)
{
  if (m_ContentWidget == 0)
  {
    QVBoxLayout* verticalLayout = new QVBoxLayout(parent);
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    QHelpEngineWrapper& helpEngine = HelpPluginActivator::getInstance()->getQHelpEngine();
    m_ContentWidget = new HelpContentWidget();
    m_ContentWidget->setModel(helpEngine.contentModel());
    m_ContentWidget->sortByColumn(0, Qt::AscendingOrder);
    connect(helpEngine.contentModel(), SIGNAL(contentsCreationStarted()),
            this, SLOT(setContentsWidgetBusy()));
    connect(helpEngine.contentModel(), SIGNAL(contentsCreated()),
            this, SLOT(unsetContentsWidgetBusy()));
    verticalLayout->addWidget(m_ContentWidget);

    m_ContentWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_ContentWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(m_ContentWidget, SIGNAL(linkActivated(QUrl)), this, SLOT(linkActivated(QUrl)));
  }
}

void HelpContentView::linkActivated(const QUrl &link)
{
  IWorkbenchPage::Pointer page = this->GetSite()->GetPage();
  HelpPluginActivator::linkActivated(page, link);
}

void HelpContentView::showContextMenu(const QPoint &pos)
{
  if (!m_ContentWidget->indexAt(pos).isValid())
      return;

  QHelpContentModel* contentModel =
      qobject_cast<QHelpContentModel*>(m_ContentWidget->model());
  QHelpContentItem *itm =
      contentModel->contentItemAt(m_ContentWidget->currentIndex());

  QMenu menu;
  QAction *curTab = menu.addAction(tr("Open Link"));
  QAction *newTab = menu.addAction(tr("Open Link in New Tab"));
  if (!HelpWebView::canOpenPage(itm->url().path()))
      newTab->setEnabled(false);

  menu.move(m_ContentWidget->mapToGlobal(pos));

  QAction *action = menu.exec();
  if (curTab == action)
  {
    linkActivated(itm->url());
  }
  else if (newTab == action)
  {
    IEditorInput::Pointer input(new HelpEditorInput(itm->url()));
    this->GetSite()->GetPage()->OpenEditor(input, HelpEditor::EDITOR_ID);
  }
}

void HelpContentView::SetFocus()
{
  m_ContentWidget->setFocus();
}

void HelpContentView::setContentsWidgetBusy()
{
  m_ContentWidget->setCursor(Qt::WaitCursor);
}

void HelpContentView::unsetContentsWidgetBusy()
{
  m_ContentWidget->unsetCursor();
}

}
