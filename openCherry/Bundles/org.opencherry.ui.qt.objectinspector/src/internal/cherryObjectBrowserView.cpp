/*=========================================================================
 
 Program:   openCherry Platform
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

#include <QToolBar>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QLabel>

#include "cherryObjectBrowserView.h"
#include "cherryDebugUtil.h"
#include "cherryDebugBreakpointManager.h"

#include <sstream>

namespace cherry
{

const std::string ObjectBrowserView::VIEW_ID = "objectbrowser";

ObjectBrowserView::ObjectBrowserView() :
  m_ActionToggleBreakpoint(this), m_ActionEnableBreakpoint(this),
      m_ActionDisableBreakpoint(this)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  m_Useful = true;
#else
  m_Useful = false;
#endif
}

void ObjectBrowserView::Init(IViewSite::Pointer site, IMemento::Pointer memento)
{
  QtViewPart::Init(site, memento);

  m_StateMemento = memento;
}

void ObjectBrowserView::CreateQtPartControl(QWidget* parent)
{
  if (m_Useful)
  {
    m_Controls.setupUi(parent);

    m_ProxyModel = new QSortFilterProxyModel(m_Controls.m_TreeView);
    m_ObjectModel = new QtObjectTableModel(m_ProxyModel);

    m_ProxyModel->setSourceModel(m_ObjectModel);
    m_Controls.m_TreeView->setModel(m_ProxyModel);
    m_Controls.m_TreeView->setSortingEnabled(true);
    m_Controls.m_TreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    m_ActionToggleBreakpoint.setText(QString("Toggle Breakpoint"));
    m_ActionToggleBreakpoint.setCheckable(true);
    m_ContextMenu.addAction(&m_ActionToggleBreakpoint);

    QToolBar* toolbar = new QToolBar(parent);
    QAction* resetAction = toolbar->addAction("Reset");
    toolbar->addAction("Show Breakpoints Only");

    connect(resetAction, SIGNAL(triggered(bool)), this, SLOT(ResetAction(bool)));
    connect(m_Controls.m_TreeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(SelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(m_Controls.m_TreeView, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ContextMenuRequested(const QPoint&)));

    // context menu actions
    connect(&m_ActionToggleBreakpoint, SIGNAL(triggered(bool)), this, SLOT(ToggleBreakpoint(bool)));

    parent->layout()->setMenuBar(toolbar);

    RestoreGuiState(m_StateMemento);
    m_StateMemento = 0;
  }
  else
  {
    QVBoxLayout* layout = new QVBoxLayout(parent);
    QLabel* label = new QLabel(parent);
    label->setText(
        "Set the CMake variable OPENCHERRY_DEBUG_SMARTPOINTER to ON for a useful object browser.");
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop);
    layout->addWidget(label);
  }
}

void ObjectBrowserView::RestoreGuiState(IMemento::Pointer memento)
{
  if (memento)
  {
    IMemento::Pointer columnWidths = memento->GetChild("columnWidths");
    if (columnWidths)
    {
      int colWidth = 0;
      if (columnWidths->GetInteger("column0", colWidth))
      {
        m_Controls.m_TreeView->setColumnWidth(0, colWidth);
      }
      if (columnWidths->GetInteger("column1", colWidth))
      {
        m_Controls.m_TreeView->setColumnWidth(1, colWidth);
      }
    }

    IMemento::Pointer splitter = memento->GetChild("splitter");
    if (splitter)
    {
      QList<int> sizes;
      int size = 200;
      splitter->GetInteger("first", size);
      sizes.push_back(size);
      splitter->GetInteger("second", size);
      sizes.push_back(size);
      m_Controls.m_Splitter->setSizes(sizes);
    }
  }
}

void ObjectBrowserView::ResetAction(bool  /*checked*/)
{
  m_ObjectModel->ResetData();
}

void ObjectBrowserView::SelectionChanged(const QItemSelection& selected,
    const QItemSelection&  /*deselected*/)
{
  QList<QModelIndex> indexes = selected.indexes();
  if (indexes.empty())
  {
    m_Controls.m_DetailsView->clear();
    return;
  }

  QModelIndex index = indexes.front();
  if (!index.parent().isValid())
  {
    m_Controls.m_DetailsView->clear();
  }

  QVariant data = m_ProxyModel->data(index, Qt::UserRole);
  if (data.isValid())
  {
    const ObjectItem* item =
        static_cast<const ObjectItem*> (data.value<void*> ());
    if (item)
    {
      const Object* obj = 0;
      if (item->type == ObjectItem::INSTANCE)
        obj = item->obj;
      else if (item->type == ObjectItem::SMARTPOINTER)
      {
        const ObjectItem* item =
            static_cast<const ObjectItem*> (m_ProxyModel->data(index.parent(),
                Qt::UserRole).value<void*> ());
        if (item)
          obj = item->obj;
      }

      if (obj)
      {
        std::stringstream ss;
        ss << *(obj) << std::endl << obj->ToString();
        m_Controls.m_DetailsView->setPlainText(QString::fromStdString(ss.str()));
      }
      else
      {
        m_Controls.m_DetailsView->setPlainText(QString("0"));
      }
    }
    else
    {
      m_Controls.m_DetailsView->setPlainText(QString("0"));
    }
  }
}

void ObjectBrowserView::ContextMenuRequested(const QPoint& p)
{
  QModelIndex index = m_Controls.m_TreeView->selectionModel()->currentIndex();
  if (index.isValid())
  {
    QVariant data = m_ProxyModel->data(index, Qt::UserRole);
    if (!data.isValid())
      return;

    const ObjectItem* item = static_cast<ObjectItem*> (data.value<void*> ());
    if (item->type == ObjectItem::CLASS)
      return;

    m_ContextMenu.exec(m_Controls.m_TreeView->mapToGlobal(p));
  }
}

void ObjectBrowserView::ToggleBreakpoint(bool checked)
{
  QModelIndex index = m_Controls.m_TreeView->selectionModel()->currentIndex();
  if (index.isValid())
  {
    QVariant data = m_ProxyModel->data(index, Qt::UserRole);
    if (!data.isValid())
      return;

    const ObjectItem* item = static_cast<ObjectItem*> (data.value<void*> ());
    if (item->type == ObjectItem::INSTANCE)
    {
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
      if (checked)
      DebugUtil::GetBreakpointManager()->AddObjectBreakpoint(item->obj->GetTraceId());
      else
      DebugUtil::GetBreakpointManager()->RemoveObjectBreakpoint(item->obj->GetTraceId());
#endif
    }
    else if (item->type == ObjectItem::SMARTPOINTER)
    {
      if (checked)
        DebugUtil::GetBreakpointManager()->AddSmartpointerBreakpoint(item->spId);
      else
        DebugUtil::GetBreakpointManager()->RemoveSmartpointerBreakpoint(
            item->spId);
    }
  }
}

void ObjectBrowserView::SetFocus()
{
  if (m_Useful)
  {
    m_Controls.m_TreeView->setFocus();
  }
}

void ObjectBrowserView::SaveState(IMemento::Pointer memento)
{
  if (!m_Useful)
    return;

  IMemento::Pointer cols = memento->CreateChild("columnWidths");
  cols->PutInteger("column0", m_Controls.m_TreeView->columnWidth(0));
  cols->PutInteger("column1", m_Controls.m_TreeView->columnWidth(1));

  QList<int> sizes(m_Controls.m_Splitter->sizes());
  IMemento::Pointer splitter = memento->CreateChild("splitter");
  splitter->PutInteger("first", sizes[0]);
  splitter->PutInteger("second", sizes[1]);
}

} //namespace cherry
