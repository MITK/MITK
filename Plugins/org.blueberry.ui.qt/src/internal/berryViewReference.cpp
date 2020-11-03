/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryViewReference.h"

#include <berryIConfigurationElement.h>

#include "berryUIException.h"
#include "tweaklets/berryWorkbenchPageTweaklet.h"
#include "berryPlatformUI.h"

#include "berryWorkbenchPage.h"
#include "berryWorkbenchConstants.h"
#include "berryViewDescriptor.h"
#include "berryViewFactory.h"
#include "berryViewRegistry.h"
#include "berryViewSite.h"
#include "berryPartTester.h"
#include "berryWorkbenchPlugin.h"
#include "berryErrorViewPart.h"

namespace berry
{

ViewReference::ViewReference(ViewFactory* fac, const QString& id,
    const QString& secId, IMemento::Pointer m) :
  factory(fac), secondaryId(secId), memento(m)
{
  ViewDescriptor::Pointer desc =
      this->factory->GetViewRegistry()->Find(id).Cast<ViewDescriptor> ();
  QIcon iDesc;
  QString title;
  if (!desc.IsNull())
  {
    iDesc = desc->GetImageDescriptor();
    title = desc->GetLabel();
  }

  QString name;

  if (!memento.IsNull())
  {
    //      name = memento.getString(IWorkbenchConstants.TAG_PART_NAME);
    //      IMemento propBag = memento.getChild(IWorkbenchConstants.TAG_PROPERTIES);
    //      if (propBag != null) {
    //        IMemento[] props = propBag
    //            .getChildren(IWorkbenchConstants.TAG_PROPERTY);
    //        for (int i = 0; i < props.length; i++) {
    //          propertyCache.put(props[i].getID(), props[i].getTextData());
    //        }
    //      }
  }
  if (name.isEmpty())
  {
    name = title;
  }

  this->Init(id, "", iDesc, name, ""); //$NON-NLS-1$//$NON-NLS-2$

}

void ViewReference::DoDisposePart()
{
  IViewPart::Pointer view = part.Cast<IViewPart> ();
  //WorkbenchPartReference::DoDisposePart();
  if (!view.IsNull())
  {
    // Free action bars, pane, etc.
    //PartSite site = (PartSite) view.getSite();
    //ViewActionBars actionBars = (ViewActionBars) site.getActionBars();
    //
    // 3.3 start
    //
    //IMenuService menuService = (IMenuService) site
    //    .getService(IMenuService.class);
    //menuService.releaseContributions((ContributionManager) site.getActionBars()
    //    .getMenuManager());
    //menuService.releaseContributions((ContributionManager) site.getActionBars()
    //    .getToolBarManager());
    // 3.3 end
    //actionBars.dispose();

    // and now dispose the delegates since the
    // PluginActionContributionItem
    // can no longer do that
    //      if (actionBuilder != null) {
    //        actionBuilder.dispose();
    //        actionBuilder = null;
    //      }

    // Free the site.
    //site.dispose();
  }

}

IWorkbenchPage::Pointer ViewReference::GetPage() const
{
  return IWorkbenchPage::Pointer(this->factory->GetWorkbenchPage());
}

QString ViewReference::GetRegisteredName()
{
  if (!part.IsNull() && !part->GetSite().IsNull())
  {
    return part->GetSite()->GetRegisteredName();
  }

  const IViewRegistry* reg = this->factory->GetViewRegistry();
  IViewDescriptor::Pointer desc = reg->Find(this->GetId());
  if (!desc.IsNull())
  {
    return desc->GetLabel();
  }
  return this->GetPartName();
}

QString ViewReference::GetSecondaryId()
{
  return secondaryId;
}

IViewPart::Pointer ViewReference::GetView(bool restore)
{
  return this->GetPart(restore).Cast<IViewPart> ();
}

IWorkbenchPart::Pointer ViewReference::CreatePart()
{

  // Check the status of this part

  IWorkbenchPart::Pointer result;
  PartInitException* exception = nullptr;

  // Try to restore the view -- this does the real work of restoring the
  // view
  //
  try
  {
    result = this->CreatePartHelper();
  }
  catch (const PartInitException& e)
  {
    exception = e.clone();
  }

  // If unable to create the part, create an error part instead
  // and pass the error to the status handling facility
  if (exception != nullptr)
  {
    //      IStatus partStatus = exception.getStatus();
    //      IStatus displayStatus = StatusUtil.newStatus(partStatus,
    //          NLS.bind(WorkbenchMessages.ViewFactory_initException, partStatus.getMessage()));
    //      IStatus logStatus = StatusUtil
    //          .newStatus(
    //              partStatus,
    //              NLS
    //                  .bind(
    //                      "Unable to create view ID {0}: {1}", getId(), partStatus.getMessage())); //$NON-NLS-1$

    // Pass the error to the status handling facility
    //      StatusManager.getManager().handle(logStatus);

    QString errorTitle = "Unable to create view ID " + this->GetId();
    WorkbenchPlugin::Log(errorTitle + ": " + exception->what());

    IViewDescriptor::Pointer desc = factory->GetViewRegistry()->Find(
        this->GetId());
    QString label = this->GetId();
    if (!desc.IsNull())
    {
      label = desc->GetLabel();
    }

    QString errorMsg(exception->what());
    delete exception;

    errorMsg
        += "<ul><li>Check your shared library for unresolved symbols</li>"
            "<li>Check your class attribute in your plugin.xml file</li>"
            "<li>Check your manifest.cpp file</li></ul>"
            "</br>For a comprehensive check-list, see <a href=\"http://www.mitk.org/wiki/How_to_fix_your_plug-in_DLL\">http://www.mitk.org/wiki/How_to_fix_your_plug-in_DLL</a>";
    ErrorViewPart::Pointer part(new ErrorViewPart(errorTitle, errorMsg));

    //PartPane pane = getPane();
    IViewReference::Pointer viewRef(this);
    ViewSite::Pointer site(new ViewSite(viewRef, part,
                                        factory->GetWorkbenchPage(), GetId(), PlatformUI::PLUGIN_ID(), label));
    //site.setActionBars(new ViewActionBars(factory.page.getActionBars(),
    //    site, (ViewPane) pane));
    try
    {
      part->Init(site);
    }
    catch (const PartInitException& e)
    {
      BERRY_ERROR << e.what();
      //StatusUtil.handleStatus(e, StatusManager.SHOW
      //    | StatusManager.LOG);
      return IWorkbenchPart::Pointer(nullptr);
    }
    part->SetPartName(label);

    QWidget* parent = pane->GetControl();

    try
    {
      part->CreatePartControl(parent);
    }
    catch (const std::exception& e)
    {
      BERRY_ERROR << "Error creating view: " << e.what() << std::endl;
      //          StatusUtil.handleStatus(e, StatusManager.SHOW
      //              | StatusManager.LOG);
      return IWorkbenchPart::Pointer(nullptr);
    }

    result = part.Cast<IWorkbenchPart> ();
  }

  return result;
}

PartPane::Pointer ViewReference::CreatePane()
{
  IWorkbenchPartReference::Pointer partRef(this);
  PartPane::Pointer pane(new PartPane(partRef,
      this->factory->GetWorkbenchPage()));
  return pane;
  //return Tweaklets::Get(WorkbenchTweaklet::KEY)->CreateViewPane(this, this->factory->GetWorkbenchPage());
}

IWorkbenchPart::Pointer ViewReference::CreatePartHelper()
{

  IWorkbenchPart::Pointer result;

  IMemento::Pointer stateMem;
  if (!memento.IsNull())
  {
    stateMem = memento->GetChild(WorkbenchConstants::TAG_VIEW_STATE);
  }

  IViewDescriptor::Pointer desc = factory->GetViewRegistry()->Find(GetId());
  if (desc.IsNull())
  {
    throw PartInitException(QString("Could not create view: ") + this->GetId());
  }

  // Create the part pane
  PartPane::Pointer pane = this->GetPane();

  // Create the pane's top-level control
  pane->CreateControl(factory->GetWorkbenchPage()->GetClientComposite());

  QString label = desc->GetLabel(); // debugging only

  // Things that will need to be disposed if an exception occurs (they are
  // listed here
  // in the order they should be disposed)
  //Composite content = null;
  IViewPart::Pointer initializedView;
  ViewSite::Pointer site;
  //ViewActionBars actionBars = null;
  // End of things that need to be explicitly disposed from the try block

  try
  {
    IViewPart::Pointer view;

    view = desc->CreateView();

    if (view.IsNull())
        return result;

    this->CreatePartProperties(view);

    // Create site
    IViewReference::Pointer viewRef(this);
    site = new ViewSite(viewRef, view, factory->GetWorkbenchPage(), desc);
    //actionBars = new ViewActionBars(factory.page.getActionBars(), site,
    //    (ViewPane) pane);
    //site.setActionBars(actionBars);


    view->Init(site, stateMem);
    // Once we've called init, we MUST dispose the view. Remember
    // the fact that
    // we've initialized the view in case an exception is thrown.
    initializedView = view;

    if (view->GetSite() != site)
    {
      throw PartInitException("View initialization failed. Site is incorrect.");
    }

    // Create the top-level composite

    {
      QWidget* parent = pane->GetControl();

      view->CreatePartControl(parent);
    }

    // Install the part's tools and menu

    {
      //
      // 3.3 start
      //
      //        IMenuService menuService = (IMenuService) site
      //            .getService(IMenuService.class);
      //        menuService.populateContributionManager(
      //            (ContributionManager) site.getActionBars()
      //                .getMenuManager(), "menu:" //$NON-NLS-1$
      //                + site.getId());
      //        menuService
      //            .populateContributionManager((ContributionManager) site
      //                .getActionBars().getToolBarManager(),
      //                "toolbar:" + site.getId()); //$NON-NLS-1$
      // 3.3 end

      //        actionBuilder = new ViewActionBuilder();
      //        actionBuilder.readActionExtensions(view);
      //        ActionDescriptor[] actionDescriptors = actionBuilder
      //            .getExtendedActions();
      //        IKeyBindingService keyBindingService = view.getSite()
      //            .getKeyBindingService();
      //
      //        if (actionDescriptors != null) {
      //          for (int i = 0; i < actionDescriptors.length; i++) {
      //            ActionDescriptor actionDescriptor = actionDescriptors[i];
      //
      //            if (actionDescriptor != null) {
      //              IAction action = actionDescriptors[i].getAction();
      //
      //              if (action != null
      //                  && action.getActionDefinitionId() != null) {
      //                keyBindingService.registerAction(action);
      //              }
      //            }
      //          }
      //        }
      //
      //        site.getActionBars().updateActionBars();
    }

    // The part should now be fully created. Exercise its public
    // interface, and sanity-check
    // it wherever possible. If it's going to throw exceptions or behave
    // badly, it's much better
    // that it does so now while we can still cancel creation of the
    // part.
    PartTester::TestView(view);

    result = view.Cast<IWorkbenchPart> ();

    //      IConfigurationElement::Pointer element = desc->GetConfigurationElement();
    //      if (!element.IsNull()) {
    //        factory.page.getExtensionTracker().registerObject(
    //            element.getDeclaringExtension(), view,
    //            IExtensionTracker.REF_WEAK);
    //      }
  }
  catch (const ctkException& e)
  {
    //      if ((e instanceof Error) && !(e instanceof LinkageError)) {
    //        throw (Error) e;
    //      }

    // An exception occurred. First deallocate anything we've allocated
    // in the try block (see the top
    // of the try block for a list of objects that need to be explicitly
    // disposed)
    //      if (content != null) {
    //        try {
    //          content.dispose();
    //        } catch (RuntimeException re) {
    //          StatusManager.getManager().handle(
    //              StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH,
    //                  re));
    //        }
    //      }
    //
    //      if (initializedView != null) {
    //        try {
    //          initializedView.dispose();
    //        } catch (RuntimeException re) {
    //          StatusManager.getManager().handle(
    //              StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH,
    //                  re));
    //        }
    //      }
    //
    //      if (site != null) {
    //        try {
    //          site.dispose();
    //        } catch (RuntimeException re) {
    //          StatusManager.getManager().handle(
    //              StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH,
    //                  re));
    //        }
    //      }
    //
    //      if (actionBars != null) {
    //        try {
    //          actionBars.dispose();
    //        } catch (RuntimeException re) {
    //          StatusManager.getManager().handle(
    //              StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH,
    //                  re));
    //        }
    //      }

    throw PartInitException(e.what(), e);
  }
  catch (const std::exception& e)
  {
    throw PartInitException(e.what());
  }

  return result;
}

IMemento::Pointer ViewReference::GetMemento()
{
  return memento;
}

} // namespace berry
