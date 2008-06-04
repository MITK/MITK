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

#include "cherryViewFactory.h"

#include "../cherryUIException.h"
#include "../cherryIViewRegistry.h"

#include "cherryViewReference.h"
#include "cherryViewDescriptor.h"
#include "cherryWorkbenchPage.h"

namespace cherry
{

const std::string ViewFactory::ID_SEP = ":"; //$NON-NLS-1$

std::string ViewFactory::GetKey(const std::string& id,
    const std::string& secondaryId)
{
  return secondaryId.empty() ? id : id + ID_SEP + secondaryId;
}

std::string ViewFactory::GetKey(IViewReference::Pointer viewRef)
{
  return GetKey(viewRef->GetId(), viewRef->GetSecondaryId());
}

std::string ViewFactory::ExtractPrimaryId(const std::string& compoundId)
{
  std::string::size_type i = compoundId.find_last_of(ID_SEP);
  if (i == std::string::npos)
  {
    return compoundId;
  }
  return compoundId.substr(0, i);
}

std::string ViewFactory::ExtractSecondaryId(const std::string& compoundId)
{
  std::string::size_type i = compoundId.find_last_of(ID_SEP);
  if (i == std::string::npos)
  {
    return "";
  }
  return compoundId.substr(i + 1);
}

bool ViewFactory::HasWildcard(const std::string& viewId)
{
  return viewId.find_first_of('*') != std::string::npos;
}

ViewFactory::ViewFactory(WorkbenchPage::Pointer p, IViewRegistry* reg)
: page(p), viewReg(reg) {
  //page.getExtensionTracker().registerHandler(this, null);
}

IViewReference::Pointer ViewFactory::CreateView(const std::string& id,
    const std::string& secondaryId)
{
  IViewDescriptor::Pointer desc = viewReg->Find(id);
  // ensure that the view id is valid
  if (desc.IsNull())
  {
    throw PartInitException("Could not create view", id);
  }
  // ensure that multiple instances are allowed if a secondary id is given
  if (secondaryId != "")
  {
    if (!desc->GetAllowMultiple())
    {
      throw PartInitException("View does not allow multiple instances", id);
    }
  }
  std::string key = this->GetKey(id, secondaryId);
  IViewReference::Pointer ref = counter.Get(key);
  if (ref.IsNull())
  {
    IMemento::Pointer memento = mementoTable[key];
    ref = new ViewReference(this, id, secondaryId, memento);
    mementoTable.erase(key);
    counter.Put(key, ref);
    this->GetWorkbenchPage()->PartAdded(ref.Cast<ViewReference>());
  }
  else 
  {
    counter.AddRef(key);
  }
  
  return ref;
}

std::vector<IViewReference::Pointer> ViewFactory::GetViewReferences()
{
  std::vector<IViewReference::Pointer> values(counter.Values());
  
  return values;
}

IViewReference::Pointer ViewFactory::GetView(const std::string& id)
{
  return this->GetView(id, "");
}

IViewReference::Pointer ViewFactory::GetView(const std::string& id,
    const std::string& secondaryId)
{
  std::string key = this->GetKey(id, secondaryId);
  return counter.Get(key);
}

const IViewRegistry* ViewFactory::GetViewRegistry() const
{
  return viewReg;
}

std::vector<IViewReference::Pointer> ViewFactory::GetViews()
{
  std::vector<IViewReference::Pointer> values(counter.Values());

  return values;
}

WorkbenchPage::Pointer ViewFactory::GetWorkbenchPage() const
{
  return page;
}

int ViewFactory::GetReferenceCount(IViewReference::Pointer viewRef) {
  std::string key = this->GetKey(viewRef);
  IViewReference::Pointer ref = counter.Get(key);
  return ref.IsNull() ? 0 : counter.GetRef(key);
}

void ViewFactory::ReleaseView(IViewReference::Pointer viewRef)
{
  std::string key = this->GetKey(viewRef);
  IViewReference::Pointer ref = counter.Get(key);
  if (ref.IsNull())
  {
    return;
  }
  int count = counter.RemoveRef(key);
  if (count <= 0)
  {
    this->GetWorkbenchPage()->PartRemoved(ref.Cast<ViewReference>());
  }
}

bool ViewFactory::RestoreState(IMemento::Pointer memento)
{
//  IMemento mem[] = memento.getChildren(IWorkbenchConstants.TAG_VIEW);
//  for (int i = 0; i < mem.length; i++)
//  {
//    //for dynamic UI - add the next line to replace subsequent code that is commented out
//    restoreViewState(mem[i]);
//  }
//  return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
  return true;
}

bool ViewFactory::SaveState(IMemento::Pointer memento)
{
//  final MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID,
//      IStatus.OK, WorkbenchMessages.ViewFactory_problemsSavingViews, null);
//
//  final IViewReference refs[] = getViews();
//  for (int i = 0; i < refs.length; i++)
//  {
//    IViewDescriptor desc = viewReg.find(refs[i].getId());
//    if (desc.isRestorable())
//    {
//      //for dynamic UI - add the following line to replace subsequent code which is commented out
//      saveViewState(memento, refs[i], result);
//    }
//  }
//  return result;
  return true;
}

//  for dynamic UI
IMemento::Pointer ViewFactory::SaveViewState(IMemento::Pointer memento,
    IViewReference::Pointer ref /*,MultiStatus res*/)
{
//  final MultiStatus result = res;
//  final IMemento viewMemento = memento
//  .createChild(IWorkbenchConstants.TAG_VIEW);
//  viewMemento.putString(IWorkbenchConstants.TAG_ID, ViewFactory
//  .getKey(ref));
//  if (ref instanceof ViewReference)
//  {
//    viewMemento.putString(IWorkbenchConstants.TAG_PART_NAME, ((ViewReference) ref).getPartName());
//  }
//  final IViewReference viewRef = ref;
//  final IViewPart view = (IViewPart) ref.getPart(false);
//  if (view != null)
//  {
//    SafeRunner.run(new SafeRunnable()
//        {
//        public void run()
//          {
//            if (view instanceof IWorkbenchPart3)
//            {
//              Map properties = ((IWorkbenchPart3) view)
//              .getPartProperties();
//              if (!properties.isEmpty())
//              {
//                IMemento propBag = viewMemento
//                .createChild(IWorkbenchConstants.TAG_PROPERTIES);
//                Iterator i = properties.entrySet().iterator();
//                while (i.hasNext())
//                {
//                  Map.Entry entry = (Map.Entry) i.next();
//                  IMemento p = propBag.createChild(
//                      IWorkbenchConstants.TAG_PROPERTY,
//                      (String) entry.getKey());
//                  p.putTextData((String) entry.getValue());
//                }
//              }
//            }
//            view.saveState(viewMemento
//                .createChild(IWorkbenchConstants.TAG_VIEW_STATE));
//          }
//
//        public void handleException(Throwable e)
//          {
//            result
//            .add(new Status(
//                    IStatus.ERROR,
//                    PlatformUI.PLUGIN_ID,
//                    0,
//                    NLS.bind(WorkbenchMessages.ViewFactory_couldNotSave, viewRef.getTitle() ),
//                    e));
//          }
//        }
//        );
//      }
//  else
//  {
//    IMemento mem = null;
//    IMemento props = null;
//
//    // if we've created the reference once, any previous workbench
//    // state memento is there.  After once, there is no previous
//    // session state, so it should be null.
//    if (ref instanceof ViewReference)
//    {
//      mem = ((ViewReference) ref).getMemento();
//      if (mem!=null)
//      {
//        props = mem.getChild(IWorkbenchConstants.TAG_PROPERTIES);
//      }
//      if (mem!=null)
//      {
//        mem = mem.getChild(IWorkbenchConstants.TAG_VIEW_STATE);
//      }
//    }
//    if (props != null)
//    {
//      viewMemento.createChild(IWorkbenchConstants.TAG_PROPERTIES)
//      .putMemento(props);
//    }
//    if (mem != null)
//    {
//      IMemento child = viewMemento
//      .createChild(IWorkbenchConstants.TAG_VIEW_STATE);
//      child.putMemento(mem);
//    }
//  }
//  return viewMemento;
  return 0;
}

// for dynamic UI
void ViewFactory::RestoreViewState(IMemento::Pointer memento)
{
//  String compoundId = memento.getString(IWorkbenchConstants.TAG_ID);
//  mementoTable.put(compoundId, memento);
}

IMemento::Pointer ViewFactory::GetViewState(const std::string& key)
{
//  IMemento memento = (IMemento) mementoTable.get(key);
//
//  if (memento == null)
//  {
//    return null;
//  }
//
//  return memento.getChild(IWorkbenchConstants.TAG_VIEW_STATE);
  return 0;
}

}
