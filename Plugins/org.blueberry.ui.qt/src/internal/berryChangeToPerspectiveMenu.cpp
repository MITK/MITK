/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryChangeToPerspectiveMenu.h"

#include <berryMenuManager.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>
#include <berryCommandContributionItem.h>
#include <berrySeparator.h>
#include <berryIPluginContribution.h>
#include <berryIPerspectiveRegistry.h>
#include <berryIPreferences.h>
#include <berryIWorkbenchCommandConstants.h>
#include <berryObjectString.h>
#include <berryObjects.h>

#include "berryCommandContributionItemParameter.h"
#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchPreferenceConstants.h"
#include "berryPreferenceConstants.h"

#include <QMenu>

namespace  berry {

const QString ChangeToPerspectiveMenu::NO_TARGETS_MSG = "<No Applicable Perspectives>";

bool PerspectiveComparator(const IPerspectiveDescriptor::Pointer& d1,
                           const IPerspectiveDescriptor::Pointer& d2)
{
  return d1->GetLabel() < d2->GetLabel();
}

ChangeToPerspectiveMenu::ChangeToPerspectiveMenu(IWorkbenchWindow* window, const QString& id)
  : ContributionItem(id)
  , window(window)
  , reg(window->GetWorkbench()->GetPerspectiveRegistry())
  , showActive(true)
  , dirty(true)
{

  CommandContributionItemParameter::Pointer showDlgItemParms(
        new CommandContributionItemParameter(
          window, QString::null, IWorkbenchCommandConstants::PERSPECTIVES_SHOW_PERSPECTIVE,
          CommandContributionItem::STYLE_PUSH));
  showDlgItemParms->label = "&Other...";
  showDlgItem = new CommandContributionItem(showDlgItemParms);

  // indicate that a open perspectives submenu has been created
  /*
  if (WorkbenchWindow* window = dynamic_cast<WorkbenchWindow*>(window))
  {
    window->AddSubmenu(WorkbenchWindow::OPEN_PERSPECTIVE_SUBMENU);
  }
  */
}

void ChangeToPerspectiveMenu::Fill(QMenu* menu, QAction* before)
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

bool ChangeToPerspectiveMenu::IsDirty() const
{
  return dirty;
}

bool ChangeToPerspectiveMenu::IsDynamic() const
{
  return true;
}

void ChangeToPerspectiveMenu::AboutToShow(IMenuManager* manager)
{
  manager->MarkDirty();
  dirty = true;
}

void ChangeToPerspectiveMenu::FillMenu(IMenuManager* manager)
{
  // Clear out the manager so that we have a blank slate.
  manager->RemoveAll();

  // Collect and sort perspective descriptors.
  QList<IPerspectiveDescriptor::Pointer> persps = GetPerspectiveShortcuts();
  qSort(persps.begin(), persps.end(), PerspectiveComparator);

  /*
   * Convert the perspective descriptors to command parameters, and filter
   * using the activity/capability mechanism.
   */
  for (IPerspectiveDescriptor::Pointer descriptor : persps)
  {
    CommandContributionItemParameter::Pointer ccip = GetItem(descriptor);

    //    if (WorkbenchActivityHelper.filterItem(ccip)) {
    //      continue;
    //    }
    CommandContributionItem::Pointer item(new CommandContributionItem(ccip));
    manager->Add(item);
  }

  IPreferences::Pointer prefs = WorkbenchPlugin::GetDefault()->GetPreferences();
  bool showOther = true;
  prefs->GetBool(WorkbenchPreferenceConstants::SHOW_OTHER_IN_PERSPECTIVE_MENU, showOther);
  if (showOther)
  {
    // Add a separator and then "Other..."
    if (!manager->IsEmpty())
    {
      IContributionItem::Pointer separator(new Separator());
      manager->Add(separator);
    }
    manager->Add(showDlgItem);
  }
}

SmartPointer<CommandContributionItemParameter> ChangeToPerspectiveMenu::GetItem(const IPerspectiveDescriptor::Pointer& desc) const
{
  IPreferences::Pointer prefs = WorkbenchPlugin::GetDefault()->GetPreferences();
  int mode = prefs->GetInt(PreferenceConstants::OPEN_PERSP_MODE, PreferenceConstants::OPM_ACTIVE_PAGE);
  IWorkbenchPage::Pointer page = window->GetActivePage();
  IPerspectiveDescriptor::Pointer persp;
  if (page.IsNotNull())
  {
    persp = page->GetPerspective();
  }

  QString perspId = desc->GetId();

  class PluginCCIP : public CommandContributionItemParameter, public IPluginContribution
  {
    QString localId;
    QString pluginId;

  public:

    typedef PluginCCIP Self;
    static const char* GetStaticClassName() { return "PluginCCIP"; }
    berryObjectTypeInfo(CommandContributionItemParameter, IPluginContribution)

    PluginCCIP(const IPerspectiveDescriptor::Pointer& v, IServiceLocator* serviceLocator,
               const QString& id, const QString& commandId, CommandContributionItem::Style style)
      : CommandContributionItemParameter(serviceLocator, id, commandId, style)
    {


      PerspectiveDescriptor::Pointer vd = v.Cast<PerspectiveDescriptor>();
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
      window, perspId, IWorkbenchCommandConstants::PERSPECTIVES_SHOW_PERSPECTIVE,
      CommandContributionItem::STYLE_PUSH));
  parms->label = desc->GetLabel();
  parms->icon = desc->GetImageDescriptor();

  Object::Pointer strId(new ObjectString(perspId));
  parms->parameters.insert(IWorkbenchCommandConstants::PERSPECTIVES_SHOW_PERSPECTIVE_PARM_ID, strId);

  // Only open a new window if user preference is set and the window
  // has an active perspective.
  if (PreferenceConstants::OPM_NEW_WINDOW == mode && persp.IsNotNull())
  {
    Object::Pointer bNewWnd(new ObjectBool(true));
    parms->parameters.insert(IWorkbenchCommandConstants::PERSPECTIVES_SHOW_PERSPECTIVE_PARM_NEWWINDOW,
                             bNewWnd);
  }

  return parms;
}

QList<SmartPointer<IPerspectiveDescriptor> > ChangeToPerspectiveMenu::GetPerspectiveShortcuts() const
{
  QList<IPerspectiveDescriptor::Pointer> list;

  IWorkbenchPage::Pointer page = window->GetActivePage();
  if (page.IsNull())
  {
    return list;
  }

  QList<QString> ids = page->GetPerspectiveShortcuts();
  for (int i = 0; i < ids.size(); i++)
  {
    IPerspectiveDescriptor::Pointer desc = reg->FindPerspectiveWithId(ids[i]);
    if (desc.IsNotNull() && !list.contains(desc))
    {
      /*
      if (WorkbenchActivityHelper::FilterItem(desc))
      {
        continue;
      }
      */
      list.push_back(desc);
    }
  }

  return list;
}

}
