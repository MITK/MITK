/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryUIExtensionTracker.h"

#include <berryDisplay.h>
#include <berryIExtension.h>
#include <berryObject.h>
#include "berryWorkbenchPlugin.h"

namespace berry {

UIExtensionTracker::UIExtensionTracker(Display* display)
  : display(display)
{
}

void UIExtensionTracker::ApplyRemove(IExtensionChangeHandler* handler, const SmartPointer<IExtension>& removedExtension, const QList<SmartPointer<Object> >& objects)
{
  if (display == nullptr) return;

  struct RemoveRunnable : public Poco::Runnable {

    IExtensionChangeHandler* const h;
    const IExtension::Pointer extension;
    const QList<Object::Pointer> objs;

    RemoveRunnable(IExtensionChangeHandler* h, const IExtension::Pointer& extension, const QList<Object::Pointer>& objs)
      : h(h)
      , extension(extension)
      , objs(objs)
    {}

    void run() override
    {
      try
      {
        h->RemoveExtension(extension, objs);
      }
      catch (const ctkException& e)
      {
        WorkbenchPlugin::Log("berry::UIExtensionTracker", "ApplyRemove", e);
      }
    }
  };

  RemoveRunnable runnable(handler, removedExtension, objects);
  display->SyncExec(&runnable);
}

void UIExtensionTracker::ApplyAdd(IExtensionChangeHandler* handler, const SmartPointer<IExtension>& addedExtension)
{
  if (display == nullptr) return;

  struct AddRunnable : public Poco::Runnable
  {
    IExtensionTracker* const tracker;
    IExtensionChangeHandler* const h;
    const IExtension::Pointer extension;

    AddRunnable(IExtensionTracker* tracker, IExtensionChangeHandler* h, const IExtension::Pointer& extension)
      : tracker(tracker)
      , h(h)
      , extension(extension)
    {}

    void run() override
    {
      try
      {
        h->AddExtension(tracker, extension);
      }
      catch (const ctkException& e)
      {
        WorkbenchPlugin::Log("berry::UIExtensionTracker", "ApplyAdd", e);
      }
    }
  };

  AddRunnable runnable(this, handler, addedExtension);
  display->SyncExec(&runnable);
}

}
