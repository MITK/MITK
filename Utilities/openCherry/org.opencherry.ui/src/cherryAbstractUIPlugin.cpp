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

#include "cherryAbstractUIPlugin.h"

#include "cherryPlatformUI.h"

namespace cherry
{

const std::string AbstractUIPlugin::FN_DIALOG_SETTINGS = "dialog_settings.xml";

AbstractUIPlugin::AbstractUIPlugin()
{

}

//    IDialogSettings getDialogSettings() {
//        if (dialogSettings == null) {
//      loadDialogSettings();
//    }
//        return dialogSettings;
//    }


//    ImageRegistry getImageRegistry() {
//        if (imageRegistry == null) {
//            imageRegistry = createImageRegistry();
//            initializeImageRegistry(imageRegistry);
//        }
//        return imageRegistry;
//    }


//    IPreferenceStore getPreferenceStore() {
//        // Create the preference store lazily.
//        if (preferenceStore == null) {
//            preferenceStore = new ScopedPreferenceStore(new InstanceScope(),getBundle().getSymbolicName());
//
//        }
//        return preferenceStore;
//    }


IWorkbench* AbstractUIPlugin::GetWorkbench()
{
  return PlatformUI::GetWorkbench();
}

//    ImageRegistry createImageRegistry()
//    {
//
//      //If we are in the UI Thread use that
//      if (Display.getCurrent() != null)
//      {
//        return new ImageRegistry(Display.getCurrent());
//      }
//
//      if (PlatformUI.isWorkbenchRunning())
//      {
//        return new ImageRegistry(PlatformUI.getWorkbench().getDisplay());
//      }
//
//      //Invalid thread access if it is not the UI Thread
//      //and the workbench is not created.
//      throw new SWTError(SWT.ERROR_THREAD_INVALID_ACCESS);
//    }


//    void initializeImageRegistry(ImageRegistry reg) {
//        // spec'ed to do nothing
//    }


//    void loadDialogSettings() {
//        dialogSettings = new DialogSettings("Workbench"); //$NON-NLS-1$
//
//        // bug 69387: The instance area should not be created (in the call to
//        // #getStateLocation) if -data @none or -data @noDefault was used
//        IPath dataLocation = getStateLocationOrNull();
//        if (dataLocation != null) {
//          // try r/w state area in the local file system
//          String readWritePath = dataLocation.append(FN_DIALOG_SETTINGS)
//                  .toOSString();
//          File settingsFile = new File(readWritePath);
//          if (settingsFile.exists()) {
//              try {
//                  dialogSettings.load(readWritePath);
//              } catch (IOException e) {
//                  // load failed so ensure we have an empty settings
//                  dialogSettings = new DialogSettings("Workbench"); //$NON-NLS-1$
//              }
//
//              return;
//          }
//        }
//
//        // otherwise look for bundle specific dialog settings
//        URL dsURL = BundleUtility.find(getBundle(), FN_DIALOG_SETTINGS);
//        if (dsURL == null) {
//      return;
//    }
//
//        InputStream is = null;
//        try {
//            is = dsURL.openStream();
//            BufferedReader reader = new BufferedReader(
//                    new InputStreamReader(is, "utf-8")); //$NON-NLS-1$
//            dialogSettings.load(reader);
//        } catch (IOException e) {
//            // load failed so ensure we have an empty settings
//            dialogSettings = new DialogSettings("Workbench"); //$NON-NLS-1$
//        } finally {
//            try {
//                if (is != null) {
//          is.close();
//        }
//            } catch (IOException e) {
//                // do nothing
//            }
//        }
//    }


//    void refreshPluginActions() {
//        // If the workbench is not started yet, or is no longer running, do nothing.
//        if (!PlatformUI.isWorkbenchRunning()) {
//      return;
//    }
//
//        // startup() is not guaranteed to be called in the UI thread,
//        // but refreshPluginActions must run in the UI thread,
//        // so use asyncExec.  See bug 6623 for more details.
//        Display.getDefault().asyncExec(new Runnable() {
//            public void run() {
//                WWinPluginAction.refreshActionList();
//            }
//        });
//    }


//    void saveDialogSettings() {
//        if (dialogSettings == null) {
//            return;
//        }
//
//        try {
//          IPath path = getStateLocationOrNull();
//          if(path == null) {
//        return;
//      }
//            String readWritePath = path
//                    .append(FN_DIALOG_SETTINGS).toOSString();
//            dialogSettings.save(readWritePath);
//        } catch (IOException e) {
//            // spec'ed to ignore problems
//        } catch (IllegalStateException e) {
//            // spec'ed to ignore problems
//        }
//    }


void AbstractUIPlugin::Start(IBundleContext::Pointer context)
{
  Plugin::Start(context);

  // Should only attempt refreshPluginActions() once the bundle
  // has been fully started.  Otherwise, action delegates
  // can be created while in the process of creating
  // a triggering action delegate (if UI events are processed during startup).
  // Also, if the start throws an exception, the bundle will be shut down.
  // We don't want to have created any delegates if this happens.
  // See bug 63324 for more details.

//  bundleListener = new BundleListener()
//  {
//  public void bundleChanged(BundleEvent event)
//    {
//      if (event.getBundle() == getBundle())
//      {
//        if (event.getType() == BundleEvent.STARTED)
//        {
//          // We're getting notified that the bundle has been started.
//          // Make sure it's still active.  It may have been shut down between
//          // the time this event was queued and now.
//          if (getBundle().getState() == Bundle.ACTIVE)
//          {
//            refreshPluginActions();
//          }
//          fc.removeBundleListener(this);
//        }
//      }
//    }
//  };
//  context.addBundleListener(bundleListener);

  // bundleListener is removed in stop(BundleContext)
}

void AbstractUIPlugin::Stop(IBundleContext::Pointer context)
{
//  try
//  {
//    if (bundleListener != null)
//    {
//      context.removeBundleListener(bundleListener);
//    }
//    saveDialogSettings();
//    savePreferenceStore();
//    preferenceStore = null;
//    if (imageRegistry != null)
//    imageRegistry.dispose();
//    imageRegistry = null;
//}

  Plugin::Stop(context);

}

//    static ImageDescriptor imageDescriptorFromPlugin(String pluginId,
//            String imageFilePath) {
//        if (pluginId == null || imageFilePath == null) {
//            throw new IllegalArgumentException();
//        }
//
//        // if the bundle is not ready then there is no image
//        Bundle bundle = Platform.getBundle(pluginId);
//        if (!BundleUtility.isReady(bundle)) {
//      return null;
//    }
//
//        // look for the image (this will check both the plugin and fragment folders
//        URL fullPathString = BundleUtility.find(bundle, imageFilePath);
//        if (fullPathString == null) {
//            try {
//                fullPathString = new URL(imageFilePath);
//            } catch (MalformedURLException e) {
//                return null;
//            }
//        }
//
//        if (fullPathString == null) {
//      return null;
//    }
//        return ImageDescriptor.createFromURL(fullPathString);
//    }

}
