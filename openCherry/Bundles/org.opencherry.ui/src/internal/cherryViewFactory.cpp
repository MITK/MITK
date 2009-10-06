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
#include "../cherryImageDescriptor.h"

#include "cherryViewReference.h"
#include "cherryViewDescriptor.h"
#include "cherryWorkbenchPage.h"
#include "cherryWorkbenchConstants.h"
#include "../util/cherrySafeRunnable.h"

#include <cherrySafeRunner.h>

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

ViewFactory::ViewFactory(WorkbenchPage::Pointer p, IViewRegistry* reg) :
  page(p), viewReg(reg)
{
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
    this->GetWorkbenchPage()->PartAdded(ref.Cast<ViewReference> ());
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

int ViewFactory::GetReferenceCount(IViewReference::Pointer viewRef)
{
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
    this->GetWorkbenchPage()->PartRemoved(ref.Cast<ViewReference> ());
  }
}

bool ViewFactory::RestoreState(IMemento::Pointer memento)
{
  std::vector<IMemento::Pointer> mem(memento->GetChildren(
      WorkbenchConstants::TAG_VIEW));
  for (std::size_t i = 0; i < mem.size(); i++)
  {
    //for dynamic UI - add the next line to replace subsequent code that is commented out
    RestoreViewState(mem[i]);
  }
  //  return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
  return true;
}

bool ViewFactory::SaveState(IMemento::Pointer memento)
{
  //  final MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID,
  //      IStatus.OK, WorkbenchMessages.ViewFactory_problemsSavingViews, null);
  bool result = true;
  std::vector<IViewReference::Pointer> refs(GetViews());
  for (std::size_t i = 0; i < refs.size(); i++)
  {
    IViewDescriptor::Pointer desc = viewReg->Find(refs[i]->GetId());
    if (desc->IsRestorable())
    {
      //for dynamic UI - add the following line to replace subsequent code which is commented out
      SaveViewState(memento, refs[i], result);
    }
  }
  return result;
}

struct SaveViewRunnable: public SafeRunnable
{
  SaveViewRunnable(IViewPart::Pointer view, IMemento::Pointer viewMemento,
      bool& result) :
    view(view), viewMemento(viewMemento), result(result)
  {
  }

  void Run()
  {

    const std::map<std::string, std::string>& properties =
        view->GetPartProperties();
    if (!properties.empty())
    {
      IMemento::Pointer propBag = viewMemento ->CreateChild(
          WorkbenchConstants::TAG_PROPERTIES);
      for (std::map<std::string, std::string>::const_iterator i =
          properties.begin(); i != properties.end(); ++i)
      {
        IMemento::Pointer p = propBag->CreateChild(
            WorkbenchConstants::TAG_PROPERTY, i->first);
        p->PutTextData(i->second);
      }
    }

    view->SaveState(viewMemento ->CreateChild(
        WorkbenchConstants::TAG_VIEW_STATE));
  }

  void HandleException(const std::exception& e)
  {
    //            result
    //            .add(new Status(
    //                    IStatus.ERR,
    //                    PlatformUI.PLUGIN_ID,
    //                    0,
    //                    NLS.bind(WorkbenchMessages.ViewFactory_couldNotSave, viewRef.getTitle() ),
    //                    e));
    result = false;
  }

private:

  IViewPart::Pointer view;
  IMemento::Pointer viewMemento;
  bool& result;
};

//  for dynamic UI
IMemento::Pointer ViewFactory::SaveViewState(IMemento::Pointer memento,
    IViewReference::Pointer ref, bool& res)
{
  //final MultiStatus result = res;
  bool& result = res;

  IMemento::Pointer viewMemento = memento->CreateChild(
      WorkbenchConstants::TAG_VIEW);
  viewMemento->PutString(WorkbenchConstants::TAG_ID, ViewFactory::GetKey(ref));
  if (ViewReference::Pointer viewRef = ref.Cast<ViewReference>())
  {
    viewMemento->PutString(WorkbenchConstants::TAG_PART_NAME,
        viewRef->GetPartName());
  }
  const IViewReference::Pointer viewRef = ref;
  const IViewPart::Pointer view = ref->GetPart(false).Cast<IViewPart> ();
  if (view)
  {
    ISafeRunnable::Pointer runnable(new SaveViewRunnable(view, viewMemento,
        result));
    SafeRunner::Run(runnable);
  }
  else
  {
    IMemento::Pointer mem;
    IMemento::Pointer props;

    // if we've created the reference once, any previous workbench
    // state memento is there.  After once, there is no previous
    // session state, so it should be null.
    if (ref.Cast<ViewReference> ())
    {
      mem = ref.Cast<ViewReference> ()->GetMemento();
      if (mem)
      {
        props = mem->GetChild(WorkbenchConstants::TAG_PROPERTIES);
        mem = mem->GetChild(WorkbenchConstants::TAG_VIEW_STATE);
      }
    }
    if (props)
    {
      viewMemento->CreateChild(WorkbenchConstants::TAG_PROPERTIES) ->PutMemento(
          props);
    }
    if (mem)
    {
      IMemento::Pointer child = viewMemento ->CreateChild(
          WorkbenchConstants::TAG_VIEW_STATE);
      child->PutMemento(mem);
    }
  }
  return viewMemento;
}

// for dynamic UI
void ViewFactory::RestoreViewState(IMemento::Pointer memento)
{
  std::string compoundId;
  memento->GetString(WorkbenchConstants::TAG_ID, compoundId);
  mementoTable.insert(std::make_pair(compoundId, memento));
}

IMemento::Pointer ViewFactory::GetViewState(const std::string& key)
{
  IMemento::Pointer memento = mementoTable[key];

  if (!memento)
    return IMemento::Pointer(0);

  return memento->GetChild(WorkbenchConstants::TAG_VIEW_STATE);
}

}
