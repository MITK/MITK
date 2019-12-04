/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryViewFactory.h"

#include "berryUIException.h"
#include "berryIViewRegistry.h"

#include "berryViewReference.h"
#include "berryViewDescriptor.h"
#include "berryWorkbenchPage.h"
#include "berryWorkbenchConstants.h"
#include "util/berrySafeRunnable.h"

#include <berrySafeRunner.h>

namespace berry
{

const QString ViewFactory::ID_SEP = ":"; //$NON-NLS-1$

QString ViewFactory::GetKey(const QString& id,
    const QString& secondaryId)
{
  return secondaryId.isEmpty() ? id : id + ID_SEP + secondaryId;
}

QString ViewFactory::GetKey(IViewReference::Pointer viewRef)
{
  return GetKey(viewRef->GetId(), viewRef->GetSecondaryId());
}

QString ViewFactory::ExtractPrimaryId(const QString& compoundId)
{
  int i = compoundId.lastIndexOf(ID_SEP);
  if (i == -1)
  {
    return compoundId;
  }
  return compoundId.left(i);
}

QString ViewFactory::ExtractSecondaryId(const QString& compoundId)
{
  int i = compoundId.lastIndexOf(ID_SEP);
  if (i == -1)
  {
    return QString();
  }
  return compoundId.mid(i + 1);
}

bool ViewFactory::HasWildcard(const QString& viewId)
{
  return viewId.indexOf('*') != -1;
}

ViewFactory::ViewFactory(WorkbenchPage* p, IViewRegistry* reg) :
  page(p), viewReg(reg)
{
  //page.getExtensionTracker().registerHandler(this, null);
}

IViewReference::Pointer ViewFactory::CreateView(const QString& id,
    const QString& secondaryId)
{
  IViewDescriptor::Pointer desc = viewReg->Find(id);
  // ensure that the view id is valid
  if (desc.IsNull())
  {
    throw PartInitException(QString("Could not create view: ") + id);
  }
  // ensure that multiple instances are allowed if a secondary id is given
  if (secondaryId != "")
  {
    if (!desc->GetAllowMultiple())
    {
      throw PartInitException(QString("View does not allow multiple instances:") + id);
    }
  }
  QString key = this->GetKey(id, secondaryId);
  IViewReference::Pointer ref = counter.Get(key);
  if (ref.IsNull())
  {
    IMemento::Pointer memento = mementoTable[key];
    ref = new ViewReference(this, id, secondaryId, memento);
    mementoTable.remove(key);
    counter.Put(key, ref);
    this->GetWorkbenchPage()->PartAdded(ref.Cast<ViewReference> ());
  }
  else
  {
    counter.AddRef(key);
  }

  return ref;
}

QList<IViewReference::Pointer> ViewFactory::GetViewReferences()
{
  QList<IViewReference::Pointer> values(counter.Values());

  return values;
}

IViewReference::Pointer ViewFactory::GetView(const QString& id)
{
  return this->GetView(id, "");
}

IViewReference::Pointer ViewFactory::GetView(const QString& id,
    const QString& secondaryId)
{
  QString key = this->GetKey(id, secondaryId);
  return counter.Get(key);
}

const IViewRegistry* ViewFactory::GetViewRegistry() const
{
  return viewReg;
}

QList<IViewReference::Pointer> ViewFactory::GetViews()
{
  QList<IViewReference::Pointer> values(counter.Values());

  return values;
}

WorkbenchPage* ViewFactory::GetWorkbenchPage() const
{
  return page;
}

int ViewFactory::GetReferenceCount(IViewReference::Pointer viewRef)
{
  QString key = this->GetKey(viewRef);
  IViewReference::Pointer ref = counter.Get(key);
  return ref.IsNull() ? 0 : counter.GetRef(key);
}

void ViewFactory::ReleaseView(IViewReference::Pointer viewRef)
{
  QString key = this->GetKey(viewRef);
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
  QList<IMemento::Pointer> mem(memento->GetChildren(
      WorkbenchConstants::TAG_VIEW));
  for (int i = 0; i < mem.size(); i++)
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
  QList<IViewReference::Pointer> refs(GetViews());
  for (int i = 0; i < refs.size(); i++)
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

  void Run() override
  {

    const QHash<QString, QString>& properties =
        view->GetPartProperties();
    if (!properties.empty())
    {
      IMemento::Pointer propBag = viewMemento ->CreateChild(
          WorkbenchConstants::TAG_PROPERTIES);
      for (QHash<QString, QString>::const_iterator i =
          properties.begin(); i != properties.end(); ++i)
      {
        IMemento::Pointer p = propBag->CreateChild(
            WorkbenchConstants::TAG_PROPERTY, i.key());
        p->PutTextData(i.value());
      }
    }

    view->SaveState(viewMemento ->CreateChild(
        WorkbenchConstants::TAG_VIEW_STATE));
  }

  void HandleException(const ctkException& /*e*/) override
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
  QString compoundId;
  memento->GetString(WorkbenchConstants::TAG_ID, compoundId);
  mementoTable.insert(compoundId, memento);
}

IMemento::Pointer ViewFactory::GetViewState(const QString& key)
{
  IMemento::Pointer memento = mementoTable[key];

  if (!memento)
    return IMemento::Pointer(nullptr);

  return memento->GetChild(WorkbenchConstants::TAG_VIEW_STATE);
}

}
