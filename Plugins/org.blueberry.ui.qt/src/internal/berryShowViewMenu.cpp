/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryShowViewMenu.h"

#include <berryMenuManager.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>
#include <berryCommandContributionItem.h>
#include <berrySeparator.h>
#include <berryIPluginContribution.h>
#include <berryIViewRegistry.h>
#include <berryIWorkbenchCommandConstants.h>
#include <berryObjectString.h>

#include "berryCommandContributionItemParameter.h"
#include "berryWorkbenchPlugin.h"
#include "berryViewDescriptor.h"
#include "intro/berryIntroConstants.h"

#include <QMenu>
#include <QSet>
#include <QPair>

namespace berry {

const QString ShowViewMenu::NO_TARGETS_MSG = "<No Applicable Views>";

struct ActionComparator {
  bool operator()(const CommandContributionItemParameter::Pointer& p1,
                  const CommandContributionItemParameter::Pointer& p2) const
  {
    return p1->label < p2->label;
  }
};

ShowViewMenu::ShowViewMenu(IWorkbenchWindow *window, const QString& id)
  : ContributionItem(id), dirty(true), window(window)
{
  CommandContributionItemParameter::Pointer showDlgItemParms(
        new CommandContributionItemParameter(
          window, QString::null, IWorkbenchCommandConstants::VIEWS_SHOW_VIEW,
          CommandContributionItem::STYLE_PUSH));
  showDlgItemParms->label = "&Other...";
  showDlgItem = new CommandContributionItem(showDlgItemParms);

//  window.getWorkbench().getHelpSystem().setHelp(showDlgAction,
//      IWorkbenchHelpContextIds.SHOW_VIEW_OTHER_ACTION);
//  // indicate that a show views submenu has been created
//  if (window instanceof WorkbenchWindow) {
//    ((WorkbenchWindow) window)
//        .addSubmenu(WorkbenchWindow.SHOW_VIEW_SUBMENU);
//  }

}

bool ShowViewMenu::IsDirty() const
{
  return dirty;
}

/**
 * Overridden to always return true and force dynamic menu building.
 */
bool ShowViewMenu::IsDynamic() const
{
  return true;
}

void ShowViewMenu::Fill(QMenu* menu, QAction* before)
{
  if (MenuManager* mm = dynamic_cast<MenuManager*>(GetParent()))
  {
    this->connect(mm, SIGNAL(AboutToShow(IMenuManager*)), SLOT(AboutToShow(IMenuManager*)));
  }

  if (!dirty)
  {
    return;
  }

  MenuManager::Pointer manager(new MenuManager());
  FillMenu(manager.GetPointer());

  QList<IContributionItem::Pointer> items = manager->GetItems();
  if (items.isEmpty())
  {
    auto   action = new QAction(NO_TARGETS_MSG, menu);
    action->setEnabled(false);
    menu->insertAction(before, action);
  }
  else
  {
    foreach (IContributionItem::Pointer item, items)
    {
      item->Fill(menu, before);
    }
  }

  dirty = false;
}

void ShowViewMenu::FillMenu(IMenuManager* innerMgr)
{
  // Remove all.
  innerMgr->RemoveAll();

  // If no page disable all.
  IWorkbenchPage::Pointer page = window->GetActivePage();
  if (page.IsNull())
  {
    return;
  }

  // If no active perspective disable all
  if (page->GetPerspective().IsNull())
  {
    return;
  }

  typedef QPair<QString,QString> ViewIdPair;

  // Get visible actions.
  QSet<ViewIdPair> viewIds = GetShortcuts(page.GetPointer());

  // add all open views
  viewIds = AddOpenedViews(page.GetPointer(), viewIds);

  QList<CommandContributionItemParameter::Pointer> actions;

  foreach (ViewIdPair id, viewIds)
  {
    if (id.first == IntroConstants::INTRO_VIEW_ID)
    {
      continue;
    }
    CommandContributionItemParameter::Pointer item = GetItem(id.first, id.second);
    if (item)
    {
      actions.append(item);
    }
  }
  qSort(actions.begin(), actions.end(), ActionComparator());
  foreach (CommandContributionItemParameter::Pointer ccip, actions)
  {
//    if (WorkbenchActivityHelper.filterItem(ccip)) {
//      continue;
//    }
    CommandContributionItem::Pointer item(new CommandContributionItem(ccip));
    innerMgr->Add(item);
  }

  // We only want to add the separator if there are show view shortcuts,
  // otherwise, there will be a separator and then the 'Other...' entry
  // and that looks weird as the separator is separating nothing
  if (!innerMgr->IsEmpty())
  {
    IContributionItem::Pointer separator(new Separator());
    innerMgr->Add(separator);
  }

  // Add Other...
  innerMgr->Add(showDlgItem);
}

QSet<QPair<QString,QString> > ShowViewMenu::GetShortcuts(IWorkbenchPage* page) const
{
  QSet<QPair<QString,QString> > list;
  QList<QString> shortcuts(page->GetShowViewShortcuts());
  for (int i = 0; i < shortcuts.size(); ++i)
  {
    list.insert(qMakePair(shortcuts[i], QString()));
  }
  return list;
}

void ShowViewMenu::AboutToShow(IMenuManager* manager)
{
  manager->MarkDirty();
  this->dirty = true;
}

CommandContributionItemParameter::Pointer ShowViewMenu::GetItem(const QString& viewId, const QString& secondaryId) const
{
  IViewRegistry* reg = WorkbenchPlugin::GetDefault()->GetViewRegistry();
  IViewDescriptor::Pointer desc = reg->Find(viewId);
  if (desc.IsNull())
  {
    return CommandContributionItemParameter::Pointer(nullptr);
  }
  QString label = desc->GetLabel();

  class PluginCCIP : public CommandContributionItemParameter, public IPluginContribution
  {
    QString localId;
    QString pluginId;

  public:

    typedef PluginCCIP Self;
    static const char* GetStaticClassName() { return "PluginCCIP"; }
    berryObjectTypeInfo(CommandContributionItemParameter, IPluginContribution)

    PluginCCIP(const IViewDescriptor::Pointer& v, IServiceLocator* serviceLocator,
               const QString& id, const QString& commandId, CommandContributionItem::Style style)
      : CommandContributionItemParameter(serviceLocator, id, commandId, style)
    {
      ViewDescriptor::Pointer vd = v.Cast<ViewDescriptor>();
      localId = vd->GetLocalId();
      pluginId = vd->GetPluginId();
    }

    QString GetLocalId() const override
    {
      return localId;
    }

    QString GetPluginId() const override
    {
      return pluginId;
    }
  };

  CommandContributionItemParameter::Pointer parms(new PluginCCIP(desc,
      window, viewId, IWorkbenchCommandConstants::VIEWS_SHOW_VIEW,
      CommandContributionItem::STYLE_PUSH));
  parms->label = label;
  parms->icon = desc->GetImageDescriptor();

  Object::Pointer strViewId(new ObjectString(viewId));
  parms->parameters.insert(IWorkbenchCommandConstants::VIEWS_SHOW_VIEW_PARM_ID, strViewId);
//  if (makeFast)
//  {
//    parms.parameters.put(
//        IWorkbenchCommandConstants.VIEWS_SHOW_VIEW_PARM_FASTVIEW,
//        "true"); //$NON-NLS-1$
//  }
  if (!secondaryId.isEmpty())
  {
    Object::Pointer strSecondaryId(new ObjectString(secondaryId));
    parms->parameters.insert(IWorkbenchCommandConstants::VIEWS_SHOW_VIEW_SECONDARY_ID,
                            strSecondaryId);
  }
  return parms;
}

QSet<QPair<QString,QString> > ShowViewMenu::AddOpenedViews(IWorkbenchPage* page,
                                                           QSet<QPair<QString,QString> >& actions) const
{
  QSet<QPair<QString,QString> > views = GetParts(page);
  return views.unite(actions);
}

QSet<QPair<QString,QString> > ShowViewMenu::GetParts(IWorkbenchPage* page) const
{
  QSet<QPair<QString,QString> > parts;
  QList<IViewReference::Pointer> refs = page->GetViewReferences();
  for (int i = 0; i < refs.size(); ++i)
  {
    parts.insert(qMakePair(refs[i]->GetId(), refs[i]->GetSecondaryId()));
  }
  return parts;
}

}
