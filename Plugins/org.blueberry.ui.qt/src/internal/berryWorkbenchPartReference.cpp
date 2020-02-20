/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWorkbenchPartReference.h"

#include <berryPlatformException.h>
#include <berryObjects.h>

#include "berryWorkbenchPlugin.h"
#include "berryUtil.h"
#include "berryPartPane.h"
#include "berryIWorkbenchPartSite.h"
#include "berryIEditorPart.h"
#include "berryIWorkbenchPartConstants.h"

#include <QIcon>

namespace berry
{

int WorkbenchPartReference::STATE_LAZY = 0;
int WorkbenchPartReference::STATE_CREATION_IN_PROGRESS = 1;
int WorkbenchPartReference::STATE_CREATED = 2;
int WorkbenchPartReference::STATE_DISPOSED = 3;

WorkbenchPartReference::PropertyChangeListener::PropertyChangeListener(
    WorkbenchPartReference* ref) :
  partRef(ref)
{

}
void WorkbenchPartReference::PropertyChangeListener::PropertyChange(
    const PropertyChangeEvent::Pointer& event)
{
  if (event->GetProperty() == IWorkbenchPartConstants::INTEGER_PROPERTY)
  {
    partRef->PropertyChanged(event->GetSource(), event->GetNewValue().Cast<ObjectInt>()->GetValue());
  }
  else
  {
    partRef->PropertyChanged(event);
  }
}

WorkbenchPartReference::WorkbenchPartReference()
  : state(STATE_LAZY)
  , pinned(false)
  , propertyChangeListener(new PropertyChangeListener(this))
{
}

WorkbenchPartReference::~WorkbenchPartReference()
{
  this->Register();
  part = nullptr;
  pane = nullptr;
  this->UnRegister(false);
}

bool WorkbenchPartReference::IsDisposed() const
{
  return state == STATE_DISPOSED;
}

void WorkbenchPartReference::CheckReference()
{
  if (state == STATE_DISPOSED)
  {
    throw Poco::RuntimeException("Error: IWorkbenchPartReference disposed"); //$NON-NLS-1$
  }
}

/**
 * Calling this with deferEvents(true) will queue all property change events until a subsequent
 * call to deferEvents(false). This should be used at the beginning of a batch of related changes
 * to prevent duplicate property change events from being sent.
 *
 * @param shouldQueue
 */
//void WorkbenchPartReference::DeferEvents(bool shouldQueue) {
//        queueEvents = shouldQueue;
//
//        if (queueEvents == false) {
//          // do not use nextSetBit, to allow compilation against JCL Foundation (bug 80053)
//          for (int i = 0, n = queuedEvents.size(); i < n; ++i) {
//            if (queuedEvents.get(i)) {
//              firePropertyChange(i);
//              queuedEvents.clear(i);
//            }
//            }
//        }
//    }

void WorkbenchPartReference::SetPartName(const QString& newPartName)
{
  if (partName == newPartName)
  {
    return;
  }

  partName = newPartName;
  this->FirePropertyChange(IWorkbenchPartConstants::PROP_PART_NAME);
}

void WorkbenchPartReference::SetContentDescription(
    const QString& newContentDescription)
{
  if (contentDescription == newContentDescription)
  {
    return;
  }

  contentDescription = newContentDescription;
  this->FirePropertyChange(IWorkbenchPartConstants::PROP_CONTENT_DESCRIPTION);
}

void WorkbenchPartReference::SetImageDescriptor(
    const QIcon& descriptor)
{
  if (imageDescriptor.cacheKey() == descriptor.cacheKey())
  {
    return;
  }

  imageDescriptor = descriptor;

  // Don't queue events triggered by image changes. We'll dispose the image
  // immediately after firing the event, so we need to fire it right away.
  this->ImmediateFirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
  if (queueEvents)
  {
    // If there's a PROP_TITLE event queued, remove it from the queue because
    // we've just fired it.
    queuedEvents.remove(IWorkbenchPartConstants::PROP_TITLE);
  }
}

void WorkbenchPartReference::SetToolTip(const QString& newToolTip)
{
  if (tooltip == newToolTip)
  {
    return;
  }

  tooltip = newToolTip;
  this->FirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
}

void WorkbenchPartReference::PropertyChanged(const Object::Pointer& /*source*/, int propId)
{

  // We handle these properties directly (some of them may be transformed
  // before firing events to workbench listeners)
  if (propId == IWorkbenchPartConstants::PROP_CONTENT_DESCRIPTION || propId
      == IWorkbenchPartConstants::PROP_PART_NAME || propId
      == IWorkbenchPartConstants::PROP_TITLE)
  {

    this->RefreshFromPart();
  }
  else
  {
    // Any other properties are just reported to listeners verbatim
    this->FirePropertyChange(propId);
  }

  // Let the model manager know as well
  //  if (propId == IWorkbenchPartConstants::PROP_DIRTY) {
  //    IWorkbenchPart actualPart = getPart(false);
  //    if (actualPart != null) {
  //  SaveablesList modelManager = (SaveablesList) actualPart.getSite().getService(ISaveablesLifecycleListener.class);
  //      modelManager.dirtyChanged(actualPart);
  //    }
  //  }
}

void WorkbenchPartReference::PropertyChanged(const PropertyChangeEvent::Pointer& event)
{
  this->FirePropertyChange(event);
}

/**
 * Refreshes all cached values with the values from the real part
 */
void WorkbenchPartReference::RefreshFromPart()
{
  this->DeferEvents(true);

  SetPartName(ComputePartName());
  SetContentDescription(ComputeContentDescription());
  SetToolTip(GetRawToolTip());
  //SetImageDescriptor(ComputeImageDescriptor());

  this->DeferEvents(false);
}

QIcon WorkbenchPartReference::ComputeImageDescriptor()
{
  if (part)
  {
    return part->GetTitleImage();
  }
  return defaultImageDescriptor;
}

void WorkbenchPartReference::Init(const QString& id,
    const QString& tooltip, const QIcon& desc,
    const QString& paneName, const QString& contentDescription)
{

  this->id = id;
  this->tooltip = tooltip;
  this->partName = paneName;
  this->contentDescription = contentDescription;
  this->defaultImageDescriptor = desc;
  this->imageDescriptor = this->ComputeImageDescriptor();
}

/**
 * @see IWorkbenchPart
 */
void WorkbenchPartReference::AddPropertyListener(IPropertyChangeListener *listener)
{
  propChangeEvents.AddListener(listener);
}

/**
 * @see IWorkbenchPart
 */
void WorkbenchPartReference::RemovePropertyListener(IPropertyChangeListener *listener)
{
  propChangeEvents.RemoveListener(listener);
}

QString WorkbenchPartReference::GetId() const
{
  if (!part.IsNull())
  {
    IWorkbenchPartSite::Pointer site = part->GetSite();
    if (!site.IsNull())
    {
      return site->GetId();
    }
  }
  return id;
}

QString WorkbenchPartReference::GetTitleToolTip() const
{
  return tooltip;
}

QString WorkbenchPartReference::GetRawToolTip() const
{
  return part->GetTitleToolTip();
}

/**
 * Returns the pane name for the part
 *
 * @return the pane name for the part
 */
QString WorkbenchPartReference::GetPartName() const
{
  return partName;
}

/**
 * Gets the part name directly from the associated workbench part,
 * or the empty string if none.
 *
 * @return
 */
QString WorkbenchPartReference::GetRawPartName() const
{
  return part->GetPartName();

}

QString WorkbenchPartReference::ComputePartName() const
{
  return this->GetRawPartName();
}

/**
 * Returns the content description for this part.
 *
 * @return the pane name for the part
 */
QString WorkbenchPartReference::GetContentDescription() const
{
  return contentDescription;
}

/**
 * Computes a new content description for the part. Subclasses may override to change the
 * default behavior
 *
 * @return the new content description for the part
 */
QString WorkbenchPartReference::ComputeContentDescription() const
{
  return this->GetRawContentDescription();
}

/**
 * Returns the content description as set directly by the part, or the empty string if none
 *
 * @return the unmodified content description from the part (or the empty string if none)
 */
QString WorkbenchPartReference::GetRawContentDescription() const
{
  return part->GetContentDescription();

}

bool WorkbenchPartReference::IsDirty() const
{
  if (part.Cast<IEditorPart> ().IsNull())
  {
    return false;
  }
  return part.Cast<IEditorPart> ()->IsDirty();
}

QIcon WorkbenchPartReference::GetTitleImage() const
{
  return imageDescriptor;
}

void WorkbenchPartReference::FireVisibilityChange()
{
  this->FirePropertyChange(IWorkbenchPartConstants::PROP_VISIBLE);
}

//    /* package */ void fireZoomChange() {
//        fireInternalPropertyChange(INTERNAL_PROPERTY_ZOOMED);
//    }

bool WorkbenchPartReference::GetVisible()
{
  if (this->IsDisposed())
  {
    return false;
  }
  return this->GetPane()->GetVisible();
}

void WorkbenchPartReference::SetVisible(bool isVisible)
{
  if (this->IsDisposed())
  {
    return;
  }
  this->GetPane()->SetVisible(isVisible);
}

void WorkbenchPartReference::DeferEvents(bool shouldQueue)
{
  queueEvents = shouldQueue;

  if (queueEvents == false)
  {
    QSet<int>::iterator iter = queuedEvents.begin();
    while (iter != queuedEvents.end())
    {
      this->FirePropertyChange(*iter);
      queuedEvents.erase(iter++);
    }
  }
}

void WorkbenchPartReference::FirePropertyChange(int id)
{

  if (queueEvents)
  {
    queuedEvents.insert(id);
    return;
  }

  this->ImmediateFirePropertyChange(id);
}

void WorkbenchPartReference::ImmediateFirePropertyChange(int id)
{
  //UIListenerLogging.logPartReferencePropertyChange(this, id);
  ObjectInt::Pointer value(new ObjectInt(id));
  Object::Pointer source(this);
  PropertyChangeEvent::Pointer
      event(
          new PropertyChangeEvent(source, IWorkbenchPartConstants::INTEGER_PROPERTY, value, value));
  propChangeEvents.propertyChange(event);
}

IWorkbenchPart::Pointer WorkbenchPartReference::GetPart(bool restore)
{
  if (this->IsDisposed())
  {
    return IWorkbenchPart::Pointer(nullptr);
  }

  if (part.IsNull() && restore)
  {

    if (state == STATE_CREATION_IN_PROGRESS)
    {
      //                IStatus result = WorkbenchPlugin.getStatus(
      //                        new PartInitException(NLS.bind("Warning: Detected recursive attempt by part {0} to create itself (this is probably, but not necessarily, a bug)",  //$NON-NLS-1$
      //                                getId())));
      WorkbenchPlugin::Log("Warning: Detected recursive attempt by part "
          + GetId()
          + " to create itself (this is probably, but not necessarily, a bug)");
      return IWorkbenchPart::Pointer(nullptr);
    }

    try
    {
      state = STATE_CREATION_IN_PROGRESS;

      IWorkbenchPart::Pointer newPart = this->CreatePart();
      if (!newPart.IsNull())
      {
        part = newPart;
        // Add a dispose listener to the part. This dispose listener does nothing but log an exception
        // if the part's widgets get disposed unexpectedly. The workbench part reference is the only
        // object that should dispose this control, and it will remove the listener before it does so.
        //this->GetPane().getControl().addDisposeListener(prematureDisposeListener);
        part->AddPropertyListener(propertyChangeListener.data());

        this->RefreshFromPart();

        this->FirePropertyChange(IWorkbenchPartConstants::PROP_OPENED);

        //ISizeProvider sizeProvider = (ISizeProvider) Util.getAdapter(part, ISizeProvider.class);
        //if (sizeProvider != null) {
        // If this part has a preferred size, indicate that the preferred size may have changed at this point
        if (this->GetSizeFlags(true) != 0 || this->GetSizeFlags(false) != 0)
        {
          this->FirePropertyChange(IWorkbenchPartConstants::PROP_PREFERRED_SIZE);
        }
        //}
      }

      state = STATE_CREATED;
    } catch (Poco::Exception& e)
    {
      state = STATE_CREATED;
      std::cerr << e.displayText() << std::flush;
      throw e;
    } catch (std::exception& e)
    {
      state = STATE_CREATED;
      throw e;
    }
  }

  return part;
}

/**
 * Returns the part pane for this part reference. Does not return null. Should not be called
 * if the reference has been disposed.
 *
 * TODO: clean up all code that has any possibility of calling this on a disposed reference
 * and make this method throw an exception if anyone else attempts to do so.
 *
 * @return
 */
PartPane::Pointer WorkbenchPartReference::GetPane()
{

  // Note: we should never call this if the reference has already been disposed, since it
  // may cause a PartPane to be created and leaked.
  if (pane.IsNull())
  {
    pane = this->CreatePane();
  }
  return pane;
}

void WorkbenchPartReference::Dispose()
{

  if (this->IsDisposed())
  {
    return;
  }

  // Store the current title, tooltip, etc. so that anyone that they can be returned to
  // anyone that held on to the disposed reference.
  partName = GetPartName();
  contentDescription = GetContentDescription();
  tooltip = GetTitleToolTip();

  if (state == STATE_CREATION_IN_PROGRESS)
  {
    //            IStatus result = WorkbenchPlugin.getStatus(
    //                    new PartInitException(NLS.bind("Warning: Blocked recursive attempt by part {0} to dispose itself during creation",  //$NON-NLS-1$
    //                            getId())));
    WorkbenchPlugin::Log("Warning: Blocked recursive attempt by part" + GetId()
        + " to dispose itself during creation");
    return;
  }

  // Disposing the pane disposes the part's widgets. The part's widgets need to be disposed before the part itself.
  //  if (pane != 0) {
  //      // Remove the dispose listener since this is the correct place for the widgets to get disposed
  //      Control targetControl = getPane().getControl();
  //      if (targetControl != null) {
  //          targetControl.removeDisposeListener(prematureDisposeListener);
  //      }
  //      pane->Dispose();
  //  }

  this->DoDisposePart();

  if (!pane.IsNull())
  {
    //pane.removeContributions();
  }

  //clearListenerList(internalPropChangeListeners);
  //clearListenerList(partChangeListeners);

  state = STATE_DISPOSED;
  imageDescriptor = AbstractUICTKPlugin::GetMissingIcon();
  defaultImageDescriptor = imageDescriptor;
  this->ImmediateFirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
  //clearListenerList(propChangeListeners);

  pane = nullptr;
}

void WorkbenchPartReference::DoDisposePart()
{
  if (part)
  {
    //this->FireInternalPropertyChange(INTERNAL_PROPERTY_CLOSED);
    this->FirePropertyChange(IWorkbenchPartConstants::PROP_CLOSED);
    // Don't let exceptions in client code bring us down. Log them and continue.
    try
    {
      part->RemovePropertyListener(propertyChangeListener.data());
//      if (part instanceof IWorkbenchPart3)
//      {
//        ((IWorkbenchPart3) part).removePartPropertyListener(
//            partPropertyChangeListener);
//      }
//      part->Dispose();
      part = nullptr;
    }
    catch (const ctkRuntimeException& e)
    {
      WorkbenchPlugin::Log(e);
    }
    catch (const std::exception& e)
    {
      QString msg("Exception in WorkbenchPartReference::DoDisposePart: ");
      msg.append(e.what());
      WorkbenchPlugin::Log(msg);
    }
    part = nullptr;
  }
}

/**
 * Clears all of the listeners in a listener list. TODO Bug 117519 Remove
 * this method when fixed.
 *
 * @param list
 *            The list to be clear; must not be <code>null</code>.
 */
//private: void clearListenerList(const ListenerList list) {
//    final Object[] listeners = list.getListeners();
//    for (int i = 0; i < listeners.length; i++) {
//      list.remove(listeners[i]);
//    }
//  }


void WorkbenchPartReference::SetPinned(bool newPinned)
{
  if (IsDisposed())
  {
    return;
  }

  if (newPinned == pinned)
  {
    return;
  }

  pinned = newPinned;

  //SetImageDescriptor(computeImageDescriptor());

  this->FirePropertyChange(IWorkbenchPartConstants::PROP_PINNED);
}

bool WorkbenchPartReference::IsPinned() const
{
  return pinned;
}

QString WorkbenchPartReference::GetPartProperty(const QString& key) const
{
  if (part != 0)
  {
    return part->GetPartProperty(key);
  }
  else
  {
    QHash<QString, QString>::const_iterator itr =
        propertyCache.find(key);
    if (itr == propertyCache.end())
      return QString();
    return itr.value();
  }
}

void WorkbenchPartReference::FirePropertyChange(
    const PropertyChangeEvent::Pointer& event)
{
  propChangeEvents.propertyChange(event);
}

void WorkbenchPartReference::CreatePartProperties(
    IWorkbenchPart::Pointer workbenchPart)
{
  for (QHash<QString, QString>::iterator iter =
      propertyCache.begin(); iter != propertyCache.end(); ++iter)
  {
    workbenchPart->SetPartProperty(iter.key(), iter.value());
  }
}

int WorkbenchPartReference::ComputePreferredSize(bool width,
    int availableParallel, int availablePerpendicular, int preferredResult)
{
  ISizeProvider* sizeProvider = Util::GetAdapter<ISizeProvider>(part);
  if (sizeProvider)
  {
    return sizeProvider->ComputePreferredSize(width, availableParallel,
        availablePerpendicular, preferredResult);
  }

  return preferredResult;
}

int WorkbenchPartReference::GetSizeFlags(bool width)
{
  ISizeProvider* sizeProvider = Util::GetAdapter<ISizeProvider>(part);
  if (sizeProvider)
  {
    return sizeProvider->GetSizeFlags(width);
  }

  return 0;
}

} // namespace berry
