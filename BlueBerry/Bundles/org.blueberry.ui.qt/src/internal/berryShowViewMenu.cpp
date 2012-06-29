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
//  final IHandlerService handlerService = (IHandlerService) window
//      .getService(IHandlerService.class);
//  final ICommandService commandService = (ICommandService) window
//      .getService(ICommandService.class);
//  final ParameterizedCommand cmd = getCommand(commandService, makeFast);

//  showDlgAction = new Action(WorkbenchMessages.ShowView_title) {
//    public void run() {
//      try {
//        handlerService.executeCommand(cmd, null);
//      } catch (final ExecutionException e) {
//        // Do nothing.
//      } catch (NotDefinedException e) {
//        // Do nothing.
//      } catch (NotEnabledException e) {
//        // Do nothing.
//      } catch (NotHandledException e) {
//        // Do nothing.
//      }
//    }
//  };

//  window.getWorkbench().getHelpSystem().setHelp(showDlgAction,
//      IWorkbenchHelpContextIds.SHOW_VIEW_OTHER_ACTION);
//  // indicate that a show views submenu has been created
//  if (window instanceof WorkbenchWindow) {
//    ((WorkbenchWindow) window)
//        .addSubmenu(WorkbenchWindow.SHOW_VIEW_SUBMENU);
//  }

//  showDlgAction.setActionDefinitionId(IWorkbenchCommandConstants.VIEWS_SHOW_VIEW);

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

QAction* ShowViewMenu::Fill(QMenu* menu, QAction* before)
{
//  if (MenuManager::Pointer mm = GetParent().Cast<MenuManager>())
//  {
//    mm->AddMenuListener(menuListener);
//  }

  if (!dirty)
  {
    return 0;
  }

  MenuManager::Pointer manager(new MenuManager());
  FillMenu(manager.GetPointer());

  QList<IContributionItem::Pointer> items = manager->GetItems();
  if (items.size() == 0)
  {
    QAction* action = new QAction("No Views registered", menu);
    action->setEnabled(false);
    menu->insertAction(before, action);
  }
  else
  {
    foreach (IContributionItem::Pointer item, items)
    {
      before = item->Fill(menu, before);
    }
  }

  dirty = false;
  return before;
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
    if (id.first == QString::fromStdString(IntroConstants::INTRO_VIEW_ID))
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
  //innerMgr->Add(showDlgAction);
}

QSet<QPair<QString,QString> > ShowViewMenu::GetShortcuts(IWorkbenchPage* page) const
{
  QSet<QPair<QString,QString> > list;
  std::vector<std::string> shortcuts(page->GetShowViewShortcuts());
  for (int i = 0; i < shortcuts.size(); ++i)
  {
    list.insert(QPair<QString,QString>(QString::fromStdString(shortcuts[i]), QString()));
  }
  return list;
}

CommandContributionItemParameter::Pointer ShowViewMenu::GetItem(const QString& viewId, const QString& secondaryId) const
{
  IViewRegistry* reg = WorkbenchPlugin::GetDefault()->GetViewRegistry();
  IViewDescriptor::Pointer desc = reg->Find(viewId.toStdString());
  if (desc.IsNull())
  {
    return CommandContributionItemParameter::Pointer(0);
  }
  std::string label = desc->GetLabel();

  class PluginCCIP : public CommandContributionItemParameter, public IPluginContribution
  {
    QString localId;
    QString pluginId;

  public:
    PluginCCIP(const IViewDescriptor::Pointer& v, IServiceLocator* serviceLocator,
               const QString& id, const QString& commandId, CommandContributionItem::Style style)
      : CommandContributionItemParameter(serviceLocator, id, commandId, style)
    {
      ViewDescriptor::Pointer vd = v.Cast<ViewDescriptor>();
      localId = vd->GetLocalId();
      pluginId = vd->GetPluginId();
    }

    QString GetLocalId() const
    {
      return localId;
    }

    QString GetPluginId() const
    {
      return pluginId;
    }
  };

  CommandContributionItemParameter::Pointer parms(new PluginCCIP(desc,
      window, viewId, IWorkbenchCommandConstants::VIEWS_SHOW_VIEW,
      CommandContributionItem::STYLE_PUSH));
  parms->label = QString::fromStdString(label);
  QIcon icon(*(reinterpret_cast<QIcon*>(desc->GetImageDescriptor()->CreateImage())));
  parms->icon = icon;

  Object::Pointer strViewId(new ObjectString(viewId.toStdString()));
  parms->parameters.insert(IWorkbenchCommandConstants::VIEWS_SHOW_VIEW_PARM_ID, strViewId);
//  if (makeFast)
//  {
//    parms.parameters.put(
//        IWorkbenchCommandConstants.VIEWS_SHOW_VIEW_PARM_FASTVIEW,
//        "true"); //$NON-NLS-1$
//  }
  if (!secondaryId.isEmpty())
  {
    Object::Pointer strSecondaryId(new ObjectString(secondaryId.toStdString()));
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
  std::vector<IViewReference::Pointer> refs = page->GetViewReferences();
  for (int i = 0; i < refs.size(); ++i)
  {
    parts.insert(qMakePair(QString::fromStdString(refs[i]->GetId()),
                           QString::fromStdString(refs[i]->GetSecondaryId())));
  }
  return parts;
}

}
