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

#include "berryHelpIndexView.h"

#include "berryHelpPluginActivator.h"
#include "berryHelpEditor.h"
#include "berryHelpEditorInput.h"
#include "berryHelpWebView.h"
#include "berryQHelpEngineWrapper.h"
#include "berryHelpTopicChooser.h"

#include <berryIWorkbenchPage.h>

#include <ctkSearchBox.h>

#include <QHelpIndexWidget>
#include <QLayout>
#include <QLabel>
#include <QMenu>
#include <QKeyEvent>

namespace berry {

HelpIndexWidget::HelpIndexWidget()
  : QListView(0)
{
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setUniformItemSizes(true);
  connect(this, SIGNAL(activated(QModelIndex)),
          this, SLOT(showLink(QModelIndex)));
}

void HelpIndexWidget::showLink(const QModelIndex &index)
{
  if (!index.isValid())
    return;

  QHelpIndexModel *indexModel = qobject_cast<QHelpIndexModel*>(model());
  if (!indexModel)
    return;
  QVariant v = indexModel->data(index, Qt::DisplayRole);
  QString name;
  if (v.isValid())
    name = v.toString();

  QMap<QString, QUrl> links = indexModel->linksForKeyword(name);
  if (links.count() == 1)
  {
    emit linkActivated(links.constBegin().value(), name);
  }
  else if (links.count() > 1)
  {
    emit linksActivated(links, name);
  }
}

void HelpIndexWidget::activateCurrentItem()
{
  showLink(currentIndex());
}

void HelpIndexWidget::filterIndices(const QString &filter, const QString &wildcard)
{
  QHelpIndexModel *indexModel = qobject_cast<QHelpIndexModel*>(model());
  if (!indexModel)
    return;
  QModelIndex idx = indexModel->filter(filter, wildcard);
  if (idx.isValid())
    setCurrentIndex(idx);
}

HelpIndexView::HelpIndexView()
  : m_IndexWidget(0)
{
}

HelpIndexView::~HelpIndexView()
{
}

void HelpIndexView::CreateQtPartControl(QWidget* parent)
{
  if (m_IndexWidget == 0)
  {
    QVBoxLayout *layout = new QVBoxLayout(parent);
    //QLabel *l = new QLabel(tr("&Look for:"));
    //layout->addWidget(l);

    m_SearchLineEdit = new ctkSearchBox(parent);
    m_SearchLineEdit->setClearIcon(QIcon(":/org.blueberry.ui.qt.help/clear.png"));
    m_SearchLineEdit->setPlaceholderText("Filter...");
    m_SearchLineEdit->setContentsMargins(2,2,2,0);
    //l->setBuddy(m_SearchLineEdit);
    connect(m_SearchLineEdit, SIGNAL(textChanged(QString)), this,
            SLOT(filterIndices(QString)));
    m_SearchLineEdit->installEventFilter(this);
    layout->setMargin(0);
    layout->setSpacing(2);
    layout->addWidget(m_SearchLineEdit);

    QHelpEngineWrapper& helpEngine = HelpPluginActivator::getInstance()->getQHelpEngine();
    m_IndexWidget = new HelpIndexWidget();
    m_IndexWidget->setModel(helpEngine.indexModel());
    connect(helpEngine.indexModel(), SIGNAL(indexCreationStarted()),
            this, SLOT(setIndexWidgetBusy()));
    connect(helpEngine.indexModel(), SIGNAL(indexCreated()),
            this, SLOT(unsetIndexWidgetBusy()));
    m_IndexWidget->installEventFilter(this);

    connect(helpEngine.indexModel(), SIGNAL(indexCreationStarted()), this,
            SLOT(disableSearchLineEdit()));
    connect(helpEngine.indexModel(), SIGNAL(indexCreated()), this,
            SLOT(enableSearchLineEdit()));
    connect(m_IndexWidget, SIGNAL(linkActivated(QUrl,QString)), this,
            SLOT(linkActivated(QUrl)));
    connect(m_IndexWidget, SIGNAL(linksActivated(QMap<QString,QUrl>,QString)),
            this, SLOT(linksActivated(QMap<QString,QUrl>,QString)));
    connect(m_SearchLineEdit, SIGNAL(returnPressed()), m_IndexWidget,
            SLOT(activateCurrentItem()));
    layout->addWidget(m_IndexWidget);

    m_IndexWidget->viewport()->installEventFilter(this);
  }
}

void HelpIndexView::SetFocus()
{
  if (!(m_IndexWidget->hasFocus() || m_SearchLineEdit->hasFocus()))
  {
    m_SearchLineEdit->setFocus();
  }
}

void HelpIndexView::filterIndices(const QString &filter)
{
  if (filter.contains(QLatin1Char('*')))
    m_IndexWidget->filterIndices(filter, filter);
  else
    m_IndexWidget->filterIndices(filter, QString());
}

bool HelpIndexView::eventFilter(QObject *obj, QEvent *e)
{
  if (obj == m_SearchLineEdit && e->type() == QEvent::KeyPress)
  {
    QKeyEvent *ke = static_cast<QKeyEvent*>(e);
    QModelIndex idx = m_IndexWidget->currentIndex();
    switch (ke->key())
    {
    case Qt::Key_Up:
      idx = m_IndexWidget->model()->index(idx.row()-1,
                                          idx.column(), idx.parent());
      if (idx.isValid())
      {
        m_IndexWidget->setCurrentIndex(idx);
        return true;
      }
      break;
    case Qt::Key_Down:
      idx = m_IndexWidget->model()->index(idx.row()+1,
                                          idx.column(), idx.parent());
      if (idx.isValid())
      {
        m_IndexWidget->setCurrentIndex(idx);
        return true;
      }
      break;
    default: ; // stop complaining
    }
  }
  else if (obj == m_IndexWidget && e->type() == QEvent::ContextMenu)
  {
    QContextMenuEvent *ctxtEvent = static_cast<QContextMenuEvent*>(e);
    QModelIndex idx = m_IndexWidget->indexAt(ctxtEvent->pos());
    if (idx.isValid())
    {
      QMenu menu;
      QAction *curTab = menu.addAction(tr("Open Link"));
      QAction *newTab = menu.addAction(tr("Open Link in New Tab"));
      menu.move(m_IndexWidget->mapToGlobal(ctxtEvent->pos()));

      QAction *action = menu.exec();
      if (curTab == action)
        m_IndexWidget->activateCurrentItem();
      else if (newTab == action)
      {
        open(m_IndexWidget, idx);
      }
    }
  }
  else if (m_IndexWidget && obj == m_IndexWidget->viewport()
           && e->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
    QModelIndex idx = m_IndexWidget->indexAt(mouseEvent->pos());
    if (idx.isValid())
    {
      Qt::MouseButtons button = mouseEvent->button();
      if (((button == Qt::LeftButton) && (mouseEvent->modifiers() & Qt::ControlModifier))
          || (button == Qt::MidButton))
      {
        open(m_IndexWidget, idx);
      }
    }
  }
#ifdef Q_OS_MAC
  else if (obj == m_IndexWidget && e->type() == QEvent::KeyPress)
  {
    QKeyEvent *ke = static_cast<QKeyEvent*>(e);
    if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
      m_IndexWidget->activateCurrentItem();
  }
#endif
  return QObject::eventFilter(obj, e);
}

void HelpIndexView::enableSearchLineEdit()
{
  m_SearchLineEdit->setDisabled(false);
  filterIndices(m_SearchLineEdit->text());
}

void HelpIndexView::disableSearchLineEdit()
{
  m_SearchLineEdit->setDisabled(true);
}

void HelpIndexView::setIndexWidgetBusy()
{
  m_IndexWidget->setCursor(Qt::WaitCursor);
}

void HelpIndexView::unsetIndexWidgetBusy()
{
  m_IndexWidget->unsetCursor();
}

void HelpIndexView::setSearchLineEditText(const QString &text)
{
  m_SearchLineEdit->setText(text);
}

QString HelpIndexView::searchLineEditText() const
{
  return m_SearchLineEdit->text();
}

void HelpIndexView::focusInEvent(QFocusEvent *e)
{
  if (e->reason() != Qt::MouseFocusReason)
  {
    m_SearchLineEdit->selectAll();
    m_SearchLineEdit->setFocus();
  }
}

void HelpIndexView::open(HelpIndexWidget* indexWidget, const QModelIndex &index)
{
  QHelpIndexModel *model = qobject_cast<QHelpIndexModel*>(indexWidget->model());
  if (model)
  {
    QString keyword = model->data(index, Qt::DisplayRole).toString();
    QMap<QString, QUrl> links = model->linksForKeyword(keyword);

    QUrl url;
    if (links.count() > 1)
    {
      HelpTopicChooser tc(m_IndexWidget, keyword, links);
      if (tc.exec() == QDialog::Accepted)
        url = tc.link();
    }
    else if (links.count() == 1)
    {
      url = links.constBegin().value();
    }
    else
    {
      return;
    }

    //if (!HelpWebView::canOpenPage(url.path()))
    IEditorInput::Pointer input(new HelpEditorInput(url));
    this->GetSite()->GetPage()->OpenEditor(input, HelpEditor::EDITOR_ID);
  }
}

void HelpIndexView::linkActivated(const QUrl& link)
{
  IWorkbenchPage::Pointer page = this->GetSite()->GetPage();

  HelpPluginActivator::linkActivated(page, link);
}

void HelpIndexView::linksActivated(const QMap<QString,QUrl>& links, const QString& keyword)
{
  HelpTopicChooser tc(m_IndexWidget, keyword, links);
  if (tc.exec() == QDialog::Accepted)
  {
    IWorkbenchPage::Pointer page = this->GetSite()->GetPage();
    HelpPluginActivator::linkActivated(page, tc.link());
  }
}

}
