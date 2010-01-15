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

#include "cherryWorkbenchPartReference.h"

#include <osgi/framework/Exceptions.h>
#include <osgi/framework/Objects.h>

#include "cherryWorkbenchPlugin.h"
#include "cherryUtil.h"
#include "../cherryPartPane.h"
#include "../cherryIWorkbenchPartSite.h"
#include "../cherryIEditorPart.h"
#include "../cherryIWorkbenchPartConstants.h"
#include "../cherryImageDescriptor.h"

namespace cherry
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
    PropertyChangeEvent::Pointer event)
{
  partRef->PropertyChanged(event);
}

WorkbenchPartReference::WorkbenchPartReference() :
  state(STATE_LAZY), pinned(false), image(0)
{
  propertyChangeListener = new PropertyChangeListener(this);
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

void WorkbenchPartReference::SetPartName(const std::string& newPartName)
{
  if (partName == newPartName)
  {
    return;
  }

  partName = newPartName;
  this->FirePropertyChange(IWorkbenchPartConstants::PROP_PART_NAME);
}

void WorkbenchPartReference::SetContentDescription(
    const std::string& newContentDescription)
{
  if (contentDescription == newContentDescription)
  {
    return;
  }

  contentDescription = newContentDescription;
  this->FirePropertyChange(IWorkbenchPartConstants::PROP_CONTENT_DESCRIPTION);
}

void WorkbenchPartReference::SetImageDescriptor(
    ImageDescriptor::Pointer descriptor)
{
  if (imageDescriptor == descriptor)
  {
    return;
  }

  void* oldImage = image;
  ImageDescriptor::Pointer oldDescriptor = imageDescriptor;
  image = 0;
  imageDescriptor = descriptor;

  // Don't queue events triggered by image changes. We'll dispose the image
  // immediately after firing the event, so we need to fire it right away.
  this->ImmediateFirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
  if (queueEvents)
  {
    // If there's a PROP_TITLE event queued, remove it from the queue because
    // we've just fired it.
    queuedEvents.erase(IWorkbenchPartConstants::PROP_TITLE);
  }

  // If we had allocated the old image, deallocate it now (AFTER we fire the property change
  // -- listeners may need to clean up references to the old image)
  if (oldImage && oldDescriptor)
  {
    //JFaceResources.getResources().destroy(oldDescriptor);
    oldDescriptor->DestroyImage(oldImage);
  }
}

void WorkbenchPartReference::SetToolTip(const std::string& newToolTip)
{
  if (tooltip == newToolTip)
  {
    return;
  }

  tooltip = newToolTip;
  this->FirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
}

void WorkbenchPartReference::PropertyChanged(Object::Pointer  /*source*/, int propId)
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

void WorkbenchPartReference::PropertyChanged(PropertyChangeEvent::Pointer event)
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

ImageDescriptor::Pointer WorkbenchPartReference::ComputeImageDescriptor()
{
  if (part)
  {
    return ImageDescriptor::CreateFromImage(part->GetTitleImage());
  }
  return defaultImageDescriptor;
}

void WorkbenchPartReference::Init(const std::string& id,
    const std::string& tooltip, ImageDescriptor::Pointer desc,
    const std::string& paneName, const std::string& contentDescription)
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
void WorkbenchPartReference::AddPropertyListener(
    IPropertyChangeListener::Pointer listener)
{
  propChangeEvents.AddListener(listener);
}

/**
 * @see IWorkbenchPart
 */
void WorkbenchPartReference::RemovePropertyListener(
    IPropertyChangeListener::Pointer listener)
{
  propChangeEvents.RemoveListener(listener);
}

std::string WorkbenchPartReference::GetId() const
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

std::string WorkbenchPartReference::GetTitleToolTip() const
{
  return tooltip;
}

std::string WorkbenchPartReference::GetRawToolTip() const
{
  return part->GetTitleToolTip();
}

/**
 * Returns the pane name for the part
 *
 * @return the pane name for the part
 */
std::string WorkbenchPartReference::GetPartName() const
{
  return partName;
}

/**
 * Gets the part name directly from the associated workbench part,
 * or the empty string if none.
 *
 * @return
 */
std::string WorkbenchPartReference::GetRawPartName() const
{
  return part->GetPartName();

}

std::string WorkbenchPartReference::ComputePartName() const
{
  return this->GetRawPartName();
}

/**
 * Returns the content description for this part.
 *
 * @return the pane name for the part
 */
std::string WorkbenchPartReference::GetContentDescription() const
{
  return contentDescription;
}

/**
 * Computes a new content description for the part. Subclasses may override to change the
 * default behavior
 *
 * @return the new content description for the part
 */
std::string WorkbenchPartReference::ComputeContentDescription() const
{
  return this->GetRawContentDescription();
}

/**
 * Returns the content description as set directly by the part, or the empty string if none
 *
 * @return the unmodified content description from the part (or the empty string if none)
 */
std::string WorkbenchPartReference::GetRawContentDescription() const
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

void* WorkbenchPartReference::GetTitleImage()
{
  if (this->IsDisposed())
  {
    //return PlatformUI.getWorkbench().getSharedImages().getImage(ISharedImages.IMG_DEF_VIEW);
    return 0;
  }

  if (!image && imageDescriptor)
  {
    //image = JFaceResources.getResources().createImageWithDefault(imageDescriptor);
    image = imageDescriptor->CreateImage();
  }
  return image;
}

ImageDescriptor::Pointer WorkbenchPartReference::GetTitleImageDescriptor() const
{
  if (this->IsDisposed())
  {
    //return PlatformUI.getWorkbench().getSharedImages().getImageDescriptor(ISharedImages.IMG_DEF_VIEW);
    return ImageDescriptor::Pointer(0);
  }

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
    std::set<int>::iterator iter = queuedEvents.begin();
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
    return IWorkbenchPart::Pointer(0);
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
      return IWorkbenchPart::Pointer(0);
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
        part->AddPropertyListener(propertyChangeListener);

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
  void* oldImage = image;
  ImageDescriptor::Pointer oldDescriptor = imageDescriptor;
  image = 0;

  state = STATE_DISPOSED;
  imageDescriptor = ImageDescriptor::GetMissingImageDescriptor();
  defaultImageDescriptor = ImageDescriptor::GetMissingImageDescriptor();
  this->ImmediateFirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
  //clearListenerList(propChangeListeners);

  if (oldImage)
  {
    //JFaceResources.getResources().destroy(oldDescriptor);
    oldDescriptor->DestroyImage(oldImage);
  }

  pane = 0;
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
      part->RemovePropertyListener(propertyChangeListener);
//      if (part instanceof IWorkbenchPart3)
//      {
//        ((IWorkbenchPart3) part).removePartPropertyListener(
//            partPropertyChangeListener);
//      }
//      part->Dispose();
      part = 0;
    } catch (const Poco::RuntimeException& e)
    {
      WorkbenchPlugin::Log(e);
    }
    catch (const std::exception& e)
    {
      std::string msg("Exception in WorkbenchPartReference::DoDisposePart: ");
      msg.append(e.what());
      WorkbenchPlugin::Log(msg);
    }
    part = 0;
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

std::string WorkbenchPartReference::GetPartProperty(const std::string& key) const
{
  if (part != 0)
  {
    return part->GetPartProperty(key);
  }
  else
  {
    std::map<std::string, std::string>::const_iterator itr =
        propertyCache.find(key);
    if (itr == propertyCache.end())
      return "";
    return itr->second;
  }
}

void WorkbenchPartReference::FirePropertyChange(
    PropertyChangeEvent::Pointer event)
{
  propChangeEvents.propertyChange(event);
}

void WorkbenchPartReference::CreatePartProperties(
    IWorkbenchPart::Pointer workbenchPart)
{
  for (std::map<std::string, std::string>::iterator iter =
      propertyCache.begin(); iter != propertyCache.end(); ++iter)
  {
    workbenchPart->SetPartProperty(iter->first, iter->second);
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

} // namespace cherry
