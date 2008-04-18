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

#include "cherryViewReference.h"

namespace cherry {

ViewReference::ViewReference(ViewFactory factory, const std::string& id,
    const std::string& secondaryId, IMemento memento) {
    super();
    this.memento = memento;
    this.factory = factory;
    ViewDescriptor desc = (ViewDescriptor) this.factory.viewReg.find(id);
    ImageDescriptor iDesc = null;
    String title = null;
    if (desc != null) {
      iDesc = desc.getImageDescriptor();
      title = desc.getLabel();
    }

    String name = null;

    if (memento != null) {
      name = memento.getString(IWorkbenchConstants.TAG_PART_NAME);
      IMemento propBag = memento.getChild(IWorkbenchConstants.TAG_PROPERTIES);
      if (propBag != null) {
        IMemento[] props = propBag
            .getChildren(IWorkbenchConstants.TAG_PROPERTY);
        for (int i = 0; i < props.length; i++) {
          propertyCache.put(props[i].getID(), props[i].getTextData());
        }
      }
    }
    if (name == null) {
      name = title;
    }

    init(id, title, "", iDesc, name, ""); //$NON-NLS-1$//$NON-NLS-2$
    this.secondaryId = secondaryId;
  }

PartPane 
ViewReference::CreatePane() {
    return new ViewPane(this, this.factory.page);
  }

void 
ViewReference::DoDisposePart() {
    IViewPart view = (IViewPart) part;
    super.doDisposePart();
    if (view != null) {
      // Free action bars, pane, etc.
      PartSite site = (PartSite) view.getSite();
      ViewActionBars actionBars = (ViewActionBars) site.getActionBars();
      //
      // 3.3 start
      //
      IMenuService menuService = (IMenuService) site
          .getService(IMenuService.class);
      menuService.releaseContributions((ContributionManager) site.getActionBars()
          .getMenuManager());
      menuService.releaseContributions((ContributionManager) site.getActionBars()
          .getToolBarManager());
      // 3.3 end
      actionBars.dispose();
      
      // and now dispose the delegates since the
      // PluginActionContributionItem
      // can no longer do that
      if (actionBuilder != null) {
        actionBuilder.dispose();
        actionBuilder = null;
      }

      // Free the site.
      site.dispose();
    }

  }

IWorkbenchPage* 
ViewReference::GetPage() {
    return this.factory.page;
  }

std::string 
ViewReference::GetRegisteredName() {
    if (part != null && part.getSite() != null) {
      return part.getSite().getRegisteredName();
    }

    IViewRegistry reg = this.factory.viewReg;
    IViewDescriptor desc = reg.find(getId());
    if (desc != null) {
      return desc.getLabel();
    }
    return getTitle();
  }

std::string 
ViewReference::ComputePartName() {
    if (part instanceof IWorkbenchPart2) {
      return super.computePartName();
    }
    return getRegisteredName();
  }

std::string 
ViewReference::ComputeContentDescription() {
    if (part instanceof IWorkbenchPart2) {
      return super.computeContentDescription();
    }
    
    String rawTitle = getRawTitle();

    if (!Util.equals(rawTitle, getRegisteredName())) {
      return rawTitle;
    }

    return ""; //$NON-NLS-1$
  }

std::string 
ViewReference::GetSecondaryId() {
    return secondaryId;
  }

IViewPart::Ptr
ViewReference::GetView(bool restore) {
    return (IViewPart) getPart(restore);
  }


IWorkbenchPart::Ptr 
ViewReference::CreatePart() {

    // Check the status of this part

    IWorkbenchPart result = null;
    PartInitException exception = null;

    // Try to restore the view -- this does the real work of restoring the
    // view
    //
    try {
      result = createPartHelper();
    } catch (PartInitException e) {
      exception = e;
    }

    // If unable to create the part, create an error part instead
    // and pass the error to the status handling facility
    if (exception != null) {
      IStatus partStatus = exception.getStatus();
      IStatus displayStatus = StatusUtil.newStatus(partStatus,
          NLS.bind(WorkbenchMessages.ViewFactory_initException, partStatus.getMessage()));
      IStatus logStatus = StatusUtil
          .newStatus(
              partStatus,
              NLS
                  .bind(
                      "Unable to create view ID {0}: {1}", getId(), partStatus.getMessage())); //$NON-NLS-1$

      // Pass the error to the status handling facility
      StatusManager.getManager().handle(logStatus);

      IViewDescriptor desc = factory.viewReg.find(getId());
      String label = getId();
      if (desc != null) {
        label = desc.getLabel();
      }

      ErrorViewPart part = new ErrorViewPart(displayStatus);

      PartPane pane = getPane();
      ViewSite site = new ViewSite(this, part, factory.page, getId(),
          PlatformUI.PLUGIN_ID, label);
      site.setActionBars(new ViewActionBars(factory.page.getActionBars(),
          site, (ViewPane) pane));
      try {
        part.init(site);
      } catch (PartInitException e) {
        StatusUtil.handleStatus(e, StatusManager.SHOW
            | StatusManager.LOG);
        return null;
      }
      part.setPartName(label);

      Composite parent = (Composite) pane.getControl();
      Composite content = new Composite(parent, SWT.NONE);
      content.setLayout(new FillLayout());

      try {
        part.createPartControl(content);
      } catch (Exception e) {
        content.dispose();
        StatusUtil.handleStatus(e, StatusManager.SHOW
            | StatusManager.LOG);
        return null;
      }

      result = part;
    }

    return result;
  }

IWorkbenchPart::Ptr 
ViewReference::CreatePartHelper()  {

    IWorkbenchPart result = null;

    IMemento stateMem = null;
    if (memento != null) {
      stateMem = memento.getChild(IWorkbenchConstants.TAG_VIEW_STATE);
    }

    IViewDescriptor desc = factory.viewReg.find(getId());
    if (desc == null) {
      throw new PartInitException(
          NLS.bind(WorkbenchMessages.ViewFactory_couldNotCreate, getId()));
    }

    // Create the part pane
    PartPane pane = getPane();

    // Create the pane's top-level control
    pane.createControl(factory.page.getClientComposite());

    String label = desc.getLabel(); // debugging only

    // Things that will need to be disposed if an exception occurs (they are
    // listed here
    // in the order they should be disposed)
    Composite content = null;
    IViewPart initializedView = null;
    ViewSite site = null;
    ViewActionBars actionBars = null;
    // End of things that need to be explicitly disposed from the try block

    try {
      IViewPart view = null;
      try {
        UIStats.start(UIStats.CREATE_PART, label);
        view = desc.createView();
      } finally {
        UIStats.end(UIStats.CREATE_PART, view, label);
      }

      if (view instanceof IWorkbenchPart3) {
        createPartProperties((IWorkbenchPart3)view);
      }
      // Create site
      site = new ViewSite(this, view, factory.page, desc);
      actionBars = new ViewActionBars(factory.page.getActionBars(), site,
          (ViewPane) pane);
      site.setActionBars(actionBars);

      try {
        UIStats.start(UIStats.INIT_PART, label);
        view.init(site, stateMem);
        // Once we've called init, we MUST dispose the view. Remember
        // the fact that
        // we've initialized the view in case an exception is thrown.
        initializedView = view;

      } finally {
        UIStats.end(UIStats.INIT_PART, view, label);
      }

      if (view.getSite() != site) {
        throw new PartInitException(
            WorkbenchMessages.ViewFactory_siteException, null);
      }
      int style = SWT.NONE;
      if (view instanceof IWorkbenchPartOrientation) {
        style = ((IWorkbenchPartOrientation) view).getOrientation();
      }

      // Create the top-level composite
      {
        Composite parent = (Composite) pane.getControl();
        content = new Composite(parent, style);
        content.setLayout(new FillLayout());

        try {
          UIStats.start(UIStats.CREATE_PART_CONTROL, label);
          view.createPartControl(content);

          parent.layout(true);
        } finally {
          UIStats.end(UIStats.CREATE_PART_CONTROL, view, label);
        }
      }

      // Install the part's tools and menu
      {
        //
        // 3.3 start
        //
        IMenuService menuService = (IMenuService) site
            .getService(IMenuService.class);
        menuService.populateContributionManager(
            (ContributionManager) site.getActionBars()
                .getMenuManager(), "menu:" //$NON-NLS-1$
                + site.getId());
        menuService
            .populateContributionManager((ContributionManager) site
                .getActionBars().getToolBarManager(),
                "toolbar:" + site.getId()); //$NON-NLS-1$
        // 3.3 end

        actionBuilder = new ViewActionBuilder();
        actionBuilder.readActionExtensions(view);
        ActionDescriptor[] actionDescriptors = actionBuilder
            .getExtendedActions();
        IKeyBindingService keyBindingService = view.getSite()
            .getKeyBindingService();

        if (actionDescriptors != null) {
          for (int i = 0; i < actionDescriptors.length; i++) {
            ActionDescriptor actionDescriptor = actionDescriptors[i];

            if (actionDescriptor != null) {
              IAction action = actionDescriptors[i].getAction();

              if (action != null
                  && action.getActionDefinitionId() != null) {
                keyBindingService.registerAction(action);
              }
            }
          }
        }

        site.getActionBars().updateActionBars();
      }

      // The editor should now be fully created. Exercise its public
      // interface, and sanity-check
      // it wherever possible. If it's going to throw exceptions or behave
      // badly, it's much better
      // that it does so now while we can still cancel creation of the
      // part.
      PartTester.testView(view);

      result = view;

      IConfigurationElement element = (IConfigurationElement) Util.getAdapter(desc,
          IConfigurationElement.class);
      if (element != null) {
        factory.page.getExtensionTracker().registerObject(
            element.getDeclaringExtension(), view,
            IExtensionTracker.REF_WEAK);
      }
    } catch (Throwable e) {
      if ((e instanceof Error) && !(e instanceof LinkageError)) {
        throw (Error) e;
      }
      
      // An exception occurred. First deallocate anything we've allocated
      // in the try block (see the top
      // of the try block for a list of objects that need to be explicitly
      // disposed)
      if (content != null) {
        try {
          content.dispose();
        } catch (RuntimeException re) {
          StatusManager.getManager().handle(
              StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH,
                  re));
        }
      }

      if (initializedView != null) {
        try {
          initializedView.dispose();
        } catch (RuntimeException re) {
          StatusManager.getManager().handle(
              StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH,
                  re));
        }
      }

      if (site != null) {
        try {
          site.dispose();
        } catch (RuntimeException re) {
          StatusManager.getManager().handle(
              StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH,
                  re));
        }
      }

      if (actionBars != null) {
        try {
          actionBars.dispose();
        } catch (RuntimeException re) {
          StatusManager.getManager().handle(
              StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH,
                  re));
        }
      }

      throw new PartInitException(WorkbenchPlugin.getStatus(e));
    }

    return result;
  }

IMemento 
ViewReference::GetMemento() {
    return memento;
  }

}  // namespace cherry
