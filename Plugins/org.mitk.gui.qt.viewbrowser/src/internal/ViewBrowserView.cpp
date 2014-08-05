/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPerspectiveRegistry.h>
#include <berryPlatformUI.h>
#include <berryIWorkbenchPage.h>

// Qmitk
#include "ViewBrowserView.h"
#include <src/internal/mitkQtPerspectiveItem.h>
#include <src/internal/mitkQtViewItem.h>

// Qt
#include <QMessageBox>
#include <QTreeView>
#include <QStandardItem>


struct ViewBrowserViewListener : public berry::IPerspectiveListener
{
  ViewBrowserViewListener(ViewBrowserView* switcher)
  : switcher(switcher)
  {}

  Events::Types GetPerspectiveEventTypes() const
  {
    return Events::ACTIVATED;
  }

  virtual void PerspectiveChanged(berry::SmartPointer<berry::IWorkbenchPage> /*page*/,
      berry::IPerspectiveDescriptor::Pointer /*perspective*/, const std::string& /*changeId*/)
  {
    switcher->FillTreeList();
  }

  void PerspectiveActivated(berry::IWorkbenchPage::Pointer /*page*/,
    berry::IPerspectiveDescriptor::Pointer perspective)
  {
    switcher->FillTreeList();
  }

private:
  ViewBrowserView* switcher;
};

const std::string ViewBrowserView::VIEW_ID = "org.mitk.views.viewbrowser";

bool compareViews(berry::IViewDescriptor::Pointer a, berry::IViewDescriptor::Pointer b)
{
  if (a.IsNull() || b.IsNull())
    return false;
  return a->GetLabel().compare(b->GetLabel()) < 0;
}

void ViewBrowserView::SetFocus()
{
}

void ViewBrowserView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.m_PluginTreeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(CustomMenuRequested(QPoint)));
  connect( m_Controls.m_PluginTreeView, SIGNAL(clicked(const QModelIndex&)), SLOT(ItemClicked(const QModelIndex&)));

  // Create a new TreeModel for the data
  m_TreeModel = new QStandardItemModel();
  FillTreeList();

  m_Controls.m_PluginTreeView->setModel(m_TreeModel);

  this->perspListener = new ViewBrowserViewListener(this);
  berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->AddPerspectiveListener(this->perspListener);
}


void ViewBrowserView::FillTreeList()
{
  m_TreeModel->clear();
  QStandardItem *item = m_TreeModel->invisibleRootItem();

  // Get all available views and create a map of all views
  std::map<std::string, berry::IViewDescriptor::Pointer> viewMap;
  berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
  std::vector<berry::IViewDescriptor::Pointer> views(viewRegistry->GetViews());
  for (unsigned int i=0; i<views.size(); i++)
  {
    viewMap[views[i]->GetId()] = views[i];
  }
  std::sort(views.begin(), views.end(), compareViews);

  // Get all available perspectives
  berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
  std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());

  // Get the current Window and the current active Perspective if possible
  berry::IWorkbenchWindow::Pointer curWin = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow();
  std::string currentPersp = "";
  if (curWin.IsNotNull())
  {
    if (curWin->GetActivePage().IsNotNull())
    {
      berry::IPerspectiveDescriptor::Pointer persps = curWin->GetActivePage()->GetPerspective();
      currentPersp=persps->GetId();
    }
  }

  // Fill the TreeModel
  for (unsigned int i=0; i<perspectives.size(); i++)
  {
    berry::IPerspectiveDescriptor::Pointer p = perspectives.at(i);
    QList< QStandardItem*> preparedRow;
    mitk::QtPerspectiveItem* pItem = new mitk::QtPerspectiveItem(QString::fromStdString(p->GetLabel()));
    pItem->m_Perspective = p;
    preparedRow << pItem;
    item->appendRow(preparedRow);

    // If the current perspective is the active perspective add current views
    if (currentPersp.compare(p->GetId())==0)
    {
      if (curWin.IsNull())
        continue;

      berry::IWorkbenchPage::Pointer activePage = curWin->GetActivePage();
      std::vector< std::string > currentViews = activePage->GetShowViewShortcuts();
      // Create a vector with all current elements
      std::vector<berry::IViewDescriptor::Pointer> activeViews;
      for (int j = 0; j < currentViews.size(); ++j)
      {
        berry::IViewDescriptor::Pointer view = viewMap[currentViews[j]];
        if (view.IsNull())
          continue;
        activeViews.push_back(view);
      }
      std::sort(activeViews.begin(), activeViews.end(), compareViews);


      for (int j = 0; j < activeViews.size(); ++j)
      {
        berry::IViewDescriptor::Pointer view =activeViews[j];
        QList<QStandardItem *> secondRow;
        QIcon* icon = static_cast<QIcon*>(view->GetImageDescriptor()->CreateImage());
        mitk::QtViewItem* vItem = new mitk::QtViewItem(*icon,QString::fromStdString(view->GetLabel()));
        vItem->m_View = view;
        secondRow << vItem;
        preparedRow.first()->appendRow(secondRow);
      }
    }
   }

  // Add a list with all available views
  QList< QStandardItem*> preparedRow;
  QStandardItem* pItem = new QStandardItem(QIcon(),"All Views");
  preparedRow << pItem;
  item->appendRow(preparedRow);
  for (int i = 0; i < views.size(); ++i)
  {
    QList<QStandardItem *> secondRow;
    QIcon* icon = static_cast<QIcon*>(views[i]->GetImageDescriptor()->CreateImage());
    mitk::QtViewItem* vItem = new mitk::QtViewItem(*icon, QString::fromStdString(views[i]->GetLabel()));
    vItem->m_View = views[i];
    secondRow << vItem;
    preparedRow.first()->appendRow(secondRow);
  }
}

void ViewBrowserView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes )
{
    // iterate all selected objects, adjust warning visibility
    foreach( mitk::DataNode::Pointer node, nodes )
    {
        if( node.IsNotNull() )
        {
            return;
        }
    }
}

void ViewBrowserView::ItemClicked(const QModelIndex &index)
{
    QStandardItem* item = m_TreeModel->itemFromIndex(index);
    if ( dynamic_cast< mitk::QtPerspectiveItem* >(item) )
    {
        try
        {
//            berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
//            page->CloseAllPerspectives(false, false);
            mitk::QtPerspectiveItem* pItem = dynamic_cast< mitk::QtPerspectiveItem* >(item);
//            page->ClosePerspective( pItem->m_Perspective, true, false );
            berry::PlatformUI::GetWorkbench()->ShowPerspective( pItem->m_Perspective->GetId(), berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow() );
        }
        catch (...)
        {
            QMessageBox::critical(0, "Opening Perspective Failed", QString("The requested perspective could not be opened.\nSee the log for details."));
        }
    }
    else if ( dynamic_cast< mitk::QtViewItem* >(item) )
    {
        berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();

        if (page.IsNotNull())
        {
            try
            {
                mitk::QtViewItem* vItem = dynamic_cast< mitk::QtViewItem* >(item);
                page->ShowView(vItem->m_View->GetId());
            }
            catch (berry::PartInitException e)
            {
                BERRY_ERROR << "Error: " << e.displayText() << std::endl;
            }
        }
    }
}
void ViewBrowserView::CustomMenuRequested(QPoint pos)
{
    //    m_ContextMenu->popup(m_Controls.m_PerspectiveTree->viewport()->mapToGlobal(pos));
}